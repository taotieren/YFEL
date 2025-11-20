#include "feldevice.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <QtConcurrent>

#ifdef _WIN32
#include "lib/libusb/libusb-MinGW-x64/include/libusb-1.0/libusb.h"
#else
#include <libusb-1.0/libusb.h>
#endif

FelDevice::FelDevice(libusb_device *dev, QObject *parent)
    : QObject(parent), m_device(dev) {
  if (m_device)
    libusb_ref_device(m_device);
  m_fel = new fel();
  initDeviceInfo();
}

FelDevice::~FelDevice() {
  m_stop = true;
  if (m_future.isRunning()) {
    m_future.waitForFinished();
  }
  delete m_fel;
  if (m_device)
    libusb_unref_device(m_device);
}

void FelDevice::initDeviceInfo() {
  if (!m_device)
    return;

  // Get device path information
  libusb_device_descriptor desc{};
  libusb_get_device_descriptor(m_device, &desc);
  m_devicePath = QString("Bus %1 Device %2")
                     .arg(libusb_get_bus_number(m_device))
                     .arg(libusb_get_device_address(m_device));

  // Try to get device serial number
  m_deviceSerial =
      QString("VID:%1 PID:%2")
          .arg(QString::number(desc.idVendor, 16), 4, QLatin1Char('0'))
          .arg(QString::number(desc.idProduct, 16), 4, QLatin1Char('0'));
}

QString FelDevice::getDevicePath() const { return m_devicePath; }

QString FelDevice::getDeviceSerial() const { return m_deviceSerial; }

void FelDevice::startBurn(const QString &imagePath) {
  m_stop = false;
  auto dev = m_device;
  auto felObj = m_fel;

  m_future = QtConcurrent::run([this, dev, felObj, imagePath]() {
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();

    try {
      QMetaObject::invokeMethod(this, "_emitStatus", Qt::QueuedConnection,
                                Q_ARG(QString, QString(tr("Opening device"))));

      // Get device descriptor to obtain port information
      libusb_device_descriptor desc{};
      int result = libusb_get_device_descriptor(dev, &desc);
      if (result != LIBUSB_SUCCESS) {
        QMetaObject::invokeMethod(
            this, "_emitError", Qt::QueuedConnection,
            Q_ARG(QString, QString(tr("Failed to get device descriptor: %1"))
                               .arg(result)));
        QMetaObject::invokeMethod(this, "_emitFinished", Qt::QueuedConnection,
                                  Q_ARG(bool, false));
        return;
      }

      QString portInfo = QString("Bus %1 Device %2")
                             .arg(libusb_get_bus_number(dev))
                             .arg(libusb_get_device_address(dev));

      try {
        felObj->fel_open_usb(dev);
        QMetaObject::invokeMethod(this, "_emitStatus", Qt::QueuedConnection,
                                  Q_ARG(QString, QString(tr("Device opened"))));
      } catch (const std::exception &e) {
        QMetaObject::invokeMethod(
            this, "_emitError", Qt::QueuedConnection,
            Q_ARG(QString,
                  QString(tr("Failed to open device: %1")).arg(e.what())));
        QMetaObject::invokeMethod(this, "_emitFinished", Qt::QueuedConnection,
                                  Q_ARG(bool, false));
        return;
      }

      // Check if image file exists
      QFile file(imagePath);
      if (!file.exists()) {
        QMetaObject::invokeMethod(
            this, "_emitError", Qt::QueuedConnection,
            Q_ARG(QString, QString(tr("Image file does not exist"))));
        QMetaObject::invokeMethod(this, "_emitFinished", Qt::QueuedConnection,
                                  Q_ARG(bool, false));
        return;
      }

      if (!file.open(QIODevice::ReadOnly)) {
        QMetaObject::invokeMethod(
            this, "_emitError", Qt::QueuedConnection,
            Q_ARG(QString, QString(tr("Cannot open image file"))));
        QMetaObject::invokeMethod(this, "_emitFinished", Qt::QueuedConnection,
                                  Q_ARG(bool, false));
        return;
      }

      qint64 fileSize = file.size();
      const int chunkSize = 4096;
      QByteArray buffer(chunkSize, 0);
      quint32 address = 0x40000000; // Default load address

      QMetaObject::invokeMethod(this, "_emitStatus", Qt::QueuedConnection,
                                Q_ARG(QString, QString(tr("Burning..."))));

      qint64 totalBytesRead = 0;
      int lastProgress =
          -1; // Track last reported progress to avoid excessive updates

      while (!file.atEnd() && !m_stop) {
        qint64 bytesRead = file.read(buffer.data(), chunkSize);
        if (bytesRead > 0) {
          try {
            // Write data to device
            felObj->fel_write(address + totalBytesRead, buffer.data(),
                              bytesRead);
          } catch (const std::exception &e) {
            QMetaObject::invokeMethod(
                this, "_emitError", Qt::QueuedConnection,
                Q_ARG(QString, QString(tr("Failed to write to device: %1"))
                                   .arg(e.what())));
            file.close();
            QMetaObject::invokeMethod(this, "_emitFinished",
                                      Qt::QueuedConnection, Q_ARG(bool, false));
            return;
          }

          totalBytesRead += bytesRead;
          int progress = static_cast<int>(
              (static_cast<double>(totalBytesRead) / fileSize) * 100);

          // Only update progress if it has changed to reduce UI updates
          if (progress != lastProgress) {
            QMetaObject::invokeMethod(this, "_emitProgress",
                                      Qt::QueuedConnection,
                                      Q_ARG(int, qMin(progress, 100)));

            // Send detailed progress information
            QString detail = QString(tr("Written %1 of %2 bytes (%3%)"))
                                 .arg(totalBytesRead)
                                 .arg(fileSize)
                                 .arg(progress);
            QMetaObject::invokeMethod(this, "_emitProgressDetail",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, detail));
            lastProgress = progress;
          }
        } else if (bytesRead < 0) {
          // Handle read error
          QMetaObject::invokeMethod(
              this, "_emitError", Qt::QueuedConnection,
              Q_ARG(QString, QString(tr("Failed to read from image file"))));
          file.close();
          QMetaObject::invokeMethod(this, "_emitFinished", Qt::QueuedConnection,
                                    Q_ARG(bool, false));
          return;
        }
      }

      file.close();

      if (m_stop) {
        QMetaObject::invokeMethod(this, "_emitStatus", Qt::QueuedConnection,
                                  Q_ARG(QString, QString(tr("Stopped"))));
        QMetaObject::invokeMethod(this, "_emitFinished", Qt::QueuedConnection,
                                  Q_ARG(bool, false));
        return;
      }

      qint64 endTime = QDateTime::currentMSecsSinceEpoch();
      qint64 duration = endTime - startTime;

      QString finishMessage = QString(tr("Finished in %1 ms")).arg(duration);
      QMetaObject::invokeMethod(this, "_emitStatus", Qt::QueuedConnection,
                                Q_ARG(QString, finishMessage));
      QMetaObject::invokeMethod(this, "_emitFinished", Qt::QueuedConnection,
                                Q_ARG(bool, true));
    } catch (const std::exception &e) {
      // Ensure file is closed in case of exception
      // File destructor will handle this, but being explicit
      QMetaObject::invokeMethod(
          this, "_emitError", Qt::QueuedConnection,
          Q_ARG(QString, QString(tr("Error: %1")).arg(e.what())));
      QMetaObject::invokeMethod(this, "_emitFinished", Qt::QueuedConnection,
                                Q_ARG(bool, false));
    }
  });
}

void FelDevice::stopBurn() { m_stop = true; }

// slots that forward to signals (invoked with QueuedConnection)
void FelDevice::_emitStatus(const QString &s) { emit statusChanged(s); }

void FelDevice::_emitProgress(int p) { emit progressChanged(p); }

void FelDevice::_emitProgressDetail(const QString &d) {
  emit progressDetailChanged(d);
}

void FelDevice::_emitFinished(bool ok) { emit finished(ok); }

void FelDevice::_emitError(const QString &e) { emit error(e); }