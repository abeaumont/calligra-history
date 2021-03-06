/*
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
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
#ifndef KIS_PRESCALED_PROJECTION_H
#define KIS_PRESCALED_PROJECTION_H

#include <QObject>

#include <krita_export.h>
#include <kis_shared.h>

#include "kis_update_info.h"

class QImage;
class QPoint;
class QRect;
class QRectF;
class QSize;
class QPainter;

class KoColorProfile;
class KisCoordinatesConverter;

#include <kis_types.h>
#include "kis_ui_types.h"

/**
 * KisPrescaledProjection is responsible for keeping around a
 * prescaled QImage representation that is always suitable for
 * painting onto the canvas.
 *
 * [deprecated]
 * Optionally, the KisPrescaledProjection can also provide a QPixmap
 * with the checkered background blended in.
 *
 * Optionally, the projection can also draw the mask and selection
 * masks and the selection outline.
 * [/deprecated]
 *
 * The following scaling methods are supported:
 *
 * <ul>
 *   <li>Qt's smooth scaling
 *   <li>Our own smooth scaling 
 *   <li>Our own sampling 
 *   <li>nearest-neighbour sampling on KisImage directly (doesn't need
 *       a QImage of the visible area)
 * </ul>
 *
 * Note: the export macro is only for the unittest.
 *
 * Note: with any method except for nearest-neighbour sampling Krita
 * keeps a QImage the size of the unscaled image in memory. This
 * should become either a QImage the size of the nearest pyramid level
 * or a tiled QImage representation like the OpenGL image textures.
 */
class KRITAUI_EXPORT KisPrescaledProjection : public QObject, public KisShared
{
    Q_OBJECT
public:

    KisPrescaledProjection();
    virtual ~KisPrescaledProjection();

    void setImage(KisImageWSP image);

    /**
     * Return the prescaled QImage. This image has a transparency
     * channel and is therefore suitable for generated a prescaled
     * representation of an image for the KritaShape. The prescaled
     * image is exactly as big as the canvas widget in pixels.
     */
    QImage prescaledQImage() const;

    void setCoordinatesConverter(KisCoordinatesConverter *coordinatesConverter);

public slots:

    /**
     * Retrieves image's data from KisImage object and updates
     * internal cache
     * @param dirtyImageRect the rect changed on the image
     * @return prefilled info object that should be used in
     * the second stage of the update
     * @see recalculateCache
     */
    KisUpdateInfoSP updateCache(const QRect &dirtyImageRect);

    /**
     * Updates the prescaled cache at current zoom level
     * @param prefilled update structure returned by updateCache
     * @see updateCache
     */
    void recalculateCache(KisUpdateInfoSP info);

    /**
     * Called whenever the configuration settings change.
     */
    void updateSettings();

    /**
     * Called whenever the view widget needs to show a different part of
     * the document
     */
    void viewportMoved(const QPointF &offset);

    /**
     * Called whenever the size of the KisImage changes
     */
    void setImageSize(qint32 w, qint32 h);

    /**
     * Checks whether it is needed to resize the prescaled image and
     * updates it. The size is given in canvas widget pixels.
     */
    void notifyCanvasSizeChanged(const QSize &widgetSize);

    void notifyZoomChanged();

    /**
     * Set the current monitor profile
     */
    void setMonitorProfile(const KoColorProfile * profile);

    /**
     * Called whenever the zoom level changes or another chunk of the
     * image becomes visible. The currently visible area of the image
     * is complete scaled again.
     */
    void preScale();

private:
    friend class KisPrescaledProjectionTest;

    KisPrescaledProjection(const KisPrescaledProjection &);
    KisPrescaledProjection operator=(const KisPrescaledProjection &);

    /**
     * Called from updateSettings to set up chosen backend:
     * now there is only one option left: KisImagePyramid
     */
    void initBackend(bool cacheKisImageAsQImage);

    void updateViewportSize();

    /**
     * preScale and draw onto the scaled projection the specified rect,
     * in canvas view pixels.
     */
    QRect preScale(const QRect & rc);

    /**
     * Prepare all the information about rects needed during
     * projection updating
     *
     * @param dirtyImageRect the part of the KisImage that is dirty
     */
    KisPPUpdateInfoSP getUpdateInformation(const QRect &viewportRect,
                                           const QRect &dirtyImageRect);

    /**
     * Initiates the process of prescaled image update
     *
     * @param info prepared information
     */
    void updateScaledImage(KisPPUpdateInfoSP info);

    /**
     * Atual drawing is done here
     * @param info prepared information
     * @param gc The painter we draw on
     */
    void drawUsingBackend(QPainter &gc, KisPPUpdateInfoSP info);

    struct Private;
    Private * const m_d;
};

#endif
