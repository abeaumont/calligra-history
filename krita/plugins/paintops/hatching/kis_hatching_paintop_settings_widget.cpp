/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_hatching_paintop_settings_widget.h"

#include "kis_hatchingop_option.h"
#include "kis_hatching_paintop_settings.h"

#include "kis_newhatchingoptions.h"

#include <kis_curve_option_widget.h>
#include <kis_pressure_opacity_option.h>

#include <kis_paintop_options_widget.h>
#include <kis_paint_action_type_option.h>

KisHatchingPaintOpSettingsWidget:: KisHatchingPaintOpSettingsWidget(QWidget* parent)
        : KisPaintOpOptionsWidget(parent)
{
    m_hatchingOption = new KisHatchingOpOption();

    addPaintOpOption(new KisHatchingOpOption());
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption()));
    addPaintOpOption(new KisPaintActionTypeOption());
    
    //propio
    m_hatchingNewOption = new KisNewHatchingOptions();
    addPaintOpOption(m_hatchingNewOption);
}

KisHatchingPaintOpSettingsWidget::~ KisHatchingPaintOpSettingsWidget()
{
}

KisPropertiesConfiguration*  KisHatchingPaintOpSettingsWidget::configuration() const
{
    KisHatchingPaintOpSettings* config = new KisHatchingPaintOpSettings();
    config->setOptionsWidget(const_cast<KisHatchingPaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "hatchingbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

void KisHatchingPaintOpSettingsWidget::changePaintOpSize(qreal x, qreal y)
{
    // if the movement is more left<->right then up<->down
    if (qAbs(x) > qAbs(y)){
        // TODO: change width and height of the dab?
    }
    else // vice-versa
    {
        // we can do something different
    }

}