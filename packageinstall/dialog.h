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
#ifndef DIALOG_H
#define DIALOG_H

#include <QtGui>
#include <QProcess>

#define APPNAME		"packageinstall"
#define APPVERSION	"1.0"
#define ICON		"rpm-package.png"

class Dialog : public QDialog
{

    Q_OBJECT

public:
    Dialog( QStringList *p, QWidget *parent = 0, Qt::WFlags flags = 0 );
    ~Dialog();
	void start();
    
private:
    QProcess *process;
	QStringList *packages;
	QLabel *status;
	QProgressBar *progress;
	QLabel *file;

private slots:
	void on_processStart();
	void on_readOutput();
	void on_readError();

};

#endif // DIALOG_H
