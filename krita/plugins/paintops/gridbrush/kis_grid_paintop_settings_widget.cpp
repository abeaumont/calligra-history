/*
 *  Copyright (c) 2008,2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_grid_paintop_settings_widget.h"

#include "kis_gridop_option.h"
#include "kis_grid_paintop_settings.h"
#include "kis_grid_shape_option.h"
#include "kis_grid_color_option.h"

#include <kis_paintop_options_widget.h>
#include <kis_paint_action_type_option.h>

KisGridPaintOpSettingsWidget:: KisGridPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpOptionsWidget(parent)
{
    m_gridOption =  new KisGridOpOption();
    m_gridShapeOption = new KisGridShapeOption();
    m_gridColorOption = new KisGridColorOption();
    m_paintActionTypeOption = new KisPaintActionTypeOption();

    addPaintOpOption(m_gridOption);
    addPaintOpOption(m_gridShapeOption);
    addPaintOpOption(m_gridColorOption);
    addPaintOpOption(m_paintActionTypeOption);
}

KisGridPaintOpSettingsWidget::~ KisGridPaintOpSettingsWidget()
{
    delete m_gridOption;
    delete m_gridShapeOption;
    delete m_gridColorOption;
    delete m_paintActionTypeOption;
}

KisPropertiesConfiguration*  KisGridPaintOpSettingsWidget::configuration() const
{
    KisGridPaintOpSettings* config = new KisGridPaintOpSettings();
    config->setOptionsWidget( const_cast<KisGridPaintOpSettingsWidget*>( this ) );
    config->setProperty("paintop", "gridbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}