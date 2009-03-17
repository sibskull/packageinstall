
include(config.pri)

SOURCES += main.cpp dialog.cpp
SRCMOC += dialog.moc

TEMPLATE = app

TRANSLATIONS = packageinstall_ru.ts

CONFIG -= debug

CONFIG += release \
		  exceptions \
		  build_all \
		  qt \
		  warn_on

TARGET = packageinstall

DESTDIR = .

DATADIR = $$PREFIX/share/apps/packageinstall/

DEFINES += DATADIR=\\\"$$DATADIR\\\"

target.path = $$PREFIX/bin/

translations.files = *.qm
translations.path = $$DATADIR

icons.files = rpm-package.png
icons.path = $$DATADIR

INSTALLS += target
INSTALLS += translations 
INSTALLS += icons
