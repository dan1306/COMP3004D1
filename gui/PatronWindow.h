#pragma once
#include <QMainWindow>
#include <memory>
#include "LibrarySystem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PatronWindow; }
QT_END_NAMESPACE

class CatalogueModel;

class PatronWindow : public QMainWindow {
    Q_OBJECT
public:
    PatronWindow(std::shared_ptr<hinlibs::LibrarySystem> system,
                 std::shared_ptr<hinlibs::Patron> patron,
                 QWidget* parent = nullptr);
    ~PatronWindow();

private slots:
    // Browse tab
    void onBorrow();
    void onPlaceHold();
    void onRefreshBrowse();

    // Account tab
    void onReturn();
    void onCancelHold();
    void onRefreshAccount();
    void onLogOut();

private:
    void populateAccountTables();

    std::unique_ptr<Ui::PatronWindow> ui;
    std::shared_ptr<hinlibs::LibrarySystem> system_;
    std::shared_ptr<hinlibs::Patron> patron_;
    CatalogueModel* catalogueModel_{nullptr};
};

