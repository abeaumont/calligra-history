/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_small_tiles_filter_plugin.h"

#include <kpluginfactory.h>

#include "kis_small_tiles_filter.h"
#include "kis_global.h"
#include "filter/kis_filter_registry.h"

K_PLUGIN_FACTORY(KisSmallTilesFilterPluginFactory, registerPlugin<KisSmallTilesFilterPlugin>();)
K_EXPORT_PLUGIN(KisSmallTilesFilterPluginFactory("krita"))

KisSmallTilesFilterPlugin::KisSmallTilesFilterPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    //setComponentData(KisSmallTilesFilterPluginFactory::componentData());
    KisFilterRegistry::instance()->add(new KisSmallTilesFilter());
}

KisSmallTilesFilterPlugin::~KisSmallTilesFilterPlugin()
{
}

