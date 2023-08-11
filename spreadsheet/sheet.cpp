#include "sheet.h" 

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {

    if (pos.IsValid()) {

        cells_.resize(std::max(pos.row + 1, int(std::size(cells_))));
        cells_[pos.row].resize(std::max(pos.col + 1, int(std::size(cells_[pos.row]))));

        if (!cells_[pos.row][pos.col]) {cells_[pos.row][pos.col] = std::make_unique<Cell>(*this);}
        cells_[pos.row][pos.col]->Set(std::move(text));

    } else {
        throw InvalidPositionException("invalid cell position. setsell");
    }
}

CellInterface* Sheet::GetCell(Position pos) {

    if (pos.IsValid()) {

        if (pos.row < int(std::size(cells_)) && pos.col < int(std::size(cells_[pos.row]))) {

            if (cells_[pos.row][pos.col].get()->GetText() == "") {
                return nullptr;

            } else {
                return cells_[pos.row][pos.col].get();
            }

        } else {
            return nullptr;
        }

    } else {
        throw InvalidPositionException("invalid cell position. getcell");
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (pos.IsValid()) {

        if (pos.row < int(std::size(cells_)) && pos.col < int(std::size(cells_[pos.row]))) {

            if (cells_[pos.row][pos.col].get()->GetText() == "") {
                return nullptr;

            } else {
                return cells_[pos.row][pos.col].get();
            }

        } else {
            return nullptr;
        }

    } else {
        throw InvalidPositionException("invalid cell position. getcell");
    }
}

Cell* Sheet::Get_Cell(Position pos) {

    if (pos.IsValid()) {

        if (pos.row < int(std::size(cells_)) && pos.col < int(std::size(cells_[pos.row]))) {
            return cells_[pos.row][pos.col].get();

        } else {
            return nullptr;
        }

    } else {
        throw InvalidPositionException("invalid cell position. get_cell");
    }

}

const Cell* Sheet::Get_Cell(Position pos) const {
    const Cell* const_result = Get_Cell(pos);
    return const_result;
}

void Sheet::ClearCell(Position pos) {

    if (pos.IsValid()) {

        if (pos.row < int(std::size(cells_)) && pos.col < int(std::size(cells_[pos.row]))) {

            if (cells_[pos.row][pos.col]) {
                cells_[pos.row][pos.col]->Clear();

                if (!cells_[pos.row][pos.col]->IsReferenced()) {
                    cells_[pos.row][pos.col].reset();
                }
            }
        }

    } else {
        throw InvalidPositionException("invalid cell position. clearcell");
    }
}

Size Sheet::GetPrintableSize() const {

    Size size;

    for (int row = 0; row < int(std::size(cells_)); ++row) {

        for (int col = (int(std::size(cells_[row])) - 1); col >= 0; --col) {

            if (cells_[row][col]) {

                if (cells_[row][col]->GetText().empty()) {
                    continue;

                } else {
                    size.rows = std::max(size.rows, row + 1);
                    size.cols = std::max(size.cols, col + 1);
                    break;
                }
            }
        }
    }

    return size;
}

void Sheet::PrintValues(std::ostream& output) const {

    for (int row = 0; row < GetPrintableSize().rows; ++row) {

        for (int col = 0; col < GetPrintableSize().cols; ++col) {

            if (col > 0) {output << '\t';}

            if (col < int(std::size(cells_[row]))) {

                if (cells_[row][col]) {std::visit([&output](const auto& value) {output << value;},
                                                  cells_[row][col]->GetValue());}
            }
        }

        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {

    for (int row = 0; row < GetPrintableSize().rows; ++row) {

        for (int col = 0; col < GetPrintableSize().cols; ++col) {

            if (col) {output << '\t';}

            if (col < int(std::size(cells_[row]))) {

                if (cells_[row][col]) {output << cells_[row][col]->GetText();}
            }
        }

        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {return std::make_unique<Sheet>();}