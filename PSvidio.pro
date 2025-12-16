QT       += core gui widgets multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 5): QT += multimedia quick
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    fileviewsubwindow.cpp \
    grayscalecommand.cpp \
    binarycommand.cpp \
    meanfiltercommand.cpp \
    gammacorrectioncommand.cpp \
    edgedetectioncommand.cpp \
    imagecommand.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    fileviewsubwindow.h \
    grayscalecommand.h \
    binarycommand.h \
    meanfiltercommand.h \
    gammacorrectioncommand.h \
    edgedetectioncommand.h \
    imagecommand.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
