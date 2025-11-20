/*
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#ifndef YFEL_MAINWINDOW_H
#define YFEL_MAINWINDOW_H

#include <QByteArray>
#include <QCloseEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMouseEvent>
#include <QProgressBar>
#include <QPushButton>
#include <QTableWidget>
#include <QVector>
#include <QWidget>

#include <lib/QHexView/QHexView.h>

#include "chip_status.h"
#include "chipop.h"
#include "feldevice.h"
#include "tpr13dialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

private slots:
  // UI control slots
  void on_scan_pushButton_clicked();
  void on_chip_chip_name_pushButton_clicked();
  void on_chip_chip_id_pushButton_2_clicked();
  void on_chip_chip_sid_pushButton_3_clicked();
  void on_chip_chip_core_pushButton_4_clicked();
  // 新增芯片信息复制按钮槽函数
  void on_chip_name_copy_pushButton_clicked();
  void on_chip_id_copy_pushButton_clicked();
  void on_chip_sid_copy_pushButton_clicked();
  void on_chip_core_copy_pushButton_clicked();
  void on_chip_spi_nor_scan_pushButton_clicked();
  void on_chip_spi_nand_scan_pushButton_clicked();
  void on_Misc_exec_addr_btn_clicked();
  void on_tabWidget_currentChanged(int index);
  void on_dram_load_preset_comboBox_currentIndexChanged();
  void on_dram_init_dram_btn_clicked();
  void on_flash_spi_erase_spi_nand_scan_button_clicked();
  void on_flash_spi_erase_spi_nand_erase_button_clicked();
  void on_flash_spi_erase_spi_nand_setall_button_clicked();
  void on_flash_spi_erase_spi_nor_scan_button_clicked();
  void on_dram_load_preset_pushButton_clicked();
  void on_flash_spi_read_pushButton_clicked();
  void on_flash_spi_write_fileOpen_button_clicked();
  void on_flash_spi_write_button_clicked();
  void on_flash_pushButton_1_clicked();
  void on_flash_pushButton_2_clicked();
  void on_flash_pushButton_3_clicked();
  void on_flash_pushButton_4_clicked();
  void on_flash_pushButton_5_clicked();
  void on_flash_pushButton_6_clicked();
  void on_flash_pushButton_7_clicked();
  void on_flash_pushButton_8_clicked();
  void on_flash_spi_read_button_clicked();
  void on_run_open_file_clicked();
  void on_run_run_button_clicked();
  void on_dump_do_dump_button_clicked();
  void on_dump_save_file_button_clicked();

  // Mass production tab slots
  void on_mass_production_image_browse_button_clicked();
  void on_mass_production_start_all_button_clicked();
  void on_mass_production_stop_all_button_clicked();

  // Custom slots
  void TPR13_value_getter(uint32_t value);

private:
  // Initialization functions
  void initMainwindowData();
  void initMenubar();
  void initMassProductionTab();
  
  // Theme functions
  void setupThemeMenu();
  void setLightTheme();
  void setDarkTheme();
  
  // UI update functions
  void updateStatusBar(const QString &message, int timeout = 0);
  void updateChipInfo();
  void updateDramInfo();

  // UI control functions
  void lockUI();
  void releaseUI();
  void clearChipInfo();

  // Device operation functions
  void scanChipWarning();
  void loadDramPresets();
  void scanSpiNand();
  void scanSpiNor();
  void chipReset();
  void enableJtag();

  // Utility functions
  QString openFileDialog(QLineEdit *lineEdit);
  void copyToClipboard(const QString &data, QPushButton *button);

  // Event handlers
  void closeEvent(QCloseEvent *event) override;
  void changeEvent(QEvent *event) override;
  bool eventFilter(QObject *obj, QEvent *event) override;

  // Static utility functions
  static void exitMenuClicked();

private:
  // UI elements
  Ui::MainWindow *ui;
  ChipOP *chip_op = new ChipOP();
  QHexView *spiNandReadHexView = new QHexView();
  QHexView *spiNandWriteHexView = new QHexView();
  QHexView *runHexView = new QHexView();
  QHexView *dumpHexView = new QHexView();
  TPR13Dialog *tpr13 = new TPR13Dialog();

  // Data members
  chip_status chipStatus;
  QVector<FelDevice *> device_list;

  // Tab constants
  enum uiTabWidgetIndex {
    tab_chip = 0,
    tab_flash = 1,
    tab_run = 2,
    tab_dump = 3,
    tab_data = 4,
    tab_dram = 5,
    tab_mass_production = 6,
    tab_misc = 7
  };

  // Mass production statistics
  int m_successCount = 0;
  int m_failureCount = 0;
  int m_totalCount = 0;

  // Device scanning and management
  void scanMassProductionDevices();
  void updateMassProductionStats();
};

#endif // YFEL_MAINWINDOW_H