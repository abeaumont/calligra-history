/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
 * Copyright (C) Lukáš Tvrdý <lukast.dev@gmail.com>, (C) 2009
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
#ifndef KIS_QPAINTER_CANVAS_H
#define KIS_QPAINTER_CANVAS_H

#include <QWidget>

#include "kis_canvas_widget_base.h"
#include "kis_prescaled_projection.h"

class QMouseEvent;
class QImage;
class QPaintEvent;
class QPoint;
class QRect;
class QPainter;
class KisCanvas2;
class KoToolProxy;

/**
 *
 * KisQPainterCanvas is the widget that shows the actual image using arthur.
 *
 * NOTE: if you change something in the event handling here, also change it
 * in the opengl canvas.
 *
 * @author Boudewijn Rempt <boud@valdyas.org>
*/
class KisQPainterCanvas : public QWidget, public KisCanvasWidgetBase
{

    Q_OBJECT

public:

    KisQPainterCanvas(KisCanvas2 * canvas, KisCoordinatesConverter *coordinatesConverter, QWidget * parent);

    virtual ~KisQPainterCanvas();

    void setPrescaledProjection(KisPrescaledProjectionSP prescaledProjection);

public: // QWidget
    /// reimplemented method from superclass
    bool event(QEvent *);

    /// reimplemented method from superclass
    void enterEvent(QEvent* e);

    /// reimplemented method from superclass
    void leaveEvent(QEvent* e);

    /// reimplemented method from superclass
    void keyPressEvent(QKeyEvent *e);

    /// reimplemented method from superclass
    void mouseMoveEvent(QMouseEvent *e);

    /// reimplemented method from superclass
    void contextMenuEvent(QContextMenuEvent *e);

    /// reimplemented method from superclass
    void mousePressEvent(QMouseEvent *e);

    /// reimplemented method from superclass
    void mouseReleaseEvent(QMouseEvent *e);

    /// reimplemented method from superclass
    void mouseDoubleClickEvent(QMouseEvent *e);

    /// reimplemented method from superclass
    void keyReleaseEvent(QKeyEvent *e);

    /// reimplemented method from superclass
    void paintEvent(QPaintEvent * ev);

    /// reimplemented method from superclass
    void tabletEvent(QTabletEvent *e);

    /// reimplemented method from superclass
    void wheelEvent(QWheelEvent *e);

    /// reimplemented method from superclass
    void resizeEvent(QResizeEvent *e);

    /// reimplemented method from superclass
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

    /// reimplemented method from superclass
    virtual void inputMethodEvent(QInputMethodEvent *event);

public: // KisAbstractCanvasWidget

    QWidget * widget() {
        return this;
    }

protected: // KisCanvasWidgetBase

    virtual bool callFocusNextPrevChild(bool next);

signals:
    void needAdjustOrigin();

private slots:
    void slotConfigChanged();

private:
#ifdef INDEPENDENT_CANVAS
    QImage m_buffer;
#endif
    class Private;
    Private * const m_d;
};

#endif
