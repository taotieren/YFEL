/*
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#ifndef YFEL_FEL_H
#define YFEL_FEL_H

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

#include "chips/chip_version.h"
#include "usb.h"

class fel : public QObject {
  Q_OBJECT
protected:
  enum FEL_COMMAND {
    FEL_VERSION = 0x001,
    FEL_WRITERAW = 0x101,
    FEL_EXEC = 0x102,
    FEL_READRAW = 0x103,
  };

  enum FEL_STATUS {
    FEL_NONE = 0x00,
    FEL_OK = 0x01,
    FEL_ERROR = 0x02,
  };

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
  struct
#ifdef __GNUC__
      __attribute__((packed))
#endif
      fel_request_t {
    uint32_t request;
    uint32_t address;
    uint32_t length;
    uint32_t pad;
  };
#ifdef _MSC_VER
#pragma pack(pop)
#endif

private:
  chip_version_t version{};
  usb usb_handler;
  FEL_STATUS fel_status = FEL_STATUS::FEL_NONE;
  libusb_device *current_device =
      nullptr; // Track current device for multi-device support

public:
  fel();

  ~fel() override;

  void fel_open_connection();

  void fel_close_connection();

  void fel_scan_chip();

  [[nodiscard]] chip_version_t fel_get_chip_version() const;

  uint32_t fel_read32(uint32_t addr);

  void fel_write32(uint32_t addr, uint32_t val);

  void fel_exec(uint32_t addr);

  uint32_t payload_arm_read32(uint32_t addr);

public:
  template <typename T> void fel_read(uint32_t addr, T *buf, size_t len) {
    // Check current fel status, if enabled long connection, skip fel open usb
    if (fel_status != FEL_STATUS::FEL_OK) {
      fel_open_usb();
    }

    try {
      // do fel read raw
      size_t n;
      while (len > 0) {
        n = len > 65536 ? 65536 : len;
        fel_read_raw(addr, (void *)buf, n);
        addr += n;
        buf += n;
        len -= n;
      }
    } catch (const std::exception &e) {
      // Ensure USB is closed on error
      if (fel_status != FEL_STATUS::FEL_OK) {
        try {
          fel_close_usb();
        } catch (...) {
          // Ignore errors during cleanup
        }
      }
      fel_status = FEL_STATUS::FEL_ERROR;
      throw;
    }

    // Check current fel status, if enabled long connection, skip fel close usb
    if (fel_status != FEL_STATUS::FEL_OK) {
      fel_close_usb();
    }
  };

  template <typename T> void fel_write(uint32_t addr, T *buf, size_t len) {
    // Check current fel status, if enabled long connection, skip fel open usb
    if (fel_status != FEL_STATUS::FEL_OK) {
      fel_open_usb();
    }

    try {
      // do fel write raw
      size_t n;
      while (len > 0) {
        n = len > 65536 ? 65536 : len;
        fel_write_raw(addr, (void *)buf, n);
        addr += n;
        buf += n;
        len -= n;
      }
    } catch (const std::exception &e) {
      // Ensure USB is closed on error
      if (fel_status != FEL_STATUS::FEL_OK) {
        try {
          fel_close_usb();
        } catch (...) {
          // Ignore errors during cleanup
        }
      }
      fel_status = FEL_STATUS::FEL_ERROR;
      throw;
    }

    // Check current fel status, if enabled long connection, skip fel close usb
    if (fel_status != FEL_STATUS::FEL_OK) {
      fel_close_usb();
    }
  };

private:
  void fel_chip_id();

  void fel_open_usb();

  // Open using a specific libusb_device (for multi-device support)
public:
  void fel_open_usb(struct libusb_device *device);

private:
  void fel_close_usb();

  void send_fel_request(int type, uint32_t addr, uint32_t length);

  void read_fel_status();

  void fel_read_raw(uint32_t addr, void *buf, uint32_t len);

  void fel_write_raw(uint32_t addr, void *buf, uint32_t len);
};

#endif // YFEL_FEL_H