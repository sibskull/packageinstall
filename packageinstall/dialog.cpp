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
#include <QProcess>
#include <iostream>
#include "dialog.h"

//#define DEBUG

static Dialog *dlg;

Dialog::Dialog( QStringList *p, QWidget *parent, Qt::WFlags flags )
    : QDialog( parent, flags )
{
    // Save package list
    packages = p;

    // State
    state = 0;
    briefed = true;

    // Make dialog available system wide
    dlg = this;

    // Open UI form
    d = new Ui_Packageinstall();
    d->setupUi( this );

    // Adapt UI
    d->title->setText( ( packages->count() == 0 ?
     tr("<b>Updating system</b><br>Please, wait...") :
     tr("<b>Installing packages</b><br>Please, wait...") ) );

    d->stack->setCurrentIndex( 0 );
    d->bInstall->hide();

    // Create process
    process = new QProcess( this );

    QStringList env = QProcess::systemEnvironment();
    env.replaceInStrings( QRegExp("^LANG=(.*)", Qt::CaseInsensitive), "LANG=C" );
    env.replaceInStrings( QRegExp("^LC_ALL=(.*)", Qt::CaseInsensitive), "LC_ALL=C" );
    process->setEnvironment( env );

    connect( process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processStop()) );
    connect( process, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()) );
    connect( process, SIGNAL(readyReadStandardError()), this, SLOT(readError()) );
    //connect( dlg, SIGNAL(rejected()), this, SLOT(cancelPressed()) );

    // Buttons
    connect( d->bDetails, SIGNAL(clicked()), this, SLOT(detailPressed()) );
    connect( d->bInstall, SIGNAL(clicked()), this, SLOT(installPressed()) );
    connect( d->bCancel,  SIGNAL(clicked()), this, SLOT(cancelPressed()) );

}

Dialog::~Dialog()
{
    if (process->state() == QProcess::Running) {
        process->kill();
    }
}


// Terminate process
void Dialog::windowClose() {
    ////qDebug( "windowClose" );
    if (process->state() == QProcess::Running) {
        process->kill();
    }
    //close();
}


// Start installation process
void Dialog::start() {
    processStart();
}

// Process close event
void Dialog::closeEvent( QCloseEvent *e ) {
    //qDebug( "closeEvent" );
    cancelPressed();
    e->ignore();
}

// Slot for start process
void Dialog::processStart() {
    //qDebug( "processStart" );

    if (process->state() == QProcess::Running) {
        return;
    }

    QStringList args;

    // Test install
    args << "-f";


    QString programm = QString( "apt-get" );
#ifdef DEBUG
    programm = QString( "./apt-get-test" );
#endif

    commit.packages = packages;

    if( packages->count() == 0 ) {
        // On empty list of packages update all system
        args << "dist-upgrade";
    } else {
        // Copy packages
        args << "install";
        for (int i = 0; i < packages->size(); ++i) {
            args << packages->at(i);
        }
    }

    // Start process
    process->start( programm, args );

    if ( ! process->waitForStarted() ) {
        QMessageBox::critical(this, tr("Error start"), tr("Could not start 'apt-get' programm.\nInstall this program.") );
        exit( 1 );
    }

    // Inform about first stage
    setStatus( tr("Check package requirements..."), 0, "" );

}


// On finish process
void Dialog::processStop() {
    //qDebug( "processStop" );

    if( d ) {
		setStatus( tr("Installation is finished successful"), 100, QString( "" ) );
		d->bInstall->hide();
		d->bCancel->setText( tr("&Exit") );
        state = 5;
    }
}

// Read from process output
void Dialog::readOutput() {

	int p;
	QString str, line, b;
	QByteArray ba;

    process->setReadChannel( QProcess::StandardOutput );

	// Read all
	ba = process->readAll();
	b = QString( ba );

	// Add to buffer
	str = tail.append( b );

	//qDebug() << QString( "%1 %3 ==%2==").arg( b.size() ).arg( b ).arg( str.indexOf( QChar( '\n' ) ) );

	// Split output to lines
	while( ( p = str.indexOf( QChar( '\n' ) ) ) > 0 ) {
		line = str.left( p );
		str = str.mid( p + 1 );

		if( d ) {
			d->log->append( line );
		}
		commit.appendString( line );
	}
	tail = str;
}


// Read from process errors
void Dialog::readError() {

    QByteArray buf;
    QString str;

    process->setReadChannel( QProcess::StandardError );

    while( process->canReadLine() ) {
        buf = process->readLine();
        str = QString( buf.data() );
        if( str.length() > 0 && d ) d->log->append( QString( str.left( str.length() - str.endsWith( "\n" ) ) ) );
        commit.appendError( str );
        if( ! str.isEmpty() ) {
            if( str.startsWith( "E: Unable to write to /var/cache/apt/" ) ) {
                QMessageBox::critical( this, tr("Unsufficient privileges"),
                                      tr("Program should be run with superuser privileges.\nCheck your rights and program installation.") );
                close();
            }
            if( str.startsWith( "E: Error while running transaction" ) ) {
                QMessageBox::critical( this, tr("Installation error"),
                                      tr("Installation is failed.\nCheck output log.") );
                close();
            }
            if( str.startsWith( "E: Couldn't find package" ) ) {
                QMessageBox::critical( this, tr("Wrong package name"),
                                      tr("Couldn't find package\n'%1'").arg( str.simplified().right( str.length() - 26 ) ) );
                close();
            }
        }
    }
}


// Button actions
void Dialog::detailPressed() {
    //qDebug() << QString( "detailPressed" ) << d->stack->currentIndex();
    if( d->stack->currentIndex() == 0 ) { // From first pane
        d->bDetails->setText( tr("Hide &details <<") );
        d->stack->setCurrentIndex( 2 );
    } else if( d->stack->currentIndex() == 2 ) { // From details pane
        d->bDetails->setText( tr("Show &details >>") );
        d->stack->setCurrentIndex( 0 );
    } else {
        if( briefed ) {
            d->total->setHtml( description );
            d->bDetails->setText( tr("Hide &details <<") );
            briefed = false;
        } else {
            d->total->setHtml( brief );
            d->bDetails->setText( tr("Show &details >>") );
            briefed = true;
        }
    }

}

void Dialog::installPressed() {
    //qDebug( "installPressed" );
    d->stack->setCurrentIndex( 0 );
    processWrite( QString( "Y\n" ) );
    d->bInstall->hide();
}


void Dialog::cancelPressed() {
    //qDebug() << QString( "cancelPressed" ) << state;
    // Quit confirmation
    if( state < 5 ) {
        QMessageBox msgBox( this );
        msgBox.setWindowTitle( tr("Quit") );
        msgBox.setIcon( QMessageBox::Question );
        msgBox.setText("Do you want to cancel installation??");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();

        if( ret == QMessageBox::Yes ) {
            windowClose();
            exit ( 1 );
        }
    } else {
        windowClose();
        exit( 0 );
    }
}


// Set status information
void Dialog::iSetStatus( QString stage, int percent, QString fileName ) {
    //qDebug( "iSetStatus" );
    if( d ) {
        d->status->setText( stage );
        d->progress->setValue( percent );
        d->file->setText( fileName );
    }
}


void Dialog::iSetStatistics( QString text, QString details, int total ) {
    //qDebug( "iSetStatistics" );
    brief = text;
    description = details;

	// Check install without pause
	if( total == packages->count() ) {
		d->stack->setCurrentIndex( 0 );
	} else if( d ) {
		// Fill statistics into dialog
        d->bInstall->show();
        d->total->setHtml( brief );
        d->stack->setCurrentIndex( 1 );
		d->bDetails->setText( tr("Show &details >>") );
    }
}


// Set status information (static function)
void setStatus( QString stage, int percent, QString fileName ) {
    if( dlg ) {
        dlg->iSetStatus( stage, percent, fileName );
    }
}


// Set statistics
void setStatistics( QString text, QString details, int total ) {
    if( dlg ) {
		dlg->iSetStatistics( text, details, total );
    }
}

// Write to process
void processWrite( QString str ) {
    if( dlg ) {
        dlg->process->write( QByteArray( qPrintable(str) ) );
        dlg->process->closeWriteChannel();
    }
}
