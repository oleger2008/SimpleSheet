#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

#include "formula.h"
#include "common.h"

#include "FormulaAST.h"



using namespace std::literals;

// ------------ FormulaError ----------------------------
FormulaError::FormulaError(Category category)
    : category_(category)
    {
        switch (category_) {
            case Category::Ref :
                description_ = "#REF!"s;
                break;
            case Category::Value :
                description_ = "#VALUE!"s;
                break;
            case Category::Div0 :
                description_ = "#DIV/0!"s;
                break;
        }
    }

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    return description_;
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}


namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression)
        : ast_(ParseFormulaAST(expression))
        {}

    Value Evaluate(const SheetInterface& sheet) const override {
        using namespace std::literals;
        try {
            return ast_.Execute([&sheet](const Position pos) {
                const CellInterface* cell_ptr;
                try {
                    cell_ptr = sheet.GetCell(pos);
                } catch (const InvalidPositionException&) {
                    throw FormulaError(FormulaError::Category::Ref);
                }
                if (!cell_ptr) {
                    return 0.;
                }

                const auto& value = cell_ptr->GetValue();
                if (std::holds_alternative<double>(value)) {
                    return std::get<double>(value);
                }
                if (std::holds_alternative<std::string>(value)) {
                    const auto& str = cell_ptr->GetText();
                    if (str.empty()) { // for EmptyImpl case
                        return 0.;
                    }
                    if (str[0] == ESCAPE_SIGN) {
                        throw FormulaError(FormulaError::Category::Value);
                    }
                    try {
                        return std::stod(str);
                    } catch (...) {
                        throw FormulaError(FormulaError::Category::Value);
                    }
                }
                if (std::holds_alternative<FormulaError>(value)) {
                    throw std::get<FormulaError>(value);
                }

                throw std::runtime_error("Reached the end of lambda while Evaluate formula in cell ["s +
                        pos.ToString() + "]"s);
            });
        } catch (const FormulaError& e) {
            return e;
        }
    }
    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }

    std::vector<Position> GetReferencedCells() const {
        const auto& cells = ast_.GetCells();
        return std::vector<Position>(cells.begin(), cells.end());
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
