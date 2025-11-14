#include <QApplication>
#include <memory>

#include "LibrarySystem.h"
#include "LoginWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    auto system = std::make_shared<hinlibs::LibrarySystem>();

    LoginWindow login(system);
    login.show();

    return app.exec();
}
