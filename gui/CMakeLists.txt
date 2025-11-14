cmake_minimum_required(VERSION 3.16)
project(HinLIBS LANGUAGES CXX)

# --- C++ Standard ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Find Qt 5 (the version on the course VM) ---
find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui)

# --- Enable Qt automatic processing (no need for qt5_wrap_ui or moc manually) ---
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# --- Source files ---
set(SRC
    main.cpp
    models/User.cpp
    models/Patron.cpp
    models/Item.cpp
    models/Book.cpp
    models/Movie.cpp
    models/VideoGame.cpp
    models/Magazine.cpp
    models/LibrarySystem.cpp
    gui/LoginWindow.cpp
    gui/PatronWindow.cpp
    gui/CatalogueModel.cpp
)

# --- Header files (for IDE organization) ---
set(HDR
    models/User.h
    models/Patron.h
    models/Item.h
    models/Book.h
    models/Movie.h
    models/VideoGame.h
    models/Magazine.h
    models/LibrarySystem.h
    gui/LoginWindow.h
    gui/PatronWindow.h
    gui/CatalogueModel.h
)

# --- UI files (Qt Designer forms) ---
set(UI
    gui/LoginWindow.ui
    gui/PatronWindow.ui
)

# --- Create the executable ---
add_executable(${PROJECT_NAME}
    ${SRC}
    ${HDR}
    ${UI}
)

# --- Include directories so we can use short includes like #include "LibrarySystem.h" ---
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/models
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
)

# --- Link with Qt libraries ---
target_link_libraries(${PROJECT_NAME}
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
)
