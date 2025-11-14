#include "LoginWindow.h"
#include "ui_LoginWindow.h"
#include "PatronWindow.h"

#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>

LoginWindow::LoginWindow(std::shared_ptr<hinlibs::LibrarySystem> system, QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::LoginWindow),
      system_(std::move(system)) {
    ui->setupUi(this);

    // Match the names in LoginWindow.ui (btnLogin, lineUsername)
    connect(ui->btnLogin, &QPushButton::clicked, this, &LoginWindow::onLogin);
}

LoginWindow::~LoginWindow() = default;

void LoginWindow::onLogin() {
    const auto name = ui->lineUsername->text().trimmed().toStdString();
    if (name.empty()) {
        QMessageBox::information(this, "Login", "Enter a username.");
        return;
    }

    // Adjust to your LibrarySystem API if the function name differs
    auto user = system_->findUserByName(name);
    if (!user) {
        QMessageBox::warning(this, "Login Failed", "User not found.");
        return;
    }
    if (user->role() != hinlibs::Role::Patron) {
        QMessageBox::information(this, "Not Implemented", "Only Patron features are implemented for D1.");
        return;
    }

    auto patron = std::static_pointer_cast<hinlibs::Patron>(user);
    auto* w = new PatronWindow(system_, patron, nullptr);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
    close(); // clear previous user context per D1
}
