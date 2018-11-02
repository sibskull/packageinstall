/***************************************************************************
 *   Copyright (с) 2009 by Andrey Cherepanov <cas@altlinux.org>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QtGui>
#include <QLibraryInfo>
#include <QDir>
#include <iostream>
#include "dialog.h"

int main( int argc, char *argv[] ) {

    // Application instance
    QApplication app( argc, argv );

    // Load localization
    QTranslator translator;
    QString locale = QLocale::system().name();
    translator.load( QString( "qtbase_" ).append( locale.split( "_" ).at( 0 ) ), QLibraryInfo::location( QLibraryInfo::TranslationsPath ) );
    translator.load( QString( DATADIR ) + QString( APPNAME ) + QString ( "_" ) + locale );
    app.installTranslator( &translator );

    // Set application details
    app.setApplicationName( APPNAME );

    // Process command line arguments

    // Show help
    if( app.arguments().contains( "--help" ) || app.arguments().contains( "-h" ) ) {
        std::cout << qPrintable( QObject::tr( "%1 %2. Program for install packages from APT repositories",
                                             "%1 is application name, %2 is application version").arg( APPNAME ).arg( APPVERSION ) ) << std::endl;
        std::cout << qPrintable( QObject::tr( "Usage: %1 [option] package...",
                                             "%1 is application name" ).arg( APPNAME ) )<< std::endl << std::endl;
        std::cout << qPrintable( QObject::tr( "Available options:" ) ) << std::endl;
        std::cout << qPrintable( QObject::tr( "    -h         This help" ) ) << std::endl;
        std::cout << qPrintable( QObject::tr( "    --version  Version information" ) ) << std::endl;
        exit( 0 );
    }

    // Print version information
    if( app.arguments().contains( "--version" ) ) {
        std::cout << qPrintable( QObject::tr( "%1 %2",
                                             "%1 is application name, %2 is application version").arg( APPNAME ).arg( APPVERSION ) ) << std::endl;
        exit( 0 );
    }

    // Fill package list
    QStringList *names = new QStringList();
    for (int i = 1; i < app.arguments().size(); ++i) {
        if( app.arguments().at(i)[0] != '-' ) {
            names->append( app.arguments().at(i) );
        }
    }

    // Main widget
    /*Dialog *dialog = new Dialog( names );
    dialog->show();

    // Run process
    dialog->start();*/
    Dialog d( names );
    d.show();
    d.start();

    return app.exec();
}
