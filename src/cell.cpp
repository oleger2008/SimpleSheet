#include <cassert>
//#include <iostream>
#include <stack>
#include <string>
#include <optional>
#include <unordered_set>

#include "cell.h"



// ------------ Cell --------------
Cell::Cell(SheetInterface* sheet)
    : impl_(std::make_unique<EmptyImpl>())
    , sheet_(sheet)
    {}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> temp_impl;

    if (text.empty()) {
        // create EmptyImpl
        temp_impl = std::make_unique<EmptyImpl>();
    } else if ((text.size() == 1u && text[0] == FORMULA_SIGN)
                                  || text[0] != FORMULA_SIGN) {
        // create TextImpl
        temp_impl = std::make_unique<TextImpl>(std::move(text));
    } else if (text[0] == FORMULA_SIGN) {
        // create FormulaImpl
        try {
            temp_impl = std::make_unique<FormulaImpl>(std::move(text));
        } catch (...) {
            throw FormulaException("Syntax err");
        }
        CheckOnCircleDependency(dynamic_cast<FormulaImpl*>(temp_impl.get())->GetReferencedCells());
    }
    CasheCleaner();
    GraphRefresh(std::move(temp_impl));
}

void Cell::Clear() {
    impl_ = nullptr;
}

Cell::Value Cell::GetValue() const {
    if (!cashe_.has_value()) {
        const auto& value = impl_->GetValue(*sheet_);
        if (std::holds_alternative<std::string>(value)) {
            return value;
        }
        cashe_.emplace(value);
    }

    return *cashe_;
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

bool Cell::IsReferenced() const {
    return impl_->IsReferenced();
}
std::vector<Position> Cell::GetReferencedCells() const {
    if (!IsReferenced()) {
        return {};
    } else {
        return dynamic_cast<FormulaImpl*>(impl_.get())->GetReferencedCells();
    }

}

bool Cell::HasInfluences() const {
    return !influences_.empty();
}

void Cell::CheckOnCircleDependency(
        const std::vector<Position>& new_dependences) const {
    using namespace std::literals;

    const auto& size = sheet_->GetPrintableSize();
    Matrix visited(size.rows, std::vector<char>(size.cols, 0));

    std::stack<Position> stck;
    for (const auto pos : new_dependences) {
        stck.push(pos);
    }

    Position temp_pos;
    Cell* temp_cell;

    while (!stck.empty()) {
        temp_pos = stck.top();
        stck.pop();

        if (auto& is_visited = visited.at(temp_pos.row).at(temp_pos.col)) {
            continue;
        } else {
            is_visited = 1;
        }

        if (!sheet_->GetCell(temp_pos)) {
            continue;
        }

        temp_cell = dynamic_cast<Cell*>(sheet_->GetCell(temp_pos));
        if (temp_cell == this) {
            throw CircularDependencyException("Circular Dependency in cell ["s
                    + temp_pos.ToString() + "]"s);
        }

        if (temp_cell->IsReferenced()) {
            for (const auto pos : temp_cell->GetReferencedCells()) {
                if (!visited.at(temp_pos.row).at(temp_pos.col)) {
                    stck.push(pos);
                }
            }
        }
    }
}

void Cell::GraphRefresh(std::unique_ptr<Impl> temp) {
    using namespace std::literals;
    Cell* cell_ptr;
    for (const auto pos : GetReferencedCells()) {
        cell_ptr = dynamic_cast<Cell*>(sheet_->GetCell(pos));
        cell_ptr->influences_.erase(this);
    }
    impl_.reset();
    impl_ = std::move(temp);
    for (const auto pos : GetReferencedCells()) {
        if (!sheet_->GetCell(pos)) {
            sheet_->SetCell(pos, ""s);
        }
        cell_ptr = dynamic_cast<Cell*>(sheet_->GetCell(pos));
        cell_ptr->influences_.insert(this);
    }
}

void Cell::CasheCleaner() {
    cashe_.reset();
    std::unordered_set<Cell*> visited;
    std::stack<Cell*> stck;
    for (const auto cell_ptr : influences_) {
        stck.push(cell_ptr);
    }
    Cell* temp_cell;

    while (!stck.empty()) {
        temp_cell = stck.top();
        stck.pop();

        if (visited.count(temp_cell)) {
            continue;
        } else {
            visited.insert(temp_cell);
        }

        temp_cell->cashe_.reset();
        for (const auto cell_ptr : temp_cell->influences_) {
            stck.push(cell_ptr);
        }
    }
}

// ------------ Cell::Impl --------------

Cell::Impl::Impl(std::string text) 
    : data_(std::move(text))
    {}

// ------------ Cell::EmptyImpl --------------

Cell::EmptyImpl::EmptyImpl()
    : Cell::Impl("")
    {}
Cell::Value Cell::EmptyImpl::GetValue(const SheetInterface&) const {
    return data_;
}
std::string Cell::EmptyImpl::GetText() const {
    return data_;
}

bool Cell::EmptyImpl::IsReferenced() const {
    return false;
}

// ------------ Cell::TextImpl --------------

Cell::TextImpl::TextImpl(std::string text)
    : Cell::Impl(std::move(text))
    {}
Cell::Value Cell::TextImpl::GetValue(const SheetInterface&) const {
    if (data_[0] == ESCAPE_SIGN) {
        return data_.substr(1u);
    }
    return data_;
}
std::string Cell::TextImpl::GetText() const {
    return data_;
}

bool Cell::TextImpl::IsReferenced() const {
    return false;
}

// ------------ Cell::FormulaImpl --------------

Cell::FormulaImpl::FormulaImpl(std::string text)
    : Cell::Impl(std::move(text))
    , formula_(ParseFormula(data_.substr(1u)))
    {}
Cell::Value Cell::FormulaImpl::GetValue(const SheetInterface& sheet) const {
    using namespace std::literals;
    const auto& value = formula_->Evaluate(sheet);
    if (std::holds_alternative<double>(value)) {
        return std::get<double>(value);
    }
    if (std::holds_alternative<FormulaError>(value)) {
        return std::get<FormulaError>(value);
    }
    return ""s;
}
std::string Cell::FormulaImpl::GetText() const {
    using namespace std::literals;
    return "="s + formula_->GetExpression();
}

bool Cell::FormulaImpl::IsReferenced() const {
    return !formula_->GetReferencedCells().empty();
}
std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}
