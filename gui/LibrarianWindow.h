#pragma once

#include <QMainWindow>
#include <memory>
#include "models/LibrarySystem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LibrarianWindow; }
QT_END_NAMESPACE

class CatalogueModel;  

class LibrarianWindow : public QMainWindow {
    Q_OBJECT
public:
    LibrarianWindow(std::shared_ptr<hinlibs::LibrarySystem> system,
                    std::shared_ptr<hinlibs::User> librarian,
                    QWidget* parent = nullptr);
    ~LibrarianWindow();

private slots:
    // Catalogue tab
    void onAddItem();
    void onRemoveItem();
    void onRefreshCatalogue();

    // Return-on-behalf tab
    void onSearchPatron();
    void onReturnOnBehalf();
    void onLogout();

private:
    void populateLoansTableForCurrentPatron();

    std::unique_ptr<Ui::LibrarianWindow> ui;
    std::shared_ptr<hinlibs::LibrarySystem> system_;
    std::shared_ptr<hinlibs::User> librarian_;

    CatalogueModel* catalogueModel_{nullptr};
    std::shared_ptr<hinlibs::Patron> selectedPatron_;
};
