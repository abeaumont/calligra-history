/* This file is part of the KDE project
   Copyright (C) 2001 Ariya Hidayat <ariyahidayat@yahoo.de>

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

#include <kinstance.h>
#include <kdebug.h>

#include "wpexport_factory.h"
#include "wpexport_factory.moc"
#include "wpexport.h"

extern "C"
{
    void* init_libwpexport()
    {
        return new WPExportFactory;
    }
};

KInstance* WPExportFactory::s_global = 0;

WPExportFactory::WPExportFactory( QObject* parent, const char* name )
    : KLibFactory( parent, name )
{
    s_global = new KInstance( "asciiexport" );
}

WPExportFactory::~WPExportFactory()
{
    delete s_global;
    s_global = 0L;
}

QObject* WPExportFactory::create( QObject* parent, const char* name, const char*, const QStringList & )
{
    if ( parent && !parent->inherits("KoFilter") )
    {
	    kdDebug(30502) << "WPExportFactory: parent does not inherit KoFilter" << endl;
	    return 0L;
    }
    WPExport *f = new WPExport( (KoFilter*)parent, name );
    emit objectCreated(f);
    return f;
}

KInstance* WPExportFactory::global()
{
    return s_global;
}
