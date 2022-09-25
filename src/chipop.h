/*
 * Copyright (c) 2022, YuzukiTsuru <GloomyGhost@GloomyGhost.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See README and LICENSE for more details.
 */

#ifndef ChipOP_H
#define ChipOP_H

#include <QString>
#include <QObject>
#include <QThread>
#include <QList>
#include <QMultiMap>
#include <QVector>
#include "chips/chip_type.h"

#include "fel.h"

class ChipOP {
private:
    enum chip_fel_e {
        fel_chip_none,
        fel_chip_ok,
    };

public:
    ChipOP();

    ~ChipOP();

    /*
     * Scan Chip, will add fel_status -> fel_chip_ok
     */
    void scan_chip();

    void reset_chip();

    void enable_jtag();

public: // getter

    chip_t get_current_chip();

private:
    void generate_chip_db();

    bool check_chip();

private:
    fel *fel_ = new fel();
    Chips *current_chip = nullptr;

    chip_version_t chip_version{};
    QVector<Chips *> chip_db;
    chip_fel_e fel_status = fel_chip_none;
};

#endif // ChipOP_H