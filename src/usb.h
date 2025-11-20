/*
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#ifndef YFEL_USB_H
#define YFEL_USB_H

#include <QList>
#include <QMultiMap>
#include <QObject>
#include <QString>
#include <QThread>
#include <stdint.h>

#ifdef _WIN32
#include "lib/libusb/libusb-MinGW-x64/include/libusb-1.0/libusb.h"
#else
#include <libusb-1.0/libusb.h>
#endif

struct libusb_device;
struct libusb_device_handle;
struct libusb_context;

// USB constants
static const unsigned int usb_timeout = 1000; // 1 second timeout
static const unsigned int usb_bulk_timeout =
    5000; // 5 second timeout for bulk transfers
static const char fel_send_magic[8] = {'A', 'W', 'U', 'B', 'S', 'U', 'B', 'M'};
static const char fel_recv_magic[4] = {'A', 'W', 'U', 'B'};

class usb : public QObject {
  Q_OBJECT
public:
  usb();

  void usb_init();

  void usb_exit();

  void open_usb();

  void close_usb() const;

  void usb_fel_init();

  void usb_write(const void *buf, size_t len);

  void usb_read(void *data, size_t len);

  // Return a list of matching FEL libusb_device* (refs are taken, caller should
  // unref)
  QList<struct libusb_device *> list_fel_devices();

  // Open a specific device (previous open_usb() kept for compatibility)
  bool open_usb(struct libusb_device *device);

  // Get the current device handle
  struct libusb_device_handle *get_device_handle() const { return ctx.hdl; }

private:
  void usb_bulk_send(int ep, uint8_t *buf, size_t len) const;

  void usb_bulk_recv(int ep, uint8_t *buf, size_t len) const;

  void send_usb_request(int type, size_t length);

  void read_usb_response();

  // Helper: ref/unref devices when returned from list_fel_devices
  static void unref_device(struct libusb_device *dev);

  struct usb_request_t {
    char magic[8];
    uint32_t length;
    uint32_t unknown1;
    uint16_t request;
    uint32_t length2;
    char pad[10];
  } __attribute__((packed));

private:
  struct {
    struct libusb_device_handle *hdl;
    uint8_t epin;
    uint8_t epout;
  } ctx;

  struct libusb_context *context{};
  struct libusb_device_descriptor desc{};
};

#endif // YFEL_USB_H