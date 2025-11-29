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
    gui/CatalogueModel.cpp \
    gui/LibrarianWindow.cpp \
    gui/AddItemDialog.cpp

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
    gui/CatalogueModel.h \
    gui/LibrarianWindow.h \
    gui/AddItemDialog.h \
    models/ItemInDB.h

FORMS += \
    gui/LoginWindow.ui \
    gui/PatronWindow.ui \
    gui/LibrarianWindow.ui \
    gui/AddItemDialog.ui

QT += core gui widgets sql

INCLUDEPATH += \
    $$PWD \
    $$PWD/models \
    $$PWD/gui


DB_FILES = db/hinlibs.sqlite3

for(file, DB_FILES) {
    COPIED_DB = $$OUT_PWD/$$file
    DBDIR = $$dirname(COPIED_DB)
    !exists($$DBDIR) {
        system(mkdir -p $$DBDIR)
    }
    system(cp $$PWD/$$file $$COPIED_DB)
}
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
