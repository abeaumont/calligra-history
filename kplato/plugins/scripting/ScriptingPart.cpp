/*
 * This file is part of KPlato
 *
 * Copyright (c) 2006 Sebastian Sauer <mail@dipe.org>
 * Copyright (c) 2008 Dag Andersen <kplato@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "ScriptingPart.h"
#include "Module.h"

#include <klocale.h>
#include <kpluginfactory.h>
#include <kstandarddirs.h>
#include <kactioncollection.h>
#include <kparts/partmanager.h>
// kross
#include <kross/core/manager.h>
#include <kross/core/interpreter.h>
#include <kross/core/action.h>
#include <kross/core/actioncollection.h>
#include <kross/ui/model.h>
// koffice
#include <KoDockRegistry.h>
#include <KoMainWindow.h>
#include <KoDocument.h>

#include <kptview.h>

int kplatoScriptingDebugArea() {
#if KDE_IS_VERSION( 4, 3, 80 )
    static int s_area = KDebug::registerArea( "kplato (Scripting)" );
#else
    static int s_area = 32010;
#endif
    return s_area;
}

K_EXPORT_PLUGIN( KPlatoScriptingFactory )

KPlatoScriptingFactory::KPlatoScriptingFactory(const char *componentName, const char *catalogName, QObject *parent )
    : KPluginFactory( componentName, catalogName, parent )
{
    kDebug(kplatoScriptingDebugArea())<<parent;
}

QObject *KPlatoScriptingFactory::create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword)
{
    kDebug(kplatoScriptingDebugArea())<<iface<<parentWidget<<parent<<args<<keyword;
    return new KPlatoScriptingPart( parent );
}

//---------------------
/// \internal d-pointer class.
class KPlatoScriptingPart::Private
{
    public:
};

KPlatoScriptingPart::KPlatoScriptingPart(QObject* parent, const QStringList& args)
    : KoScriptingPart(new Scripting::Module(parent), args)
    , d(new Private())
{
    setComponentData(KPlatoScriptingPart::componentData());
    setXMLFile(KStandardDirs::locate("data","kplato/kpartplugins/scripting.rc"), true);
    kDebug(kplatoScriptingDebugArea()) <<"KPlatoScripting plugin. Class:" << metaObject()->className() <<", Parent:" <<(parent?parent->metaObject()->className():"0");

}

KPlatoScriptingPart::~KPlatoScriptingPart()
{
    delete d;
}

#include "ScriptingPart.moc"
