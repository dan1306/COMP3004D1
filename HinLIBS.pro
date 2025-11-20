QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    main.cpp \
    models/User.cpp \
    models/Patron.cpp \
    models/Item.cpp \
    models/Book.cpp \
    models/Movie.cpp \
    models/VideoGame.cpp \
    models/Magazine.cpp \
    models/LibrarySystem.cpp \
    models/hinlibs.cpp \
    gui/LoginWindow.cpp \
    gui/PatronWindow.cpp \
    gui/CatalogueModel.cpp

HEADERS += \
    models/User.h \
    models/Patron.h \
    models/Item.h \
    models/Book.h \
    models/Movie.h \
    models/VideoGame.h \
    models/Magazine.h \
    models/LibrarySystem.h \
    models/hinlibs.h \
    gui/LoginWindow.h \
    gui/PatronWindow.h \
    gui/CatalogueModel.h

FORMS += \
    gui/LoginWindow.ui \
    gui/PatronWindow.ui

QT += core gui widgets sql

INCLUDEPATH += \
    $$PWD \
    $$PWD/models \
    $$PWD/gui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
