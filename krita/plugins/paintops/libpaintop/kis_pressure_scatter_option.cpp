/* 
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_pressure_scatter_option.h"

#include <klocale.h>

#include <kis_paint_device.h>
#include <widgets/kis_curve_widget.h>

#include <KoColor.h>
#include <KoColorSpace.h>

KisPressureScatterOption::KisPressureScatterOption()
        : KisCurveOption(i18n("Scatter"), "Scatter", KisPaintOpOption::brushCategory(), false)
{
    m_axisX = true;
    m_axisY = true;
    m_scatterAmount = 1.0;
}

void KisPressureScatterOption::enableAxisX(bool enable)
{
    m_axisX = enable;
}

void KisPressureScatterOption::enableAxisY(bool enable)
{
    m_axisY = enable;
}

bool KisPressureScatterOption::isAxisXEnabled()
{
    return m_axisX;
}

bool KisPressureScatterOption::isAxisYEnabled()
{
    return m_axisY;
}

void KisPressureScatterOption::setScatterAmount(qreal amount)
{
    m_scatterAmount = amount;
}

qreal KisPressureScatterOption::scatterAmount()
{
    return m_scatterAmount;
}


void KisPressureScatterOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty(SCATTER_X, m_axisX);
    setting->setProperty(SCATTER_Y, m_axisY);
    setting->setProperty(SCATTER_AMOUNT, m_scatterAmount);
}

void KisPressureScatterOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOption::readOptionSetting(setting);
    m_axisX = setting->getBool(SCATTER_X, true);
    m_axisY = setting->getBool(SCATTER_Y, true);
    m_scatterAmount = setting->getDouble(SCATTER_AMOUNT, 1.0);
}

QPointF KisPressureScatterOption::apply(const KisPaintInformation& info, qreal diameter) const
{
    if ((!m_axisX && !m_axisY) || (!isChecked())) {
        return info.pos();
    }
    
    qreal sensorValue = computeValue(info);

    qreal jitter = (2.0 * drand48() - 1.0) * diameter * m_scatterAmount * sensorValue;
    QPointF result(0.0, 0.0);

    if (m_axisX && m_axisY){
        qreal jitterY = (2.0 * drand48() - 1.0) * diameter * m_scatterAmount * sensorValue;
        result = QPointF(jitter, jitterY);
        return info.pos() + result;
    }
    
    QVector2D movement = toQVector2D( info.movement() ).normalized();
    if (m_axisX)
    {
        movement *= jitter;
        result = movement.toPointF();
        
    }
    else if (m_axisY)
    {
        QVector2D movementNormal( -movement.y(), movement.x() );    
        movementNormal *= jitter;
        result = movementNormal.toPointF();
    }

    
    return info.pos() + result;
}






