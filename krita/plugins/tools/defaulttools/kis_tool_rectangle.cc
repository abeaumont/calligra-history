/*
 *  kis_tool_rectangle.cc - part of Krita
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@k.org>
 *  Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_tool_rectangle.h"

#include <QPainter>

#include <kis_debug.h>
#include <klocale.h>

#include <KoPathShape.h>
#include <KoShapeManager.h>
#include <KoShapeRegistry.h>
#include <KoShapeController.h>

#include "kis_painter.h"
#include "kis_paintop_registry.h"
#include "kis_cursor.h"
#include "kis_layer.h"
#include "KoCanvasBase.h"
#include <kis_selection.h>
#include <kis_paint_device.h>
#include "kis_shape_tool_helper.h"

#include <recorder/kis_action_recorder.h>
#include <recorder/kis_recorded_shape_paint_action.h>
#include <recorder/kis_node_query_path.h>

#include <KoCanvasController.h>

KisToolRectangle::KisToolRectangle(KoCanvasBase * canvas)
        : KisToolRectangleBase(canvas, KisCursor::load("tool_rectangle_cursor.png", 6, 6))
{
    setObjectName("tool_rectangle");
}

KisToolRectangle::~KisToolRectangle()
{
}

void KisToolRectangle::finishRect(const QRectF &rect)
{
    if (rect.isNull())
        return;
    
    if (image()) {
        KisRecordedShapePaintAction linePaintAction(KisNodeQueryPath::absolutePath(currentNode()), currentPaintOpPreset(), KisRecordedShapePaintAction::Rectangle, rect);
        setupPaintAction(&linePaintAction);
        image()->actionRecorder()->addAction(linePaintAction);
    }

    if (!currentNode()->inherits("KisShapeLayer")) {
        KisPaintDeviceSP device = currentNode()->paintDevice();
        if (!device) return;

        KisPainter painter(device, currentSelection());

        painter.beginTransaction(i18n("Rectangle"));
        setupPainter(&painter);
        painter.paintRect(rect);
        painter.endTransaction(image()->undoAdapter());

        QRegion bound = painter.takeDirtyRegion();
        device->setDirty(bound);
        notifyModified();
    } else {
        QRectF r = convertToPt(rect);
        KoShape* shape = KisShapeToolHelper::createRectangleShape(r);

        QUndoCommand * cmd = canvas()->shapeController()->addShape(shape);
        canvas()->addCommand(cmd);
    }
}

#include "kis_tool_rectangle.moc"
