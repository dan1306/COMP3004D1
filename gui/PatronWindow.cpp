#include "PatronWindow.h"
#include "ui_PatronWindow.h"
#include "CatalogueModel.h"
#include "LoginWindow.h"


#include <QPushButton>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QTableView>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QItemSelectionModel>



PatronWindow::PatronWindow(std::shared_ptr<hinlibs::LibrarySystem> system,
                           std::shared_ptr<hinlibs::Patron> patron,
                           QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::PatronWindow),
      system_(std::move(system)),
      patron_(std::move(patron)) {
    ui->setupUi(this);

    // --- Browse tab ---
    catalogueModel_ = new CatalogueModel(system_, this);
    ui->browseTable->setModel(catalogueModel_);
    ui->browseTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->browseTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->browseTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->browseTable->horizontalHeader()->setStretchLastSection(true);

    connect(ui->btnBorrow, &QPushButton::clicked, this, &PatronWindow::onBorrow);
    connect(ui->btnPlaceHold, &QPushButton::clicked, this, &PatronWindow::onPlaceHold);
    connect(ui->btnRefreshBrowse, &QPushButton::clicked, this, &PatronWindow::onRefreshBrowse);

    // --- Account tab ---
    connect(ui->btnReturn, &QPushButton::clicked, this, &PatronWindow::onReturn);
    connect(ui->btnCancelHold, &QPushButton::clicked, this, &PatronWindow::onCancelHold);
    connect(ui->btnRefreshAccount, &QPushButton::clicked, this, &PatronWindow::onRefreshAccount);
    connect(ui->logoutBtn, &QPushButton::clicked, this, &PatronWindow::onLogOut);



    populateAccountTables();
}

PatronWindow::~PatronWindow() = default;

// --- Browse actions ---

void PatronWindow::onBorrow() {
    auto sel = ui->browseTable->selectionModel()->selectedRows();
    if (sel.isEmpty()) { QMessageBox::information(this, "Borrow", "Select an item first."); return; }
    const int row = sel.first().row();
    const int itemId = catalogueModel_->itemIdAtRow(row);
    if (itemId < 0) return;

    if (!system_->borrowItem(patron_->id(), itemId)) {
        QMessageBox::warning(this, "Borrow Failed",
                             "Cannot borrow this item (unavailable, queue fairness, or loan limit).");
        return;
    }
//    system_->logUserActivity(patron_->id(), "Borrowed Item with Id " + std::to_string(itemId));
    onRefreshBrowse();
    populateAccountTables();
}

void PatronWindow::onPlaceHold() {
    auto sel = ui->browseTable->selectionModel()->selectedRows();
    if (sel.isEmpty()) { QMessageBox::information(this, "Place Hold", "Select an item first."); return; }
    const int row = sel.first().row();
    const int itemId = catalogueModel_->itemIdAtRow(row);
    if (itemId < 0) return;

    if(system_->isLoanedBy(patron_->id(), itemId)){
        QMessageBox::warning(this, "Place Hold Failed",
                             "You can not place a hold on an item you have already checked-out.");
        return;
    }

    if (!system_->placeHold(patron_->id(), itemId)) {
        QMessageBox::warning(this, "Place Hold Failed",
                             "Holds are only allowed on checked-out items, and duplicates are not allowed.");
        return;
    }
//    system_->logUserActivity(patron_->id(), "Placed hold on Item with Id " + std::to_string(itemId));
    populateAccountTables();
}

void PatronWindow::onRefreshBrowse() {
    catalogueModel_->refresh();
    ui->browseTable->resizeColumnsToContents();
}

// --- Account actions ---

void PatronWindow::onReturn() {
    auto sel = ui->loansTable->selectionModel()->selectedRows();
    if (sel.isEmpty()) { QMessageBox::information(this, "Return", "Select a loan first."); return; }
    const int row = sel.first().row();
    const int itemId = ui->loansTable->model()->index(row, 0).data().toInt();

    if (!system_->returnItem(patron_->id(), itemId)) {
        QMessageBox::warning(this, "Return Failed", "This item is not loaned by you.");
        return;
    }
//    system_->logUserActivity(patron_->id(), "Returned Item with Id " + std::to_string(itemId));
    populateAccountTables();
    onRefreshBrowse();
}

void PatronWindow::onCancelHold() {
    auto sel = ui->holdsTable->selectionModel()->selectedRows();
    if (sel.isEmpty()) { QMessageBox::information(this, "Cancel Hold", "Select a hold first."); return; }
    const int row = sel.first().row();
    const int itemId = ui->holdsTable->model()->index(row, 0).data().toInt();

    if (!system_->cancelHold(patron_->id(), itemId)) {
        QMessageBox::warning(this, "Cancel Hold Failed", "Could not cancel this hold.");
        return;
    }
//    system_->logUserActivity(patron_->id(), "Cancelled hold on Item with Id " + std::to_string(itemId));
    populateAccountTables();
}

// --- Account population ---

void PatronWindow::populateAccountTables() {
    // Loans
    auto loans = system_->getAccountLoans(patron_->id());
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
    ui->loansTable->setModel(loansModel);
    ui->loansTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->loansTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->loansTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->loansTable->horizontalHeader()->setStretchLastSection(true);

    // Holds
    auto holds = system_->getAccountHolds(patron_->id());
    auto* holdsModel = new QStandardItemModel(this);
    holdsModel->setHorizontalHeaderLabels({"Item ID", "Title", "Queue Position"});
    for (const auto& h : holds) {
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::number(h.itemId));
        row << new QStandardItem(QString::fromStdString(h.title));
        row << new QStandardItem(QString::number(h.queuePosition));
        holdsModel->appendRow(row);
    }
    ui->holdsTable->setModel(holdsModel);
    ui->holdsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->holdsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->holdsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->holdsTable->horizontalHeader()->setStretchLastSection(true);

    // logs
//    auto logs = system_->getPatronUserActivities(patron_->id());
//    auto* logsModel = new QStandardItemModel(this);
//    logsModel->setHorizontalHeaderLabels({"User Account Activities"});
//    for (const auto& l : logs) {
//        QList<QStandardItem*> row;
//        row << new QStandardItem(QString::fromStdString(l));
//        logsModel->appendRow(row);
//    }
//    ui->logsTable->setModel(logsModel);
//    ui->logsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
//    ui->logsTable->setSelectionMode(QAbstractItemView::SingleSelection);
//    ui->logsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    ui->logsTable->horizontalHeader()->setStretchLastSection(true);
}

void PatronWindow::onRefreshAccount() {
    populateAccountTables();
}

void PatronWindow::onLogOut() {
    close();

    auto* newLogin = new LoginWindow(system_, nullptr);
    newLogin->setAttribute(Qt::WA_DeleteOnClose);
    newLogin->show();
}
