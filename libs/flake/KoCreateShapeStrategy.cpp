/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoCreateShapeStrategy.h"
#include "KoCreateShapesTool.h"
#include "KoShapeControllerBase.h"
#include "KoShapeRegistry.h"
#include "KoCommand.h"
#include "KoCanvasBase.h"

KoCreateShapeStrategy::KoCreateShapeStrategy( KoCreateShapesTool *tool, KoCanvasBase *canvas, const QPointF &clicked)
: KoShapeRubberSelectStrategy(tool, canvas, clicked)
, m_tool(tool)
{
}

KCommand* KoCreateShapeStrategy::createCommand() {
    KoShapeFactory *factory = KoShapeRegistry::instance()->get(
            static_cast<KoCreateShapesTool*>(m_parent)->shapeId());
    Q_ASSERT(factory);
    KoShape *shape = factory->createDefaultShape();
    QRectF rect = selectRect();
    shape->setPosition(rect.topLeft());
    shape->resize(rect.size());
    Q_ASSERT(m_tool->controller() /*controller was set on parent tool*/);
    KoShapeCreateCommand *cmd = new KoShapeCreateCommand(m_tool->controller(), shape);
    cmd->execute();
    return cmd;
}

void KoCreateShapeStrategy::finishInteraction() {
    m_canvas->updateCanvas(selectRect());
}
