#pragma once

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    Cell();
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    void ResetCache();

private:

    std::unique_ptr<Impl> impl_;
    SheetInterface& sheet_;
    mutable std::optional<Value> cache_;  

    set<Cell*> calculated_cells_;		//€чейки в которых вычисл€ем значение
    set<Cell*> using_cells_;		//€чейки используемые при вычислении 

    class Impl {
    public:

        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;

        virtual ~Impl() = default;
    };
    
    class EmptyImpl : public Impl {
    public:    
  
        Value GetValue() const override;
        std::string GetText() const override;      
    };
    
    class TextImpl : public Impl {
    public:
        
        explicit TextImpl(std::string text); 
        Value GetValue() const override;       
        std::string GetText() const override;
        
    private:
        std::string text_;        
    };
    
    class FormulaImpl : public Impl {
    public:
        
        explicit FormulaImpl(std::string text, const SheetInterface& sheet);
        Value GetValue() const override;
        std::string GetText() const override;

	std::vector<Position> GetReferencedCells() const override;
        
    private:
        std::unique_ptr<FormulaInterface> formula_ptr_; 
    };

    

    
  
};