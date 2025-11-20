#ifndef FELDEVICE_H
#define FELDEVICE_H

#include <QFuture>
#include <QList>
#include <QObject>
#include <QString>
#include <atomic>
#include <chrono>
#include <stdint.h>

struct libusb_device;
struct libusb_device_descriptor;

#include "fel.h"

class FelDevice : public QObject {
  Q_OBJECT
public:
  explicit FelDevice(struct libusb_device *dev, QObject *parent = nullptr);
  ~FelDevice() override;

  void startBurn(const QString &imagePath);
  void stopBurn();

  // Get device information
  QString getDevicePath() const;
  QString getDeviceSerial() const;

  struct libusb_device *device() const { return m_device; }

signals:
  void statusChanged(const QString &status);
  void progressChanged(int percent);
  void progressDetailChanged(
      const QString &detail); // Add detailed progress information
  void finished(bool ok);
  void error(const QString &err);

public slots:
  void _emitStatus(const QString &s);
  void _emitProgress(int p);
  void _emitProgressDetail(const QString &d); // Detailed progress signal slot
  void _emitFinished(bool ok);
  void _emitError(const QString &e);

private:
  struct libusb_device *m_device{nullptr};
  fel *m_fel{nullptr};
  QFuture<void> m_future;
  std::atomic<bool> m_stop{false};

  // Device information
  QString m_devicePath;
  QString m_deviceSerial;

  // Initialize device information
  void initDeviceInfo();
};

#endif // FELDEVICE_H