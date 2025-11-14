#pragma once
#include <QMainWindow>
#include <memory>
#include "LibrarySystem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LoginWindow; }
QT_END_NAMESPACE

class LoginWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit LoginWindow(std::shared_ptr<hinlibs::LibrarySystem> system,
                         QWidget* parent = nullptr);
    ~LoginWindow();

private slots:
    void onLogin();

private:
    std::unique_ptr<Ui::LoginWindow>       ui;
    std::shared_ptr<hinlibs::LibrarySystem> system_;
};
