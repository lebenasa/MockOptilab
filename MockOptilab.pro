TEMPLATE = app

QT += qml quick widgets core gui

PRECOMPILED_HEADER = stdafx.h
CONFIG += c++11

SOURCES += main.cpp \
    camera.cpp \
    cameramodel.cpp \
    serialcapture.cpp \
    sminterface.cpp \
    stepper.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    camera.h \
    cameramodel.h \
    serialcapture.h \
    sminterface.h \
    stepper.h \
    stdafx.h

win32: LIBS += -L$$PWD/../../../../../Libraries/opencv/x86/vc12/lib/ -lopencv_core300

INCLUDEPATH += $$PWD/../../../../../Libraries/opencv/include
DEPENDPATH += $$PWD/../../../../../Libraries/opencv/include

win32: LIBS += -L$$PWD/../../../../../Libraries/opencv/x86/vc12/lib/ -lopencv_imgproc300

INCLUDEPATH += $$PWD/../../../../../Libraries/opencv/include
DEPENDPATH += $$PWD/../../../../../Libraries/opencv/include

win32: LIBS += -L$$PWD/../../../../../Libraries/opencv/x86/vc12/lib/ -lopencv_imgcodecs300

INCLUDEPATH += $$PWD/../../../../../Libraries/opencv/include
DEPENDPATH += $$PWD/../../../../../Libraries/opencv/include
