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

#include "f1c100s.h"

f1c100s::f1c100s(class fel *f, chip_version_t chip_version) : Chips(f, chip_version) {
    chip_info.chip_name = "F1C100A/F1C100s/F1C200s/F1C500s/F1C600/F1C700/F1C800/R6/RT1806";
    chip_info.chip_id = 0x00166300;
    chip_info.chip_type = chip_type_e::Normal;
    chip_info.chip_core = core_name_.ARM9;
    chip_info.chip_core_count = 1;
    chip_info.chip_core_count_str = core_count_.core_count_1;

    // dram presets
    dram_info.append(f1c100s_dram_param);
}

chip_function_e f1c100s::chip_detect() {
    if (chip_info.chip_version.id == chip_info.chip_id)
        return chip_function_e::Success;
    return chip_function_e::Fail;
}

chip_function_e f1c100s::chip_reset() {
    uint32_t val = fel_->fel_read32(0x01c20ca0 + 0x18);
    val &= ~(0xf << 4);
    val |= (1 << 4) | (0x1 << 0);
    fel_->fel_write32(0x01c20ca0 + 0x18, val);
    fel_->fel_write32(0x01c20ca0 + 0x10, (0xa57 << 1) | (1 << 0));
    return chip_function_e::Success;
}

chip_function_e f1c100s::chip_sid() {
    // f1c100s have no sid
    chip_info.chip_sid = "1663 Series not support SID";
    return chip_function_e::Success;
}

chip_function_e f1c100s::chip_jtag() {
    const uint8_t payload[] = {
            0xff, 0xff, 0xff, 0xea, 0x40, 0x00, 0xa0, 0xe3, 0x00, 0xd0, 0x80, 0xe5,
            0x04, 0xe0, 0x80, 0xe5, 0x00, 0xe0, 0x0f, 0xe1, 0x08, 0xe0, 0x80, 0xe5,
            0x10, 0xef, 0x11, 0xee, 0x0c, 0xe0, 0x80, 0xe5, 0x10, 0xef, 0x11, 0xee,
            0x10, 0xe0, 0x80, 0xe5, 0x1a, 0x00, 0x00, 0xeb, 0x04, 0x00, 0xa0, 0xe3,
            0x65, 0x10, 0xa0, 0xe3, 0x00, 0x10, 0xc0, 0xe5, 0x47, 0x10, 0xa0, 0xe3,
            0x01, 0x10, 0xc0, 0xe5, 0x4f, 0x10, 0xa0, 0xe3, 0x02, 0x10, 0xc0, 0xe5,
            0x4e, 0x10, 0xa0, 0xe3, 0x03, 0x10, 0xc0, 0xe5, 0x2e, 0x10, 0xa0, 0xe3,
            0x04, 0x10, 0xc0, 0xe5, 0x46, 0x10, 0xa0, 0xe3, 0x05, 0x10, 0xc0, 0xe5,
            0x45, 0x10, 0xa0, 0xe3, 0x06, 0x10, 0xc0, 0xe5, 0x4c, 0x10, 0xa0, 0xe3,
            0x07, 0x10, 0xc0, 0xe5, 0x40, 0x00, 0xa0, 0xe3, 0x00, 0xd0, 0x90, 0xe5,
            0x04, 0xe0, 0x90, 0xe5, 0x10, 0x10, 0x90, 0xe5, 0x10, 0x1f, 0x01, 0xee,
            0x0c, 0x10, 0x90, 0xe5, 0x10, 0x1f, 0x01, 0xee, 0x08, 0x10, 0x90, 0xe5,
            0x01, 0xf0, 0x29, 0xe1, 0x1e, 0xff, 0x2f, 0xe1, 0x40, 0x30, 0x9f, 0xe5,
            0xb4, 0x28, 0x93, 0xe5, 0x0f, 0x20, 0xc2, 0xe3, 0x03, 0x20, 0x82, 0xe3,
            0xb4, 0x28, 0x83, 0xe5, 0xb4, 0x28, 0x93, 0xe5, 0xf0, 0x20, 0xc2, 0xe3,
            0x30, 0x20, 0x82, 0xe3, 0xb4, 0x28, 0x83, 0xe5, 0xb4, 0x28, 0x93, 0xe5,
            0x0f, 0x2a, 0xc2, 0xe3, 0x03, 0x2a, 0x82, 0xe3, 0xb4, 0x28, 0x83, 0xe5,
            0xb4, 0x28, 0x93, 0xe5, 0x0f, 0x26, 0xc2, 0xe3, 0x03, 0x26, 0x82, 0xe3,
            0xb4, 0x28, 0x83, 0xe5, 0x1e, 0xff, 0x2f, 0xe1, 0x00, 0x00, 0xc2, 0x01
    };
    fel_->fel_write(0x00008800, &payload[0], sizeof(payload));
    fel_->fel_exec(0x00008800);
    return chip_function_e::Success;
}

chip_function_e f1c100s::chip_ddr(chip_ddr_type_e dram_type) {
    fel_->fel_write(0x00008800, &dram_payload, sizeof(dram_payload));
    fel_->fel_exec(0x00008800);
    return chip_function_e::Success;
}

chip_function_e f1c100s::chip_ddr(dram_param_t param) {
    fel_->fel_write(0x00008800, &dram_payload, sizeof(dram_payload));
    fel_->fel_exec(0x00008800);
    return chip_function_e::Success;
}

chip_function_e f1c100s::chip_spi_init(uint32_t *swap_buf, uint32_t *swap_len, uint32_t *cmd_len) {
    const uint8_t payload[] = {
            0xff, 0xff, 0xff, 0xea, 0x40, 0x00, 0xa0, 0xe3, 0x00, 0xd0, 0x80, 0xe5,
            0x04, 0xe0, 0x80, 0xe5, 0x00, 0xe0, 0x0f, 0xe1, 0x08, 0xe0, 0x80, 0xe5,
            0x10, 0xef, 0x11, 0xee, 0x0c, 0xe0, 0x80, 0xe5, 0x10, 0xef, 0x11, 0xee,
            0x10, 0xe0, 0x80, 0xe5, 0x26, 0x0b, 0xa0, 0xe3, 0x85, 0x00, 0x00, 0xeb,
            0x04, 0x00, 0xa0, 0xe3, 0x65, 0x10, 0xa0, 0xe3, 0x00, 0x10, 0xc0, 0xe5,
            0x47, 0x10, 0xa0, 0xe3, 0x01, 0x10, 0xc0, 0xe5, 0x4f, 0x10, 0xa0, 0xe3,
            0x02, 0x10, 0xc0, 0xe5, 0x4e, 0x10, 0xa0, 0xe3, 0x03, 0x10, 0xc0, 0xe5,
            0x2e, 0x10, 0xa0, 0xe3, 0x04, 0x10, 0xc0, 0xe5, 0x46, 0x10, 0xa0, 0xe3,
            0x05, 0x10, 0xc0, 0xe5, 0x45, 0x10, 0xa0, 0xe3, 0x06, 0x10, 0xc0, 0xe5,
            0x4c, 0x10, 0xa0, 0xe3, 0x07, 0x10, 0xc0, 0xe5, 0x40, 0x00, 0xa0, 0xe3,
            0x00, 0xd0, 0x90, 0xe5, 0x04, 0xe0, 0x90, 0xe5, 0x10, 0x10, 0x90, 0xe5,
            0x10, 0x1f, 0x01, 0xee, 0x0c, 0x10, 0x90, 0xe5, 0x10, 0x1f, 0x01, 0xee,
            0x08, 0x10, 0x90, 0xe5, 0x01, 0xf0, 0x29, 0xe1, 0x1e, 0xff, 0x2f, 0xe1,
            0xf0, 0x40, 0x2d, 0xe9, 0x00, 0x50, 0x51, 0xe2, 0xf0, 0x80, 0xbd, 0x08,
            0xc8, 0xe0, 0x9f, 0xe5, 0xc8, 0x60, 0x9f, 0xe5, 0xc8, 0x40, 0x9f, 0xe5,
            0x00, 0x70, 0xe0, 0xe3, 0x40, 0x00, 0x55, 0xe3, 0x05, 0x20, 0xa0, 0x31,
            0x40, 0x20, 0xa0, 0x23, 0x00, 0x30, 0xa0, 0xe3, 0x07, 0x10, 0xa0, 0xe1,
            0x30, 0x20, 0x8e, 0xe5, 0x34, 0x20, 0x8e, 0xe5, 0x38, 0x20, 0x8e, 0xe5,
            0x01, 0x30, 0x83, 0xe2, 0x03, 0x00, 0x52, 0xe1, 0x00, 0x10, 0xc6, 0xe5,
            0xfb, 0xff, 0xff, 0xca, 0x08, 0x30, 0x9e, 0xe5, 0x02, 0x31, 0x83, 0xe3,
            0x08, 0x30, 0x8e, 0xe5, 0x1c, 0x30, 0x9e, 0xe5, 0xff, 0x30, 0x03, 0xe2,
            0x03, 0x00, 0x52, 0xe1, 0xfb, 0xff, 0xff, 0x8a, 0x00, 0x30, 0xa0, 0xe3,
            0x00, 0xc0, 0xd4, 0xe5, 0x00, 0x00, 0x50, 0xe3, 0x00, 0x10, 0xa0, 0xe1,
            0xff, 0xc0, 0x0c, 0xe2, 0x07, 0x00, 0x00, 0x0a, 0x01, 0x30, 0x83, 0xe2,
            0x03, 0x00, 0x52, 0xe1, 0x01, 0xc0, 0xc1, 0xe4, 0x01, 0x00, 0xa0, 0xe1,
            0xf5, 0xff, 0xff, 0xca, 0x02, 0x50, 0x55, 0xe0, 0xdf, 0xff, 0xff, 0x1a,
            0xf0, 0x80, 0xbd, 0xe8, 0x01, 0x10, 0x83, 0xe2, 0x02, 0x00, 0x51, 0xe1,
            0x02, 0x30, 0x83, 0xe2, 0xf8, 0xff, 0xff, 0xaa, 0x03, 0x00, 0x52, 0xe1,
            0x00, 0x10, 0xd4, 0xe5, 0xf5, 0xff, 0xff, 0xda, 0x00, 0x10, 0xd4, 0xe5,
            0x01, 0x10, 0x83, 0xe2, 0x02, 0x00, 0x51, 0xe1, 0x02, 0x30, 0x83, 0xe2,
            0xf7, 0xff, 0xff, 0xba, 0x02, 0x50, 0x55, 0xe0, 0xd0, 0xff, 0xff, 0x1a,
            0xf0, 0x80, 0xbd, 0xe8, 0x00, 0x50, 0xc0, 0x01, 0x00, 0x52, 0xc0, 0x01,
            0x00, 0x53, 0xc0, 0x01, 0xf0, 0x40, 0x2d, 0xe9, 0x00, 0x60, 0x51, 0xe2,
            0xf0, 0x80, 0xbd, 0x08, 0xa4, 0x10, 0x9f, 0xe5, 0xa4, 0x50, 0x9f, 0xe5,
            0xa4, 0x40, 0x9f, 0xe5, 0x00, 0x70, 0xe0, 0xe3, 0x40, 0x00, 0x56, 0xe3,
            0x06, 0x20, 0xa0, 0x31, 0x40, 0x20, 0xa0, 0x23, 0x00, 0x00, 0x50, 0xe3,
            0x30, 0x20, 0x81, 0xe5, 0x34, 0x20, 0x81, 0xe5, 0x38, 0x20, 0x81, 0xe5,
            0x17, 0x00, 0x00, 0x0a, 0x00, 0xc0, 0xa0, 0xe1, 0x00, 0x30, 0xa0, 0xe3,
            0x01, 0xe0, 0xdc, 0xe4, 0x01, 0x30, 0x83, 0xe2, 0x03, 0x00, 0x52, 0xe1,
            0x00, 0xe0, 0xc5, 0xe5, 0xfa, 0xff, 0xff, 0xca, 0x08, 0x30, 0x91, 0xe5,
            0x02, 0x31, 0x83, 0xe3, 0x08, 0x30, 0x81, 0xe5, 0x1c, 0x30, 0x91, 0xe5,
            0xff, 0x30, 0x03, 0xe2, 0x03, 0x00, 0x52, 0xe1, 0xfb, 0xff, 0xff, 0x8a,
            0x00, 0x30, 0xa0, 0xe3, 0x01, 0x30, 0x83, 0xe2, 0x03, 0x00, 0x52, 0xe1,
            0x00, 0xc0, 0xd4, 0xe5, 0xfb, 0xff, 0xff, 0xca, 0x00, 0x00, 0x50, 0xe3,
            0x02, 0x00, 0x80, 0x10, 0x02, 0x60, 0x56, 0xe0, 0xe0, 0xff, 0xff, 0x1a,
            0xf0, 0x80, 0xbd, 0xe8, 0x00, 0x30, 0xa0, 0xe1, 0x07, 0xc0, 0xa0, 0xe1,
            0x01, 0x30, 0x83, 0xe2, 0x03, 0x00, 0x52, 0xe1, 0x00, 0xc0, 0xc5, 0xe5,
            0xfb, 0xff, 0xff, 0xca, 0xe7, 0xff, 0xff, 0xea, 0x00, 0x50, 0xc0, 0x01,
            0x00, 0x52, 0xc0, 0x01, 0x00, 0x53, 0xc0, 0x01, 0xf0, 0x43, 0x2d, 0xe9,
            0x50, 0x82, 0x9f, 0xe5, 0x50, 0x52, 0x9f, 0xe5, 0x50, 0x72, 0x9f, 0xe5,
            0x14, 0xd0, 0x4d, 0xe2, 0x00, 0x60, 0xa0, 0xe1, 0x06, 0x40, 0xa0, 0xe1,
            0x01, 0x30, 0xd4, 0xe4, 0x01, 0x00, 0x53, 0xe3, 0x1e, 0x00, 0x00, 0x0a,
            0x02, 0x00, 0x53, 0xe3, 0x45, 0x00, 0x00, 0x0a, 0x03, 0x00, 0x53, 0xe3,
            0x48, 0x00, 0x00, 0x0a, 0x04, 0x00, 0x53, 0xe3, 0x4c, 0x00, 0x00, 0x0a,
            0x05, 0x00, 0x53, 0xe3, 0x51, 0x00, 0x00, 0x0a, 0x06, 0x00, 0x53, 0xe3,
            0x60, 0x00, 0x00, 0x0a, 0x07, 0x00, 0x53, 0xe3, 0x6f, 0x00, 0x00, 0x0a,
            0x08, 0x00, 0x53, 0xe3, 0x7c, 0x00, 0x00, 0x1a, 0x0d, 0x90, 0xa0, 0xe1,
            0x08, 0x60, 0x8d, 0xe2, 0xb0, 0x80, 0xcd, 0xe1, 0x02, 0x10, 0xa0, 0xe3,
            0x09, 0x00, 0xa0, 0xe1, 0xb0, 0xff, 0xff, 0xeb, 0x01, 0x10, 0xa0, 0xe3,
            0x06, 0x00, 0xa0, 0xe1, 0x73, 0xff, 0xff, 0xeb, 0x08, 0x30, 0xdd, 0xe5,
            0x01, 0x00, 0x13, 0xe3, 0xf6, 0xff, 0xff, 0x1a, 0x04, 0x60, 0xa0, 0xe1,
            0x06, 0x40, 0xa0, 0xe1, 0x01, 0x30, 0xd4, 0xe4, 0x01, 0x00, 0x53, 0xe3,
            0xe0, 0xff, 0xff, 0x1a, 0x48, 0x38, 0x97, 0xe5, 0xb8, 0x21, 0x9f, 0xe5,
            0x0f, 0x30, 0xc3, 0xe3, 0x02, 0x30, 0x83, 0xe3, 0x48, 0x38, 0x87, 0xe5,
            0x48, 0x38, 0x97, 0xe5, 0xf0, 0x30, 0xc3, 0xe3, 0x20, 0x30, 0x83, 0xe3,
            0x48, 0x38, 0x87, 0xe5, 0x48, 0x38, 0x97, 0xe5, 0x0f, 0x3c, 0xc3, 0xe3,
            0x02, 0x3c, 0x83, 0xe3, 0x48, 0x38, 0x87, 0xe5, 0x48, 0x38, 0x97, 0xe5,
            0x0f, 0x3a, 0xc3, 0xe3, 0x02, 0x3a, 0x83, 0xe3, 0x48, 0x38, 0x87, 0xe5,
            0xc0, 0x32, 0x97, 0xe5, 0x01, 0x36, 0x83, 0xe3, 0xc0, 0x32, 0x87, 0xe5,
            0x60, 0x30, 0x97, 0xe5, 0x01, 0x36, 0x83, 0xe3, 0x60, 0x30, 0x87, 0xe5,
            0x24, 0x20, 0x85, 0xe5, 0x04, 0x30, 0x95, 0xe5, 0x02, 0x31, 0x83, 0xe3,
            0x83, 0x30, 0x83, 0xe3, 0x04, 0x30, 0x85, 0xe5, 0x04, 0x30, 0x95, 0xe5,
            0x00, 0x00, 0x53, 0xe3, 0xfc, 0xff, 0xff, 0xba, 0x08, 0x30, 0x95, 0xe5,
            0x04, 0x60, 0xa0, 0xe1, 0x03, 0x30, 0xc3, 0xe3, 0x44, 0x30, 0x83, 0xe3,
            0x08, 0x30, 0x85, 0xe5, 0x18, 0x30, 0x95, 0xe5, 0x02, 0x31, 0x83, 0xe3,
            0x02, 0x39, 0x83, 0xe3, 0x18, 0x30, 0x85, 0xe5, 0xb3, 0xff, 0xff, 0xea,
            0x08, 0x30, 0x95, 0xe5, 0x04, 0x60, 0xa0, 0xe1, 0xb0, 0x30, 0xc3, 0xe3,
            0x08, 0x30, 0x85, 0xe5, 0xae, 0xff, 0xff, 0xea, 0x08, 0x30, 0x95, 0xe5,
            0x04, 0x60, 0xa0, 0xe1, 0xb0, 0x30, 0xc3, 0xe3, 0x80, 0x30, 0x83, 0xe3,
            0x08, 0x30, 0x85, 0xe5, 0xa8, 0xff, 0xff, 0xea, 0x01, 0x90, 0xd6, 0xe5,
            0x02, 0x00, 0x86, 0xe2, 0x09, 0x10, 0xa0, 0xe1, 0x01, 0x60, 0x89, 0xe2,
            0x6c, 0xff, 0xff, 0xeb, 0x06, 0x60, 0x84, 0xe0, 0xa1, 0xff, 0xff, 0xea,
            0x05, 0x20, 0xd6, 0xe5, 0x06, 0x90, 0xd6, 0xe5, 0x01, 0x30, 0xd6, 0xe5,
            0x02, 0x40, 0xd6, 0xe5, 0x07, 0xe0, 0xd6, 0xe5, 0x03, 0xc0, 0xd6, 0xe5,
            0x08, 0x10, 0xd6, 0xe5, 0x04, 0x00, 0xd6, 0xe5, 0x09, 0x24, 0x82, 0xe1,
            0x04, 0x34, 0x83, 0xe1, 0x0e, 0x28, 0x82, 0xe1, 0x0c, 0x38, 0x83, 0xe1,
            0x01, 0x1c, 0x82, 0xe1, 0x00, 0x0c, 0x83, 0xe1, 0x5b, 0xff, 0xff, 0xeb,
            0x09, 0x60, 0x86, 0xe2, 0x90, 0xff, 0xff, 0xea, 0x05, 0x20, 0xd6, 0xe5,
            0x06, 0x90, 0xd6, 0xe5, 0x01, 0x30, 0xd6, 0xe5, 0x02, 0x40, 0xd6, 0xe5,
            0x07, 0xe0, 0xd6, 0xe5, 0x03, 0xc0, 0xd6, 0xe5, 0x08, 0x10, 0xd6, 0xe5,
            0x04, 0x00, 0xd6, 0xe5, 0x09, 0x24, 0x82, 0xe1, 0x04, 0x34, 0x83, 0xe1,
            0x0e, 0x28, 0x82, 0xe1, 0x0c, 0x38, 0x83, 0xe1, 0x01, 0x1c, 0x82, 0xe1,
            0x00, 0x0c, 0x83, 0xe1, 0x10, 0xff, 0xff, 0xeb, 0x09, 0x60, 0x86, 0xe2,
            0x7f, 0xff, 0xff, 0xea, 0x05, 0x30, 0xa0, 0xe3, 0x0d, 0x90, 0xa0, 0xe1,
            0x08, 0x60, 0x8d, 0xe2, 0x00, 0x30, 0xcd, 0xe5, 0x01, 0x10, 0xa0, 0xe3,
            0x09, 0x00, 0xa0, 0xe1, 0x41, 0xff, 0xff, 0xeb, 0x01, 0x10, 0xa0, 0xe3,
            0x06, 0x00, 0xa0, 0xe1, 0x04, 0xff, 0xff, 0xeb, 0x08, 0x30, 0xdd, 0xe5,
            0x01, 0x00, 0x13, 0xe3, 0xf6, 0xff, 0xff, 0x1a, 0x04, 0x60, 0xa0, 0xe1,
            0x8f, 0xff, 0xff, 0xea, 0x14, 0xd0, 0x8d, 0xe2, 0xf0, 0x83, 0xbd, 0xe8,
            0x0f, 0xc0, 0xff, 0xff, 0x00, 0x50, 0xc0, 0x01, 0x00, 0x00, 0xc2, 0x01,
            0x01, 0x10, 0x00, 0x00
    };

    fel_->fel_write(0x00008800, &payload[0], sizeof(payload));
    if (swap_buf)
        *swap_buf = 0x0000a800;
    if (swap_len)
        *swap_len = 3584;
    if (cmd_len)
        *cmd_len = 4096;
    return chip_function_e::Success;
}

chip_function_e f1c100s::chip_spi_run(uint8_t *cbuf, uint32_t clen) {
    fel_->fel_write(0x00009800, cbuf, clen);
    fel_->fel_exec(0x00008800);
    return chip_function_e::Success;
}