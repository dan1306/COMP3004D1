#include "CatalogueModel.h"

CatalogueModel::CatalogueModel(std::shared_ptr<hinlibs::LibrarySystem> system, QObject* parent)
    : QAbstractTableModel(parent), system_(std::move(system)) {
    refresh();
}

int CatalogueModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(rows_.size());
}

QVariant CatalogueModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) return {};
    const auto& r = rows_.at(index.row());
    switch (index.column()) {
        case 0: return r.id;
        case 1: return r.title;
        case 2: return r.creator;
        case 3: return r.typeName;
        case 4: return r.availability;
    }
    return {};
}

QVariant CatalogueModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return {};
    switch (section) {
        case 0: return "ID";
        case 1: return "Title";
        case 2: return "Author/Creator";
        case 3: return "Type";
        case 4: return "Availability";
    }
    return {};
}

void CatalogueModel::refresh() {
    beginResetModel();
    rows_.clear();
    const auto& items = system_->allItems();
    rows_.reserve(items.size());
    for (const auto& it : items) {
        rows_.push_back({
            it->id(),
            QString::fromStdString(it->title()),
            QString::fromStdString(it->creator()),
            QString::fromStdString(it->typeName()),
            it->status() == hinlibs::ItemStatus::Available ? "Available" : "Checked Out"
        });
    }
    endResetModel();
}

int CatalogueModel::itemIdAtRow(int row) const {
    if (row < 0 || row >= static_cast<int>(rows_.size())) return -1;
    return rows_[row].id;
}
