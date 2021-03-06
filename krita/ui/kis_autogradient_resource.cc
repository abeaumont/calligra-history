/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2004 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_autogradient_resource.h"

#include "KoColorSpaceRegistry.h"
#include "KoColorSpace.h"

// XXX: use the same #define as in kis_gradient.cc, probably best customizable?
#define PREVIEW_WIDTH 64
#define PREVIEW_HEIGHT 64


void KisAutogradientResource::createSegment(int interpolation, int colorInterpolation, double startOffset, double endOffset, double middleOffset, const QColor & left, const QColor & right)
{
    pushSegment(new KoGradientSegment(interpolation, colorInterpolation, startOffset, middleOffset, endOffset, KoColor(left, colorSpace()), KoColor(right, colorSpace())));

}

const QList<double> KisAutogradientResource::getHandlePositions() const
{
    QList<double> handlePositions;

    handlePositions.push_back(m_segments[0]->startOffset());
    for (int i = 0; i < m_segments.count(); i++) {
        handlePositions.push_back(m_segments[i]->endOffset());
    }
    return handlePositions;
}

const QList<double> KisAutogradientResource::getMiddleHandlePositions() const
{
    QList<double> middleHandlePositions;

    for (int i = 0; i < m_segments.count(); i++) {
        middleHandlePositions.push_back(m_segments[i]->middleOffset());
    }
    return middleHandlePositions;
}

void KisAutogradientResource::moveSegmentStartOffset(KoGradientSegment* segment, double t)
{
    QList<KoGradientSegment*>::iterator it = qFind(m_segments.begin(), m_segments.end(), segment);
    if (it != m_segments.end()) {
        if (it == m_segments.begin()) {
            segment->setStartOffset(0.0);
            return;
        }
        KoGradientSegment* previousSegment = (*(it - 1));
        if (t > segment->startOffset()) {
            if (t > segment->middleOffset())
                t = segment->middleOffset();
        } else {
            if (t < previousSegment->middleOffset())
                t = previousSegment->middleOffset();
        }
        previousSegment->setEndOffset(t);
        segment->setStartOffset(t);
    }
}

void KisAutogradientResource::moveSegmentEndOffset(KoGradientSegment* segment, double t)
{
    QList<KoGradientSegment*>::iterator it = qFind(m_segments.begin(), m_segments.end(), segment);
    if (it != m_segments.end()) {
        if (it + 1 == m_segments.end()) {
            segment->setEndOffset(1.0);
            return;
        }
        KoGradientSegment* followingSegment = (*(it + 1));
        if (t < segment->endOffset()) {
            if (t < segment->middleOffset())
                t = segment->middleOffset();
        } else {
            if (t > followingSegment->middleOffset())
                t = followingSegment->middleOffset();
        }
        followingSegment->setStartOffset(t);
        segment->setEndOffset(t);
    }
}

void KisAutogradientResource::moveSegmentMiddleOffset(KoGradientSegment* segment, double t)
{
    if (segment) {
        if (t > segment->endOffset())
            segment->setMiddleOffset(segment->endOffset());
        else if (t < segment->startOffset())
            segment->setMiddleOffset(segment->startOffset());
        else
            segment->setMiddleOffset(t);
    }
}

void KisAutogradientResource::splitSegment(KoGradientSegment* segment)
{
    Q_ASSERT(segment != 0);
    QList<KoGradientSegment*>::iterator it = qFind(m_segments.begin(), m_segments.end(), segment);
    if (it != m_segments.end()) {
        KoColor midleoffsetColor(segment->endColor().colorSpace());
        segment->colorAt(midleoffsetColor, segment->middleOffset());
        KoGradientSegment* newSegment = new KoGradientSegment(
            segment->interpolation(), segment->colorInterpolation(),
            segment ->startOffset(),
            (segment->middleOffset() - segment->startOffset()) / 2 + segment->startOffset(),
            segment->middleOffset(),
            segment->startColor(),
            midleoffsetColor);
        m_segments.insert(it, newSegment);
        segment->setStartColor(midleoffsetColor);
        segment->setStartOffset(segment->middleOffset());
        segment->setMiddleOffset((segment->endOffset() - segment->startOffset()) / 2 + segment->startOffset());
    }
}

void KisAutogradientResource::duplicateSegment(KoGradientSegment* segment)
{
    Q_ASSERT(segment != 0);
    QList<KoGradientSegment*>::iterator it = qFind(m_segments.begin(), m_segments.end(), segment);
    if (it != m_segments.end()) {
        double middlePostionPercentage = (segment->middleOffset() - segment->startOffset()) / segment->length();
        double center = segment->startOffset() + segment->length() / 2;
        KoGradientSegment* newSegment = new KoGradientSegment(
            segment->interpolation(), segment->colorInterpolation(),
            segment ->startOffset(),
            segment->length() / 2 * middlePostionPercentage + segment->startOffset(),
            center, segment->startColor(),
            segment->endColor());
        m_segments.insert(it, newSegment);
        segment->setStartOffset(center);
        segment->setMiddleOffset(segment->length() * middlePostionPercentage  + segment->startOffset());
    }
}

void KisAutogradientResource::mirrorSegment(KoGradientSegment* segment)
{
    Q_ASSERT(segment != 0);
    KoColor tmpColor = segment->startColor();
    segment->setStartColor(segment->endColor());
    segment->setEndColor(tmpColor);
    segment->setMiddleOffset(segment->endOffset() - (segment->middleOffset() - segment->startOffset()));

    if (segment->interpolation() == INTERP_SPHERE_INCREASING)
        segment->setInterpolation(INTERP_SPHERE_DECREASING);
    else if (segment->interpolation() == INTERP_SPHERE_DECREASING)
        segment->setInterpolation(INTERP_SPHERE_INCREASING);

    if (segment->colorInterpolation() == COLOR_INTERP_HSV_CW)
        segment->setColorInterpolation(COLOR_INTERP_HSV_CCW);
    else if (segment->colorInterpolation() == COLOR_INTERP_HSV_CCW)
        segment->setColorInterpolation(COLOR_INTERP_HSV_CW);
}

KoGradientSegment* KisAutogradientResource::removeSegment(KoGradientSegment* segment)
{
    Q_ASSERT(segment != 0);
    if (m_segments.count() < 2)
        return 0;
    QList<KoGradientSegment*>::iterator it = qFind(m_segments.begin(), m_segments.end(), segment);
    if (it != m_segments.end()) {
        double middlePostionPercentage;
        KoGradientSegment* nextSegment;
        if (it == m_segments.begin()) {
            nextSegment = (*(it + 1));
            middlePostionPercentage = (nextSegment->middleOffset() - nextSegment->startOffset()) / nextSegment->length();
            nextSegment->setStartOffset(segment->startOffset());
            nextSegment->setMiddleOffset(middlePostionPercentage * nextSegment->length() + nextSegment->startOffset());
        } else {
            nextSegment = (*(it - 1));
            middlePostionPercentage = (nextSegment->middleOffset() - nextSegment->startOffset()) / nextSegment->length();
            nextSegment->setEndOffset(segment->endOffset());
            nextSegment->setMiddleOffset(middlePostionPercentage * nextSegment->length() + nextSegment->startOffset());
        }

        delete segment;
        m_segments.erase(it);
        return nextSegment;
    }
    return 0;
}

bool KisAutogradientResource::removeSegmentPossible() const
{
    if (m_segments.count() < 2)
        return false;
    return true;
}
