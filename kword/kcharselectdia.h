/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KCHARSELECTDIA_H
#define KCHARSELECTDIA_H

#include <kdialogbase.h>
#include <qstring.h>

class QWidget;
class QGridLayout;
class QPushButton;
class KCharSelect;
class KButtonBox;

/******************************************************************/
/* class KCharSelectDia                                           */
/******************************************************************/

class KCharSelectDia : public KDialogBase
{
    Q_OBJECT

public:

    // constructor - destructor
    KCharSelectDia( QWidget *parent, const char *name, const QChar &_chr, const QString &_font, bool _enableFont );

    //constructor when you want to insert multi char
    KCharSelectDia( QWidget *parent, const char *name, const QString &_font, const QChar &_chr );

    // select char dialog
    static bool selectChar( QString &_font, QChar &_chr, bool _enableFont = true );

    // internal
    QChar chr();
    QString font();

private:
    void initDialog(const QChar &_chr, const QString &_font, bool _enableFont);

private slots:
    void slotUser1();

protected:
    // dialog objects
    QGridLayout *grid;
    KButtonBox *bbox;
    QPushButton *bOk, *bCancel;
    KCharSelect *charSelect;

 signals:
    void insertChar(QChar,const QString &);
};

#endif
