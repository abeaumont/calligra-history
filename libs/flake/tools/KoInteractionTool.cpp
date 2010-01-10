/* This file is part of the KDE project

   Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoInteractionTool.h"
#include "KoPointerEvent.h"
#include "KoInteractionStrategy.h"
#include "KoCanvasBase.h"
#include "KoPanTool.h"

class KoInteractionTool::Private
{
public:
    Private()
        : currentStrategy(0)
    {
    }
    ~Private() {
        delete currentStrategy;
    }

    QPointF lastPoint;
    KoInteractionStrategy *currentStrategy;
};

KoInteractionTool::KoInteractionTool(KoCanvasBase *canvas)
        : KoTool(canvas),
        d(new Private())
{
}

KoInteractionTool::~KoInteractionTool()
{
    delete d;
}

void KoInteractionTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (d->currentStrategy)
        d->currentStrategy->paint(painter, converter);
}

void KoInteractionTool::mousePressEvent(KoPointerEvent *event)
{
    if (d->currentStrategy) { // possible if the user presses an extra mouse button
        cancelCurrentStrategy();
        return;
    }
    d->currentStrategy = createStrategy(event);
    if (d->currentStrategy == 0)
        event->ignore();
}

void KoInteractionTool::mouseMoveEvent(KoPointerEvent *event)
{
    d->lastPoint = event->point;
    if (d->currentStrategy)
        d->currentStrategy->handleMouseMove(d->lastPoint, event->modifiers());
    else
        event->ignore();
}

void KoInteractionTool::mouseReleaseEvent(KoPointerEvent *event)
{
    if (d->currentStrategy) {
        d->currentStrategy->finishInteraction(event->modifiers());
        QUndoCommand *command = d->currentStrategy->createCommand();
        if (command)
            m_canvas->addCommand(command);
        delete d->currentStrategy;
        d->currentStrategy = 0;
        repaintDecorations();
    } else
        event->ignore();
}

void KoInteractionTool::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
    if (d->currentStrategy &&
            (event->key() == Qt::Key_Control ||
             event->key() == Qt::Key_Alt || event->key() == Qt::Key_Shift ||
             event->key() == Qt::Key_Meta)) {
        d->currentStrategy->handleMouseMove(d->lastPoint, event->modifiers());
        event->accept();
    }
}

void KoInteractionTool::keyReleaseEvent(QKeyEvent *event)
{
    if (d->currentStrategy == 0) { // catch all cases where no current strategy is needed
        if (event->key() == Qt::Key_Space)
            emit activateTemporary(KoPanTool_ID);
    } else if (event->key() == Qt::Key_Escape) {
        cancelCurrentStrategy();
        event->accept();
    } else if (event->key() == Qt::Key_Control ||
               event->key() == Qt::Key_Alt || event->key() == Qt::Key_Shift ||
               event->key() == Qt::Key_Meta) {
        d->currentStrategy->handleMouseMove(d->lastPoint, event->modifiers());
    }
}

KoInteractionStrategy *KoInteractionTool::currentStrategy()
{
    return d->currentStrategy;
}

void KoInteractionTool::cancelCurrentStrategy()
{
    if (d->currentStrategy) {
        d->currentStrategy->cancelInteraction();
        delete d->currentStrategy;
        d->currentStrategy = 0;
    }
}

#include <KoInteractionTool.moc>
