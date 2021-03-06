/*
 *  Copyright (c) 2004,2007-2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_auto_brush.h"

#include <kis_debug.h>
#include <math.h>

#include <QDomElement>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_datamanager.h"
#include "kis_paint_device.h"

#include "kis_mask_generator.h"
#include "kis_boundary.h"

struct KisAutoBrush::Private {
    KisMaskGenerator* shape;
    qreal randomness;
    qreal density;
    mutable QVector<quint8> precomputedQuarter;
};

KisAutoBrush::KisAutoBrush(KisMaskGenerator* as, qreal angle, qreal randomness, qreal density)
        : KisBrush()
        , d(new Private)
{
    d->shape = as;
    d->randomness = randomness;
    d->density = density;
    setBrushType(MASK);
    setWidth(d->shape->width());
    setHeight(d->shape->height());
    setAngle(angle);
    QImage image = createBrushPreview();
    setImage(image);
}

KisAutoBrush::~KisAutoBrush()
{
    delete d->shape;
    delete d;
}



void KisAutoBrush::generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst,
        KisBrush::ColoringInformation* coloringInformation,
        double scaleX, double scaleY, double angle,
        const KisPaintInformation& info,
        double subPixelX , double subPixelY, qreal softnessFactor) const
{
    Q_UNUSED(info);

    // Generate the paint device from the mask
    const KoColorSpace* cs = dst->colorSpace();
    quint32 pixelSize = cs->pixelSize();
    
    // mask dimension methods already includes KisBrush::angle()
    int dstWidth = maskWidth(scaleX, angle); 
    int dstHeight = maskHeight(scaleY, angle);
    
    angle += KisBrush::angle();
    
    // if there's coloring information, we merely change the alpha: in that case,
    // the dab should be big enough!
    if (coloringInformation) {

        // old bounds
        QRect oldBounds = dst->bounds();

        // new bounds. we don't care if there is some extra memory occcupied.
        dst->setRect(QRect(0, 0, dstWidth, dstHeight));

        if (dstWidth * dstHeight <= oldBounds.width() * oldBounds.height()) {
            // just clear the data in dst,
            memset(dst->data(), OPACITY_TRANSPARENT_U8, dstWidth * dstHeight * dst->pixelSize());
        } else {
            // enlarge the data
            dst->initialize();
        }
    } else {
        if (dst->data() == 0 || dst->bounds().isEmpty()) {
            qWarning() << "Creating a default black dab: no coloring info and no initialized paint device to mask";
            dst->clear(QRect(0, 0, dstWidth, dstHeight));
        }
        Q_ASSERT(dst->bounds().width() >= dstWidth && dst->bounds().height() >= dstHeight);
    }

    quint8* dabPointer = dst->data();

    quint8* color = 0;
    if (coloringInformation) {
        if (dynamic_cast<PlainColoringInformation*>(coloringInformation)) {
            color = const_cast<quint8*>(coloringInformation->color());
        }
    } else {
        // Mask everything out
        cs->setOpacity(dst->data(), OPACITY_TRANSPARENT_U8, dst->bounds().width() * dst->bounds().height());
    }

    int rowWidth = dst->bounds().width();

    double invScaleX = 1.0 / scaleX;
    double invScaleY = 1.0 / scaleY;

    double centerX = dstWidth  * 0.5 - 0.5 + subPixelX;
    double centerY = dstHeight * 0.5 - 0.5 + subPixelY;

    // the results differ, sometimes this code is faster, sometimes it is not
    // more investigation is probably needed
    // valueAt is costly similary to interpolation is some cases
    if (false && isBrushSymmetric(angle) && (dynamic_cast<PlainColoringInformation*>(coloringInformation))){
        // round eg. 14.3 to 15 so that we can interpolate
        // we have to add one pixel because of subpixel precision (see the centerX, centerY computation) 
        // and add one pixel because of interpolation
        int halfWidth = qRound((dstWidth - centerX) ) + 2;
        int halfHeight = qRound((dstHeight - centerY) ) + 2;
        
        int size = halfWidth * halfHeight;
        if (d->precomputedQuarter.size() != size)
        {
            d->precomputedQuarter.resize(size);
        }
        
        // precompute the table for interpolation 
        int pos = 0;
        d->shape->setSoftness(softnessFactor);
        for (int y = 0; y < halfHeight; y++){
            for (int x = 0; x < halfWidth; x++, pos++){
                double maskX = x * invScaleX;
                double maskY = y * invScaleY;
                d->precomputedQuarter[pos] = d->shape->valueAt(maskX, maskY);
            }
        }
        
        qreal random = 1.0;
        quint8 alphaValue = OPACITY_TRANSPARENT_U8;
        for (int y = 0; y < dstHeight; y++) {
            for (int x = 0; x < dstWidth; x++) {

                double maskX = (x - centerX);
                double maskY = (y - centerY);

                if (coloringInformation) {
                    if (color) {
                        memcpy(dabPointer, color, pixelSize);
                    } else {
                        memcpy(dabPointer, coloringInformation->color(), pixelSize);
                        coloringInformation->nextColumn();
                    }
                }
                if (d->randomness != 0.0){
                    random = (1.0 - d->randomness) + d->randomness * qreal(rand()) / RAND_MAX;
                }
                
                alphaValue = quint8( ( OPACITY_OPAQUE_U8 - interpolatedValueAt(maskX, maskY,d->precomputedQuarter,halfWidth) ) * random);
                if (d->density != 1.0){
                    // compute density only for visible pixels of the mask
                    if (alphaValue != OPACITY_TRANSPARENT_U8){
                        if ( !(d->density >= drand48()) ){
                            alphaValue = OPACITY_TRANSPARENT_U8;
                        }
                    }
                }   

                cs->setOpacity(dabPointer, alphaValue, 1);
                dabPointer += pixelSize;

            }//endfor x
            //printf("\n");

            if (!color && coloringInformation) {
                coloringInformation->nextRow();
            }
            //TODO: this never happens probably? 
            if (dstWidth < rowWidth) {
                dabPointer += (pixelSize * (rowWidth - dstWidth));
            }
            
        }//endfor y
        
    }
    else
    {
        double cosa = cos(angle);
        double sina = sin(angle);
        
        qreal random = 1.0;
        quint8 alphaValue = OPACITY_TRANSPARENT_U8;
        d->shape->setSoftness( softnessFactor );
        for (int y = 0; y < dstHeight; y++) {
            for (int x = 0; x < dstWidth; x++) {

                double x_ = (x - centerX) * invScaleX;
                double y_ = (y - centerY) * invScaleY;
                double maskX = cosa * x_ - sina * y_;
                double maskY = sina * x_ + cosa * y_;

                if (coloringInformation) {
                    if (color) {
                        memcpy(dabPointer, color, pixelSize);
                    } else {
                        memcpy(dabPointer, coloringInformation->color(), pixelSize);
                        coloringInformation->nextColumn();
                    }
                }
                
                if (d->randomness != 0.0){
                    random = (1.0 - d->randomness) + d->randomness * float(rand()) / RAND_MAX;
                }
                
                alphaValue = quint8( (OPACITY_OPAQUE_U8 - d->shape->valueAt(maskX, maskY)) * random);

                // avoid computation of random numbers if density is full
                if (d->density != 1.0){
                    // compute density only for visible pixels of the mask
                    if (alphaValue != OPACITY_TRANSPARENT_U8){
                        if ( !(d->density >= drand48()) ){
                            alphaValue = OPACITY_TRANSPARENT_U8;
                        }
                    }
                }   

                cs->setOpacity(dabPointer, alphaValue, 1);
                dabPointer += pixelSize;
            }//endfor x
            if (!color && coloringInformation) {
                coloringInformation->nextRow();
            }
            //TODO: this never happens probably? 
            if (dstWidth < rowWidth) {
                dabPointer += (pixelSize * (rowWidth - dstWidth));
            }
        }//endfor y
    }//else 
}


void KisAutoBrush::toXML(QDomDocument& doc, QDomElement& e) const
{
    QDomElement shapeElt = doc.createElement("MaskGenerator");
    d->shape->toXML(doc, shapeElt);
    e.appendChild(shapeElt);
    e.setAttribute("type", "auto_brush");
    e.setAttribute("spacing", spacing());
    e.setAttribute("angle", KisBrush::angle());
    e.setAttribute("randomness", d->randomness);
    e.setAttribute("density", d->density);
}

QImage KisAutoBrush::createBrushPreview()
{
    srand(0);
    srand48(0);
    int width = maskWidth(1.0, 0.0);
    int height = maskHeight(1.0, 0.0);
    
    KisPaintInformation info(QPointF(width * 0.5, height * 0.5), 0.5, 0, 0, KisVector2D::Zero(), 0, 0);

    KisFixedPaintDeviceSP fdev = new KisFixedPaintDevice( KoColorSpaceRegistry::instance()->rgb8() );
    fdev->setRect(QRect(0, 0, width, height));
    fdev->initialize();

    mask(fdev,KoColor(Qt::black, fdev->colorSpace()),1.0, 1.0, 0.0, info);
    return fdev->convertToQImage();
}


const KisMaskGenerator* KisAutoBrush::maskGenerator() const
{
    return d->shape;
}

qreal KisAutoBrush::density() const
{
    return d->density;
}

qreal KisAutoBrush::randomness() const
{
    return d->randomness;
}


bool KisAutoBrush::isBrushSymmetric(double angle) const
{       
    // small brushes compute directly   
    if (d->shape->height() < 3 ) return false;          
    // even spikes are symmetric        
    if ((d->shape->spikes() % 2) != 0) return false;    
    // main condition, if not rotated or use optimization for rotated circles - rotated circle is circle again          
    if ( angle == 0.0 || ( ( d->shape->type() == KisMaskGenerator::CIRCLE ) && ( d->shape->width() == d->shape->height() ) ) ) return true;     
    // in other case return false       
    return false;
}


quint8 KisAutoBrush::interpolatedValueAt(double x, double y,const QVector<quint8> &precomputedQuarter,int width) const 
{
    x = qAbs(x);
    y = qAbs(y);
    
    double x_i = floor(x);         
    double x_f = x - x_i;       
    double x_f_r = 1.0 - x_f;   

    double y_i = floor(y);      
    double y_f = fabs(y - y_i);         
    double y_f_r = 1.0 - y_f;   
    
    return (x_f_r * y_f_r * valueAt(x_i , y_i, precomputedQuarter, width) +        
            x_f   * y_f_r * valueAt(x_i + 1, y_i, precomputedQuarter, width) +     
            x_f_r * y_f   * valueAt(x_i,  y_i + 1, precomputedQuarter, width) +    
            x_f   * y_f   * valueAt(x_i + 1,  y_i + 1, precomputedQuarter, width));
}


void KisAutoBrush::setImage(const QImage& image)
{
    m_image = image;
    clearScaledBrushes();
}

QPainterPath KisAutoBrush::outline() const
{
    bool simpleOutline = (d->density < 1.0);
    if (simpleOutline){
        QPainterPath path;
        QRectF brushBoundingbox(0,0,width(), height());
        if (maskGenerator()->type() == KisMaskGenerator::CIRCLE){
            path.addEllipse(brushBoundingbox);
        }else // if (maskGenerator()->type() == KisMaskGenerator::RECTANGLE)
        {
            path.addRect(brushBoundingbox);
        }
        
        return path;
    }
    
    return KisBrush::boundary()->path();
}
