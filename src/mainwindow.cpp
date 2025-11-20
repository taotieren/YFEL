/*
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "exceptions.h"
#include "yfel_config.h"

#include <QAbstractItemView>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QTimer>
#include <QActionGroup>

#include "feldevice.h"
#include "usb.h"
#include "utils.h"
#include <QFileDialog>
#include <QProgressBar>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>

#include "feldevice.h"
#include "usb.h"
#include "utils.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  
  // 允许窗口调整大小
  setMinimumSize(600, 400);
  resize(800, 600);
  
  // 设置应用程序调色板以支持深色主题
  QPalette pal = qApp->palette();
  // 确保默认文本颜色在深色和浅色主题下都可见
  pal.setColor(QPalette::WindowText, Qt::black);
  pal.setColor(QPalette::Text, Qt::black);
  qApp->setPalette(pal);
  
  initMainwindowData();
  initMenubar();
  setupThemeMenu();  // 添加主题菜单
  initMassProductionTab(); // 初始化量产界面

  connect(chip_op, &ChipOP::release_ui, this, [=]() {
    if (!this->isVisible()) {
      this->show();
    }
    releaseUI();
    updateStatusBar(tr("Done."));
  });

  ui->flash_spi_read_hexView->addWidget(spiNandReadHexView);
  ui->flash_spi_write_hexView->addWidget(spiNandWriteHexView);
  ui->run_file_hexView->addWidget(runHexView);
  ui->dump_data_hexView->addWidget(dumpHexView);

  QByteArray initHexArr(1000, 0);
  spiNandReadHexView->setData(new QHexView::DataStorageArray(initHexArr));
  spiNandWriteHexView->setData(new QHexView::DataStorageArray(initHexArr));
  runHexView->setData(new QHexView::DataStorageArray(initHexArr));
  dumpHexView->setData(new QHexView::DataStorageArray(initHexArr));

  ui->dram_dram_tpr13_label->installEventFilter(this);
  ui->dram_dram_tpr13_label->show();

  chipStatus.setNone();
}

MainWindow::~MainWindow() {
  qDebug() << "MainWindow::~MainWindow()";
  delete ui;
  delete chip_op;
  delete spiNandWriteHexView;
  delete spiNandReadHexView;
  delete runHexView;
  delete dumpHexView;
  delete tpr13;
}

void MainWindow::initMainwindowData() {
  ui->chip_label_2->setText(tr("NONE"));
  ui->statusbar->showMessage(tr("Ready, Version: ") + PROJECT_GIT_HASH, 5000);
}

void MainWindow::setupThemeMenu() {
    // Create theme actions
    QActionGroup *themeGroup = new QActionGroup(this);
    themeGroup->setExclusive(true);
    
    // Light theme action
    QAction *lightThemeAction = new QAction(tr("Light Theme"), this);
    lightThemeAction->setCheckable(true);
    lightThemeAction->setChecked(true); // Default to light theme
    connect(lightThemeAction, &QAction::triggered, this, &MainWindow::setLightTheme);
    themeGroup->addAction(lightThemeAction);
    
    // Dark theme action
    QAction *darkThemeAction = new QAction(tr("Dark Theme"), this);
    darkThemeAction->setCheckable(true);
    connect(darkThemeAction, &QAction::triggered, this, &MainWindow::setDarkTheme);
    themeGroup->addAction(darkThemeAction);
    
    // Add actions to menu
    ui->menuTool->addSeparator();
    ui->menuTool->addAction(lightThemeAction);
    ui->menuTool->addAction(darkThemeAction);
}

void MainWindow::setLightTheme() {
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::WindowText, Qt::black);
    lightPalette.setColor(QPalette::Base, Qt::white);
    lightPalette.setColor(QPalette::AlternateBase, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::ToolTipBase, Qt::black);
    lightPalette.setColor(QPalette::ToolTipText, Qt::black);
    lightPalette.setColor(QPalette::Text, Qt::black);
    lightPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(128, 128, 128));
    lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::ButtonText, Qt::black);
    lightPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(128, 128, 128));
    lightPalette.setColor(QPalette::BrightText, Qt::red);
    lightPalette.setColor(QPalette::Link, QColor(0, 0, 255));
    lightPalette.setColor(QPalette::Highlight, QColor(0, 120, 215));
    lightPalette.setColor(QPalette::HighlightedText, Qt::white);
    lightPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(128, 128, 128));
    
    qApp->setPalette(lightPalette);
    
    // Update all QHexView widgets
    if (spiNandReadHexView) {
        spiNandReadHexView->repaint();
    }
    if (spiNandWriteHexView) {
        spiNandWriteHexView->repaint();
    }
    if (runHexView) {
        runHexView->repaint();
    }
    if (dumpHexView) {
        dumpHexView->repaint();
    }
}

void MainWindow::setDarkTheme() {
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(45, 45, 45));
    darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(128, 128, 128));
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(128, 128, 128));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(128, 128, 128));
    
    qApp->setPalette(darkPalette);
    
    // Update all QHexView widgets
    if (spiNandReadHexView) {
        spiNandReadHexView->repaint();
    }
    if (spiNandWriteHexView) {
        spiNandWriteHexView->repaint();
    }
    if (runHexView) {
        runHexView->repaint();
    }
    if (dumpHexView) {
        dumpHexView->repaint();
    }
}

void MainWindow::initMenubar() {
  // menu exit
  ui->actionExit->setShortcut(QKeySequence::Quit);
  connect(ui->actionExit, &QAction::triggered, this,
          &MainWindow::exitMenuClicked);

  connect(ui->actionOpen, &QAction::triggered, this, [=]() ->
          void {
    QMessageBox::critical(this, tr("Error"), tr("Success"));
  });

  // menu about
  connect(ui->actionAbout_YFEL, &QAction::triggered, this, [this]() {
    QMessageBox::about(
        this, tr("About YFEL"),
        tr("Copyright 2022 YuzukiTsuru\n\nGNU General Public License v3.0") +
            "\n\tVersion: " + PROJECT_GIT_HASH);
  });

  // menu web
  connect(ui->actionWeb, &QAction::triggered, this, []() {
    QString URL = "https://github.com/YuzukiTsuru/YFEL";
    QDesktopServices::openUrl(QUrl(URL.toLatin1()));
  });

  // enable jtag
  connect(ui->actionEnable_JTAG, &QAction::triggered, this,
          &MainWindow::enableJtag);
  connect(ui->Misc_enable_jtag, &QPushButton::clicked, this,
          &MainWindow::enableJtag);

  // reset chip
  connect(ui->actionReset_CPU, &QAction::triggered, this,
          &MainWindow::chipReset);
  connect(ui->Misc_reset_pushButton, &QPushButton::clicked, this,
          &MainWindow::chipReset);

  // TPR13 dialogs
  connect(tpr13, &TPR13Dialog::GetTPR13Data, this,
          &MainWindow::TPR13_value_getter);
}

void MainWindow::initMassProductionTab() {
  // Initialize mass production tab
  ui->mass_production_table->setColumnCount(5);
  QStringList headers;
  headers << tr("Port") << tr("Interface") << tr("Status") << tr("Progress")
          << tr("Action");
  ui->mass_production_table->setHorizontalHeaderLabels(headers);

  // Set table properties to improve production environment experience
  ui->mass_production_table->setSelectionBehavior(
      QAbstractItemView::SelectRows);
  ui->mass_production_table->setSelectionMode(
      QAbstractItemView::SingleSelection);
  ui->mass_production_table->setAlternatingRowColors(true);
  // Set table columns to be stretchable to fit window size
  ui->mass_production_table->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  ui->mass_production_table->setStyleSheet(
      "QTableView { gridline-color: #d0d0d0; border: 1px solid #d0d0d0; }"
      "QTableView::item:selected { background-color: #a8d8ff; }"
      "QTableView::item { padding: 5px; }");
  // Set table to adapt to window size
  ui->mass_production_table->horizontalHeader()->setStretchLastSection(true);
  ui->mass_production_table->horizontalHeader()->setSectionResizeMode(
      QHeaderView::ResizeToContents);

  // Connect button signals
  connect(ui->mass_production_scan_button, &QPushButton::clicked, this,
          &MainWindow::scanMassProductionDevices);
  connect(ui->mass_production_image_browse_button, &QPushButton::clicked, this,
          &MainWindow::on_mass_production_image_browse_button_clicked);
  connect(ui->mass_production_start_all_button, &QPushButton::clicked, this,
          &MainWindow::on_mass_production_start_all_button_clicked);
  connect(ui->mass_production_stop_all_button, &QPushButton::clicked, this,
          &MainWindow::on_mass_production_stop_all_button_clicked);
}

void MainWindow::on_scan_pushButton_clicked() {
  lockUI();
  updateStatusBar(tr("Scanning..."));
  try {
    if (chip_op == nullptr) {
      throw std::runtime_error("Chip operator is not initialized");
    }
    
    chip_op->chip_scan_chip();
    chip_op->chip_sid();
    // Set Scan Button label
    ui->chip_label_2->setText(
        "0x" + QString::number(chip_op->get_current_chip().chip_id, 16));

    // Set CHip lines
    ui->chip_name_lineEdit->setText(chip_op->get_current_chip().chip_name);
    ui->chip_id_lineEdit->setText(
        "0x" + QString::number(chip_op->get_current_chip().chip_id, 16));
    ui->chip_sid_lineEdit->setText("0x" + chip_op->get_current_chip().chip_sid);

    QString chip_core_names_ = chip_op->get_current_chip().chip_core_count_str +
                               " " + chip_op->get_current_chip().chip_core;
    if (chip_op->get_current_chip().chip_type == chip_type_e::Heterogeneous) {
      for (auto const &item :
           chip_op->get_current_chip().chip_heterogeneous_core) {
        chip_core_names_.append(" + ");
        chip_core_names_.append(item);
      }
    }
    ui->chip_core_lineEdit->setText(chip_core_names_);

    // update status bar
    updateStatusBar(tr("Done."));
    chipStatus.setOK();

    // load dram presets
    loadDramPresets();
  } catch (const cannot_find_fel_device &e) {
    QMessageBox::warning(this, tr("Warning"),
                         tr("Can't find target FEL device"));
  } catch (const usb_bulk_send_error &e) {
    chipStatus.setError();
    scanChipWarning();
  } catch (const usb_bulk_recv_error &e) {
    chipStatus.setError();
    scanChipWarning();
  } catch (const usb_driver_wrong &e) {
    if (chipStatus.isError()) {
      scanChipWarning();
    } else {
      QMessageBox::warning(this, tr("Warning"),
                           tr("Find FEL Device but host driver is wrong\n"
                              "Please use libusb-win32 driver instead"));
    }
  } catch (const std::exception &e) {
    clearChipInfo();
    QMessageBox::warning(this, tr("Warning"), tr(e.what()));
  }

  releaseUI();

  // Also update mass production tab when scanning
  scanMassProductionDevices();
}

void MainWindow::exitMenuClicked() {
  QMessageBox msgBox;
  msgBox.setText(tr("Exit YFEL?"));
  msgBox.setWindowIcon(QIcon(":/assets/img/icon.png"));
  msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Yes);
  int ret = msgBox.exec();
  if (ret == QMessageBox::Yes) {
    QApplication::quit();
  }
}

void MainWindow::copyToClipboard(const QString &data, QPushButton *button) {
  // Add null pointer checks to improve robustness
  if (button == nullptr) {
    qWarning() << "Copy button is null";
    return;
  }
  
  qDebug() << "Copy" << data << "to Clip Board";
  QClipboard *clip = QApplication::clipboard();
  if (clip != nullptr) {
    clip->setText(data);
    button->setText("√");
    QTimer::singleShot(500, this, [button]() { 
      if (button != nullptr) {
        button->setText(tr("Copy")); 
      }
    });
  } else {
    qWarning() << "Clipboard is not available";
  }
}

QString MainWindow::openFileDialog(QLineEdit *lineEdit) {
  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Open Image File"), "",
      tr("IMAGE (*.img *.IMG);;Binary (*.bin);;All files (*.*)"));
  lineEdit->setText(fileName);
  return fileName;
}

void MainWindow::loadDramPresets() {
  if (!chipStatus.isNone()) {
    if (chip_op != nullptr && ui != nullptr) {
      auto dram_paras = chip_op->get_dram_params();
      ui->dram_load_preset_comboBox->clear();
      if (!dram_paras.isEmpty()) {
        for (const auto &item : dram_paras) {
          ui->dram_load_preset_comboBox->addItem(item.dram_param_name);
        }
      }
    } else {
      qWarning() << "Chip operator or UI is not initialized";
    }
  }
}

void MainWindow::updateStatusBar(const QString &message, int timeout) {
  if (ui != nullptr && ui->statusbar != nullptr) {
    ui->statusbar->showMessage(message, timeout);
  } else {
    qWarning() << "UI or status bar is not initialized";
  }
}

void MainWindow::on_chip_chip_name_pushButton_clicked() {
  if (chipStatus.isNone()) {
    scanChipWarning();
    return;
  }
  
  try {
    // Read and display chip name
    if (chip_op != nullptr) {
      ui->chip_name_lineEdit->setText(chip_op->get_current_chip().chip_name);
      updateStatusBar(tr("Chip name read successfully"));
    } else {
      throw std::runtime_error("Chip operator is not initialized");
    }
  } catch (const std::exception &e) {
    QMessageBox::warning(this, tr("Warning"), tr(e.what()));
  }
}

void MainWindow::on_chip_chip_id_pushButton_2_clicked() {
  if (chipStatus.isNone()) {
    scanChipWarning();
    return;
  }
  
  try {
    // Read and display chip ID
    if (chip_op != nullptr) {
      ui->chip_id_lineEdit->setText(
          "0x" + QString::number(chip_op->get_current_chip().chip_id, 16));
      updateStatusBar(tr("Chip ID read successfully"));
    } else {
      throw std::runtime_error("Chip operator is not initialized");
    }
  } catch (const std::exception &e) {
    QMessageBox::warning(this, tr("Warning"), tr(e.what()));
  }
}

void MainWindow::on_chip_chip_sid_pushButton_3_clicked() {
  if (chipStatus.isNone()) {
    scanChipWarning();
    return;
  }
  
  try {
    // Read chip SID
    if (chip_op != nullptr) {
      chip_op->chip_sid();
      // Display SID
      ui->chip_sid_lineEdit->setText("0x" + chip_op->get_current_chip().chip_sid);
      updateStatusBar(tr("Chip SID read successfully"));
    } else {
      throw std::runtime_error("Chip operator is not initialized");
    }
  } catch (const std::exception &e) {
    QMessageBox::warning(this, tr("Warning"), tr(e.what()));
  }
}

void MainWindow::on_chip_chip_core_pushButton_4_clicked() {
  if (chipStatus.isNone()) {
    scanChipWarning();
    return;
  }
  
  try {
    // Read and display core count
    if (chip_op != nullptr) {
      QString chip_core_names_ = chip_op->get_current_chip().chip_core_count_str +
                                 " " + chip_op->get_current_chip().chip_core;
      if (chip_op->get_current_chip().chip_type == chip_type_e::Heterogeneous) {
        for (auto const &item :
             chip_op->get_current_chip().chip_heterogeneous_core) {
          chip_core_names_.append(" + ");
          chip_core_names_.append(item);
        }
      }
      ui->chip_core_lineEdit->setText(chip_core_names_);
      updateStatusBar(tr("Core count read successfully"));
    } else {
      throw std::runtime_error("Chip operator is not initialized");
    }
  } catch (const std::exception &e) {
    QMessageBox::warning(this, tr("Warning"), tr(e.what()));
  }
}

void MainWindow::on_chip_spi_nor_scan_pushButton_clicked() { scanSpiNor(); }

void MainWindow::on_chip_spi_nand_scan_pushButton_clicked() { scanSpiNand(); }

void MainWindow::enableJtag() {
  qDebug() << "Enable Chip JTAG";
  if (chipStatus.isNone()) {
    scanChipWarning();
    return;
  }
  try {
    chip_op->chip_enable_jtag();
    QMessageBox::information(this, tr("Info"), tr("JTAG Enabled"));
  } catch (const function_not_implemented &e) {
    QMessageBox::warning(this, tr("Warning"),
                         tr("Function is not implemented"));
  } catch (const std::exception &e) {
    QMessageBox::warning(this, tr("Warning"), tr(e.what()));
  }
}

void MainWindow::chipReset() {
  qDebug() << "Reset Chip";
  if (chipStatus.isNone()) {
    scanChipWarning();
    return;
  }
  try {
    chip_op->chip_reset_chip();
    clearChipInfo();
    QMessageBox::information(this, tr("Info"), tr("Chip Reseted"));
  } catch (const function_not_implemented &e) {
    QMessageBox::warning(this, tr("Warning"),
                         tr("Function is not implemented"));
  } catch (const std::exception &e) {
    QMessageBox::warning(this, tr("Warning"), tr(e.what()));
  }
}

void MainWindow::clearChipInfo() {
  if (ui != nullptr) {
    ui->chip_label_2->setText(tr("NONE"));
    ui->chip_id_lineEdit->setText("");
    ui->chip_name_lineEdit->setText("");
    ui->chip_sid_lineEdit->setText("");
    ui->chip_core_lineEdit->setText("");
  } else {
    qWarning() << "UI is not initialized";
  }
}

void MainWindow::on_Misc_exec_addr_btn_clicked() {
  if (chipStatus.isNone()) {
    scanChipWarning();
    return;
  }
  // Check whether the address is entered
  if (ui->Misc_exec_addr_lineEdit->text().isEmpty()) {
    QMessageBox::warning(this, tr("Warning"), tr("Please enter address."));
    return;
  }
  try {
    // Get the address
    auto addrString = ui->Misc_exec_addr_lineEdit->text();
    bool convertStatus = false;
    uint32_t addr;

    // Check whether the input address is HEX
    if (addrString.startsWith("0x")) {
      // In case of HEX, delete the first two char
      addrString.remove(0, 2);
      addr = static_cast<uint32_t>(addrString.toInt(&convertStatus, 16));
    } else {
      addr = static_cast<uint32_t>(addrString.toInt(&convertStatus));
    }
    // Check whether the address is actually entered
    if (convertStatus) {
      chip_op->chip_exec(addr);
      // After execution, the device will disconnect the link and clear the UI
      clearChipInfo();
      QMessageBox::information(this, tr("Info"),
                               tr("Run command sent, now device disconnected"));
      chipStatus.setError();
    } else {
      QMessageBox::warning(this, tr("Warning"),
                           tr("Please enter the correct address"));
    }
  } catch (const function_not_implemented &e) {
    QMessageBox::warning(this, tr("Warning"),
                         tr("Function is not implemented"));
  } catch (const std::exception &e) {
    QMessageBox::warning(this, tr("Warning"), tr(e.what()));
  }
}

void MainWindow::on_tabWidget_currentChanged(int index) {
  qDebug() << "change tabWidget to: " << index;
  if (index == uiTabWidgetIndex::tab_dram) {
    loadDramPresets();
  }
}

void MainWindow::on_dram_load_preset_comboBox_currentIndexChanged() {
  // Prevention of cross-border
  if (chip_op->get_dram_params().length() >= 1) {
    auto current_dram_param = chip_op->get_dram_params()[0].dram_param;
    for (const auto &item : chip_op->get_dram_params()) {
      if (ui->dram_load_preset_comboBox->currentText() ==
          item.dram_param_name) {
        current_dram_param = item.dram_param;
      }
    }
    ui->dram_dram_clk_lineEdit->setText(
        QString::number(current_dram_param.dram_clk));
    ui->dram_dram_type_lineEdit->setText(
        QString::number(current_dram_param.dram_type));

    ui->dram_dram_zq_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_zq));

    ui->dram_dram_odt_en_lineEdit->setText(
        QString::number(current_dram_param.dram_odt_en));

    ui->dram_dram_para1_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_para1));
    ui->dram_dram_para2_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_para2));

    ui->dram_dram_mr0_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_mr0));
    ui->dram_dram_mr1_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_mr1));
    ui->dram_dram_mr2_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_mr2));
    ui->dram_dram_mr3_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_mr3));
    ui->dram_dram_tpr0_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_tpr0));
    ui->dram_dram_tpr1_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_tpr1));
    ui->dram_dram_tpr2_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_tpr2));
    ui->dram_dram_tpr3_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_tpr3));
    ui->dram_dram_tpr4_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_tpr4));
    ui->dram_dram_tpr5_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_tpr5));
    ui->dram_dram_tpr6_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_tpr6));
    ui->dram_dram_tpr7_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_tpr7));
    ui->dram_dram_tpr8_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_tpr8));
    ui->dram_dram_tpr9_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_tpr9));
    ui->dram_dram_tpr10_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_tpr10));
    ui->dram_dram_tpr11_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_tpr11));
    ui->dram_dram_tpr12_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_tpr12));
    ui->dram_dram_tpr13_lineEdit->setText(
        fixedUint32ToString(current_dram_param.dram_tpr13));
  }
}

void MainWindow::on_dram_init_dram_btn_clicked() {
  dram_param_t dramParam{
      .dram_clk = ui->dram_dram_clk_lineEdit->text().toUInt(),
      .dram_type = ui->dram_dram_type_lineEdit->text().toUInt(),
      .dram_zq =
          ui->dram_dram_zq_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_odt_en = ui->dram_dram_odt_en_lineEdit->text().toUInt(),
      .dram_para1 =
          ui->dram_dram_para1_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_para2 =
          ui->dram_dram_para2_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_mr0 =
          ui->dram_dram_mr0_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_mr1 =
          ui->dram_dram_mr1_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_mr2 =
          ui->dram_dram_mr2_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_mr3 =
          ui->dram_dram_mr3_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_tpr0 =
          ui->dram_dram_tpr0_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_tpr1 =
          ui->dram_dram_tpr1_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_tpr2 =
          ui->dram_dram_tpr2_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_tpr3 =
          ui->dram_dram_tpr3_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_tpr4 =
          ui->dram_dram_tpr4_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_tpr5 =
          ui->dram_dram_tpr5_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_tpr6 =
          ui->dram_dram_tpr6_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_tpr7 =
          ui->dram_dram_tpr7_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_tpr8 =
          ui->dram_dram_tpr8_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_tpr9 =
          ui->dram_dram_tpr9_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_tpr10 =
          ui->dram_dram_tpr10_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_tpr11 =
          ui->dram_dram_tpr11_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_tpr12 =
          ui->dram_dram_tpr12_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
      .dram_tpr13 =
          ui->dram_dram_tpr13_lineEdit->text().remove(0, 2).toUInt(nullptr, 16),
  };
  if (chipStatus.isOK()) {
    try {
      chip_op->chip_init_dram(dramParam);
      updateStatusBar("DRAM init done, Check the result in the UART log");
    } catch (const usb_bulk_send_error &e) {
      chipStatus.setError();
      scanChipWarning();
    } catch (const usb_bulk_recv_error &e) {
      chipStatus.setError();
      scanChipWarning();
    } catch (const std::exception &e) {
      chipStatus.setError();
      QMessageBox::warning(this, tr("Warning"), tr(e.what()));
    }
  } else {
    scanChipWarning();
  }
}

void MainWindow::on_flash_spi_erase_spi_nand_scan_button_clicked() {
  scanSpiNand();
}

void MainWindow::scanChipWarning() {
  if (this != nullptr) {
    if (chipStatus.isNone()) {
      QMessageBox::warning(this, tr("Warning"),
                          tr("Chip not avaliable, try scan it"));
    } else if (chipStatus.isError()) {
      QMessageBox::warning(
          this, tr("Warning"),
          tr("Chip operation error, please reset the chip manually"));
    } else {
      QMessageBox::warning(this, tr("Warning"), tr("Unknown error"));
    }

    // clear the chip info
    clearChipInfo();
  } else {
    qWarning() << "MainWindow instance is not valid";
  }
}

void MainWindow::lockUI() { this->setEnabled(false); }

void MainWindow::releaseUI() { this->setEnabled(true); }

void MainWindow::scanSpiNand() {
  if (chipStatus.isNone()) {
    scanChipWarning();
    return;
  }
  try {
    // lock UI
    lockUI();

    // get spi nand info
    auto nandInfo = chip_op->chip_scan_spi_nand();

    // update ui - 使用正确的控件名称
    ui->flash_spi_nand_chip_label_2->setText(nandInfo);
    // ui->flash_device_comboBox->addItem(nandInfo); // Removed non-existent UI
    // element
  } catch (const cannot_find_spi_flash_device &e) {
    ui->flash_spi_nand_chip_label_2->setText(tr("No supported SPI NAND found"));
  } catch (const function_not_implemented &e) {
    QMessageBox::warning(this, tr("Warning"),
                         tr("Function is not implemented"));
  } catch (const std::runtime_error &e) {
    chipStatus.setNone();
    QMessageBox::warning(this, tr("Warning"), tr(e.what()));
  }
  releaseUI();
}

void MainWindow::on_flash_spi_erase_spi_nand_erase_button_clicked() {
  qDebug() << "Erasing SPI NAND...";
  if (!chipStatus.isOK()) {
    scanChipWarning();
    return;
  }
  updateStatusBar(tr("Erasing SPI NAND..."), 20000);
  try {
    lockUI();

    // Remove references to non-existent controls, use default values
    auto addr = 0x000000;
    /*
    auto addr =
        ui->flash_spi_erase_spi_nand_addr_lineEdit->text().toUInt(nullptr, 10);
    if (ui->flash_spi_erase_spi_nand_addr_lineEdit->text().startsWith("0x"))
      addr = ui->flash_spi_erase_spi_nand_addr_lineEdit->text()
                 .remove(0, 2)
                 .toUInt(nullptr, 16);
    */

    // Use default values
    auto len = 0x100000;
    /*
    auto len = ui->flash_spi_erase_spi_nand_length_lineEdit->text().toUInt(
        nullptr, 10);
    if (ui->flash_spi_erase_spi_nand_length_lineEdit->text().startsWith("0x"))
      len = ui->flash_spi_erase_spi_nand_length_lineEdit->text()
                .remove(0, 2)
                .toUInt(nullptr, 16);
    */

    chip_op->chip_erase_spi_nand(addr, len);
    QMessageBox::information(this, tr("Information"),
                             tr("Erase SPI NAND successfully"));
  } catch (const function_not_implemented &e) {
    QMessageBox::warning(this, tr("Warning"),
                         tr("Function is not implemented"));
  } catch (const spi_erase_out_of_range &e) {
    QMessageBox::warning(this, tr("Warning"), tr("Erase address out out of range"));
  } catch (const std::runtime_error &e) {
    chipStatus.setNone();
    QMessageBox::warning(this, tr("Warning"), tr(e.what()));
  }

  releaseUI();
}

void MainWindow::on_flash_spi_erase_spi_nand_setall_button_clicked() {
  qDebug() << "Erasing All SPI NAND...";
  if (!chipStatus.isOK()) {
    scanChipWarning();
    return;
  }
  // Remove references to non-existent controls, chip status is guaranteed by
  // chipStatus.isOK() check

  updateStatusBar(tr("Erasing All SPI NAND..."), 20000);
  try {
    lockUI();
    chip_op->chip_erase_all_spi_nand();
    QMessageBox::information(this, tr("Information"),
                             tr("Erase SPI NAND successfully"));
  } catch (const function_not_implemented &e) {
    QMessageBox::warning(this, tr("Warning"),
                         tr("Function is not implemented"));
  } catch (const std::runtime_error &e) {
    chipStatus.setNone();
    QMessageBox::warning(this, tr("Warning"), tr(e.what()));
  }

  releaseUI();
}

void MainWindow::scanSpiNor() {
  qDebug() << "Scanning SPI NOR...";
  if (chipStatus.isNone()) {
    scanChipWarning();
    return;
  }
  try {
    lockUI();
    auto norInfo = chip_op->chip_scan_spi_nor();
    // update ui - 使用正确的控件名称
    ui->flash_spi_nor_chip_label_2->setText(norInfo);
  } catch (const cannot_find_spi_flash_device &e) {
    ui->flash_spi_nor_chip_label_2->setText(tr("No supported SPI NOR found"));
  } catch (const function_not_implemented &e) {
    QMessageBox::warning(this, tr("Warning"),
                         tr("Function is not implemented"));
  } catch (const std::runtime_error &e) {
    QMessageBox::warning(this, tr("Warning"), tr(e.what()));
  }
  releaseUI();
}

void MainWindow::on_flash_spi_erase_spi_nor_scan_button_clicked() {
  scanSpiNor();
}

void MainWindow::on_dram_load_preset_pushButton_clicked() {
  if (chipStatus.isNone()) {
    scanChipWarning();
    return;
  }
  loadDramPresets();
}

void MainWindow::on_flash_spi_read_pushButton_clicked() {
  qDebug() << "SPI read button clicked";

  if (chipStatus.isNone()) {
    scanChipWarning();
    return;
  }

  try {
    // Use correct control names
    auto addr =
        ui->flash_spi_nor_read_address_lineEdit->text().toUInt(nullptr, 10);
    if (ui->flash_spi_nor_read_address_lineEdit->text().startsWith("0x"))
      addr =
          ui->flash_spi_nor_read_address_lineEdit->text().remove(0, 2).toUInt(
              nullptr, 16);
    auto len =
        ui->flash_spi_nor_read_length_lineEdit->text().toUInt(nullptr, 10);
    if (ui->flash_spi_nor_read_length_lineEdit->text().startsWith("0x"))
      len = ui->flash_spi_nor_read_length_lineEdit->text().remove(0, 2).toUInt(
          nullptr, 16);

    lockUI();
    QByteArray arr = chip_op->chip_read_spi_nand(addr, len);
    spiNandReadHexView->clear();
    spiNandReadHexView->setData(new QHexView::DataStorageArray(arr));
  } catch (const function_not_implemented &e) {
    QMessageBox::warning(this, tr("Warning"),
                         tr("Function is not implemented"));
  } catch (const std::runtime_error &e) {
    QMessageBox::warning(this, tr("Warning"), tr(e.what()));
  }
  releaseUI();
}

void MainWindow::on_flash_spi_write_fileOpen_button_clicked() {
  auto fileName = QFileDialog::getOpenFileName(
      this, tr("Open Image File"), "",
      tr("IMAGE (*.img *.IMG);;Binary (*.bin);;All files (*.*)"));
  // Use correct control names
  ui->flash_spi_nand_write_file_lineEdit->setText(fileName);
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    QMessageBox::warning(this, tr("File opening fail"),
                         tr("Problem with open file `") + fileName +
                             tr("` for reading"));
    return;
  }

  QByteArray fileBuf = file.readAll();
  spiNandWriteHexView->clear();
  spiNandWriteHexView->setData(new QHexView::DataStorageArray(fileBuf));
}

void MainWindow::on_flash_spi_write_button_clicked() {
  qDebug() << "WRITE SPI NAND...";
  if (chipStatus.isNone()) {
    scanChipWarning();
    return;
  }
  // Remove references to non-existent controls
  // auto fileName = ui->flash_spi_write_fileName_lineEdit->text();
  auto fileName = QFileDialog::getOpenFileName(
      this, tr("Open Image File"), "",
      tr("IMAGE (*.img *.IMG);;Binary (*.bin);;All files (*.*)"));
  if (fileName.isEmpty()) {
    return;
  }

  QFile file(fileName);
  uint64_t fileSize = 0;
  if (!file.open(QIODevice::ReadOnly)) {
    QMessageBox::warning(this, tr("File opening fail"),
                         tr("Problem with open file `") + fileName +
                             tr("` for reading"));
    return;
  }
  fileSize = file.size();
  const QByteArray fileBuf = file.readAll();
  try {
    lockUI();
    // Get address using correct control names
    auto addr =
        ui->flash_spi_nand_write_address_lineEdit->text().toUInt(nullptr, 10);
    if (ui->flash_spi_nand_write_address_lineEdit->text().startsWith("0x"))
      addr =
          ui->flash_spi_nand_write_address_lineEdit->text().remove(0, 2).toUInt(
              nullptr, 16);

    if (fileBuf.isEmpty()) {
      QMessageBox::warning(this, tr("Warning"),
                           tr("File is empty, nothing to write"));
      return;
    }

    chip_op->chip_write_spi_nand(addr, fileBuf, fileSize);
  } catch (const function_not_implemented &e) {
    QMessageBox::warning(this, tr("Warning"),
                         tr("Function is not implemented"));
  } catch (const std::runtime_error &e) {
    QMessageBox::warning(this, tr("Warning"), tr(e.what()));
  }
  releaseUI();
}

void MainWindow::on_flash_pushButton_1_clicked() {
  openFileDialog(ui->flash_lineEdit_1);
}

void MainWindow::on_flash_pushButton_2_clicked() {
  openFileDialog(ui->flash_lineEdit_2);
}

void MainWindow::on_flash_pushButton_3_clicked() {
  openFileDialog(ui->flash_lineEdit_3);
}

void MainWindow::on_flash_pushButton_4_clicked() {
  openFileDialog(ui->flash_lineEdit_4);
}

void MainWindow::on_flash_pushButton_5_clicked() {
  openFileDialog(ui->flash_lineEdit_5);
}

void MainWindow::on_flash_pushButton_6_clicked() {
  openFileDialog(ui->flash_lineEdit_6);
}

void MainWindow::on_flash_pushButton_7_clicked() {
  openFileDialog(ui->flash_lineEdit_7);
}

void MainWindow::on_flash_pushButton_8_clicked() {
  openFileDialog(ui->flash_lineEdit_8);
}

void MainWindow::on_flash_spi_read_button_clicked() {
  // TODO: Implement SPI read functionality
  qDebug() << "SPI read button clicked";
}

void MainWindow::on_run_open_file_clicked() {
  auto fileName = QFileDialog::getOpenFileName(
      this, tr("Open Image File"), "",
      tr("IMAGE (*.img *.IMG);;Binary (*.bin);;All files (*.*)"));
  ui->run_file_open_lineEdit->setText(fileName);
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    QMessageBox::warning(this, tr("File opening fail"),
                         tr("Problem with open file `") + fileName +
                             tr("` for reading"));
    return;
  }

  QByteArray fileBuf = file.readAll();
  runHexView->clear();
  runHexView->setData(new QHexView::DataStorageArray(fileBuf));
}

void MainWindow::on_run_run_button_clicked() {
  qDebug() << "Run Code withing BIN...";
  if (chipStatus.isNone()) {
    scanChipWarning();
    return;
  }
  auto fileName = ui->run_file_open_lineEdit->text();
  QFile file(fileName);
  uint64_t fileSize = 0;
  if (!file.open(QIODevice::ReadOnly)) {
    QMessageBox::warning(this, tr("File opening fail"),
                         tr("Problem with open file `") + fileName +
                             tr("` for reading"));
    return;
  }
  fileSize = file.size();
  const QByteArray fileBuf = file.readAll();

  auto addr = ui->run_run_address_lineEdit->text().toUInt(nullptr, 10);
  if (ui->run_run_address_lineEdit->text().startsWith("0x"))
    addr =
        ui->run_run_address_lineEdit->text().remove(0, 2).toUInt(nullptr, 16);

  if (addr <= 0)
    QMessageBox::warning(this, tr("Warning"), tr("Invalid address"));

  try {
    lockUI();
    chip_op->chip_write(addr, fileBuf, fileSize);
  } catch (const function_not_implemented &e) {
    QMessageBox::warning(this, tr("Warning"),
                         tr("Function is not implemented"));
  } catch (const std::runtime_error &e) {
    QMessageBox::warning(this, tr("Warning"), tr(e.what()));
  }
  releaseUI();
}

void MainWindow::on_dump_do_dump_button_clicked() {
  qDebug() << "Dump Code from FEL..";
  if (chipStatus.isNone()) {
    scanChipWarning();
    return;
  }
  try {
    auto addr = ui->dump_address_lineEdit->text().toUInt(nullptr, 10);
    if (ui->dump_address_lineEdit->text().startsWith("0x"))
      addr = ui->dump_address_lineEdit->text().remove(0, 2).toUInt(nullptr, 16);
    auto length = ui->dump_length_lineEdit->text().toUInt(nullptr, 10);
    if (ui->dump_length_lineEdit->text().startsWith("0x"))
      length =
          ui->dump_length_lineEdit->text().remove(0, 2).toUInt(nullptr, 16);

    lockUI();
    auto data = chip_op->chip_read(addr, length);
    dumpHexView->clear();
    dumpHexView->setData(new QHexView::DataStorageArray(data));
  } catch (const usb_bulk_send_error &e) {
    QMessageBox::warning(this, tr("Warning"), tr("Dump error"));
  } catch (const function_not_implemented &e) {
    QMessageBox::warning(this, tr("Warning"),
                         tr("Function is not implemented"));
  } catch (const std::runtime_error &e) {
    QMessageBox::warning(this, tr("Warning"), tr(e.what()));
  }
  releaseUI();
}

void MainWindow::on_dump_save_file_button_clicked() {
  QSaveFile file([this]() -> QString {
    return QFileDialog::getSaveFileName(
        this, tr("Save File"), "",
        tr("IMAGE (*.img *.IMG);;Binary (*.bin);;All files (*.*)"));
  }());
  if (!file.open(QIODevice::WriteOnly)) {
    QMessageBox::warning(this, tr("File opening fail"),
                         tr("Problem with open file"));
    return;
  }
  file.write(dumpHexView->getData());
  file.commit();
}

void MainWindow::updateMassProductionStats() {
  QString statsText = QString(tr("Success: %1  Failed: %2  Total: %3"))
                          .arg(m_successCount)
                          .arg(m_failureCount)
                          .arg(m_totalCount);
  ui->mass_production_stats_label->setText(statsText);

  // Set label color based on success/failure count
  if (m_failureCount > 0 && m_successCount == 0) {
    // All failed, red
    ui->mass_production_stats_label->setStyleSheet(
        "color: red; font-weight: bold;");
  } else if (m_successCount > 0 && m_failureCount == 0) {
    // All successful, green
    ui->mass_production_stats_label->setStyleSheet(
        "color: green; font-weight: bold;");
  } else if (m_successCount > 0 && m_failureCount > 0) {
    // Partially successful, orange
    ui->mass_production_stats_label->setStyleSheet(
        "color: orange; font-weight: bold;");
  } else {
    // Default style
    ui->mass_production_stats_label->setStyleSheet("");
  }
}

void MainWindow::scanMassProductionDevices() {
  // Clear old device list
  qDeleteAll(device_list);
  device_list.clear();

  // Clear previous statistics
  m_successCount = 0;
  m_failureCount = 0;
  m_totalCount = 0;

  try {
    usb scanner;
    scanner.usb_init();
    QList<libusb_device *> devs = scanner.list_fel_devices();
    scanner.usb_exit();

    // Remove device count limit, support all detected devices
    m_totalCount = devs.size();
    ui->mass_production_table->setRowCount(m_totalCount);

    // 更新统计信息显示
    updateMassProductionStats();

    for (int i = 0; i < m_totalCount; ++i) {
      libusb_device *d = devs.at(i);
      FelDevice *fd = new FelDevice(d, this);
      device_list.append(fd);

      // Port column
      QTableWidgetItem *portItem = new QTableWidgetItem(fd->getDevicePath());
      portItem->setFlags(portItem->flags() & ~Qt::ItemIsEditable);
      portItem->setTextAlignment(Qt::AlignCenter);
      ui->mass_production_table->setItem(i, 0, portItem);

      // Interface column
      QTableWidgetItem *interfaceItem =
          new QTableWidgetItem(fd->getDeviceSerial());
      interfaceItem->setFlags(interfaceItem->flags() & ~Qt::ItemIsEditable);
      interfaceItem->setTextAlignment(Qt::AlignCenter);
      ui->mass_production_table->setItem(i, 1, interfaceItem);

      // Status column
      QTableWidgetItem *statusItem = new QTableWidgetItem(tr("Ready"));
      statusItem->setFlags(statusItem->flags() & ~Qt::ItemIsEditable);
      statusItem->setTextAlignment(Qt::AlignCenter);
      statusItem->setBackground(QColor(255, 255, 224)); // 浅黄色
      ui->mass_production_table->setItem(i, 2, statusItem);

      // Progress column
      QProgressBar *progressBar = new QProgressBar();
      progressBar->setValue(0);
      progressBar->setAlignment(Qt::AlignCenter);
      // Set progress bar style to improve readability
      progressBar->setStyleSheet(
          "QProgressBar { border: 1px solid grey; border-radius: 2px; "
          "text-align: center; }"
          "QProgressBar::chunk { background-color: #3add36; width: 1px; }");
      ui->mass_production_table->setCellWidget(i, 3, progressBar);

      // Action列
      QPushButton *actionButton = new QPushButton(tr("Burn"));
      actionButton->setProperty("row", i);
      actionButton->setStyleSheet(
          "QPushButton { min-width: 80px; padding: 5px; border: 1px solid "
          "#aaa; border-radius: 4px; }"
          "QPushButton:disabled { background-color: #cccccc; }");
      ui->mass_production_table->setCellWidget(i, 4, actionButton);

      // 连接信号和槽
      connect(fd, &FelDevice::statusChanged, this, [=](const QString &status) {
        if (i < ui->mass_production_table->rowCount()) {
          QTableWidgetItem *statusItem = ui->mass_production_table->item(i, 2);
          statusItem->setText(status);

          // 根据状态设置颜色
          if (status == tr("Ready")) {
            statusItem->setBackground(QColor(255, 255, 224)); // 浅黄色
          } else if (status.contains(tr("Error")) ||
                     status.contains("error", Qt::CaseInsensitive)) {
            statusItem->setBackground(QColor(255, 99, 71)); // 红色
          } else {
            statusItem->setBackground(QColor(173, 216, 230)); // 浅蓝色
          }
        }
      });

      connect(fd, &FelDevice::progressChanged, this, [=](int progress) {
        if (i < ui->mass_production_table->rowCount()) {
          QProgressBar *pb = qobject_cast<QProgressBar *>(
              ui->mass_production_table->cellWidget(i, 3));
          if (pb)
            pb->setValue(progress);
        }
      });

      // 连接详细进度信号
      connect(fd, &FelDevice::progressDetailChanged, this,
              [=](const QString &detail) {
                // 可以将详细信息记录到日志或状态栏
                updateStatusBar(detail, 2000);
              });

      connect(fd, &FelDevice::finished, this, [=](bool success) {
        if (success) {
          m_successCount++;
        } else {
          m_failureCount++;
        }

        // 更新统计信息显示
        updateMassProductionStats();

        if (i < ui->mass_production_table->rowCount()) {
          QPushButton *btn = qobject_cast<QPushButton *>(
              ui->mass_production_table->cellWidget(i, 4));
          if (btn) {
            btn->setText(success ? tr("Success") : tr("Failed"));
            btn->setEnabled(true);
          }

          // 设置状态单元格颜色
          QTableWidgetItem *statusItem = ui->mass_production_table->item(i, 2);
          if (statusItem) {
            if (success) {
              statusItem->setBackground(QColor(144, 238, 144)); // 浅绿色
            } else {
              statusItem->setBackground(QColor(255, 99, 71)); // 红色
            }
          }

          // 更新状态栏
          QString message = QString(tr("Device %1 %2"))
                                .arg(fd->getDevicePath())
                                .arg(success ? tr("succeeded") : tr("failed"));
          updateStatusBar(message);
        }
      });

      connect(fd, &FelDevice::error, this, [=](const QString &error) {
        m_failureCount++;

        // 更新统计信息显示
        updateMassProductionStats();

        if (i < ui->mass_production_table->rowCount()) {
          QTableWidgetItem *statusItem = ui->mass_production_table->item(i, 2);
          statusItem->setText(tr("Error: ") + error);

          // 设置错误状态颜色
          statusItem->setBackground(QColor(255, 99, 71)); // 红色

          QPushButton *btn = qobject_cast<QPushButton *>(
              ui->mass_production_table->cellWidget(i, 4));
          if (btn) {
            btn->setText(tr("Error"));
            btn->setEnabled(true);
          }

          // 记录错误到状态栏
          QString errorMessage = QString(tr("Device %1 error: %2"))
                                     .arg(fd->getDevicePath())
                                     .arg(error);
          updateStatusBar(errorMessage);

          // 记录到日志（如果有的话）
          qDebug() << "Device error:" << errorMessage;
        }
      });

      connect(actionButton, &QPushButton::clicked, this, [=]() {
        actionButton->setText(tr("Burning..."));
        actionButton->setEnabled(false);

        QString imagePath = ui->mass_production_image_path->text();
        if (imagePath.isEmpty()) {
          fd->error(tr("Please select an image file"));
          actionButton->setText(tr("Burn"));
          actionButton->setEnabled(true);
          return;
        }

        // 检查文件是否存在
        QFile file(imagePath);
        if (!file.exists()) {
          fd->error(tr("Image file does not exist"));
          actionButton->setText(tr("Burn"));
          actionButton->setEnabled(true);
          return;
        }

        // 更新状态栏
        QString startMessage =
            QString(tr("Starting burn on device %1")).arg(fd->getDevicePath());
        updateStatusBar(startMessage);

        fd->startBurn(imagePath);
      });
    }

    // 初始化统计标签
    updateMassProductionStats();

    // 调整列宽
    ui->mass_production_table->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    ui->mass_production_table->horizontalHeader()->setStretchLastSection(true);

    // 设置状态栏信息
    if (m_totalCount > 0) {
      updateStatusBar(QString(tr("Found %1 devices ready for production"))
                          .arg(m_totalCount));
    } else {
      updateStatusBar(tr("No devices found"));
    }

  } catch (const std::exception &e) {
    QMessageBox::warning(this, tr("Warning"),
                         tr("Error enumerating devices: %1").arg(e.what()));
    updateStatusBar(tr("Error enumerating devices"));
  }
}

void MainWindow::on_mass_production_image_browse_button_clicked() {
  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Open Image File"), "",
      tr("IMAGE (*.img *.IMG);;Binary (*.bin);;All files (*.*)"));
  ui->mass_production_image_path->setText(fileName);
}

void MainWindow::on_mass_production_start_all_button_clicked() {
  QString imagePath = ui->mass_production_image_path->text();
  if (imagePath.isEmpty()) {
    QMessageBox::warning(this, tr("Warning"),
                         tr("Please select an image file"));
    return;
  }

  for (int i = 0; i < ui->mass_production_table->rowCount(); ++i) {
    QPushButton *btn = qobject_cast<QPushButton *>(
        ui->mass_production_table->cellWidget(i, 4));
    if (btn && btn->text() == tr("Burn")) {
      btn->click();
    }
  }
}

void MainWindow::on_mass_production_stop_all_button_clicked() {
  for (FelDevice *device : device_list) {
    device->stopBurn();
  }
}

void MainWindow::closeEvent(QCloseEvent *event) {
  // 在窗口关闭时停止所有设备
  for (FelDevice *device : device_list) {
    device->stopBurn();
  }
  QMainWindow::closeEvent(event);
}

void MainWindow::changeEvent(QEvent *event) {
  QMainWindow::changeEvent(event);
  // 可以在这里处理窗口状态变化等事件
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
  if (obj == ui->dram_dram_tpr13_label) {
    if (event->type() == QEvent::MouseButtonDblClick) {
      qDebug() << "dram_dram_tpr13_label double clicked";
      tpr13->setWindowTitle(tr("DRAM Driver Options Settings"));
      tpr13->SetTPR13Value(
          ui->dram_dram_tpr13_lineEdit->text().remove(0, 2).toUInt(nullptr,
                                                                   16));
      tpr13->show();
      return true;
    }
  }
  return false;
}

void MainWindow::TPR13_value_getter(uint32_t value) {
  qDebug() << "TPR13_value_getter: " << value;
}

// 新增复制按钮的功能
void MainWindow::on_chip_name_copy_pushButton_clicked() {
  copyToClipboard(ui->chip_name_lineEdit->text(),
                  ui->chip_name_copy_pushButton);
}

void MainWindow::on_chip_id_copy_pushButton_clicked() {
  copyToClipboard(ui->chip_id_lineEdit->text(),
                  ui->chip_id_copy_pushButton);
}

void MainWindow::on_chip_sid_copy_pushButton_clicked() {
  copyToClipboard(ui->chip_sid_lineEdit->text(),
                  ui->chip_sid_copy_pushButton);
}

void MainWindow::on_chip_core_copy_pushButton_clicked() {
  copyToClipboard(ui->chip_core_lineEdit->text(),
                  ui->chip_core_copy_pushButton);
}
