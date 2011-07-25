SOURCES += main.cpp \
    dialog.cpp \
    aptcommit.cpp
HEADERS += dialog.h \
    aptcommit.h
TEMPLATE = app
TRANSLATIONS = packageinstall_ru.ts
CONFIG -= debug
CONFIG += release \
    exceptions \
    qt \
    warn_on
TARGET = packageinstall
DESTDIR = $$(DESTDIR)
PREFIX = $$(PREFIX)
isEmpty( PREFIX ):PREFIX = /usr/local
DATADIR = $$DESTDIR$$PREFIX/share/apps/packageinstall/
DEFINES += DATADIR=\\\"$$PREFIX/share/apps/packageinstall/\\\"
target.path = $$DESTDIR/$$PREFIX/bin/
translations.files = *.qm
translations.path = $$DATADIR
icons.files = rpm-package.png
icons.path = $$DATADIR
INSTALLS += target
INSTALLS += translations
INSTALLS += icons

FORMS += \
    packageinstall.ui

RESOURCES += \
    packageinstall.qrc

OTHER_FILES += \
    apt-get-test
