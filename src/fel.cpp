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

#include "exceptions.h"
#include "fel.h"
#include "x.h"

fel::fel() { usb_handler.usb_init(); };

fel::~fel() {
  qDebug() << "Release FEL";
  usb_handler.usb_exit();
}

void fel::fel_open_connection() {
  fel_open_usb();
  fel_status = FEL_STATUS::FEL_OK;
}

void fel::fel_close_connection() {
  fel_close_usb();
  fel_status = FEL_STATUS::FEL_NONE;
}

void fel::fel_scan_chip() {
  // Check current fel status, if enabled long connection, skip fel open usb
  if (fel_status != FEL_STATUS::FEL_OK) {
    fel_open_usb();
  }
  fel_chip_id();
  // Check current fel status, if enabled long connection, skip fel close usb
  if (fel_status != FEL_STATUS::FEL_OK) {
    fel_close_usb();
  }
}

void fel::fel_open_usb() {
  qDebug() << "fel::fel_open_usb()";
  usb_handler.open_usb();
  usb_handler.usb_fel_init();
}

void fel::fel_open_usb(libusb_device *device) {
  qDebug() << "fel::fel_open_usb(device)";
  if (!usb_handler.open_usb(device)) {
    throw usb_driver_wrong();
  }
  usb_handler.usb_fel_init();
  current_device = device; // Track current device
}

void fel::fel_close_usb() {
  qDebug() << "fel::fel_close_usb()";
  usb_handler.close_usb();
}

// Add long connect
void fel::fel_chip_id() {
  send_fel_request(FEL_COMMAND::FEL_VERSION, 0, 0);
  usb_handler.usb_read(&version, sizeof(version));
  read_fel_status();

  // byte order convert
  version.id = le32_to_cpu(version.id);
  version.firmware = le32_to_cpu(version.firmware);
  version.protocol = le16_to_cpu(version.protocol);
  version.scratchpad = le32_to_cpu(version.scratchpad);
  version.dlength = le32_to_cpu(version.dlength);
  version.dflag = le32_to_cpu(version.dflag);
  // Debug
  qDebug("chip id: 0x%x", version.id);
  qDebug("chip firmware: 0x%x", version.firmware);
  qDebug("chip protocol: 0x%x", version.protocol);
  qDebug("chip scratchpad: 0x%x", version.scratchpad);
  qDebug("chip dlength: 0x%x", version.dlength);
  qDebug("chip dflag: 0x%x", version.dflag);
}

chip_version_t fel::fel_get_chip_version() const { return version; }

void fel::send_fel_request(int type, uint32_t addr, uint32_t length) {
  struct fel_request_t req = {.request = cpu_to_le32(type),
                              .address = cpu_to_le32(addr),
                              .length = cpu_to_le32(length)};
  qDebug("send_fel_request: type: %d, addr: 0x%x, length: 0x%x", type, addr,
         length);
  usb_handler.usb_write(&req, sizeof(struct fel_request_t));
}

void fel::read_fel_status() {
  uint8_t buf[8];
  usb_handler.usb_read(buf, sizeof(buf));
  qDebug("read_fel_status 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", buf[0],
         buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
}

void fel::fel_read_raw(uint32_t addr,
                       void *buf,
                       uint32_t len) {
  send_fel_request(FEL_COMMAND::FEL_READRAW, addr, len);
  usb_handler.usb_read(buf, len);
  read_fel_status();
}

void fel::fel_write_raw(uint32_t addr,
                        void *buf,
                        uint32_t len) {
  send_fel_request(FEL_COMMAND::FEL_WRITERAW, addr, len);
  usb_handler.usb_write(buf, len);
  read_fel_status();
}

uint32_t fel::fel_read32(uint32_t addr) {
  uint32_t val;
  fel_read(addr, &val, sizeof(val));
  return val;
}

void fel::fel_write32(uint32_t addr, uint32_t val) {
  fel_write(addr, &val, sizeof(val));
}

void fel::fel_exec(uint32_t addr) {
  send_fel_request(FEL_COMMAND::FEL_EXEC, addr, 0);
  usb_handler.usb_exit();
  usb_handler.usb_init();
}

uint32_t fel::payload_arm_read32(uint32_t addr) {
  uint32_t swapped;
  swapped = (addr >> 24) & 0xff;
  swapped |= ((addr >> 16) & 0xff) << 8;
  swapped |= ((addr >> 8) & 0xff) << 16;
  swapped |= (addr & 0xff) << 24;
  return swapped;
}