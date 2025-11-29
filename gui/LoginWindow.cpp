#include "LoginWindow.h"
#include "ui_LoginWindow.h"
#include "PatronWindow.h"
#include "LibrarianWindow.h"

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

    auto user = system_->findUserByName(name);
    if (!user) {
        QMessageBox::warning(this, "Login Failed", "User not found.");
        return;
    }

    using hinlibs::Role;

    switch (user->role()) {
    case Role::Patron: {
        auto patron = std::static_pointer_cast<hinlibs::Patron>(user);
        auto* w = new PatronWindow(system_, patron, nullptr);
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
        close(); // clear previous user context
        break;
    }
    case Role::Librarian: {
        auto* w = new LibrarianWindow(system_, user, nullptr);
        w->setAttribute(Qt::WA_DeleteOnClose);
        w->show();
        close();
        break;
    }
    case Role::Administrator: {
        QMessageBox::information(this, "Admin",
                                 "Administrator UI is not implemented for this deliverable.");
        break;
    }
    }
}
