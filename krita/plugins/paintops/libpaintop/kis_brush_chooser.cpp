/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2009 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_brush_chooser.h"

#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPainter>
#include <QAbstractItemDelegate>
#include <klocale.h>

#include <KoResourceItemChooser.h>
#include <KoResourceServer.h>
#include <KoResourceServerAdapter.h>
#include <KoResourceServerProvider.h>

#include "kis_brush_registry.h"
#include "kis_brush_server.h"
#include "widgets/kis_slider_spin_box.h"

#include "kis_global.h"
#include "kis_gbr_brush.h"

/// The resource item delegate for rendering the resource preview
class KisBrushDelegate : public QAbstractItemDelegate
{
public:
    KisBrushDelegate(QObject * parent = 0) : QAbstractItemDelegate(parent) {}
    virtual ~KisBrushDelegate() {}
    /// reimplemented
    virtual void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;
    /// reimplemented
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const {
        return option.decorationSize;
    }
};

void KisBrushDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    if (! index.isValid())
        return;

    KisBrush * brush = static_cast<KisBrush*>(index.internalPointer());
    if (!brush)
        return;

    QRect itemRect = option.rect;
    QImage thumbnail = brush->image();

    if (thumbnail.height() > itemRect.height() || thumbnail.width() > itemRect.width()) {
        thumbnail = thumbnail.scaled(itemRect.size() , Qt::KeepAspectRatio);
    }

    painter->save();
    int dx = (itemRect.width() - thumbnail.width()) / 2;
    int dy = (itemRect.height() - thumbnail.height()) / 2;
    painter->drawImage(itemRect.x() + dx, itemRect.y() + dy, thumbnail);

    if (option.state & QStyle::State_Selected) {
        painter->setPen(QPen(option.palette.highlight(), 2.0));
        painter->drawRect(option.rect);
    }

    painter->restore();
}


KisBrushChooser::KisBrushChooser(QWidget *parent, const char *name)
        : QWidget(parent)
{
    setObjectName(name);

    m_lbScale = new QLabel(i18n("Scale: "), this);
    m_slScale = new KisDoubleSliderSpinBox(this);
    m_slScale->setRange(0.0, 2.0, 2);
    m_slScale->setValue(1.0);
    QObject::connect(m_slScale, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetItemScale(qreal)));

    m_lbRotation = new QLabel(i18n("Rotation: "), this);
    m_slRotation = new KisDoubleSliderSpinBox(this);
    m_slRotation->setRange(0.0, 360, 2);
    m_slRotation->setValue(0.0);
    QObject::connect(m_slRotation, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetItemRotation(qreal)));

    m_lbSpacing = new QLabel(i18n("Spacing: "), this);
    m_slSpacing = new KisDoubleSliderSpinBox(this);
    m_slSpacing->setRange(0.0, 10, 2);
    m_slSpacing->setValue(0.1);
    m_slSpacing->setExponentRatio(3.0);
    QObject::connect(m_slSpacing, SIGNAL(valueChanged(qreal)), this, SLOT(slotSetItemSpacing(qreal)));

    m_chkColorMask = new QCheckBox(i18n("Use color as mask"), this);
    QObject::connect(m_chkColorMask, SIGNAL(toggled(bool)), this, SLOT(slotSetItemUseColorAsMask(bool)));

    m_lbName = new QLabel(this);

    KoResourceServer<KisBrush>* rServer = KisBrushServer::instance()->brushServer();
    KoResourceServerAdapter<KisBrush>* adapter = new KoResourceServerAdapter<KisBrush>(rServer);
    m_itemChooser = new KoResourceItemChooser(adapter, this);
    m_itemChooser->setColumnCount(10);
    m_itemChooser->setRowHeight(30);
    m_itemChooser->setItemDelegate(new KisBrushDelegate(this));
    m_itemChooser->setCurrentItem(0, 0);

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResource *)),
            this, SLOT(update(KoResource *)));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setObjectName("main layout");
    mainLayout->setMargin(2);
    mainLayout->setSpacing(2);

    mainLayout->addWidget(m_lbName);
    mainLayout->addWidget(m_itemChooser, 10);

    QGridLayout *spacingLayout = new QGridLayout();

    mainLayout->addLayout(spacingLayout, 1);

    spacingLayout->addWidget(m_lbScale, 0, 0);
    spacingLayout->addWidget(m_slScale, 0, 1);
    spacingLayout->addWidget(m_lbRotation, 1, 0);
    spacingLayout->addWidget(m_slRotation, 1, 1);
    spacingLayout->addWidget(m_lbSpacing, 2, 0);
    spacingLayout->addWidget(m_slSpacing, 2, 1);
    spacingLayout->setColumnStretch(1, 3);

    spacingLayout->addWidget(m_chkColorMask, 3, 0, 1, 2);

    slotActivatedBrush(m_itemChooser->currentResource());
}

KisBrushChooser::~KisBrushChooser()
{
}

void KisBrushChooser::setBrush(KisBrushSP _brush)
{
    KoResource *resource = static_cast<KoResource*>(_brush.data());
    m_itemChooser->setCurrentResource( resource );
    update( resource );
    /*
      XXX: why is this uncommented?

        KisGbrBrush* brush = static_cast<KisGbrBrush*>(_brush.data());

        QString text = QString("%1 (%2 x %3)")
                       .arg(brush->name())
                       .arg(brush->width())
                       .arg(brush->height());

        m_lbName->setText(text);
        m_slSpacing->setValue(brush->spacing());
        m_chkColorMask->setChecked(brush->useColorAsMask());
        m_chkColorMask->setEnabled(brush->hasColor());

        m_brush = brush;
    */
}

void KisBrushChooser::slotSetItemScale(qreal scaleValue)
{
    KoResource * resource = static_cast<KoResource *>(m_itemChooser->currentResource());

    if (resource) {
        KisBrush *brush = static_cast<KisBrush *>(resource);
        brush->setScale(scaleValue);
        slotActivatedBrush(brush);

        emit sigBrushChanged();
    }
}
void KisBrushChooser::slotSetItemRotation(qreal rotationValue)
{
    KoResource * resource = static_cast<KoResource *>(m_itemChooser->currentResource());

    if (resource) {
        KisBrush *brush = static_cast<KisBrush *>(resource);
        brush->setAngle(rotationValue / 180.0 * M_PI);
        slotActivatedBrush(brush);

        emit sigBrushChanged();
    }
}

void KisBrushChooser::slotSetItemSpacing(qreal spacingValue)
{
    KoResource * resource = static_cast<KoResource *>(m_itemChooser->currentResource());

    if (resource) {
        KisBrush *brush = static_cast<KisBrush *>(resource);
        brush->setSpacing(spacingValue);
        slotActivatedBrush(brush);

        emit sigBrushChanged();
    }
}

void KisBrushChooser::slotSetItemUseColorAsMask(bool useColorAsMask)
{
    KoResource * resource = static_cast<KoResource *>(m_itemChooser->currentResource());

    if (resource) {
        KisGbrBrush* brush = static_cast<KisGbrBrush*>(resource);
        brush->setUseColorAsMask(useColorAsMask);
        slotActivatedBrush(brush);

        emit sigBrushChanged();
    }
}

void KisBrushChooser::update(KoResource * resource)
{
    KisBrush* brush = static_cast<KisBrush*>(resource);
    
    QString text = QString("%1 (%2 x %3)")
                   .arg(i18n(brush->name().toUtf8().data()))
                   .arg(brush->width())
                   .arg(brush->height());

    m_lbName->setText(text);
    m_slSpacing->setValue(brush->spacing());
    m_slRotation->setValue(brush->angle() * 180 / M_PI);
    m_slScale->setValue(brush->scale());
    
    
    // useColorAsMask support is only in gimp brush so far
    if (KisGbrBrush * gimpBrush = dynamic_cast<KisGbrBrush*>(resource)){
        m_chkColorMask->setChecked(gimpBrush->useColorAsMask());
    }
    m_chkColorMask->setEnabled(brush->hasColor());
    
    emit sigBrushChanged();
}

void KisBrushChooser::slotActivatedBrush(KoResource * resource)
{
    KisBrush* brush = dynamic_cast<KisBrush*>(resource);
    if (brush) {
        m_brush = brush;
    }
}

void KisBrushChooser::setBrushSize(qreal xPixels, qreal yPixels)
{
        Q_UNUSED(yPixels);
        qreal oldWidth = m_brush->width() * m_brush->scale(); // or maybe m_slScale->value()
        qreal newWidth = oldWidth + xPixels;
        if (newWidth <= 0.0) {
            return;
        }
        
        qreal newScale = newWidth / m_brush->width();
        // signal valueChanged will care about call to slotSetItemScale
        m_slScale->setValue(newScale);
}


#include "kis_brush_chooser.moc"
