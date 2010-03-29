/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_AIRBRUSHOP_SETTINGS_WIDGET_H_
#define KIS_AIRBRUSHOP_SETTINGS_WIDGET_H_

#include <kis_paintop_options_widget.h>
#include <kis_image.h>

class KisBrushOption;
class KisPressureOpacityOption;
class KisPressureDarkenOption;
class KisPressureSizeOption;
class KisPaintActionTypeOption;

class KisAirbrushOpSettingsWidget : public KisPaintOpOptionsWidget
{

    Q_OBJECT

public:

    KisAirbrushOpSettingsWidget(QWidget* parent = 0);

    ~KisAirbrushOpSettingsWidget();

    KisPropertiesConfiguration* configuration() const;

    void setImage(KisImageWSP image);

public:

    KisBrushOption * m_brushOption;
    // XXX: Add a rate option?
};



#endif // KIS_AIRBRUSHOP_SETTINGS_WIDGET_H_