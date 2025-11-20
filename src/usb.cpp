/*
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#include <QDebug>
#include <QtEndian>

#include "exceptions.h"
#include "usb.h"
#include "x.h"

usb::usb() {
  context = nullptr;
  desc = {};
  ctx = {};
}

void usb::usb_init() {
  int res = libusb_init(&context);
  if (res != 0) {
    throw QException();
  }
  libusb_set_debug(context, 3);
}

void usb::usb_exit() {
  if (ctx.hdl) {
    libusb_release_interface(ctx.hdl, 0);
    libusb_close(ctx.hdl);
    ctx.hdl = nullptr;
  }
  if (context) {
    libusb_exit(context);
    context = nullptr;
  }
}

void usb::open_usb() {
  // 查找并打开第一个FEL设备
  QList<libusb_device *> devices = list_fel_devices();
  if (devices.isEmpty()) {
    throw cannot_find_fel_device();
  }

  libusb_device *device = devices.first();
  if (!open_usb(device)) {
    throw usb_driver_wrong();
  }

  // 释放设备引用
  unref_device(device);
}

bool usb::open_usb(libusb_device *device) {
  int rc;
  libusb_device_handle *handle;

  rc = libusb_open(device, &handle);
  if (rc != 0) {
    qWarning() << "Failed to open device:" << libusb_error_name(rc);
    return false;
  }

  // 获取设备描述符
  rc = libusb_get_device_descriptor(device, &desc);
  if (rc != 0) {
    qWarning() << "Failed to get device descriptor:" << libusb_error_name(rc);
    libusb_close(handle);
    return false;
  }

  // 尝试获取配置描述符以查找端点
  libusb_config_descriptor *config;
  rc = libusb_get_active_config_descriptor(device, &config);
  if (rc != 0) {
    qWarning() << "Failed to get config descriptor:" << libusb_error_name(rc);
    libusb_close(handle);
    return false;
  }

  // 查找端点
  const libusb_interface *interface = &config->interface[0];
  const libusb_interface_descriptor *interface_desc = &interface->altsetting[0];
  for (int i = 0; i < interface_desc->bNumEndpoints; i++) {
    const libusb_endpoint_descriptor *endpoint = &interface_desc->endpoint[i];
    if (endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN) {
      ctx.epin = endpoint->bEndpointAddress;
    } else {
      ctx.epout = endpoint->bEndpointAddress;
    }
  }

  libusb_free_config_descriptor(config);

  // 尝试获取内核驱动
  if (libusb_kernel_driver_active(handle, 0) == 1) {
    if (libusb_detach_kernel_driver(handle, 0) != 0) {
      qWarning() << "Failed to detach kernel driver";
      libusb_close(handle);
      return false;
    }
  }

  // 声明接口
  rc = libusb_claim_interface(handle, 0);
  if (rc != 0) {
    qWarning() << "Failed to claim interface:" << libusb_error_name(rc);
    libusb_close(handle);
    return false;
  }

  ctx.hdl = handle;
  return true;
}

void usb::close_usb() const {
  if (ctx.hdl) {
    libusb_release_interface(ctx.hdl, 0);
    libusb_close(ctx.hdl);
  }
}

void usb::usb_fel_init() {
  uint8_t buf[1];
  int rc;

  // 清空可能存在的旧数据
  do {
    rc = libusb_bulk_transfer(ctx.hdl, ctx.epin, buf, sizeof(buf), nullptr,
                              100);
  } while (rc == 0);
}

void usb::usb_write(const void *buf, size_t len) {
  usb_bulk_send(ctx.epout, (uint8_t *)buf, len);
}

void usb::usb_read(void *data, size_t len) {
  usb_bulk_recv(ctx.epin, (uint8_t *)data, len);
}

QList<libusb_device *> usb::list_fel_devices() {
  QList<libusb_device *> fel_devices;
  libusb_device **list;
  ssize_t count = libusb_get_device_list(context, &list);
  if (count < 0) {
    qWarning() << "Failed to get device list:" << libusb_error_name(count);
    return fel_devices;
  }

  for (ssize_t i = 0; i < count; i++) {
    libusb_device *device = list[i];
    libusb_device_descriptor desc;

    int rc = libusb_get_device_descriptor(device, &desc);
    if (rc != 0) {
      continue;
    }

    // 检查是否为Allwinner FEL设备 (VID: 0x1f3a)
    if (desc.idVendor == 0x1f3a) {
      // 增加引用计数
      libusb_ref_device(device);
      fel_devices.append(device);
    }
  }

  libusb_free_device_list(list, 1);
  return fel_devices;
}

void usb::unref_device(libusb_device *dev) {
  if (dev) {
    libusb_unref_device(dev);
  }
}

void usb::usb_bulk_send(int ep, uint8_t *buf, size_t len) const {
  int rc;
  int transferred;

  while (len > 0) {
    rc = libusb_bulk_transfer(ctx.hdl, ep, buf, len, &transferred,
                              usb_bulk_timeout);
    if (rc != 0) {
      qWarning() << "Bulk write failed:" << libusb_error_name(rc);
      throw usb_bulk_send_error();
    }
    buf += transferred;
    len -= transferred;
  }
}

void usb::usb_bulk_recv(int ep, uint8_t *buf, size_t len) const {
  int rc;
  int transferred;

  while (len > 0) {
    rc = libusb_bulk_transfer(ctx.hdl, ep, buf, len, &transferred,
                              usb_bulk_timeout);
    if (rc != 0) {
      qWarning() << "Bulk read failed:" << libusb_error_name(rc);
      throw usb_bulk_recv_error();
    }
    buf += transferred;
    len -= transferred;
  }
}

void usb::send_usb_request(int type, size_t length) {
  struct usb_request_t req = {.magic = {'A', 'W', 'U', 'B', 'S', 'U', 'B', 'M'},
                              .length = qToLittleEndian((uint32_t)length),
                              .unknown1 = 0,
                              .request = qToLittleEndian((uint16_t)type),
                              .length2 = qToLittleEndian((uint32_t)length),
                              .pad = {0}};

  usb_write(&req, sizeof(req));
}

void usb::read_usb_response() {
  char magic[4];
  usb_read(magic, sizeof(magic));

  if (memcmp(magic, "AWUB", 4) != 0) {
    throw read_usb_response_failed();
  }
}