/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Thomas Zander <zander@kde.org>
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

#include "KWPageRemoveCommand.h"
#include "KWFrameDeleteCommand.h"
#include "frames/KWTextFrameSet.h"
#include "KWDocument.h"
#include "KWPage.h"
#include "frames/KWFrame.h"

#include <KoShapeMoveCommand.h>

#include <KLocale>

KWPageRemoveCommand::KWPageRemoveCommand(KWDocument *document, KWPage page, QUndoCommand *parent)
        : QUndoCommand(i18n("Remove Page"), parent),
        m_document(document),
        m_pageSide(page.pageSide()),
        m_pageLayout(page.pageStyle().pageLayout()),
        m_orientation(page.orientationHint()),
        m_pageNumber(page.pageNumber()),
        m_masterPageName(page.pageStyle().name()),
        m_direction(page.directionHint())
{
    Q_ASSERT(page.isValid());
    Q_ASSERT(document);
    Q_ASSERT(page.pageStyle().isValid());
    Q_ASSERT(document->pageManager()->page(m_pageNumber) == page);

    const qreal top = page.offsetInDocument();
    KoInsets padding = document->pageManager()->padding();
    const qreal bottom = top + page.height();
    const qreal height = page.height() + padding.top + padding.bottom; // size from previous page

    QList<KoShape*> shapesToMove;
    QList<QPointF> previousPositions;
    QList<QPointF> newPositions;
    foreach (KWFrameSet *fs, document->frameSets()) {
        KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*>(fs);
        bool autoGeneratedFrameSet = tfs
                && (tfs->textFrameSetType() == KWord::OddPagesHeaderTextFrameSet ||
                 tfs->textFrameSetType() == KWord::EvenPagesHeaderTextFrameSet ||
                 tfs->textFrameSetType() == KWord::OddPagesFooterTextFrameSet ||
                 tfs->textFrameSetType() == KWord::EvenPagesFooterTextFrameSet ||
                 tfs->textFrameSetType() == KWord::MainTextFrameSet);
        int count = -1;
        foreach (KWFrame *frame, fs->frames()) {
            ++count;
            KoShape *shape = frame->shape();
            QPointF pos = shape->position();
            if (pos.y() < top)
                continue;
            if (autoGeneratedFrameSet) {
                // this is a bit special; we can't refer to them using pointers as they are regenerated.
                AutoGenFrameSet afs;
                afs.frameSet = tfs;
                afs.deleteFromFrame = count;
                m_autoGenFrameSets << afs;
                break;
            }
            else if (pos.y() > bottom) { // move up
                newPositions << QPointF(pos.x(), pos.y() - height);
                previousPositions << pos;
                shapesToMove << shape;
            }
            else { // remove shapes that are *on* the page we are about to remove.
                new KWFrameDeleteCommand(m_document, frame, this);
            }
        }
    }
    if (! shapesToMove.isEmpty())
        new KoShapeMoveCommand(shapesToMove, previousPositions, newPositions, this);
}

KWPageRemoveCommand::~KWPageRemoveCommand()
{
}

void KWPageRemoveCommand::redo()
{
    KWPage page = m_document->pageManager()->page(m_pageNumber);
    Q_ASSERT(page.isValid());
    // remove the page in KWPageManager
    m_document->pageManager()->removePage(page);
    QUndoCommand::redo();

    foreach (const AutoGenFrameSet &agf, m_autoGenFrameSets) {
        KWTextFrameSet *tfs = agf.frameSet;
        while (tfs->frameCount() > agf.deleteFromFrame) {
            KWFrame *frame = tfs->frames().at(agf.deleteFromFrame);
            tfs->removeFrame(frame);
            delete frame->shape();
        }
    }
    // update changes
    m_document->firePageSetupChanged();

    // TODO Alter frame properties to not auto-create a frame again.

    // TODO remove the text include possible pagebreak displayed on the page from the
    // mainframe's QTextDocument. Atm this results in a new page being added after the
    // selected page got removed :-/

    //m_document->relayout(); //needed?
}

void KWPageRemoveCommand::undo()
{
    QUndoCommand::undo();

    // insert the page
    KWPage page = m_document->pageManager()->insertPage(m_pageNumber);
    page.setPageSide(m_pageSide);
    m_pageLayout.orientation = m_orientation;
    page.pageStyle().setPageLayout(m_pageLayout);
    page.setDirectionHint(m_direction);
    KWPageStyle pageStyle = m_document->pageManager()->pageStyle(m_masterPageName);
    if (pageStyle.isValid())
        page.setPageStyle(pageStyle);

    m_document->firePageSetupChanged();
    //m_document->relayout(); //needed?
}

