/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_paintop_settings.h"

#include <QImage>
#include <QColor>
#include <QPointer>

#include <KoPointerEvent.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoViewConverter.h>

#include "kis_node.h"
#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_paintop_registry.h"
#include "kis_paint_information.h"
#include "kis_paintop_settings_widget.h"

struct KisPaintOpSettings::Private {
    KisNodeSP node;
    QPointer<KisPaintOpSettingsWidget> settingsWidget;
    QString modelName;
};

KisPaintOpSettings::KisPaintOpSettings()
        : d(new Private)
{
}

KisPaintOpSettings::~KisPaintOpSettings()
{
    delete d;
}

void KisPaintOpSettings::setOptionsWidget(KisPaintOpSettingsWidget* widget)
{
    d->settingsWidget = widget;
}

bool KisPaintOpSettings::mousePressEvent(const KisPaintInformation &pos, Qt::KeyboardModifiers modifiers){
    Q_UNUSED(pos);
    Q_UNUSED(modifiers);
    return true; // ignore the event by default
}


KisPaintOpSettingsSP KisPaintOpSettings::clone() const
{
    QString paintopID = getString("paintop");
    if(paintopID.isEmpty())
        return 0;

    KisPaintOpSettingsSP settings = KisPaintOpRegistry::instance()->settings(KoID(paintopID, ""));
    QMapIterator<QString, QVariant> i(getProperties());
    while (i.hasNext()) {
        i.next();
        settings->setProperty(i.key(), QVariant(i.value()));
    }
    return settings;
}

void KisPaintOpSettings::activate()
{
}

void KisPaintOpSettings::setNode(KisNodeSP node)
{
    d->node = node;
}

KisNodeSP KisPaintOpSettings::node() const
{
    return d->node;
}

QImage KisPaintOpSettings::sampleStroke(const QSize& size)
{
//    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
//    int width = size.width();
//    int height = size.height();
//
//    KisLayerSP layer = new KisPaintLayer(0, "temporary for stroke sample", OPACITY_OPAQUE_U8, cs);
//    KisImageSP image = new KisImage(0, width, height, cs, "stroke sample image", false);
//    KisPainter painter(layer->paintDevice());
//    painter.setPaintColor(KoColor(Qt::black, cs));
//
//    KisPaintOpPresetSP preset = new KisPaintOpPreset();
//    preset->setSettings(this);   // This clones
//    preset->settings()->setNode(layer);
//
//    painter.setPaintOpPreset(preset, image);
//
//    QPointF p1(0                , 7.0 / 12.0 * height);
//    QPointF p2(1.0 / 2.0 * width  , 7.0 / 12.0 * height);
//    QPointF p3(width - 4.0, height - 4.0);
//
//    KisPaintInformation pi1(p1, 0.0);
//    KisPaintInformation pi2(p2, 0.95);
//    KisPaintInformation pi3(p3, 0.0);
//
//    QPointF c1(1.0 / 4.0* width , height - 2.0);
//    QPointF c2(c1);
//    KisDistanceInformation saveddist = painter.paintBezierCurve(pi1, c1, c2, pi2, KisDistanceInformation());
//
//    c1.setX(3.0 / 4.0 * width);
//    c1.setY(0);
//    c2.setX(c1.x());
//    c2.setY(c1.y());
//    painter.paintBezierCurve(pi2, c1, c2, pi3, saveddist);
//
//    return layer->paintDevice()->convertToQImage(0);
    QImage img = QImage(size, QImage::Format_ARGB32);
    QPainter gc(&img);
    gc.fillRect(0, 0, size.width(), size.height(), Qt::red);
    gc.end();
    return img;

}

QRectF KisPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageWSP image, OutlineMode _mode) const
{
    Q_UNUSED(_mode);
    QRectF rect = QRectF(-5, -5, 10, 10);
    return image->pixelToDocument(rect).translated(pos);
}

void KisPaintOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter &painter, OutlineMode _mode) const
{
    if (_mode != CursorIsOutline) return;
    QRectF rect2 = paintOutlineRect(pos, image, _mode);
    painter.drawLine(rect2.topLeft(), rect2.bottomRight());
    painter.drawLine(rect2.topRight(), rect2.bottomLeft());
}


void KisPaintOpSettings::changePaintOpSize(qreal x, qreal y)
{
    if(!d->settingsWidget.isNull()) {
        d->settingsWidget.data()->changePaintOpSize(x, y);
        d->settingsWidget.data()->writeConfiguration(this);
    }
}

QString KisPaintOpSettings::modelName() const
{
    return d->modelName;
}

void KisPaintOpSettings::setModelName(const QString & modelName)
{
    d->modelName = modelName;
}

KisPaintOpSettingsWidget* KisPaintOpSettings::optionsWidget() const
{
    if(d->settingsWidget.isNull())
        return 0;

    return d->settingsWidget.data();
}

bool KisPaintOpSettings::isValid()
{
    return true;
}

QPainterPath KisPaintOpSettings::brushOutline(const QPointF& pos, OutlineMode mode, qreal scale, qreal rotation) const
{
    QPainterPath path;
    if (mode == CursorIsOutline){
        QRectF rc(-5,-5, 10, 10);
        path.moveTo(rc.topLeft());
        path.lineTo(rc.bottomRight());
        path.moveTo(rc.topRight());
        path.lineTo(rc.bottomLeft());
        QTransform m;
        m.reset(); m.scale(scale,scale); m.rotateRadians(rotation);
        path = m.map(path);
        path.translate(pos);
    }
    return path;
}

QPainterPath KisPaintOpSettings::ellipseOutline(qreal width, qreal height, qreal scale, qreal rotation) const
{
    QPainterPath path;
    QRectF ellipse(0,0,width * scale,height * scale);
    ellipse.translate(-ellipse.center());
    path.addEllipse(ellipse);
        
    QTransform m;
    m.reset();
    m.rotate( rotation );
    path = m.map(path);
    return path;
}
