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

#include <iostream>
#include "aptcommit.h"
#include "dialog.h"

AptCommit::AptCommit()
    : QObject()
{
    // Clear lists
    output.clear();
    errors.clear();
    installed.clear();
    upgraded.clear();
    removed.clear();
    kept.clear();
    all.clear();
}


// Append string from stdout
int AptCommit::appendString( QString str ) {

    QStringList pkgs;
    output << str;

    // debug output
    std::cout << qPrintable( str ) << std::endl;

    if( str == QString( "The following packages will be upgraded" )) { stage = Upgraded; return 0; }
    if( str == QString( "The following NEW packages will be installed:" )) { stage = Installed; return 0; }
    if( str == QString( "The following extra packages will be installed:" )) { stage = Installed; return 0; }
    if( str == QString( "The following packages will be REPLACED:" )) { stage = Unknown; return 0; }
    if( str == QString( "The following packages will be REMOVED:" )) { stage = Removed; return 0; }
    if( str == QString( "The following packages have been kept back" )) { stage = Kept; return 0; }

    // Packages
    if( str[0] == ' ' ) {
        pkgs = str.split( ' ', QString::SkipEmptyParts );
        switch( stage ) {
            case Upgraded:
                upgraded << pkgs;
                break;
            case Installed:
                installed << pkgs;
                break;
            case Removed:
                removed << pkgs;
                break;
            case Kept:
                kept << pkgs;
                break;
            default:
                break;
        }
        return 0;
    }

    // Prepare to install
    //if( str.startsWith( "Do you want to continue? [Y/n]" ) ) {
    if( str.contains( "not upgraded." ) ) {

        // Sorts lists
        upgraded.sort();
        installed.sort();
        removed.sort();
        kept.sort();

        // Keep only unique package to install
        installed.removeDuplicates();

        QString statistics = tr( "<p><b>Processed packages:</b></p>\n" );
        // Show only non-empty categories one per line
	//std::cout << "upgraded: " << qPrintable(upgraded.join(' ')) << std::endl;
	//std::cout << "installed: " << qPrintable(installed.join(' ')) << std::endl;
	//std::cout << "removed: " << qPrintable(removed.join(' ')) << std::endl;
        if( upgraded.count() > 0 )  statistics.append( tr( "&#9658; upgraded: %1<br>" ).arg( upgraded.count() ) );
        if( installed.count() > 0 ) statistics.append( tr( "&#9658; installed: %1<br>" ).arg( installed.count() ) );
        if( removed.count() > 0 )   statistics.append( tr( "&#9658; removed: %1<br>" ).arg( removed.count() ) );
        if( kept.count() > 0 )      statistics.append( tr( "&#9658; kept: %1<br>" ).arg( kept.count() ) );

        statistics.append( tr( "<br>Do you want to install this packages?") );

        QString details("");

        if( upgraded.count() > 0 ) {
            details += tr("<p><b>Packages to be upgraded:</b></p>");
            details += upgraded.join( " " );
        }
        if( installed.count() > 0 ) {
            details += tr("<p><b>Packages to be installed:</b></p>");
            details += installed.join( " " );
        }
        if( removed.count() > 0 ) {
            details += tr("<p><b>Packages to be removed:</b></p>");
            details += removed.join( " " );
        }
        if( kept.count() > 0 ) {
            details += tr("<p><b>Packages to be kept back:</b></p>");
            details += kept.join( " " );
        }

        all << installed << upgraded;
        all.sort();
        totalPackages = all.count();

        if( totalPackages == 0 ) {
            statistics = tr("<b>Nothing to install.</b><br>There are newest version of packages.");
        }

        // Show dialog
        setStatistics( statistics, details, totalPackages );

    }

    // Begin preparing
    if( str.startsWith( "Preparing..." ) ) {
        stage = Preparing;
        setStatus( tr("Preparing..."), 0, QString( "" ) );
        count = str.count( "#" );
        numHashes = count;
        // +2 because first line during prepare contains numHashes symbols and we need to show gap to 100% for postinstall actions and triggers
        total = ( totalPackages + 2 ) * numHashes;
        currentFile = QString( "" );
        return 0;
    }

    // Cleaning up / removing...
    if( str.startsWith( "Cleaning up / removing..." ) ) {
        setStatus( tr("Cleaning up / removing packages"), 99, QString( "" ) );
        stage = Cleaning;
        return 0;
    }

    // Done
    if( str.startsWith( "Done." ) ) {
        setStatus( tr("Installation is finished successful"), 100, QString( "" ) );
        return 0;
    }

    // New line
    if( stage == Preparing && str.isEmpty() ) {
        return 0;
    }

    // Read packages
    count += str.count( "#" );
    if( stage == Preparing && str.length() != str.count( "#" ) && totalPackages > -1 ) {
        currentFile = str;

        // Process package name
        currentFile.replace( QRegExp( "^[0-9]*: ([^#\\s]*)[\\s]*#*.*$" ), "\\1" );
        currentFile = currentFile.trimmed(); // should be full package name with version and release (like LibreOffice-still-qt5-7.0.6.2-alt1)

        // Remove version and release from package name (remove last two parts)
        QStringList nameParts = currentFile.split('-');
        if( nameParts.size() > 2 ) {
            nameParts.removeLast(); // remove release
            nameParts.removeLast(); // remove version
        }
        currentFile = nameParts.join('-');

        //std::cout << qPrintable(currentFile) << std::endl;

        // Lookup in package names
        int pos = all.indexOf( QRegExp( QString("^") + currentFile ) );
        //std::cout << "debug: lookup " << qPrintable(currentFile) << " " << pos << std::endl;

        if( pos > -1 ) {
            currentFile = all.at( pos );
        } else {
            //if( currentFile.startsWith( "Running" ) ) {
            //}
            currentFile.clear();
        }
        //std::cout << "debug: totalPackages: " << totalPackages << " %: " << (int)( ( 100 * count ) / total ) << " count: " << count << " total: " << total << std::endl;
        totalPackages--;
    }

    // Progress
    if( stage == Preparing ) {
        if( count < 1 ) { // Preparing
            setStatus( tr("Preparing..."), (int)( ( 100 * count ) / total ), currentFile );
        } else { // Post actions
            setStatus( tr("Installing package..."), (int)( ( 100 * count ) / total ), currentFile );
        }
        return 0;
    }

    return 0;
}

// Append string from stderr
int AptCommit::appendError( QString str ) {

    errors << str;
    std::cerr << qPrintable( str );
    return 0;
}

