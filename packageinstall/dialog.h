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
#include "aptcommit.h"
#include "ui_packageinstall.h"

#define APPNAME		"packageinstall"
#define APPVERSION	"1.1.0"
#define ICON		"rpm-package.png"

class AptCommit;
void setStatus( QString stage, int percent, QString fileName );
void setStatistics( QString text, QString details );
void processWrite( QString str );

class Dialog : public QDialog
{

    Q_OBJECT

public:
    Dialog( QStringList *p, QWidget *parent = 0, Qt::WFlags flags = 0 );
    ~Dialog();
    void start();
    void iSetStatus( QString stage, int percent, QString fileName );
    void iSetStatistics( QString text, QString details );
    QProcess *process;

private:
    AptCommit    commit;
    QStringList  *packages;
    Ui_Packageinstall *d;
    bool briefed;
    int state;
    QString brief;
    QString description;

private:
    void closeEvent( QCloseEvent *e );

private slots:
    void processStart();
    void readOutput();
    void readError();
    void windowClose();
    void processStop();

    // Button actions
    void detailPressed();
    void installPressed();
    void cancelPressed();

};

#endif // DIALOG_H
