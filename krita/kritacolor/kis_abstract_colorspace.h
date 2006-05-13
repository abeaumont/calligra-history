/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
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
#ifndef KIS_ABSTRACT_COLORSPACE_H_
#define KIS_ABSTRACT_COLORSPACE_H_

#include <QMap>
#include <QColor>
#include <QStringList>
#include <QPair>
//Added by qt3to4:
#include <Q3MemArray>

#include "kis_global.h"
#include "kis_channelinfo.h"
#include "kis_profile.h"
#include "kis_id.h"
#include "kis_composite_op.h"
#include "kis_colorspace.h"

class QPainter;
class KisPixelRO;
class KisColorSpaceFactoryRegistry;


/**
 * A colorspace strategy is the definition of a certain color model
 * in Krita.
 */
class KRITACOLOR_EXPORT KisAbstractColorSpace : public KisColorSpace {


public:

    /**
     * @param id The unique human and machine readable identifiation of this colorspace
     * @param cmType the lcms type indentification for this colorspace, may be 0
     * @param colorSpaceSignature the icc identification for this colorspace, may be 0
     * @param parent the registry that owns this instance
     * @param profile the profile this colorspace uses for transforms
     */
    KisAbstractColorSpace(const KisID & id,
                          DWORD cmType,
                          icColorSpaceSignature colorSpaceSignature,
                          KisColorSpaceFactoryRegistry * parent,
                          KisProfile *profile);

    void init();

    virtual ~KisAbstractColorSpace();

    virtual bool operator==(const KisAbstractColorSpace& rhs) const {
        return (m_id == rhs.m_id && m_profile == rhs.m_profile);
    }


//================== Information about this color strategy ========================//

public:


    //========== Channels =====================================================//

    // Return a vector describing all the channels this color model has.
    virtual Q3ValueVector<KisChannelInfo *> channels() const = 0;

    virtual quint32 nChannels() const = 0;

    virtual quint32 nColorChannels() const = 0;

    virtual quint32 nSubstanceChannels() const { return 0; };

    virtual quint32 pixelSize() const = 0;

    virtual QString channelValueText(const quint8 *pixel, quint32 channelIndex) const = 0;

    virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const = 0;

    virtual quint8 scaleToU8(const quint8 * srcPixel, qint32 channelPos) = 0;

    virtual quint16 scaleToU16(const quint8 * srcPixel, qint32 channelPos) = 0;

    virtual void getSingleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex);

    //========== Identification ===============================================//

    virtual KisID id() const { return m_id; }

    void setColorSpaceType(quint32 type) { m_cmType = type; }
    quint32 colorSpaceType() { return m_cmType; }

    virtual icColorSpaceSignature colorSpaceSignature() { return m_colorSpaceSignature; }

    //========== Capabilities =================================================//

    virtual KisCompositeOpList userVisiblecompositeOps() const = 0;

    /**
     * Returns true if the colorspace supports channel values outside the
     * (normalised) range 0 to 1.
     */
    virtual bool hasHighDynamicRange() const { return false; }

    //========== Display profiles =============================================//

    virtual KisProfile * getProfile() { return m_profile; };


//================= Conversion functions ==================================//


    virtual void fromQColor(const QColor& c, quint8 *dst, KisProfile * profile = 0);
    virtual void fromQColor(const QColor& c, quint8 opacity, quint8 *dst, KisProfile * profile = 0);

    virtual void toQColor(const quint8 *src, QColor *c, KisProfile * profile = 0);
    virtual void toQColor(const quint8 *src, QColor *c, quint8 *opacity, KisProfile * profile = 0);

    virtual QImage convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                   KisProfile *  dstProfile,
                                   qint32 renderingIntent = INTENT_PERCEPTUAL,
                                   float exposure = 0.0f);

    virtual bool convertPixelsTo(const quint8 * src,
                                 quint8 * dst, KisColorSpace * dstColorSpace,
                                 quint32 numPixels,
                                 qint32 renderingIntent = INTENT_PERCEPTUAL);

//============================== Manipulation fucntions ==========================//


//
// The manipulation functions have default implementations that _convert_ the pixel
// to a QColor and back. Reimplement these methods in your color strategy!
//
    virtual KisColorAdjustment *createBrightnessContrastAdjustment(quint16 *transferValues);

    virtual KisColorAdjustment *createDesaturateAdjustment();

    virtual KisColorAdjustment *createPerChannelAdjustment(quint16 **transferValues);

    virtual void applyAdjustment(const quint8 *src, quint8 *dst, KisColorAdjustment *, qint32 nPixels);

    virtual void invertColor(quint8 * src, qint32 nPixels);

    virtual quint8 difference(const quint8* src1, const quint8* src2);

    virtual void mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const;

    virtual void convolveColors(quint8** colors, qint32* kernelValues, KisChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nPixels) const;

    virtual void darken(const quint8 * src, quint8 * dst, qint32 shade, bool compensate, double compensation, qint32 nPixels) const;

    virtual quint8 intensity8(const quint8 * src) const;

    virtual KisID mathToolboxID() const;

    virtual void bitBlt(quint8 *dst,
                qint32 dststride,
                KisColorSpace * srcSpace,
                const quint8 *src,
                qint32 srcRowStride,
                const quint8 *srcAlphaMask,
                qint32 maskRowStride,
                quint8 opacity,
                qint32 rows,
                qint32 cols,
                const KisCompositeOp& op);

//========================== END of Public API ========================================//

protected:


    /**
     * Compose two byte arrays containing pixels in the same color
     * model together.
     */
    virtual void bitBlt(quint8 *dst,
                qint32 dstRowSize,
                const quint8 *src,
                qint32 srcRowStride,
                const quint8 *srcAlphaMask,
                qint32 maskRowStride,
                quint8 opacity,
                qint32 rows,
                qint32 cols,
                const KisCompositeOp& op) = 0;

    virtual cmsHTRANSFORM createTransform(KisColorSpace * dstColorSpace,
                          KisProfile *  srcProfile,
                          KisProfile *  dstProfile,
                          qint32 renderingIntent);

    virtual void compositeCopy(quint8 *dstRowStart, qint32 dstRowStride, const quint8 *srcRowStart, qint32 srcRowStride, const quint8 *maskRowStart, qint32 maskRowStride, qint32 rows, qint32 numColumns, quint8 opacity);

protected:

    QStringList m_profileFilenames;
    quint8 * m_qcolordata; // A small buffer for conversion from and to qcolor.
    qint32 m_alphaPos; // The position in _bytes_ of the alpha channel
    qint32 m_alphaSize; // The width in _bytes_ of the alpha channel

    Q3ValueVector<KisChannelInfo *> m_channels;

    KisColorSpaceFactoryRegistry * m_parent;

private:

    cmsHTRANSFORM m_defaultToRGB;    // Default transform to 8 bit sRGB
    cmsHTRANSFORM m_defaultFromRGB;  // Default transform from 8 bit sRGB

    cmsHPROFILE   m_lastRGBProfile;  // Last used profile to transform to/from RGB
    cmsHTRANSFORM m_lastToRGB;       // Last used transform to transform to RGB
    cmsHTRANSFORM m_lastFromRGB;     // Last used transform to transform from RGB

    cmsHTRANSFORM m_defaultToLab;
    cmsHTRANSFORM m_defaultFromLab;

    KisProfile *  m_profile;
    KisColorSpace *m_lastUsedDstColorSpace;
    cmsHTRANSFORM m_lastUsedTransform;

    KisID m_id;
    DWORD m_cmType;                           // The colorspace type as defined by littlecms
    icColorSpaceSignature m_colorSpaceSignature; // The colorspace signature as defined in icm/icc files

    // cmsHTRANSFORM is a void *, so this should work.
    typedef QMap<KisColorSpace *, cmsHTRANSFORM>  TransformMap;
    TransformMap m_transforms; // Cache for existing transforms

    KisAbstractColorSpace(const KisAbstractColorSpace&);
    KisAbstractColorSpace& operator=(const KisAbstractColorSpace&);

    Q3MemArray<quint8> m_conversionCache; // XXX: This will be a bad problem when we have threading.
};

#endif // KIS_STRATEGY_COLORSPACE_H_
