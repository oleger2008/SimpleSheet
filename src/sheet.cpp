#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <iostream>
#include <cassert>
using namespace std::literals;

// ----------- Sheet -------------------

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException(""s);
    }
    AdjustSize(pos);
    if (!text.empty()) {
        AdjustPrintableSize(pos);
    }

    Cell* cell;
    if (cells_.at(pos.row).at(pos.col)) {
        cell = dynamic_cast<Cell*>(cells_.at(pos.row).at(pos.col).get());
        if (cell->GetText() == text) {
            return;
        }
    } else {
        cells_.at(pos.row).at(pos.col) = std::make_unique<Cell>(dynamic_cast<SheetInterface*>(this));
        cell = dynamic_cast<Cell*>(cells_.at(pos.row).at(pos.col).get());
    }
    cell->Set(std::move(text));
    RelaxPrintableSize();
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException(""s);
    }
    if (size_.rows > pos.row && size_.cols > pos.col) {
        return cells_.at(pos.row).at(pos.col).get();
    }
    return nullptr;
}
CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException(""s);
    }
    if ((size_.rows >= pos.row + 1) && (size_.cols >= pos.col + 1)) {
        return cells_.at(pos.row).at(pos.col).get();
    }
    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException(""s);
    }
    if (pos.row >= size_.rows || pos.col >= size_.cols) {
        return;
    }

    // очистить ячейку
    auto& cell = cells_.at(pos.row).at(pos.col);
    if (!cell || cell->GetText().empty()) {
        return;
    }
    cell.reset();

    // проверить размер на предмет уменьшения печатной области
    RelaxPrintableSize();
}

Size Sheet::GetPrintableSize() const {
    return printable_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < printable_size_.rows; ++i) {
        for (int j = 0; j < printable_size_.cols; ++j) {
            const auto& cell = cells_.at(i).at(j);
            if (cell) {
                std::visit(ValueGetter{output}, cell->GetValue());
            }
            if (j != printable_size_.cols - 1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < printable_size_.rows; ++i) {
        for (int j = 0; j < printable_size_.cols; ++j) {
            const auto& cell = cells_.at(i).at(j);
            if (cell) {
                output << cell->GetText();
            }
            if (j != printable_size_.cols - 1) {
                output << '\t';
            }
        }
        output << '\n';
    }
}

void Sheet::AdjustSize(Position pos) {
    if (size_.rows < pos.row + 1) {
        cells_.resize(pos.row + 1);
        for (int i = size_.rows; i < pos.row + 1; ++i) {
            cells_.at(i).resize(size_.cols);
        }
        size_.rows = pos.row + 1;
    }
    if (size_.cols < pos.col + 1) {
        size_.cols = pos.col + 1;
        for (int i = 0; i < size_.rows; ++i) {
            cells_.at(i).resize(size_.cols);
        }
    }
}

void Sheet::AdjustPrintableSize(Position pos) {
    if (printable_size_.rows < pos.row + 1) {
        printable_size_.rows = pos.row + 1;
    }
    if (printable_size_.cols < pos.col + 1) {
        printable_size_.cols = pos.col + 1;
    }
}

void Sheet::RelaxPrintableSize() {
    for (int i = printable_size_.rows - 1; i >= 0; --i) {
        bool is_empty = true;
        for (int j = 0; j < printable_size_.cols; ++j) {
            const auto& cell = cells_.at(i).at(j);
            if (cell && !cell->GetText().empty()) {
                is_empty = false;
                break;
            }
        }
        if (is_empty) {
            --printable_size_.rows;
        } else {
            break;
        }
    }

    if (!printable_size_.rows) {
        printable_size_.cols = 0;
        return;
    }

    for (int j = printable_size_.cols - 1; j >= 0; --j) {
        bool is_empty = true;
        for (int i = 0; i < printable_size_.rows; ++i) {
            const auto& cell = cells_.at(i).at(j);
            if (cell && !cell->GetText().empty()) {
                is_empty = false;
                break;
            }
        }
        if (is_empty) {
            --printable_size_.cols;
        } else {
            break;
        }
    }
}

// ----------- other_funcs -------------------

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
