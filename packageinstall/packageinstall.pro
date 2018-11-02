QT += widgets
SOURCES += main.cpp \
    dialog.cpp \
    aptcommit.cpp
HEADERS += dialog.h \
    aptcommit.h
TEMPLATE = app
TRANSLATIONS = packageinstall_ru.ts
CONFIG -= debug
CONFIG += release
QMAKE_CXXFLAGS += -pedantic

DEFINES += DATADIR=\\\"/usr/share/apps/packageinstall/\\\"

TARGET = packageinstall
target.path = /usr/bin/

translations.files = *.qm
translations.path = /usr/share/apps/packageinstall

icons.files = rpm-package.png
icons.path = /usr/share/apps/packageinstall

INSTALLS += target
INSTALLS += translations
INSTALLS += icons

FORMS += \
    packageinstall.ui

RESOURCES += \
    packageinstall.qrc

OTHER_FILES += \
    apt-get-test
