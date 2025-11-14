#pragma once
#include <QAbstractTableModel>
#include <vector>
#include <memory>
#include "models/LibrarySystem.h"

class CatalogueModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit CatalogueModel(std::shared_ptr<hinlibs::LibrarySystem> system, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override { Q_UNUSED(parent); return 5; }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void refresh();

    int itemIdAtRow(int row) const;

private:
    struct Row {
        int id;
        QString title;
        QString creator;
        QString typeName;
        QString availability;
    };
    std::shared_ptr<hinlibs::LibrarySystem> system_;
    std::vector<Row> rows_;
};
