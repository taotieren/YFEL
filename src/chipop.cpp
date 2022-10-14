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
#include <utility>

#include "chips/chip_db.h"
#include "chipop.h"
#include "exceptions.h"

ChipOP::ChipOP() = default;

ChipOP::~ChipOP() {
    qDebug() << "ChipOP::~ChipOP()";
    delete fel_;
}

void ChipOP::chip_release_ui() {
    qDebug() << "ChipOP::chip_release_ui()";
    emit release_ui();
}

void ChipOP::generate_chip_db() {
    chip_db.push_back(new d1(fel_, chip_version));
    chip_db.push_back(new r528(fel_, chip_version));
}

void ChipOP::chip_scan_chip() {
    fel_->fel_scan_chip();
    chip_version = fel_->fel_get_chip_version();

    // Generate chip db
    generate_chip_db();

    // check chip type form chip id
    if (!check_chip()) {
        throw std::runtime_error("Unsupported Chip\nfunction not implemented");
    }
}

void ChipOP::chip_reset_chip() {
    try {
        if (current_chip->chip_reset() == chip_function_e::NotSupport)
            throw function_not_implemented();
    } catch (const usb_bulk_recv_error &error) { // Catch usb_bulk_recv_error handle as success
        qDebug() << "Reset Done";
    } catch (const usb_bulk_send_error &error) { // Catch usb_bulk_send_error handle as success
        qDebug() << "Reset Done";
    }
}

void ChipOP::chip_enable_jtag() {
    if (current_chip->chip_jtag() == chip_function_e::NotSupport)
        throw function_not_implemented();

}

chip_t ChipOP::get_current_chip() {
    return current_chip->get_chip_info();
}

bool ChipOP::check_chip() {
    for (auto item: qAsConst(chip_db)) {
        qDebug() << "Checking chip " << item->get_chip_info().chip_id;
        if (item->chip_detect() == chip_function_e::Success) {
            current_chip = item;
            qDebug() << "Current Chip" << current_chip->get_chip_info().chip_name;
            return true;
        }
    }
    return false;
}

void ChipOP::chip_init_dram(const dram_info_t &info) {
    current_chip->chip_ddr(info.dram_param);
}

void ChipOP::chip_init_dram(const dram_param_t &param) {
    current_chip->chip_ddr(param);
}

QString ChipOP::chip_scan_spi_nand() {
    fel_->fel_open_connection();
    // TODO: Need try...catch to handle exception
    spi_nand spinand(current_chip, fel_);
    spinand.init();
    fel_->fel_close_connection();

    chip_release_ui();
    if (spinand.get_spi_nand_size() == 0) {
        return {"No supported SPI NAND found"};
    } else {
        return spinand.get_spi_nand_name() + " "
               + QString::number(spinand.get_spi_nand_size() / 1024 / 1024) + "MB 0x"
               + QString::number(spinand.get_spi_nand_size(), 16);
    }
}

void ChipOP::chip_erase_spi_nand(uint32_t addr, uint32_t len) {
    fel_->fel_open_connection();
    // TODO: Need try...catch to handle exception
    spi_nand spinand(current_chip, fel_);
    connect(&spinand, &spi_nand::release_ui, this, &ChipOP::chip_release_ui);
    spinand.init();
    spinand.erase(addr, len);
    disconnect(&spinand, &spi_nand::release_ui, this, &ChipOP::chip_release_ui);
    fel_->fel_close_connection();
}

void ChipOP::chip_erase_all_spi_nand() {
    fel_->fel_open_connection();
    // TODO: Need try...catch to handle exception
    spi_nand spinand(current_chip, fel_);
    connect(&spinand, &spi_nand::release_ui, this, &ChipOP::chip_release_ui);
    spinand.init();
    spinand.erase(0x00, spinand.get_spi_nand_size());
    disconnect(&spinand, &spi_nand::release_ui, this, &ChipOP::chip_release_ui);
    fel_->fel_close_connection();
}

void ChipOP::chip_exec(uint32_t addr) {
    fel_->fel_exec(addr);
}

QVector<dram_info_t> ChipOP::get_dram_params() {
    return current_chip->get_chip_dram_info();
}

void ChipOP::chip_sid() {
    // Read SID Here
    current_chip->chip_sid();
}

void ChipOP::chip_write_spi_nand(const uint64_t addr, uint8_t *buf, const uint64_t len) {
    spi_nand spinand(current_chip, fel_);
    connect(&spinand, &spi_nand::release_ui, this, &ChipOP::chip_release_ui);
    spinand.init();
    spinand.write(addr, buf, len);
    disconnect(&spinand, &spi_nand::release_ui, this, &ChipOP::chip_release_ui);
}


