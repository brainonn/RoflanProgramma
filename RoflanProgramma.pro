#-------------------------------------------------
#
# Project created by QtCreator 2016-11-07T15:08:27
#
#-------------------------------------------------

QT       += core gui serialport testlib
CONFIG   += c++11 warn_on
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

QT.testlib.CONFIG -= console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

QMAKE_LFLAGS_RELEASE += -static -static-libgcc

QMAKE_CFLAGS_RELEASE += -O2
QMAKE_CFLAGS += -Wall -Wextra -Wfloat-equal -Wundef -Wwrite-strings -Wlogical-op -Wmissing-declarations -Wshadow -Wdiv-by-zero
QMAKE_CFLAGS += -isystem $$[QT_INSTALL_HEADERS]

QMAKE_CXXFLAGS_RELEASE += -O2
QMAKE_CXXFLAGS += -Wall -Wextra -Wfloat-equal -Wundef -Wwrite-strings -Wlogical-op -Wmissing-declarations -Wshadow -Wdiv-by-zero
QMAKE_CXXFLAGS += -isystem $$[QT_INSTALL_HEADERS]

TARGET = RoflanProgramma
RC_ICONS = icon.ico
TEMPLATE = app

SOURCES += main.cpp\
    comsettings.cpp \
        mainwindow.cpp \
    piercestat.cpp \
    qcustomplot.cpp

HEADERS  += mainwindow.h \
    comsettings.h \
    piercestat.h \
    qcustomplot.h

FORMS    += mainwindow.ui \
    comsettings.ui
