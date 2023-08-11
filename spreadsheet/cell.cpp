#include "cell.h" 
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

Cell::Cell(Sheet& sheet) : impl_(std::make_unique<EmptyImpl>()),
                           sheet_(sheet) {}
Cell::~Cell() = default;

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> iterim;

    if (text.empty()) {
        iterim = std::make_unique<EmptyImpl>();
    } else if (text.size() >= 2 && text[0] == FORMULA_SIGN) {
        iterim = std::make_unique<FormulaImpl>(std::move(text), sheet_);
    } else {
        iterim= std::make_unique<TextImpl>(std::move(text));
    }

    if (CheckCircularDependencies(*iterim)) {
        throw CircularDependencyException("circular dependency detected");
    }

    // Обновление зависимостей
    for (Cell* used : using_cells_) {
        used->calculated_cells_.erase(this);
    }

    using_cells_.clear();

    for (const auto& pos : impl_->GetReferencedCells()) {
        Cell* used = sheet_.Get_Cell(pos);
        if (!used ) {
            sheet_.SetCell(pos, "");
            used  = sheet_.Get_Cell(pos);
        }
        using_cells_.insert(used );
        used ->calculated_cells_.insert(this);
    }

    impl_ = std::move(iterim);

    // Инвалидация кеша
    ResetCache(true);
}

bool Cell::CheckCircularDependencies(const Impl& new_impl) const {
    const auto& new_ref_cells = new_impl.GetReferencedCells();

    if (!new_ref_cells.empty()) {
        std::set<const Cell*> calc_;
        std::vector<const Cell*> insert_;
        std::set<const Cell*> using_;

        for (const auto& position : new_ref_cells) {
            const Cell* ref_cell = sheet_.Get_Cell(position);
            if (ref_cell) {
                using_.insert(ref_cell);
            } else {
                sheet_.SetCell(position, "");
                using_.insert(sheet_.Get_Cell(position));
            }
        }

        insert_.push_back(this);

        while (!insert_.empty()) {
            const Cell* current = insert_.back();
            insert_.pop_back();
            calc_.insert(current);

            for (const Cell* dependent : current->calculated_cells_) {
                if (calc_.find(dependent) == calc_.end()) {
                    insert_.push_back(dependent);
                } else {
                    return true; // Найдена циклическая зависимость
                }
            }
        }
    }

    return false; // Циклических зависимостей не найдено
}



void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {return impl_->GetValue();}
std::string Cell::GetText() const {return impl_->GetText();}
std::vector<Position> Cell::GetReferencedCells() const {return impl_->GetReferencedCells();}
bool Cell::IsReferenced() const {return !calculated_cells_.empty();}

void Cell::ResetCache(bool flag = false) {
    if (impl_->HasCache() || flag) {
        impl_->ResetCache();

        for (Cell* dependent : calculated_cells_) {
            dependent->ResetCache();
        }
    }
}

std::vector<Position> Cell::Impl::GetReferencedCells() const {return {};}
bool Cell::Impl::HasCache() {return true;}
void Cell::Impl::ResetCache() {}

Cell::Value Cell::EmptyImpl::GetValue() const {return "";}
std::string Cell::EmptyImpl::GetText() const {return "";}

Cell::TextImpl::TextImpl(std::string text) : text_(std::move(text)) {}

Cell::Value Cell::TextImpl::GetValue() const {

    if (text_.empty()) {
        throw FormulaException("it is empty impl, not text");

    } else if (text_.at(0) == ESCAPE_SIGN) {
        return text_.substr(1);

    } else {
        return text_;
    }
}

std::string Cell::TextImpl::GetText() const {return text_;}

Cell::FormulaImpl::FormulaImpl(std::string text, SheetInterface& sheet) : formula_ptr_(ParseFormula(text.substr(1)))
        , sheet_(sheet) {}

Cell::Value Cell::FormulaImpl::GetValue() const {

    if (!cache_) {cache_ = formula_ptr_->Evaluate(sheet_);}
    return std::visit([](auto& helper){return Value(helper);}, *cache_);
}

std::string Cell::FormulaImpl::GetText() const {return FORMULA_SIGN + formula_ptr_->GetExpression();}
std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {return formula_ptr_->GetReferencedCells();}
bool Cell::FormulaImpl::HasCache() {return cache_.has_value();}
void Cell::FormulaImpl::ResetCache() {cache_.reset();}