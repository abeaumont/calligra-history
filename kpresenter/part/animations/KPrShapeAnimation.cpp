/* This file is part of the KDE project
 * Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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

#include "KPrShapeAnimation.h"
#include "KPrAnimationBase.h"
#include "kdebug.h"
KPrShapeAnimation::KPrShapeAnimation(KoShape *shape, KoTextBlockData *textBlockData)
: m_shape(shape)
, m_textBlockData(textBlockData)
{
}

KPrShapeAnimation::~KPrShapeAnimation()
{
}

bool KPrShapeAnimation::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    return false;
}

void KPrShapeAnimation::saveOdf(KoShapeSavingContext &context) const
{
}

KoShape * KPrShapeAnimation::shape() const
{
    return m_shape;
}

void KPrShapeAnimation::init(KPrAnimationCache *animationCache, int step)
{
    for (int i = 0; i < this->animationCount(); ++i) {
        QAbstractAnimation * animation = this->animationAt(i);
        if (KPrAnimationBase * a = dynamic_cast<KPrAnimationBase *>(animation)) {
            a->init(animationCache, step);
        }
    }
}

bool KPrShapeAnimation::visibilityChange()
{
    return true;
}

bool KPrShapeAnimation::visible()
{
    return true;
}

// we could have a loader that would put the data into the correct pos
// KPrShapeAnimation would get all the data it would need
// onClick would create a new animation
// when putting data in it could check if the shape is the correct one if not create a parallel one (with previous)