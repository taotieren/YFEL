#include <QDebug>

#include "chips/chip_db.h"
#include "chipdb.h"

ChipDB::ChipDB(chip_version_t chip_version) : chip_version(chip_version) {
    generate_chip_db();
    check_chip();
}

void ChipDB::generate_chip_db() {
    chip_db.push_back(new d1(chip_version));
    chip_db.push_back(new r528(chip_version));
}

void ChipDB::check_chip() {
    for (auto item: chip_db) {
        if (item->chip_detect() == chip_function_e::Success) {
            current_chip = item;
            // Read SID Here
            current_chip->chip_sid();
            qDebug() << "Current Chip" << current_chip->get_chip_info().chip_name;
            break;
        } else {
            throw std::runtime_error("Unsupported Chip\nfunction not implemented");
        }
    }
}

chip_t ChipDB::get_current_chip() {
    return current_chip->get_chip_info();
}