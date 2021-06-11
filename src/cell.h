#pragma once

#include <vector>
#include <optional>
#include <unordered_set>

#include "common.h"
#include "formula.h"
#include "sheet.h"



class Cell : public CellInterface {
public:
    Cell(SheetInterface* sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    bool IsReferenced() const;
    std::vector<Position> GetReferencedCells() const override;

    bool HasInfluences() const;

private:
    class Impl {
    public:
        explicit Impl(std::string text);
        virtual ~Impl() = default;

        virtual Value GetValue(const SheetInterface&) const = 0;
        virtual std::string GetText() const = 0;

        virtual bool IsReferenced() const = 0;

    protected:
        std::string data_;
    };
    
    class EmptyImpl : public Impl {
    public:
        explicit EmptyImpl();
        Value GetValue(const SheetInterface&) const override;
        std::string GetText() const override;

        bool IsReferenced() const override;
    };
    
    class TextImpl : public Impl {
    public:
        explicit TextImpl(std::string text);
        Value GetValue(const SheetInterface&) const override;
        std::string GetText() const override;

        bool IsReferenced() const override;
    };
    
    class FormulaImpl : public Impl {
    public:
        explicit FormulaImpl(std::string text);

        Value GetValue(const SheetInterface& sheet) const override;
        std::string GetText() const override;

        bool IsReferenced() const override;
        std::vector<Position> GetReferencedCells() const;

    private:
        std::unique_ptr<FormulaInterface> formula_;
    };
    
private:
    std::unique_ptr<Impl> impl_;
    mutable SheetInterface* sheet_;
    std::unordered_set<Cell*> influences_;
    mutable std::optional<Value> cashe_;

private:
    using Matrix = std::vector<std::vector<char>>;

    void CheckOnCircleDependency(const std::vector<Position>& new_dependences) const;
    void GraphRefresh(std::unique_ptr<Impl> temp);
    void CasheCleaner();
};
