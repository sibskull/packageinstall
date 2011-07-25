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
#ifndef APTCOMMIT_H
#define APTCOMMIT_H

#include <QtCore>

class AptCommit : public QObject
{

    Q_OBJECT

    enum Type { Unknown, Upgraded, Installed, Removed, Kept, Preparing, Progress };

public:
    AptCommit();
    int appendString( QString str );
    int appendError( QString str );
    QStringList *packages;

private:
    QStringList output;
    QStringList errors;

    // Package lists
    QStringList installed;
    QStringList upgraded;
    QStringList removed;
    QStringList kept;
    QStringList all;

    // Status
    Type stage;
    int count;
    int total;
    QString currentFile;
    int totalPackages;
};

#endif // APTCOMMIT_H
