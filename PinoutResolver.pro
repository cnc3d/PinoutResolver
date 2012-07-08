#-------------------------------------------------
#
# Project created by QtCreator 2012-07-06T23:56:02
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PinoutResolver
TEMPLATE = app


SOURCES += main.cpp\
        pinoutresolver.cpp

HEADERS  += pinoutresolver.h

FORMS    += pinoutresolver.ui

OTHER_FILES += \
    devices/STM32F407.xml


install_devices.path = $$OUT_PWD/devices
install_devices.files += devices/*.xml

INSTALLS += install_devices
