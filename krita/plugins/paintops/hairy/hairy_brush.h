/*
 *  Copyright (c) 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef _HAIRY_BRUSH_H_
#define _HAIRY_BRUSH_H_

#include <QVector>
#include <QList>
#include <QTransform>

#include <KoColor.h>

#include "trajectory.h"
#include "bristle.h"

#include <kis_paint_device.h>
#include <kis_paint_information.h>
#include <kis_random_accessor.h>

class KoCompositeOp;

class KoColorSpace;

class KisHairyProperties{
public:
    quint16 radius;
    quint16 inkAmount;
    qreal sigma;
    QVector<qreal> inkDepletionCurve;
    bool inkDepletionEnabled;
    bool isbrushDimension1D;
    bool useMousePressure;
    bool useSaturation;
    bool useOpacity;
    bool useWeights;

    bool useSoakInk;
    bool connectedPath;
    bool antialias;
    bool useCompositing;
    
    quint8 pressureWeight;
    quint8 bristleLengthWeight;
    quint8 bristleInkAmountWeight;
    quint8 inkDepletionWeight;

    qreal shearFactor;
    qreal randomFactor;
    qreal scaleFactor;
    qreal threshold;
    
};

class HairyBrush
{

public:
    HairyBrush();
    ~HairyBrush();

    void paintLine(KisPaintDeviceSP dab, KisPaintDeviceSP layer, const KisPaintInformation &pi1, const KisPaintInformation &pi2, qreal scale, qreal rotation);
    /// set ink color for the whole bristle shape
    void setInkColor(const KoColor &color){ m_color = color; }
    /// set parameters for the brush engine
    void setProperties(KisHairyProperties * properties){ m_properties = properties; }
    /// set the shape of the bristles according the dab
    void fromDabWithDensity(KisFixedPaintDeviceSP dab, qreal density);

private:
    /// paints single bristle
    void addBristleInk(Bristle *bristle, QPointF pos, const KoColor &color);
    /// composite single pixel to dab
    void plotPixel(int wx, int wy, const KoColor &color);
    /// check the opacity of dab pixel and if the opacity is less then color, it will copy color to dab
    void darkenPixel(int wx, int wy, const KoColor &color);
    /// paint wu particle by copying the color and setup just the opacity, weight is complementary to opacity of the color
    void paintParticle(QPointF pos, const KoColor& color,qreal weight);
    /// paint wu particle using composite operation 
    void paintParticle(QPointF pos, const KoColor& color);
    /// similar to sample input color in spray
    void colorifyBristles(KisRandomConstAccessor& acc, KoColorSpace * cs, QPointF point);
    
    void repositionBristles(double angle, double slope);
    /// compute mouse pressure according distance
    double computeMousePressure(double distance);

    /// simulate running out of saturation 
    void saturationDepletion(Bristle * bristle, KoColor &bristleColor, qreal pressure, qreal inkDeplation);
    /// simulate running out of ink through opacity decreasing
    void opacityDepletion(Bristle * bristle, KoColor &bristleColor, qreal pressure, qreal inkDeplation);
    /// fetch actaul ink status according depletion curve
    qreal fetchInkDepletion(Bristle * bristle, int inkDepletionSize);

    void initAndCache();
    
private:
    const KisHairyProperties * m_properties;
    
    QVector<Bristle*> m_bristles;
    QTransform m_transform;
  
    // used for interpolation the path of bristles
    Trajectory m_trajectory;
    QHash<QString, QVariant> m_params;    
    // temporary device
    KisPaintDeviceSP m_dab;
    KisRandomAccessor * m_dabAccessor;
    const KoCompositeOp * m_compositeOp;
    quint32 m_pixelSize;

    int m_counter;

    double m_lastAngle;
    double m_lastSlope;

    double m_angle;
    double m_oldPressure;
    KoColor m_color;
    
    int m_saturationId;
    KoColorTransformation * m_transfo;
    
    // internal counter counts the calls of paint, the counter is 1 when the first call occurs
    inline bool firstStroke(){
        return (m_counter == 1);
    }
};

#endif
