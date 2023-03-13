/*
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

//
// Created by gloom on 2022/9/23.
//

#include "r528.h"

r528::r528(class fel *f, chip_version_t chip_version) : Chips(f, chip_version) {
    chip_info.chip_name = "R528-S1/R528-S2/R528-S3/R528-S4/T113-I/T113-S3/H133";
    chip_info.chip_id = 0x00185900;
    chip_info.chip_type = chip_type_e::Heterogeneous;
    chip_info.chip_core = core_name_.CortexA7;
    chip_info.chip_core_count = 2;
    chip_info.chip_core_count_str = core_count_.core_count_2;
    chip_info.chip_heterogeneous_core.push_back("HIFI4");

    // dram presets
    dram_info.append(r528_s3_ddr3);
    dram_info.append(t113_s3_ddr3);
    dram_info.append(t113_s3_ddr3_oc_1008);
}

chip_function_e r528::check_chip_id() {
    chip_id_map_[0x6000] = QString("T113-S3");
    chip_id_map_[0x6800] = QString("T113-S4");

    auto chip_id = fel_->payload_arm_read32(0x3006200) & 0xffff;
    qDebug() << "Get Chip ID: 0x" << QString::number(chip_id, 16);

    for (const auto &i: chip_id_map_) {
        if (i.first == chip_id) {
            chip_info.chip_name = i.second;
        }
    }
    return chip_function_e::Success;
}

chip_function_e r528::chip_detect() {
    if (chip_info.chip_version.id == chip_info.chip_id)
        // Check 0 addr is 0x43014281, ARM Cortex-A7
        if (fel_->fel_read32(0x00000000) == 0xea000019)
            return chip_function_e::Success;
    return chip_function_e::Fail;
}

chip_function_e r528::chip_reset() {
    // Timer::WDOG_IRQ_EN_REG
    fel_->fel_write32(0x020500a0 + 0x08, (0x16aa << 16) | (0x1 << 0));
    return chip_function_e::Success;
}

chip_function_e r528::chip_sid() {
    uint32_t id[4];
    id[0] = fel_->payload_arm_read32(0x03006200 + 0x0);
    id[1] = fel_->payload_arm_read32(0x03006200 + 0x4);
    id[2] = fel_->payload_arm_read32(0x03006200 + 0x8);
    id[3] = fel_->payload_arm_read32(0x03006200 + 0xc);

    chip_info.chip_sid = "";
    for (const uint32_t &j: id) {
        chip_info.chip_sid.append(QString::number(j, 16));
    }
    return chip_function_e::Success;
}

chip_function_e r528::chip_jtag() {
    const uint8_t payload[] = {
            0x00, 0x00, 0xa0, 0xe3, 0x17, 0x0f, 0x08, 0xee, 0x15, 0x0f, 0x07, 0xee,
            0xd5, 0x0f, 0x07, 0xee, 0x9a, 0x0f, 0x07, 0xee, 0x95, 0x0f, 0x07, 0xee,
            0xff, 0xff, 0xff, 0xea, 0x58, 0x00, 0x9f, 0xe5, 0x00, 0xd0, 0x80, 0xe5,
            0x04, 0xe0, 0x80, 0xe5, 0x00, 0xe0, 0x0f, 0xe1, 0x08, 0xe0, 0x80, 0xe5,
            0x10, 0xef, 0x11, 0xee, 0x0c, 0xe0, 0x80, 0xe5, 0x10, 0xef, 0x1c, 0xee,
            0x10, 0xe0, 0x80, 0xe5, 0x10, 0xef, 0x11, 0xee, 0x14, 0xe0, 0x80, 0xe5,
            0x0c, 0x00, 0x00, 0xeb, 0x28, 0x00, 0x9f, 0xe5, 0x00, 0xd0, 0x90, 0xe5,
            0x04, 0xe0, 0x90, 0xe5, 0x14, 0x10, 0x90, 0xe5, 0x10, 0x1f, 0x01, 0xee,
            0x10, 0x10, 0x90, 0xe5, 0x10, 0x1f, 0x0c, 0xee, 0x0c, 0x10, 0x90, 0xe5,
            0x10, 0x1f, 0x01, 0xee, 0x08, 0x10, 0x90, 0xe5, 0x01, 0xf0, 0x29, 0xe1,
            0x1e, 0xff, 0x2f, 0xe1, 0xe0, 0x7f, 0x04, 0x00, 0x02, 0x34, 0xa0, 0xe3,
            0xf0, 0x20, 0x93, 0xe5, 0x0f, 0x20, 0xc2, 0xe3, 0x03, 0x20, 0x82, 0xe3,
            0xf0, 0x20, 0x83, 0xe5, 0xf0, 0x20, 0x93, 0xe5, 0xf0, 0x20, 0xc2, 0xe3,
            0x30, 0x20, 0x82, 0xe3, 0xf0, 0x20, 0x83, 0xe5, 0xf0, 0x20, 0x93, 0xe5,
            0x0f, 0x2a, 0xc2, 0xe3, 0x03, 0x2a, 0x82, 0xe3, 0xf0, 0x20, 0x83, 0xe5,
            0xf0, 0x20, 0x93, 0xe5, 0x0f, 0x26, 0xc2, 0xe3, 0x03, 0x26, 0x82, 0xe3,
            0xf0, 0x20, 0x83, 0xe5, 0x1e, 0xff, 0x2f, 0xe1, 0x04, 0x00, 0x00, 0x00,
            0x14, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x47, 0x4e, 0x55, 0x00,
            0xe4, 0x69, 0x4d, 0x72, 0xa6, 0x44, 0x29, 0x93, 0x6b, 0x31, 0xb1, 0x33,
            0x26, 0xa9, 0xde, 0x4e, 0xce, 0x0e, 0x29, 0xd6, 0x2f, 0x6c, 0x69, 0x62,
            0x2f, 0x6c, 0x64, 0x2d, 0x6c, 0x69, 0x6e, 0x75, 0x78, 0x2d, 0x61, 0x72,
            0x6d, 0x68, 0x66, 0x2e, 0x73, 0x6f, 0x2e, 0x33, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xf5, 0xfe, 0xff, 0x6f, 0x1c, 0x81, 0x02, 0x00, 0x05, 0x00, 0x00, 0x00,
            0x18, 0x81, 0x02, 0x00, 0x06, 0x00, 0x00, 0x00, 0x08, 0x81, 0x02, 0x00,
            0x0a, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
            0x10, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x1e, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0xfb, 0xff, 0xff, 0x6f,
            0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x38, 0x81, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
    };
    fel_->fel_write(0x00028000, &payload[0], sizeof(payload));
    fel_->fel_exec(0x00028000);
    return chip_function_e::Success;
}

chip_function_e r528::chip_ddr(chip_ddr_type_e dram_type) {
    // default using R528 ddr init code
    if (dram_type == chip_ddr_type_e::DDR3) {
        fel_->fel_write(0x00028000, &ddr3_dram_payload[0], sizeof(ddr3_dram_payload));
        fel_->fel_write(0x00028038, &r528_s3_ddr3, sizeof(r528_s3_ddr3));
        fel_->fel_exec(0x00028000);
        return chip_function_e::Success;
    }
    return chip_function_e::NotSupport;
}

chip_function_e r528::chip_ddr(dram_param_t param) {
    if (param.dram_type == chip_ddr_type_e::DDR3) {
        fel_->fel_write(0x00028000, &ddr3_dram_payload[0], sizeof(ddr3_dram_payload));
    } else {
        return chip_function_e::NotSupport;
    }
    fel_->fel_write(0x00028038, &param, sizeof(param));
    fel_->fel_exec(0x00028000);
    return chip_function_e::Success;
}

chip_function_e r528::chip_spi_init(uint32_t *swap_buf, uint32_t *swap_len, uint32_t *cmd_len) {
    const uint8_t payload[] = {
            0x00, 0x00, 0xa0, 0xe3, 0x17, 0x0f, 0x08, 0xee, 0x15, 0x0f, 0x07, 0xee,
            0xd5, 0x0f, 0x07, 0xee, 0x9a, 0x0f, 0x07, 0xee, 0x95, 0x0f, 0x07, 0xee,
            0xff, 0xff, 0xff, 0xea, 0x5c, 0x00, 0x9f, 0xe5, 0x00, 0xd0, 0x80, 0xe5,
            0x04, 0xe0, 0x80, 0xe5, 0x00, 0xe0, 0x0f, 0xe1, 0x08, 0xe0, 0x80, 0xe5,
            0x10, 0xef, 0x11, 0xee, 0x0c, 0xe0, 0x80, 0xe5, 0x10, 0xef, 0x1c, 0xee,
            0x10, 0xe0, 0x80, 0xe5, 0x10, 0xef, 0x11, 0xee, 0x14, 0xe0, 0x80, 0xe5,
            0x29, 0x0a, 0xa0, 0xe3, 0x77, 0x00, 0x00, 0xeb, 0x28, 0x00, 0x9f, 0xe5,
            0x00, 0xd0, 0x90, 0xe5, 0x04, 0xe0, 0x90, 0xe5, 0x14, 0x10, 0x90, 0xe5,
            0x10, 0x1f, 0x01, 0xee, 0x10, 0x10, 0x90, 0xe5, 0x10, 0x1f, 0x0c, 0xee,
            0x0c, 0x10, 0x90, 0xe5, 0x10, 0x1f, 0x01, 0xee, 0x08, 0x10, 0x90, 0xe5,
            0x01, 0xf0, 0x29, 0xe1, 0x1e, 0xff, 0x2f, 0xe1, 0xe0, 0x7f, 0x04, 0x00,
            0xf0, 0x40, 0x2d, 0xe9, 0x00, 0x50, 0x51, 0xe2, 0xf0, 0x80, 0xbd, 0x08,
            0x05, 0xea, 0xa0, 0xe3, 0x52, 0x6c, 0xa0, 0xe3, 0x02, 0xe4, 0x40, 0xe3,
            0x53, 0x4c, 0xa0, 0xe3, 0x02, 0x64, 0x40, 0xe3, 0x00, 0x70, 0xe0, 0xe3,
            0x02, 0x44, 0x40, 0xe3, 0x40, 0x00, 0x55, 0xe3, 0x00, 0x30, 0xa0, 0xe3,
            0x07, 0x10, 0xa0, 0xe1, 0x05, 0x20, 0xa0, 0x31, 0x40, 0x20, 0xa0, 0x23,
            0x30, 0x20, 0x8e, 0xe5, 0x34, 0x20, 0x8e, 0xe5, 0x38, 0x20, 0x8e, 0xe5,
            0x00, 0x10, 0xc6, 0xe5, 0x01, 0x30, 0x83, 0xe2, 0x03, 0x00, 0x52, 0xe1,
            0xfb, 0xff, 0xff, 0xca, 0x08, 0x30, 0x9e, 0xe5, 0x02, 0x31, 0x83, 0xe3,
            0x08, 0x30, 0x8e, 0xe5, 0x1c, 0x30, 0x9e, 0xe5, 0x73, 0x30, 0xef, 0xe6,
            0x03, 0x00, 0x52, 0xe1, 0xfb, 0xff, 0xff, 0x8a, 0x00, 0x30, 0xa0, 0xe3,
            0x00, 0xc0, 0xd4, 0xe5, 0x00, 0x00, 0x50, 0xe3, 0x00, 0x10, 0xa0, 0xe1,
            0x01, 0x30, 0x83, 0xe2, 0x7c, 0xc0, 0xef, 0xe6, 0x06, 0x00, 0x00, 0x0a,
            0x03, 0x00, 0x52, 0xe1, 0x01, 0xc0, 0xc1, 0xe4, 0x01, 0x00, 0xa0, 0xe1,
            0xf5, 0xff, 0xff, 0xca, 0x02, 0x50, 0x55, 0xe0, 0xdf, 0xff, 0xff, 0x1a,
            0xf0, 0x80, 0xbd, 0xe8, 0x03, 0x00, 0x52, 0xe1, 0x01, 0x10, 0x83, 0xe2,
            0x02, 0x30, 0x83, 0xe2, 0xf8, 0xff, 0xff, 0xda, 0x01, 0x00, 0x52, 0xe1,
            0x00, 0x10, 0xd4, 0xe5, 0xf5, 0xff, 0xff, 0xda, 0x03, 0x00, 0x52, 0xe1,
            0x00, 0x10, 0xd4, 0xe5, 0x01, 0x10, 0x83, 0xe2, 0x02, 0x30, 0x83, 0xe2,
            0xf7, 0xff, 0xff, 0xca, 0x02, 0x50, 0x55, 0xe0, 0xd0, 0xff, 0xff, 0x1a,
            0xf0, 0x80, 0xbd, 0xe8, 0xf0, 0x40, 0x2d, 0xe9, 0x00, 0x60, 0x51, 0xe2,
            0xf0, 0x80, 0xbd, 0x08, 0x05, 0x1a, 0xa0, 0xe3, 0x52, 0x5c, 0xa0, 0xe3,
            0x02, 0x14, 0x40, 0xe3, 0x53, 0x4c, 0xa0, 0xe3, 0x02, 0x54, 0x40, 0xe3,
            0x00, 0x70, 0xe0, 0xe3, 0x02, 0x44, 0x40, 0xe3, 0x40, 0x00, 0x56, 0xe3,
            0x06, 0x20, 0xa0, 0x31, 0x40, 0x20, 0xa0, 0x23, 0x30, 0x20, 0x81, 0xe5,
            0x00, 0x00, 0x50, 0xe3, 0x34, 0x20, 0x81, 0xe5, 0x38, 0x20, 0x81, 0xe5,
            0x17, 0x00, 0x00, 0x0a, 0x00, 0xc0, 0xa0, 0xe1, 0x00, 0x30, 0xa0, 0xe3,
            0x01, 0xe0, 0xdc, 0xe4, 0x01, 0x30, 0x83, 0xe2, 0x03, 0x00, 0x52, 0xe1,
            0x00, 0xe0, 0xc5, 0xe5, 0xfa, 0xff, 0xff, 0xca, 0x08, 0x30, 0x91, 0xe5,
            0x02, 0x31, 0x83, 0xe3, 0x08, 0x30, 0x81, 0xe5, 0x1c, 0x30, 0x91, 0xe5,
            0x73, 0x30, 0xef, 0xe6, 0x03, 0x00, 0x52, 0xe1, 0xfb, 0xff, 0xff, 0x8a,
            0x00, 0x30, 0xa0, 0xe3, 0x00, 0xc0, 0xd4, 0xe5, 0x01, 0x30, 0x83, 0xe2,
            0x03, 0x00, 0x52, 0xe1, 0xfb, 0xff, 0xff, 0xca, 0x00, 0x00, 0x50, 0xe3,
            0x02, 0x00, 0x80, 0x10, 0x02, 0x60, 0x56, 0xe0, 0xe0, 0xff, 0xff, 0x1a,
            0xf0, 0x80, 0xbd, 0xe8, 0x00, 0x30, 0xa0, 0xe1, 0x07, 0xc0, 0xa0, 0xe1,
            0x00, 0xc0, 0xc5, 0xe5, 0x01, 0x30, 0x83, 0xe2, 0x03, 0x00, 0x52, 0xe1,
            0xfb, 0xff, 0xff, 0xca, 0xe7, 0xff, 0xff, 0xea, 0xf0, 0x43, 0x2d, 0xe9,
            0x0f, 0x80, 0x0c, 0xe3, 0xff, 0x8f, 0x4f, 0xe3, 0x05, 0x5a, 0xa0, 0xe3,
            0x02, 0x54, 0x40, 0xe3, 0x01, 0x7a, 0xa0, 0xe3, 0x00, 0x72, 0x40, 0xe3,
            0x14, 0xd0, 0x4d, 0xe2, 0x00, 0x60, 0xa0, 0xe1, 0x06, 0x40, 0xa0, 0xe1,
            0x01, 0x30, 0xd4, 0xe4, 0x01, 0x00, 0x53, 0xe3, 0x1e, 0x00, 0x00, 0x0a,
            0x02, 0x00, 0x53, 0xe3, 0x54, 0x00, 0x00, 0x0a, 0x03, 0x00, 0x53, 0xe3,
            0x57, 0x00, 0x00, 0x0a, 0x04, 0x00, 0x53, 0xe3, 0x5b, 0x00, 0x00, 0x0a,
            0x05, 0x00, 0x53, 0xe3, 0x60, 0x00, 0x00, 0x0a, 0x06, 0x00, 0x53, 0xe3,
            0x6f, 0x00, 0x00, 0x0a, 0x07, 0x00, 0x53, 0xe3, 0x7e, 0x00, 0x00, 0x0a,
            0x08, 0x00, 0x53, 0xe3, 0x8b, 0x00, 0x00, 0x1a, 0xb0, 0x80, 0xcd, 0xe1,
            0x0d, 0x90, 0xa0, 0xe1, 0x08, 0x60, 0x8d, 0xe2, 0x02, 0x10, 0xa0, 0xe3,
            0x09, 0x00, 0xa0, 0xe1, 0xad, 0xff, 0xff, 0xeb, 0x01, 0x10, 0xa0, 0xe3,
            0x06, 0x00, 0xa0, 0xe1, 0x70, 0xff, 0xff, 0xeb, 0x08, 0x30, 0xdd, 0xe5,
            0x01, 0x00, 0x13, 0xe3, 0xf6, 0xff, 0xff, 0x1a, 0x04, 0x60, 0xa0, 0xe1,
            0x06, 0x40, 0xa0, 0xe1, 0x01, 0x30, 0xd4, 0xe4, 0x01, 0x00, 0x53, 0xe3,
            0xe0, 0xff, 0xff, 0x1a, 0x02, 0x34, 0xa0, 0xe3, 0x01, 0x1a, 0xa0, 0xe3,
            0x60, 0x20, 0x93, 0xe5, 0x0f, 0x2c, 0xc2, 0xe3, 0x02, 0x2c, 0x82, 0xe3,
            0x60, 0x20, 0x83, 0xe5, 0x60, 0x20, 0x93, 0xe5, 0x0f, 0x2a, 0xc2, 0xe3,
            0x02, 0x2a, 0x82, 0xe3, 0x60, 0x20, 0x83, 0xe5, 0x60, 0x20, 0x93, 0xe5,
            0x0f, 0x28, 0xc2, 0xe3, 0x02, 0x28, 0x82, 0xe3, 0x60, 0x20, 0x83, 0xe5,
            0x60, 0x20, 0x93, 0xe5, 0x0f, 0x26, 0xc2, 0xe3, 0x02, 0x26, 0x82, 0xe3,
            0x60, 0x20, 0x83, 0xe5, 0x6c, 0x39, 0x97, 0xe5, 0x01, 0x38, 0x83, 0xe3,
            0x6c, 0x39, 0x87, 0xe5, 0x40, 0x39, 0x97, 0xe5, 0x02, 0x31, 0x83, 0xe3,
            0x40, 0x39, 0x87, 0xe5, 0x6c, 0x39, 0x97, 0xe5, 0x01, 0x30, 0x83, 0xe3,
            0x6c, 0x39, 0x87, 0xe5, 0x40, 0x39, 0x97, 0xe5, 0x03, 0x34, 0xc3, 0xe3,
            0x01, 0x34, 0x83, 0xe3, 0x40, 0x39, 0x87, 0xe5, 0x40, 0x39, 0x97, 0xe5,
            0x03, 0x3c, 0xc3, 0xe3, 0x40, 0x39, 0x87, 0xe5, 0x40, 0x39, 0x97, 0xe5,
            0x0f, 0x30, 0xc3, 0xe3, 0x05, 0x30, 0x83, 0xe3, 0x40, 0x39, 0x87, 0xe5,
            0x24, 0x10, 0x85, 0xe5, 0x04, 0x30, 0x95, 0xe5, 0x02, 0x31, 0x83, 0xe3,
            0x83, 0x30, 0x83, 0xe3, 0x04, 0x30, 0x85, 0xe5, 0x04, 0x30, 0x95, 0xe5,
            0x00, 0x00, 0x53, 0xe3, 0xfc, 0xff, 0xff, 0xba, 0x08, 0x30, 0x95, 0xe5,
            0x04, 0x60, 0xa0, 0xe1, 0x03, 0x30, 0xc3, 0xe3, 0x44, 0x30, 0x83, 0xe3,
            0x08, 0x30, 0x85, 0xe5, 0x18, 0x30, 0x95, 0xe5, 0x02, 0x31, 0x83, 0xe3,
            0x02, 0x39, 0x83, 0xe3, 0x18, 0x30, 0x85, 0xe5, 0xa4, 0xff, 0xff, 0xea,
            0x08, 0x30, 0x95, 0xe5, 0x04, 0x60, 0xa0, 0xe1, 0xb0, 0x30, 0xc3, 0xe3,
            0x08, 0x30, 0x85, 0xe5, 0x9f, 0xff, 0xff, 0xea, 0x08, 0x30, 0x95, 0xe5,
            0x04, 0x60, 0xa0, 0xe1, 0xb0, 0x30, 0xc3, 0xe3, 0x80, 0x30, 0x83, 0xe3,
            0x08, 0x30, 0x85, 0xe5, 0x99, 0xff, 0xff, 0xea, 0x01, 0x90, 0xd6, 0xe5,
            0x02, 0x00, 0x86, 0xe2, 0x09, 0x10, 0xa0, 0xe1, 0x01, 0x60, 0x89, 0xe2,
            0x06, 0x60, 0x84, 0xe0, 0x59, 0xff, 0xff, 0xeb, 0x92, 0xff, 0xff, 0xea,
            0x06, 0x10, 0xd6, 0xe5, 0x09, 0x60, 0x86, 0xe2, 0x07, 0x00, 0x56, 0xe5,
            0x04, 0x20, 0x56, 0xe5, 0x08, 0x30, 0x56, 0xe5, 0x02, 0xe0, 0x56, 0xe5,
            0x06, 0xc0, 0x56, 0xe5, 0x01, 0x24, 0x82, 0xe1, 0x00, 0x34, 0x83, 0xe1,
            0x01, 0x10, 0x56, 0xe5, 0x05, 0x00, 0x56, 0xe5, 0x0e, 0x28, 0x82, 0xe1,
            0x0c, 0x38, 0x83, 0xe1, 0x01, 0x1c, 0x82, 0xe1, 0x00, 0x0c, 0x83, 0xe1,
            0x48, 0xff, 0xff, 0xeb, 0x81, 0xff, 0xff, 0xea, 0x06, 0x10, 0xd6, 0xe5,
            0x09, 0x60, 0x86, 0xe2, 0x07, 0x00, 0x56, 0xe5, 0x04, 0x20, 0x56, 0xe5,
            0x08, 0x30, 0x56, 0xe5, 0x02, 0xe0, 0x56, 0xe5, 0x06, 0xc0, 0x56, 0xe5,
            0x01, 0x24, 0x82, 0xe1, 0x00, 0x34, 0x83, 0xe1, 0x01, 0x10, 0x56, 0xe5,
            0x05, 0x00, 0x56, 0xe5, 0x0e, 0x28, 0x82, 0xe1, 0x0c, 0x38, 0x83, 0xe1,
            0x01, 0x1c, 0x82, 0xe1, 0x00, 0x0c, 0x83, 0xe1, 0xfd, 0xfe, 0xff, 0xeb,
            0x70, 0xff, 0xff, 0xea, 0x0d, 0x90, 0xa0, 0xe1, 0x08, 0x60, 0x8d, 0xe2,
            0x05, 0x30, 0xa0, 0xe3, 0x00, 0x30, 0xcd, 0xe5, 0x01, 0x10, 0xa0, 0xe3,
            0x09, 0x00, 0xa0, 0xe1, 0x2f, 0xff, 0xff, 0xeb, 0x01, 0x10, 0xa0, 0xe3,
            0x06, 0x00, 0xa0, 0xe1, 0xf2, 0xfe, 0xff, 0xeb, 0x08, 0x30, 0xdd, 0xe5,
            0x01, 0x00, 0x13, 0xe3, 0xf6, 0xff, 0xff, 0x1a, 0x04, 0x60, 0xa0, 0xe1,
            0x80, 0xff, 0xff, 0xea, 0x14, 0xd0, 0x8d, 0xe2, 0xf0, 0x83, 0xbd, 0xe8,
            0x04, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
            0x47, 0x4e, 0x55, 0x00, 0x56, 0xf9, 0xd9, 0x45, 0x24, 0x2e, 0xf0, 0x7f,
            0xfd, 0xb5, 0xd0, 0xfc, 0xc7, 0x17, 0xf6, 0xf7, 0xcf, 0x29, 0x57, 0x51,
            0x2f, 0x6c, 0x69, 0x62, 0x2f, 0x6c, 0x64, 0x2d, 0x6c, 0x69, 0x6e, 0x75,
            0x78, 0x2d, 0x61, 0x72, 0x6d, 0x68, 0x66, 0x2e, 0x73, 0x6f, 0x2e, 0x33,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xf5, 0xfe, 0xff, 0x6f, 0x28, 0x85, 0x02, 0x00, 0x05, 0x00, 0x00, 0x00,
            0x24, 0x85, 0x02, 0x00, 0x06, 0x00, 0x00, 0x00, 0x14, 0x85, 0x02, 0x00,
            0x0a, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
            0x10, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x1e, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0xfb, 0xff, 0xff, 0x6f,
            0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x40, 0x85, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
    };

    fel_->fel_write(0x00028000, &payload[0], sizeof(payload));
    if (swap_buf)
        *swap_buf = 0x0002a000;
    if (swap_len)
        *swap_len = 65536;
    if (cmd_len)
        *cmd_len = 4096;
    return chip_function_e::Success;
}

chip_function_e r528::chip_spi_run(uint8_t *cbuf, uint32_t clen) {
    fel_->fel_write(0x00029000, cbuf, clen);
    fel_->fel_exec(0x00028000);
    return chip_function_e::Success;
}
