/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#include <kaboutdata.h>
#include <kinstance.h>
#include <kiconloader.h>
#include <klocale.h>

#include "karbon_factory.h"
#include "karbon_part.h"

#include <kdebug.h>

extern "C"
{
	void* init_libkarbonpart()
	{
		return new KarbonFactory();
	}
};

KInstance* KarbonFactory::s_instance = 0L;
KAboutData* KarbonFactory::s_aboutData = 0L;

KarbonFactory::KarbonFactory( QObject* parent, const char* name )
	: KoFactory( parent, name )
{
	instance();
}

KarbonFactory::~KarbonFactory()
{
	if ( s_instance )
	{
		delete s_instance;
		s_instance = 0L;
	}

	if ( s_aboutData )
	{
		delete s_aboutData;
		s_aboutData = 0L;
	}
}

KParts::Part*
KarbonFactory::createPartObject( QWidget* parentWidget, const char* widgetName,
	QObject* parent, const char* name, const char* classname, const QStringList& )
{
	// If classname is "KoDocument", our host is a koffice application
	// otherwise, the host wants us as a simple part, so switch to readonly and
 	// single view.
	bool bWantKoDocument = ( strcmp( classname, "KoDocument" ) == 0 );

	// parentWidget and widgetName are used by KoDocument for the
 	// "readonly+singleView" case.
	KarbonPart* part =
		new KarbonPart( parentWidget, widgetName, parent, name, !bWantKoDocument );

	if ( !bWantKoDocument )
	  part->setReadWrite( false );

	// Tell the factory base class that we created the object (mandatory)
	emit objectCreated( part );

	return part;
}

KAboutData*
KarbonFactory::aboutData()
{
	if ( !s_aboutData )
	{
		s_aboutData = new KAboutData(
			"karbon",
			I18N_NOOP( "Karbon14" ),
			"0.0.1",
			I18N_NOOP( "Yet another vector graphics application." ),
			KAboutData::License_GPL,
			I18N_NOOP( "(c) 2001, 2002 The Karbon Developers" ),
			I18N_NOOP( "You are invited to participate in any way." ),
			I18N_NOOP( "http://www.xs4all.nl/~rwlbuis/karbon/" ) );
		s_aboutData->addAuthor(
			"Lennart Kudling",
			0,
			"kudling@kde.org",
			0 );
		s_aboutData->addAuthor(
			"Rob Buis",
			0,
			"buis@kde.org",
			0 );
		s_aboutData->addAuthor(
			"Tomislav Lukman",
			0,
			"tomislav.lukman@ck.tel.hr",
			0 );
// TODO: add the names of some helpfull souls
	}
	return s_aboutData;
}

KInstance*
KarbonFactory::instance()
{
	if ( !s_instance )
	{
		s_instance = new KInstance( aboutData() );
		// Add any application-specific resource directories here

		// Tell the iconloader about share/apps/koffice/icons
		s_instance->iconLoader()->addAppDir( "koffice" );
	}
	return s_instance;
}

#include "karbon_factory.moc"
