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

static Dialog *dlg;

Dialog::Dialog( QStringList *p, QWidget *parent, Qt::WFlags flags )
        : QDialog( parent, flags )
{
	// Save package list
	packages = p;

	// Make dialog available system wide
	dlg = this;

	// Forbid maximized
	//setWindowState(  Qt::WindowMinimized | Qt::WindowActive );
	setWindowModality(Qt::ApplicationModal);
	setMinimumSize( 450, 250 );
	setMaximumSize( 450, 250 );
	
	// Set properties
	setWindowIcon( QIcon( QString( DATADIR ) + QString( ICON ) ) );
	setWindowTitle( tr( "Installing packages" ) );
	
	// Widgets
	QHBoxLayout *title = new QHBoxLayout();
	title->setSpacing( 15 );
	
	QLabel *icon    = new QLabel();
	icon->setAlignment( Qt::AlignTop | Qt::AlignLeft );
	icon->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
	icon->setPixmap( QString( DATADIR ) + QString( ICON ) );
	
	QLabel *message = new QLabel( ( packages->count() == 0 ? 
		tr("<b>Updating system</b><br>Please, wait...") : 
		tr("<b>Installing packages</b><br>Please, wait...") ) );
	message->setWordWrap( true );
	message->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
	message->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum ) );

	title->addWidget( icon );
	title->addWidget( message );

	status   = new QLabel( "" );
	progress = new QProgressBar();
	progress->setTextVisible( true );
	progress->setMinimum( 0 );
	progress->setMaximum( 100 );
	progress->setValue( 0 );
	file     = new QLabel( "" );

	cancel  = new QPushButton( tr( "&Cancel" ) );
	cancel->setWhatsThis( tr( "Quit from application" ) );

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->setSpacing( 10 );
	
	QFrame *line = new QFrame();                                                   
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

	mainLayout->addLayout( title );
	mainLayout->addWidget( line );

	// Information block
	mainLayout->addWidget( status );
	mainLayout->addWidget( progress );
	mainLayout->addWidget( file );
	
	QSpacerItem *verticalSpacer = new QSpacerItem( 400, 100, QSizePolicy::Minimum, QSizePolicy::Expanding );
	mainLayout->addItem( verticalSpacer );

	QDialogButtonBox *buttonBox = new QDialogButtonBox( Qt::Horizontal );
    buttonBox->addButton( cancel,  QDialogButtonBox::RejectRole );

	mainLayout->addWidget( buttonBox );
	
	// Set layout
	setLayout( mainLayout );

	// Connect signals and slots
    connect( buttonBox, SIGNAL(rejected()), this, SLOT(reject()) );	

	// Create process
	process = new QProcess( this );
	QStringList env = QProcess::systemEnvironment();
	env.replaceInStrings( QRegExp("^LANG=(.*)", Qt::CaseInsensitive), "LANG=C" );
	process->setEnvironment( env );
	
	connect( process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_processStop()) );
	connect( process, SIGNAL(readyReadStandardOutput()), this, SLOT(on_readOutput()) );
	connect( process, SIGNAL(readyReadStandardError()), this, SLOT(on_readError()) );
	connect( this, SIGNAL(destroyed()), this, SLOT(on_windowClose()) );
	connect( this, SIGNAL(rejected()), this, SLOT(on_windowClose()) );

}

Dialog::~Dialog()
{
	on_windowClose();
}


// Terminate process
void Dialog::on_windowClose() {
	if (process->state() == QProcess::Running) {
        process->terminate();
        process->waitForFinished(3000);
    }
}


// Start installation process
void Dialog::start() {
	on_processStart();
}


// Slot for start process
void Dialog::on_processStart() {
	
    if (process->state() == QProcess::Running) {
		return;
	}

	QStringList args;
   
	// Test install
	args << "-f";
	
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
	process->start( "apt-get", args );

	if ( ! process->waitForStarted() ) {
        QMessageBox::critical(this, tr("Error start"), tr("Could not start 'apt-get'. Install this program.") );
        exit( 1 );
    }
	
	// Inform about first stage
	setStatus( tr("Check package requirements..."), 0, "" );
	
}


// On finish process
void Dialog::on_processStop() {
	if( cancel ) {
		cancel->setText( tr("&Exit") );
	}
}

// Read from process output
void Dialog::on_readOutput() {
	
	QByteArray buf;

	process->setReadChannel( QProcess::StandardOutput );
	
	QByteArray bytes = process->readAll();
	QStringList lines = QString( bytes ).split( "\n" );
	foreach( QString line, lines ) {
		//if( ! line.isEmpty() ) {
			commit.appendString( line );
		//}
	}

}


// Read from process errors
void Dialog::on_readError() {
	
	QByteArray buf;
	QString str;
	
	process->setReadChannel( QProcess::StandardError );
	
	while( process->canReadLine() ) {
		buf = process->readLine();
		str = QString( buf.data() );
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
		}
	}
}


// Set status information
void Dialog::iSetStatus( QString stage, int percent, QString fileName ) {
	status->setText( stage );
	progress->setValue( percent );
	file->setText( fileName );
}


// Set status information (static function)
void setStatus( QString stage, int percent, QString fileName ) {
	if( dlg )
		dlg->iSetStatus( stage, percent, fileName );
}


// Show statistics
int showDialog( QString title, QString text, QString details ) {
			
	if( dlg ) {
		QMessageBox msgBox;
		
		msgBox.setWindowTitle( title );
		msgBox.setText( text );
		if( ! details.isEmpty() ) {
			msgBox.setDetailedText( details );
			msgBox.setStandardButtons( QMessageBox::Ok | QMessageBox::Cancel );
		} else {
			msgBox.setStandardButtons( QMessageBox::Ok );
		}
		msgBox.setDefaultButton( QMessageBox::Ok );
		
		int ret = msgBox.exec();
		
		if( ret == QMessageBox::Cancel ) {
			dlg->close();
		} else {
			msgBox.close();
		}
		return ret;
	}
	return 0;
}


// Write to process
void processWrite( QString str ) {
	if( dlg ) {
		dlg->process->write( QByteArray( qPrintable(str) ) );
		dlg->process->closeWriteChannel();
	}
}
