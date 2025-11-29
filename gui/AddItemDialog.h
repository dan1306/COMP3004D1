#pragma once

#include <QDialog>
#include <memory>

#include "models/itemInDB.h"  

QT_BEGIN_NAMESPACE
namespace Ui { class AddItemDialog; }
QT_END_NAMESPACE

class AddItemDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddItemDialog(QWidget* parent = nullptr);
    ~AddItemDialog();


    hinlibs::ItemInDB itemData() const { return itemData_; }

protected:
    void accept() override; 

private slots:
    void onTypeChanged(int index);

private:
    void updateFieldVisibility();

    std::unique_ptr<Ui::AddItemDialog> ui;
    hinlibs::ItemInDB itemData_;
};
