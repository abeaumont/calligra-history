/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */

#include <kdebug.h>
#include <kglobal.h>
#include <qimage.h>
#include <qdatetime.h>

#include "kis_types.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_layer_visitor.h"
#include "kis_debug_areas.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_merge_visitor.h"
#include "kis_fill_painter.h"

KisGroupLayer::KisGroupLayer(KisImage *img, const QString &name, Q_UINT8 opacity) :
    super(img, name, opacity),
    m_x(0),
    m_y(0)
{
    m_projection = new KisPaintDevice(this, img->colorSpace(), name.latin1());
}

KisGroupLayer::KisGroupLayer(const KisGroupLayer &rhs) :
    super(rhs),
    m_x(rhs.m_x),
    m_y(rhs.m_y)
{
    for(vKisLayerSP_cit it = rhs.m_layers.begin(); it != rhs.m_layers.end(); ++it)
    {
        m_layers.push_back( it->data()->clone() );
    }
    m_projection = new KisPaintDevice(*rhs.m_projection.data());
    m_projection->setParentLayer(this);
}

KisLayerSP KisGroupLayer::clone() const
{
    return new KisGroupLayer(*this);
}

KisGroupLayer::~KisGroupLayer()
{
    m_layers.clear();
}


void KisGroupLayer::setDirty()
{
    KisLayer::setDirty();
    emit (sigDirty(m_dirtyRect));
}

void KisGroupLayer::setDirty(const QRect & rc)
{
    KisLayer::setDirty(rc);
    emit sigDirty(rc);
}


void KisGroupLayer::resetProjection()
{
    m_projection = new KisPaintDevice(this, image()->colorSpace(), name().latin1());
}

KisPaintDeviceSP KisGroupLayer::projection(const QRect & rect)
{
    //kdDebug() << "Call for projection. " << name() << ", Dirty =" << dirty() << endl;
    
    // No need for updates, we're clean
    if (!dirty()) {
        //kdDebug() << name() << " No need for updates, we're clean\n";
        return m_projection;
    }
    // No need for updates -- the desired area wasn't dirty
    if (!rect.intersects(m_dirtyRect)) {
        //kdDebug() << name() << " No need for updates, the desired area was not dirty\n";
        return m_projection;
    }

    // Okay, we need to update the intersection between
    // what's dirty and what's asked us to be updated.
    const QRect rc = rect.intersect(m_dirtyRect);

    QTime t;
    t.start();
    updateProjection(rc);
    kdDebug() << ">>> Updating projection " << name() << " for " << rc.x() << ", " << rc.y() << ", " << rc.width() << ", " << rc.height() << " took: " << t.elapsed() << endl;
    setClean(rect);

    return m_projection;
}

uint KisGroupLayer::childCount() const
{
    return m_layers.count();
}

KisLayerSP KisGroupLayer::firstChild() const
{
    return at(0);
}

KisLayerSP KisGroupLayer::lastChild() const
{
    return at(childCount() - 1);
}

KisLayerSP KisGroupLayer::at(int index) const
{
    if (childCount() && index >= 0 && kClamp(uint(index), uint(0), childCount() - 1) == uint(index))
        return m_layers.at(reverseIndex(index));
    return 0;
}

int KisGroupLayer::index(KisLayerSP layer) const
{
    if (layer -> parent().data() == this)
        return layer -> index();
    return -1;
}

void KisGroupLayer::setIndex(KisLayerSP layer, int index)
{
    if (layer -> parent().data() != this)
        return;
    //TODO optimize
    removeLayer(layer);
    addLayer(layer, index);
}

bool KisGroupLayer::addLayer(KisLayerSP newLayer, int x)
{
    if (x < 0 || kClamp(uint(x), uint(0), childCount()) != uint(x) ||
        newLayer -> parent() || m_layers.contains(newLayer))
    {
        kdWarning() << "invalid input to KisGroupLayer::addLayer(KisLayerSP newLayer, int x)!" << endl;
        //kdDebug(41001) << "layer: " << newLayer << ", x: " << x
        //        << ", parent: " << newLayer->parent() << ", contains: " << m_layers.contains(newLayer)
        //          << " stack: " << kdBacktrace() << "\n";
        return false;
    }
    uint index(x);
    if (index == 0)
        m_layers.append(newLayer);
    else
        m_layers.insert(m_layers.begin() + reverseIndex(index) + 1, newLayer);
    for (uint i = childCount() - 1; i > index; i--)
        at(i) -> m_index++;
    newLayer -> m_parent = this;
    newLayer -> m_index = index;
    newLayer -> setImage(image());
    setDirty();
        return true;
}

bool KisGroupLayer::addLayer(KisLayerSP newLayer, KisLayerSP aboveThis)
{
    if (aboveThis && aboveThis -> parent().data() != this)
    {
        kdWarning() << "invalid input to KisGroupLayer::addLayer(KisLayerSP newLayer, KisLayerSP aboveThis)!" << endl;
        return false;
    }
    return addLayer(newLayer, aboveThis ? aboveThis -> index() : childCount());
}

bool KisGroupLayer::removeLayer(int x)
{
    if (x >= 0 && kClamp(uint(x), uint(0), childCount() - 1) == uint(x))
    {
        uint index(x);
        for (uint i = childCount() - 1; i > index; i--)
            at(i) -> m_index--;
        at(index) -> m_parent = 0;
        at(index) -> m_index = -1;
        m_layers.erase(m_layers.begin() + reverseIndex(index));
        setDirty(); // XXX: Set only the extent of the removed layer to dirty
        if (childCount() < 1) {
            // No children, nothing to show for it.
            m_projection->clear();
        }
        return true;
    }
    kdWarning() << "invalid input to KisGroupLayer::removeLayer()!" << endl;
    return false;
}

bool KisGroupLayer::removeLayer(KisLayerSP layer)
{
    if (layer -> parent().data() != this)
    {
        kdWarning() << "invalid input to KisGroupLayer::removeLayer()!" << endl;
        return false;
    }
    return removeLayer(layer -> index());
}

void KisGroupLayer::setImage(KisImage *image)
{
    super::setImage(image);
    for (vKisLayerSP_it it = m_layers.begin(); it != m_layers.end(); ++it)
    {
        (*it)->setImage(image);
    }
}

QRect KisGroupLayer::extent() const
{
    QRect groupExtent;

    for (vKisLayerSP_cit it = m_layers.begin(); it != m_layers.end(); ++it)
    {
        groupExtent |= (*it)->extent();
    }

    return groupExtent;
}

QRect KisGroupLayer::exactBounds() const
{
    QRect groupExactBounds;

    for (vKisLayerSP_cit it = m_layers.begin(); it != m_layers.end(); ++it)
    {
        groupExactBounds |= (*it)->exactBounds();
    }

    return groupExactBounds;
}

Q_INT32 KisGroupLayer::x() const
{
    return m_x;
}

void KisGroupLayer::setX(Q_INT32 x)
{
    Q_INT32 delta = x - m_x;

    for (vKisLayerSP_cit it = m_layers.begin(); it != m_layers.end(); ++it)
    {
        KisLayerSP layer = *it;
        layer->setX(layer->x() + delta);
    }

    m_x = x;
}

Q_INT32 KisGroupLayer::y() const
{
    return m_y;
}

void KisGroupLayer::setY(Q_INT32 y)
{
    Q_INT32 delta = y - m_y;

    for (vKisLayerSP_cit it = m_layers.begin(); it != m_layers.end(); ++it)
    {
        KisLayerSP layer = *it;
        layer->setY(layer->y() + delta);
    }

    m_y = y;
}

QImage KisGroupLayer::createThumbnail(Q_INT32 w, Q_INT32 h)
{
    return m_projection->createThumbnail(w, h);
}

void KisGroupLayer::updateProjection(const QRect & rc)
{
    if (!m_dirtyRect.isValid()) return;
        
    // Get the first layer in this group to start compositing with
    KisLayerSP child = lastChild();

    // No child -- clear the projection. Without children, a group layer is empty.
    if (!child) m_projection->clear();
    
    KisLayerSP startWith = child;
    KisAdjustmentLayerSP adjLayer = 0;

    // Look through all the child layers, searching for the first dirty layer
    // if it's found, and if we have found an adj. layer before the the dirty layer,
    // composite from the first adjustment layer searching back from the first dirty layer
    while (child) {
        KisAdjustmentLayerSP tmpAdjLayer = dynamic_cast<KisAdjustmentLayer*>(child.data());
        if (tmpAdjLayer) {

            // If this adjustment layer is dirty, start compositing with the
            // previous adj. layer, if there's one.
            if (tmpAdjLayer->dirty(rc) && adjLayer != 0) {
                startWith = adjLayer->prevSibling();
                break;
            }
            else {
                // This is the first adj. layer that is not dirty -- the perfect starting point
                adjLayer = tmpAdjLayer;
            }
        }
        else {
            // A non-adjustmentlayer that's dirty; if there's an adjustmentlayer
            // with a cache, we'll start from there.
            if (child->dirty(rc)) {
                if (adjLayer != 0) {
                    // the first layer on top of the adj. layer
                    startWith = adjLayer->prevSibling();
                }
                // break here: if there's no adj layer, we'll start with the layer->lastChild
                break;
            }
        }
        child = child->prevSibling();
    }

    if (startWith == 0) {
        // The projection is apparently still up to date, but why was it marked dirty, then?
        return;
    }

    bool first = true; // The first layer in a stack needs special compositing

    // Fill the projection either with the cached data, or erase it.
    KisFillPainter gc(m_projection);
    if (adjLayer != 0) {
        gc.bitBlt(rc.left(), rc.top(),
                  COMPOSITE_COPY, adjLayer->cachedPaintDevice(), OPACITY_OPAQUE,
                  rc.left(), rc.top(), rc.width(), rc.height());
        first = false;
    }
    else {
        gc.eraseRect(rc);
        first = true;
    }
    gc.end();
        
    KisMergeVisitor visitor(m_projection, rc);

    child = startWith;

    while(child)
    {
        if(first)
        {
            // Copy the lowest layer rather than compositing it with the background
            // or an empty image. This means the layer's composite op is ignored,
            // which is consistent with Photoshop and gimp.
            const KisCompositeOp cop = child->compositeOp();
            const bool block = child->signalsBlocked();
            child->blockSignals(true);
            child->m_compositeOp = COMPOSITE_COPY;
            child->blockSignals(block);
            child->accept(visitor);
            child->blockSignals(true);
            child->m_compositeOp = cop;
            child->blockSignals(block);
            first = false;
        }
        else
            child->accept(visitor);

        child = child->prevSibling();
    }

    //emit sigProjectionUpdated(rc);
}

#include "kis_group_layer.moc"
