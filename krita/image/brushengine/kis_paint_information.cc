/*
 *  Copyright (c) 2007,2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_paint_information.h"
#include <QDomElement>

struct KisPaintInformation::Private {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    QPointF pos;
    qreal pressure;
    qreal xTilt;
    qreal yTilt;
    KisVector2D movement;
    qreal angle;
    qreal rotation;
    qreal tangentialPressure;
    int time;
};

KisPaintInformation::KisPaintInformation(const QPointF & pos_, qreal pressure_,
        qreal xTilt_, qreal yTilt_,
        const KisVector2D& movement_,
        qreal rotation_,
        qreal tangentialPressure_,
        int time)
        : d(new Private)
{
    d->pos = pos_;
    d->pressure = pressure_;
    d->xTilt = xTilt_;
    d->yTilt = yTilt_;
    d->movement = movement_;
    d->rotation = rotation_;
    d->tangentialPressure = tangentialPressure_;
    d->angle = atan2(movement_.y(), movement_.x());
    d->time = time;
}

KisPaintInformation::KisPaintInformation(const KisPaintInformation& rhs) : d(new Private(*rhs.d))
{
}

void KisPaintInformation::operator=(const KisPaintInformation & rhs)
{
    *d = *rhs.d;
}

KisPaintInformation::~KisPaintInformation()
{
    delete d;
}


void KisPaintInformation::toXML(QDomDocument&, QDomElement& e) const
{
    e.setAttribute("pointX", QString::number(pos().x(), 'g', 15));
    e.setAttribute("pointY", QString::number(pos().y(), 'g', 15));
    e.setAttribute("pressure", QString::number(pressure(), 'g', 15));
    e.setAttribute("xTilt", QString::number(xTilt(), 'g', 15));
    e.setAttribute("yTilt", QString::number(yTilt(), 'g', 15));
    e.setAttribute("movementX", QString::number(movement().x(), 'g', 15));
    e.setAttribute("movementY", QString::number(movement().y(), 'g', 15));
    e.setAttribute("rotation", QString::number(rotation(), 'g', 15));
    e.setAttribute("tangentialPressure", QString::number(tangentialPressure(), 'g', 15));
    e.setAttribute("time", d->time);
}

KisPaintInformation KisPaintInformation::fromXML(const QDomElement& e)
{
    qreal pointX = qreal(e.attribute("pointX", "0.0").toDouble());
    qreal pointY = qreal(e.attribute("pointY", "0.0").toDouble());
    qreal pressure = qreal(e.attribute("pressure", "0.0").toDouble());
    qreal rotation = qreal(e.attribute("rotation", "0.0").toDouble());
    qreal tangentialPressure = qreal(e.attribute("tangentialPressure", "0.0").toDouble());
    qreal xTilt = qreal(e.attribute("xTilt", "0.0").toDouble());
    qreal yTilt = qreal(e.attribute("yTilt", "0.0").toDouble());
    qreal movementX = qreal(e.attribute("movementX", "0.0").toDouble());
    qreal movementY = qreal(e.attribute("movementY", "0.0").toDouble());
    int time = e.attribute("time", "0").toInt();

    return KisPaintInformation(QPointF(pointX, pointY), pressure, xTilt, yTilt, KisVector2D(movementX, movementY),
                               rotation, tangentialPressure, time);
}

const QPointF& KisPaintInformation::pos() const
{
    return d->pos;
}

void KisPaintInformation::setPos(const QPointF& p)
{
    d->pos = p;
}

qreal KisPaintInformation::pressure() const
{
    return d->pressure;
}

void KisPaintInformation::setPressure(qreal p)
{
    d->pressure = p;
}

qreal KisPaintInformation::xTilt() const
{
    return d->xTilt;
}

qreal KisPaintInformation::yTilt() const
{
    return d->yTilt;
}

KisVector2D KisPaintInformation::movement() const
{
    return d->movement;
}

qreal KisPaintInformation::angle() const
{
    return d->angle;
}

qreal KisPaintInformation::rotation() const
{
    return d->rotation;
}

qreal KisPaintInformation::tangentialPressure() const
{
    return d->tangentialPressure;
}

int KisPaintInformation::currentTime() const
{
    return d->time;
}

QDebug operator<<(QDebug dbg, const KisPaintInformation &info)
{
#ifdef NDEBUG
    Q_UNUSED(info);
#else
    dbg.nospace() << "Position: " << info.pos();
    dbg.nospace() << ", Pressure: " << info.pressure();
    dbg.nospace() << ", X Tilt: " << info.xTilt();
    dbg.nospace() << ", Y Tilt: " << info.yTilt();
    dbg.nospace() << ", Movement: " << toQPointF(info.movement());
    dbg.nospace() << ", Rotation: " << info.rotation();
    dbg.nospace() << ", Tangential Pressure: " << info.tangentialPressure();
    dbg.nospace() << ", Angle: " << info.angle();
    dbg.nospace() << ", Time: " << info.currentTime();
#endif
    return dbg.space();
}

KisPaintInformation KisPaintInformation::mix(const QPointF& p, qreal t, const KisPaintInformation& pi1, const KisPaintInformation& pi2, const KisVector2D& movement)
{
    qreal pressure = (1 - t) * pi1.pressure() + t * pi2.pressure();
    qreal xTilt = (1 - t) * pi1.xTilt() + t * pi2.xTilt();
    qreal yTilt = (1 - t) * pi1.yTilt() + t * pi2.yTilt();
    qreal rotation = (1 - t) * pi1.rotation() + t * pi2.rotation();
    qreal tangentialPressure = (1 - t) * pi1.tangentialPressure() + t * pi2.tangentialPressure();
    int time = (1 - t) * pi1.currentTime() + t * pi2.currentTime();
    return KisPaintInformation(p, pressure, xTilt, yTilt, movement, rotation, tangentialPressure, time);
}
