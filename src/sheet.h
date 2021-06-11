#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include "cell.h"
#include "common.h"



class Sheet : public SheetInterface {
public:
    using Table = std::vector<std::vector<std::unique_ptr<CellInterface>>>;

    Sheet() = default;
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;


private:
    Table cells_;
    Size size_;
    Size printable_size_;

private:
    struct ValueGetter {
        std::ostream& out;

        void operator()(const std::string& s) {
            out << s;
        }
        void operator()(double value) {
            out << value;
        }
        void operator()(const FormulaError& e) {
            out << e;
        }
    };

    void AdjustSize(Position pos);
    void AdjustPrintableSize(Position pos);
    void RelaxPrintableSize();
};
