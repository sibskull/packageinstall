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
	
	// Test output
	//if( stage == Preparing ) {
	//	qDebug() << str;
	//}
	
	if( str == QString( "The following packages will be upgraded" )) { stage = Upgraded; return 0; }
	if( str == QString( "The following NEW packages will be installed:" )) { stage = Installed; return 0; }
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
	if( str.startsWith( "Do you want to continue? [Y/n]" ) ) {
		QString statistics = tr("There are %1 upgraded, %2 installed,\n%3 removed, %4 kept packages.\nDo you want to install this packages?").arg( 
				upgraded.count() ).arg(
				installed.count() ).arg(
				removed.count() ).arg(
				kept.count() );
				
		QString details("");
		
		upgraded.sort();
		installed.sort();
		removed.sort();
		kept.sort();
		
		if( installed.count() > 0 ) {
			details += tr("Packages to be installed:\n");
			details += installed.join( " " );
		}
		if( upgraded.count() > 0 ) {
			details += tr("\nPackages to be upgraded:\n");
			details += upgraded.join( " " );
		}
		if( removed.count() > 0 ) {
			details += tr("\nPackages to be removed:\n");
			details += removed.join( " " );
		}
		if( kept.count() > 0 ) {
			details += tr("\nPackages to be kept back:\n");
			details += kept.join( " " );
		}
		
		int ret;
		all << installed << upgraded;
		all.sort();
		totalPackages = all.count();
		
		if( totalPackages == 0 ) {
			showDialog( tr("Nothing to install"), tr("Nothing to install. There are newest version of packages."), QString() );
			ret = 0;
		} else {
			if( packages->count() < totalPackages ) {
				ret = showDialog( tr("Statistics"), statistics, details );
			} else {
				ret = 1;
			}
		}
		
		if( ret == 0 ) {
			QApplication::exit( 0 );
		} else {
			processWrite( QString( "Y\n" ) );	
		}
		
		return 0;
	}
	
	// Begin preparing
	if( str.startsWith( "Preparing..." ) ) {
		stage = Preparing;
		setStatus( tr("Preparing..."), 0, QString( "" ) );
		count = 0;
		total = ( totalPackages + 1 ) * 50;
		currentFile = QString( "" );
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
	if( stage == Preparing && str.length() != str.count( "#" ) && totalPackages > -1 ) {
		currentFile = str;
		
		// Process package name
		currentFile.replace( QRegExp( "^([0-9]*:)?([^#[:blank:]]*)([[:blank:]]*)#*</i>" ), "\\2" );
		currentFile = currentFile.trimmed();
		
		// Lookup in package names
		int pos = all.indexOf( QRegExp( QString("^") + currentFile ) );
		//qDebug() << currentFile << pos << all;
		
		if( pos > -1 ) { 
			currentFile = all.at( pos );
		} else {
			//if( currentFile.startsWith( "Running" ) ) {
			//}
			currentFile.clear();
		}
		//qDebug() << totalPackages << (int)( ( 100 * count ) / total ) << count << total;
		totalPackages--;
	}

	// Progress
	if( stage == Preparing ) {
		if( count < 50 ) { // Preparing
			setStatus( tr("Preparing..."), (int)( ( 100 * count ) / total ), currentFile );
		} else { // Post actions
			if( currentFile.isEmpty() ) {
				setStatus( tr("Finishing installation..."), (int)( ( 100 * count ) / total ), currentFile );
			} else {
				setStatus( tr("Installing package..."), (int)( ( 100 * count ) / total ), currentFile );
			}
		}
	
		count += str.count( "#" );
		return 0;
	}

	return 0;
}

// Append string from stderr
int AptCommit::appendError( QString str ) {

	errors << str;
	printf( "%s", qPrintable( str ));
	return 0;
}

