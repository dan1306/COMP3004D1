#include "LibrarianWindow.h"
#include "ui_LibrarianWindow.h"

#include "CatalogueModel.h"
#include "LoginWindow.h"
#include "AddItemDialog.h"

#include <QMessageBox>
#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QHeaderView>

LibrarianWindow::LibrarianWindow(std::shared_ptr<hinlibs::LibrarySystem> system,
                                 std::shared_ptr<hinlibs::User> librarian,
                                 QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::LibrarianWindow),
      system_(std::move(system)),
      librarian_(std::move(librarian)) {

    ui->setupUi(this);

    // ========== Catalogue Tab ==========
    catalogueModel_ = new CatalogueModel(system_, this);
    ui->tableCatalogue->setModel(catalogueModel_);
    ui->tableCatalogue->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableCatalogue->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableCatalogue->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableCatalogue->horizontalHeader()->setStretchLastSection(true);

    connect(ui->btnAddItem, &QPushButton::clicked, this, &LibrarianWindow::onAddItem);
    connect(ui->btnRemoveItem, &QPushButton::clicked, this, &LibrarianWindow::onRemoveItem);
    connect(ui->btnRefreshCatalogue, &QPushButton::clicked, this, &LibrarianWindow::onRefreshCatalogue);

    // ========== Return-On-Behalf Tab ==========
    connect(ui->btnSearchPatron, &QPushButton::clicked, this, &LibrarianWindow::onSearchPatron);
    connect(ui->btnReturnOnBehalf, &QPushButton::clicked, this, &LibrarianWindow::onReturnOnBehalf);

    // Logout button
    connect(ui->btnLogout, &QPushButton::clicked, this, &LibrarianWindow::onLogout);
}

LibrarianWindow::~LibrarianWindow() = default;


// CATALOGUE TAB


void LibrarianWindow::onRefreshCatalogue() {
    catalogueModel_->refresh();
    ui->tableCatalogue->resizeColumnsToContents();
}

void LibrarianWindow::onAddItem() {
    AddItemDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    auto data = dlg.itemData();
    if (!system_->addItemToCatalogue(librarian_->id(), data)) {
        QMessageBox::warning(this, "Add Item Failed",
                             "The item could not be added to the catalogue.");
        return;
    }

    onRefreshCatalogue();
    QMessageBox::information(this, "Success", "Item added to catalogue.");
}

void LibrarianWindow::onRemoveItem() {
    auto sel = ui->tableCatalogue->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        QMessageBox::information(this, "Remove Item", "Select an item first.");
        return;
    }

    const int row = sel.first().row();
    const int itemId = catalogueModel_->itemIdAtRow(row);

    if (!system_->removeItemFromCatalogue(librarian_->id(), itemId)) {
        QMessageBox::warning(this, "Remove Failed",
                             "Item could not be removed.\n"
                             "Make sure it is Available (not checked out).");
        return;
    }

    onRefreshCatalogue();
    QMessageBox::information(this, "Success", "Item removed successfully.");
}


// RETURN ON BEHALF TAB

void LibrarianWindow::onSearchPatron() {
    const auto name = ui->linePatronSearch->text().trimmed().toStdString();
    if (name.empty()) {
        QMessageBox::information(this, "Search Patron", "Enter a patron name.");
        return;
    }

    auto user = system_->LibrarianFindPatronByName(name);
    if (!user) {
        QMessageBox::warning(this, "Search Patron", "No patron found with that name.");
        selectedPatron_.reset();
        return;
    }

    selectedPatron_ = std::static_pointer_cast<hinlibs::Patron>(user);
    populateLoansTableForCurrentPatron();
}

void LibrarianWindow::populateLoansTableForCurrentPatron() {
    if (!selectedPatron_) return;

    auto loans = system_->getAccountLoans(selectedPatron_->id());

    auto* loansModel = new QStandardItemModel(this);
    loansModel->setHorizontalHeaderLabels({"Item ID", "Title", "Due Date", "Days Remaining"});

    for (const auto& l : loans) {
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::number(l.itemId));
        row << new QStandardItem(QString::fromStdString(l.title));
        row << new QStandardItem(l.dueDate.toString("yyyy-MM-dd"));
        row << new QStandardItem(QString::number(l.daysRemaining));
        loansModel->appendRow(row);
    }

    ui->tableLoans->setModel(loansModel);
    ui->tableLoans->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableLoans->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableLoans->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableLoans->horizontalHeader()->setStretchLastSection(true);
}

void LibrarianWindow::onReturnOnBehalf() {
    if (!selectedPatron_) {
        QMessageBox::information(this, "Return Item", "Search for a patron first.");
        return;
    }

    auto sel = ui->tableLoans->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        QMessageBox::information(this, "Return Item", "Select a loan to return.");
        return;
    }

    const int row = sel.first().row();
    const int itemId = ui->tableLoans->model()->index(row, 0).data().toInt();

    if (!system_->returnItem(selectedPatron_->id(), itemId)) {
        QMessageBox::warning(this, "Return Failed",
                             "Unable to return this item.\n"
                             "Make sure the patron actually has it.");
        return;
    }

    populateLoansTableForCurrentPatron();
    onRefreshCatalogue();
    QMessageBox::information(this, "Success", "Item returned successfully.");
}


// LOGOUT


void LibrarianWindow::onLogout() {
    close();
    auto* login = new LoginWindow(system_, nullptr);
    login->setAttribute(Qt::WA_DeleteOnClose);
    login->show();
}
