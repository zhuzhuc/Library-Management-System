cmake_minimum_required(VERSION 3.16)
project(library_system_gui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt automoc/autorcc/auto features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets Test REQUIRED)

# existing core sources
set(CORE_SOURCES
    Book.cpp
    Library.cpp
    Borrower.cpp
    FileManager.cpp
    Student.cpp
    Teacher.cpp
)

set(CLI_SOURCES
    src/cli/LibraryCliController.cpp
    src/cli/BookRecommendationService.cpp
)

# Optional MySQL support - enabled by default
option(USE_MYSQL "Enable MySQL support" ON)
if(USE_MYSQL)
    # Try to find MySQL client library
    find_path(MYSQL_INCLUDE_DIR mysql.h 
        PATHS 
        /usr/local/include/mysql
        /usr/include/mysql
        /usr/local/mysql/include
        /opt/homebrew/include/mysql
        /opt/homebrew/opt/mysql/include
    )
    find_library(MYSQLCLIENT_LIB mysqlclient 
        PATHS
        /usr/local/lib
        /usr/lib
        /usr/local/mysql/lib
        /opt/homebrew/lib
        /opt/homebrew/opt/mysql/lib
    )
    if(MYSQL_INCLUDE_DIR AND MYSQLCLIENT_LIB)
        message(STATUS "MySQL found: ${MYSQL_INCLUDE_DIR}, ${MYSQLCLIENT_LIB}")
        add_definitions(-DUSE_MYSQL)
        # add DB manager implementation to core sources when MySQL enabled
        list(APPEND CORE_SOURCES src/db/DBManager.cpp)
        set(MYSQL_INCLUDE_DIR_FOUND TRUE)
    else()
        message(WARNING "MySQL client not found. Install mysql-client or set MYSQL_INCLUDE_DIR/MYSQLCLIENT_LIB. Continuing without MySQL support.")
        set(USE_MYSQL OFF)
        set(MYSQL_INCLUDE_DIR_FOUND FALSE)
    endif()
endif()

# GUI sources
set(GUI_SOURCES
    src/gui/main_qt.cpp
    src/gui/MainWindow.cpp
    src/gui/BookTableModel.cpp
    src/gui/LibraryController.cpp
    src/gui/AddBookDialog.cpp
    src/gui/AddUserDialog.cpp
    src/gui/EditBookDialog.cpp
    src/gui/LoginDialog.cpp
    src/gui/ResetPasswordDialog.cpp
    src/gui/BorrowDaysDialog.cpp
    src/gui/MyBorrowsDialog.cpp
    src/gui/BookDetailDialog.cpp
    src/gui/UsersListDialog.cpp
    src/gui/BorrowRecordsDialog.cpp
    src/gui/RecommendBooK.cpp
    src/gui/AppSettings.cpp
    src/gui/AppearanceDialog.cpp
    src/gui/UiTheme.cpp
)

add_executable(library_gui ${CORE_SOURCES} ${GUI_SOURCES})

target_include_directories(library_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(library_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

add_executable(library_cli main.cpp ${CORE_SOURCES} ${CLI_SOURCES})

target_include_directories(library_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(library_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

target_link_libraries(library_gui PRIVATE Qt6::Widgets)

# Link MySQL client library if enabled
if(USE_MYSQL AND MYSQLCLIENT_LIB AND MYSQL_INCLUDE_DIR_FOUND)
    target_include_directories(library_gui PRIVATE ${MYSQL_INCLUDE_DIR})
    target_link_libraries(library_gui PRIVATE ${MYSQLCLIENT_LIB})
    target_include_directories(library_cli PRIVATE ${MYSQL_INCLUDE_DIR})
    target_link_libraries(library_cli PRIVATE ${MYSQLCLIENT_LIB})
endif()

enable_testing()

add_executable(library_core_tests tests/LibraryCoreTests.cpp ${CORE_SOURCES})
target_include_directories(library_core_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME library_core_tests COMMAND library_core_tests)

add_executable(library_gui_tests tests/UiThemeTest.cpp src/gui/UiTheme.cpp)
target_include_directories(library_gui_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(library_gui_tests PRIVATE Qt6::Test Qt6::Widgets)
add_test(NAME library_gui_tests COMMAND library_gui_tests)

if(USE_MYSQL AND MYSQLCLIENT_LIB AND MYSQL_INCLUDE_DIR_FOUND)
    target_include_directories(library_core_tests PRIVATE ${MYSQL_INCLUDE_DIR})
    target_link_libraries(library_core_tests PRIVATE ${MYSQLCLIENT_LIB})
endif()
