# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-ble_scanner

QT += bluetooth dbus

OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
RCC_DIR=build

DESTDIR = bin

CONFIG += sailfishapp

SOURCES += src/ble_scanner.cpp \
    src/device.cpp \
    src/characteristicinfo.cpp \
    src/serviceinfo.cpp \
    src/deviceinfo.cpp

OTHER_FILES += qml/ble_scanner.qml \
    qml/cover/CoverPage.qml \
    translations/*.ts \
    harbour-ble_scanner.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 256x256

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
#TRANSLATIONS += translations/ble_scanner-de.ts

HEADERS += \
    src/device.h \
    src/characteristicinfo.h \
    src/deviceinfo.h \
    src/serviceinfo.h

DISTFILES += \
    qml/pages/DevicesPage.qml \
    qml/pages/Characteristics.qml \
    qml/pages/Dialog.qml \
    qml/pages/Header.qml \
    qml/pages/Label.qml \
    qml/pages/Menu.qml \
    qml/pages/Services.qml \
    qml/pages/MainPage.qml \
    qml/pages/ApplicationPage.qml \
    rpm/harbour-ble_scanner.changes.in \
    rpm/harbour-ble_scanner.yaml \
    rpm/harbour-ble_scanner.spec

