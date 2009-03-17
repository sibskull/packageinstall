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

Dialog::Dialog( QStringList *p, QWidget *parent, Qt::WFlags flags )
        : QDialog( parent, flags )
{
	// Save package list
	packages = p;

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
	
	QLabel *message = new QLabel( tr("<b>Installing packages</b><br>Please, wait...") );
	message->setWordWrap( true );
	message->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
	message->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum ) );

	title->addWidget( icon );
	title->addWidget( message );

	status   = new QLabel( "" );
	progress = new QProgressBar();
	file     = new QLabel( "" );

	QPushButton *install = new QPushButton( tr( "I&nstall" ) );
	install->setAutoDefault( true );
	install->setWhatsThis( tr( "Install RPM packages" ) );
	QPushButton *cancel  = new QPushButton( tr( "&Cancel" ) );
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
	
	//connect( process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(close()) ); //TODO
	connect( process, SIGNAL(readyReadStandardOutput()), this, SLOT(on_readOutput()) );
	connect( process, SIGNAL(readyReadStandardError()), this, SLOT(on_readError()) );

}

Dialog::~Dialog()
{
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
	if( packages->count() == 0 ) {
		// On empty list of packages update all system
		args << "dist-upgrade";
	} else {
		// Copy packages
		args << "install";
		for (int i = 1; i < packages->size(); ++i) {
			args << packages->at(i);
		}
	}
	
	// Start process
	process->start( "apt-get", args );

	if ( ! process->waitForStarted() ) {
        QMessageBox::critical(this, tr("Error start"), tr("Could not start apt-get") );
        exit( 1 );
    }
	
}


// Read from process output
void Dialog::on_readOutput() {
	
	QByteArray buf;

	process->setReadChannel( QProcess::StandardOutput );
	
	//while( process->readyRead() ) {
		QByteArray bytes = process->readAll();
		QStringList lines = QString( bytes ).split( "\n" );
		foreach( QString line, lines ) {
			if( ! line.isEmpty() )
				printf( "%s\n", qPrintable( line ) ); 
		}
	//}
}


// Read from process errors
void Dialog::on_readError() {
	
	QByteArray buf;
	
	process->setReadChannel( QProcess::StandardError );
	
	while( process->canReadLine() ) {
		buf = process->readLine();
		printf( "> %s", buf.data() );
	}
}


#include "dialog.moc"
