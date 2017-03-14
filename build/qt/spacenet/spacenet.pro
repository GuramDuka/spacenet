TEMPLATE = app

QT += qml quick
CONFIG += c++14

SOURCES += \
    ../../../src/cdc512.cpp \
    ../../../src/indexer.cpp \
    ../../../src/main.cpp \
    ../../../tests/cdc512_test.cpp \
    ../../../tests/indexer_test.cpp \
    ../../../tests/locale_traits_test.cpp \
    ../../../src/sqlite3.c \
    ../../../tests/tests.cpp \
    ../../../src/locale_traits.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    ../../../include/cdc512.hpp \
    ../../../include/config.h \
    ../../../include/indexer.hpp \
    ../../../include/locale_traits.hpp \
    ../../../include/scope_exit.hpp \
    ../../../include/std_ext.hpp \
    ../../../include/version.h \
    ../../../include/sqlite3pp/sqlite3pp.h \
    ../../../include/sqlite3pp/sqlite3ppext.h \
    ../../../include/sqlite/sqlite_modern_cpp.h \
    ../../../include/sqlite/sqlite_modern_cpp/sqlcipher.h \
    ../../../include/sqlite/sqlite_modern_cpp/utility/function_traits.h \
    ../../../include/sqlite/sqlite_modern_cpp/utility/uncaught_exceptions.h \
    ../../../include/sqlite/sqlite_modern_cpp/utility/variant.h \
    ../../../include/sqlite/sqlite3.h \
    ../../../include/sqlite/sqlite3ext.h

INCLUDEPATH += .
INCLUDEPATH += ../../../include

#QMAKE_CXXFLAGS += -std=c++17
