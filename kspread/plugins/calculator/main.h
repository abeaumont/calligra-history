/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#ifndef __my_app_h__
#define __my_app_h__

#include <qptrlist.h>
#include <qstring.h>
#include <qobject.h>

#include <klibloader.h>
#include <kparts/plugin.h>

class KSpreadView;
class QtCalculator;
class KInstance;

class CalcFactory : public KLibFactory
{
    Q_OBJECT
public:
    CalcFactory( QObject* parent = 0, const char* name = 0 );
    ~CalcFactory();

    virtual QObject* createObject( QObject* parent = 0, const char* name = 0,
			     const char* classname = "QObject", const QStringList &args = QStringList() );

    static KInstance* global();

private:
    static KInstance* s_global;
};

class KDE_EXPORT Calculator : public KParts::Plugin
{
    Q_OBJECT
public:
    Calculator( KSpreadView* parent, const char* name = 0 );
    ~Calculator();

    KSpreadView* view() { return m_view; }

protected slots:
    void showCalculator();

protected:
    bool eventFilter( QObject*, QEvent* );

private:
    QtCalculator* m_calc;
    KSpreadView* m_view;
};

#endif
