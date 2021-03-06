/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2009-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Johannes Simon <johannes.simon@gmail.com>
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

#include "KoTextDocumentLayout.h"
#include "KoTextShapeData.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoStyleManager.h"
#include "KoTextBlockData.h"
#include "KoTextBlockBorderData.h"
#include "KoInlineTextObjectManager.h"

#include <KoInsets.h>
#include <KoPostscriptPaintDevice.h>
#include <KoShape.h>

#include <kdebug.h>
#include <QTextBlock>
#include <QTextTable>
#include <QTextTableCell>
#include <QTextList>
#include <QTimer>

class LayoutStateDummy : public KoTextDocumentLayout::LayoutState
{
public:
    LayoutStateDummy() {}
    bool start() {
        return false;
    }
    void end() {}
    void reset() {}
    bool isInterrupted() const {
        return false;
    }
    int numColumns() {
        return 0;
    }
    qreal width() {
        return 0;
    }
    qreal x() {
        return 0;
    }
    qreal y() {
        return 0;
    }
    qreal docOffsetInShape() const {
        return 0;
    }
    QRectF expandVisibleRect(const QRectF &rect) const {
        return rect;
    }
    bool addLine(QTextLine &, bool) {
        return true;
    }
    bool nextParag() {
        return false;
    }
    bool previousParag() {
        return false;
    }
    qreal documentOffsetInShape() {
        return 0;
    }

    QRectF selectionBoundingBox(QTextCursor &cursor) {
        return QRectF();
    }

    void draw(QPainter *, const KoTextDocumentLayout::PaintContext &) {}

    bool setFollowupShape(KoShape *) {
        return false;
    }
    void clearTillEnd() {}
    int cursorPosition() const {
        return 0;
    }
    void registerInlineObject(const QTextInlineObject &) {}
    QTextTableCell hitTestTable(QTextTable *, const QPointF &) {
        return QTextTableCell();
    }
    qreal maxLineHeight() const {
        return 0;
    }
};

class KoTextDocumentLayout::Private
{
public:
    Private(KoTextDocumentLayout *parent_)
            : inlineTextObjectManager(0),
            scheduled(false),
            parent(parent_),
            resizeMethod(KoTextDocument::NoResize)
    {
    }

    ~Private()
    {
        delete paintDevice;
    }

    void relayoutPrivate() {
        scheduled = false;
        parent->relayout();
    }

    void postLayoutHook() {
        Q_ASSERT(parent);
        Q_ASSERT(parent->m_state);
        KoShape *shape = parent->m_state->shape;
        if (shape == 0)
            return;
        KoTextShapeData *data = qobject_cast<KoTextShapeData*>(shape->userData());
        Q_ASSERT(data);
        // clears the cached documentSize
        documentSize = QSizeF();
        // adjust the size of the shape before re-positioning
        adjustSize();
        // now adjust the position of the shape
        qreal offset = 0;
        if (data->verticalAlignment() == Qt::AlignVCenter) {
           offset = (shape->size().height() - (parent->m_state->y() - data->documentOffset())) / 2.;
        } else if (data->verticalAlignment() == Qt::AlignBottom) {
           offset = shape->size().height() - (parent->m_state->y() - data->documentOffset());
        }
        if (offset != 0) {
            data->setDocumentOffset(data->documentOffset() - offset);
        }
    }

    void adjustSize();

    QList<KoShape *> shapes;
    KoInlineTextObjectManager *inlineTextObjectManager;
    bool scheduled;
    KoTextDocumentLayout *parent;
    KoTextDocument::ResizeMethod resizeMethod;
    KoPostscriptPaintDevice *paintDevice;
    QSizeF documentSize;
};

void KoTextDocumentLayout::Private::adjustSize()
{
    KoTextDocument::ResizeMethod resize = parent->resizeMethod();
    if (resize != KoTextDocument::AutoResize && resize != KoTextDocument::AutoGrowWidth && resize != KoTextDocument::AutoGrowHeight && resize != KoTextDocument::AutoGrowWidthAndHeight)
        return; // nothing to do for us

    QSizeF documentSize = parent->documentSize();
    foreach(KoShape *shape, parent->shapes()) {
        QSizeF size = shape->size();
        qreal width = 0;
        qreal height = 0;

        if (resize == KoTextDocument::AutoResize || resize == KoTextDocument::AutoGrowWidth || resize == KoTextDocument::AutoGrowWidthAndHeight) {
            width = documentSize.width();
            if (resize != KoTextDocument::AutoResize)
                width = qMax(size.width(), width); // only grow but don't shrink
        } else {
            width = size.width();
        }

        if (resize == KoTextDocument::AutoResize || resize == KoTextDocument::AutoGrowHeight || resize == KoTextDocument::AutoGrowWidthAndHeight) {
            height = documentSize.height();
            if (resize != KoTextDocument::AutoResize)
                height = qMax(size.height(), height); // only grow but don't shrink
        } else {
            height = size.height();
        }

        // and finally adjust the shapes size
        shape->setSize(QSizeF(width, height));
    }
}

// ------------------- KoTextDocumentLayout --------------------
KoTextDocumentLayout::KoTextDocumentLayout(QTextDocument *doc, KoTextDocumentLayout::LayoutState *layout)
        : QAbstractTextDocumentLayout(doc),
        m_state(layout),
        d(new Private(this))
{
    d->paintDevice = new KoPostscriptPaintDevice();
    setPaintDevice(d->paintDevice);
    if (m_state == 0)
        m_state = new LayoutStateDummy();

    connect (this, SIGNAL(finishedLayout()), this, SLOT(postLayoutHook()));
}

KoTextDocumentLayout::~KoTextDocumentLayout()
{
    delete d;
    delete m_state;
    m_state = 0;
}

void KoTextDocumentLayout::setLayout(LayoutState *layout)
{
    Q_ASSERT(layout);
    delete m_state;
    m_state = layout;
    scheduleLayout();
}

bool KoTextDocumentLayout::hasLayouter() const
{
    if (dynamic_cast<LayoutStateDummy*>(m_state) != 0)
        return false;
    if (m_state == 0)
        return false;
    return true;
}

void KoTextDocumentLayout::addShape(KoShape *shape)
{
    d->shapes.append(shape);

    KoTextShapeData *data = qobject_cast<KoTextShapeData*>(shape->userData());
    if (data) {
        data->foul();
        m_state->reset();
    }
    emit shapeAdded(shape);
}

void KoTextDocumentLayout::setInlineTextObjectManager(KoInlineTextObjectManager *iom)
{
    d->inlineTextObjectManager = iom;
}

KoInlineTextObjectManager *KoTextDocumentLayout::inlineTextObjectManager()
{
    return d->inlineTextObjectManager;
}

QRectF KoTextDocumentLayout::blockBoundingRect(const QTextBlock &block) const
{
    QTextLayout *layout = block.layout();
    return layout->boundingRect();
}

QSizeF KoTextDocumentLayout::documentSize() const
{
    if (!d->documentSize.isValid()) {
        QTextFrame *frame = document()->rootFrame();
        d->documentSize = frameBoundingRect(frame).size();
    }
    return d->documentSize;
}

QRectF KoTextDocumentLayout::expandVisibleRect(const QRectF &rect) const
{
    return m_state->expandVisibleRect(rect);
}

QRectF KoTextDocumentLayout::selectionBoundingBox(QTextCursor &cursor)
{
    return m_state->selectionBoundingBox(cursor);
}


void KoTextDocumentLayout::draw(QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context)
{
    PaintContext pc;
    pc.textContext = context;
    m_state->draw(painter, pc);
}

void KoTextDocumentLayout::draw(QPainter * painter, const KoTextDocumentLayout::PaintContext & context)
{
    m_state->draw(painter, context);
}

QRectF KoTextDocumentLayout::frameBoundingRect(QTextFrame *frame) const
{
    QRectF rect(0,0,0,0);
    for (QTextFrame::iterator it = frame->begin(); !(it.atEnd()); ++it) {
        QTextBlock block = it.currentBlock();
        if (!block.isValid()) continue;
        QTextLayout *layout = block.layout();
        QRectF r = layout->boundingRect();
        if(rect.isNull()) {
            rect = r;
        } else {
            if(r.left() < rect.left()) rect.setLeft(r.left());
            if(r.top() < rect.top()) rect.setTop(r.top());
            if(r.right() > rect.right()) rect.setRight(r.right());
            if(r.bottom() > rect.bottom()) rect.setBottom(r.bottom());
        }
    }
    return rect;
}

int KoTextDocumentLayout::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
    int position = hitTestIterated(document()->rootFrame()->begin(),
                        document()->rootFrame()->end(), point, accuracy);
    if (accuracy != Qt::ExactHit && position == -1)
        return document()->rootFrame()->lastPosition();
    return position;
}

int KoTextDocumentLayout::hitTestIterated(QTextFrame::iterator begin, QTextFrame::iterator end, const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
    int position = -1;
    bool basicallyFound = false;
    QTextFrame::iterator it = begin;
    for (it = begin; it != end; ++it) {
        QTextBlock block = it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(it.currentFrame());
        QTextFrame *subFrame = it.currentFrame();

        if (table) {
            QTextTableCell cell = m_state->hitTestTable(table, point);
            if (cell.isValid()) {
                position = hitTestIterated(cell.begin(), cell.end(), point,
                                accuracy);
                if (position == -1)
                    position = cell.lastPosition();
                return position;
            }
            continue;
        } else if (subFrame) {
            position = hitTestIterated(subFrame->begin(), subFrame->end(), point, accuracy);
            if (position != -1)
                return position;
            continue;
        } else {
            if (!block.isValid())
                continue;
        }
        if (basicallyFound) // a subsequent table or lines have now had their chance
            return position;

        // kDebug(32500) <<"hitTest[" << point.x() <<"," << point.y() <<"]";
        QTextLayout *layout = block.layout();
        if (point.y() > layout->boundingRect().bottom()) {
            // just skip this block.
            continue;
        }

        for (int i = 0; i < layout->lineCount(); i++) {
            QTextLine line = layout->lineAt(i);
            // kDebug(32500) <<" + line[" << line.textStart() <<"]:" << line.y() <<"-" << line.height();
            if (point.y() > line.y() + line.height()) {
                position = block.position() + line.textStart() + line.textLength();
                continue;
            }
            if (accuracy == Qt::ExactHit && point.y() < line.y()) { // between lines
                return -1;
            }
            if (accuracy == Qt::ExactHit && // left or right of line
                    (point.x() < line.naturalTextRect().left() || point.x() > line.naturalTextRect().right())) {
                return -1;
            }
            if (point.x() > line.x() + line.naturalTextWidth() && layout->textOption().textDirection() == Qt::RightToLeft) {
                // totally right of RTL text means the position is the start of the text.
                //TODO how about the other side?
                return block.position() + line.textStart();
            }
            if (point.x() > line.x() + line.naturalTextWidth()) {
                // right of line
                basicallyFound = true;
                position = block.position() + line.textStart() + line.textLength();
                continue;
            }
            return block.position() + line.xToCursor(point.x());
        }
    }
    return position;
}

int KoTextDocumentLayout::pageCount() const
{
    return 1;
}

void KoTextDocumentLayout::documentChanged(int position, int charsRemoved, int charsAdded)
{
    Q_UNUSED(charsAdded);
    Q_UNUSED(charsRemoved);
    if (shapes().count() == 0) // nothing to do.
        return;

    int from = position;
    const int to = from + charsAdded;
    while (from < to) { // find blocks that have been added
        QTextBlock block = document()->findBlock(from);
        if (! block.isValid())
            break;
        if (from == block.position() && block.textList()) {
            KoTextBlockData *data = dynamic_cast<KoTextBlockData*>(block.userData());
            if (data)
                data->setCounterWidth(-1); // invalidate whole list.
        }

        from = block.position() + block.length();
    }

    foreach(KoShape *shape, shapes()) {
        KoTextShapeData *data = qobject_cast<KoTextShapeData*>(shape->userData());
        Q_ASSERT(data);
        if (data && data->position() <= position && data->endPosition() >= position) {
            // found our (first) shape to re-layout
            data->foul();
            m_state->reset();
            scheduleLayout();
            return;
        }
    }
    // if still here; then the change was not in any frame, lets relayout the last for now.
    KoShape *shape = shapes().last();
    KoTextShapeData *data = qobject_cast<KoTextShapeData*>(shape->userData());
    Q_ASSERT(data);
    data->foul();
    m_state->reset();
    scheduleLayout();
}

void KoTextDocumentLayout::drawInlineObject(QPainter *painter, const QRectF &rect, QTextInlineObject object, int position, const QTextFormat &format)
{
    Q_ASSERT(format.isCharFormat());
    if (d->inlineTextObjectManager == 0)
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObject *obj = d->inlineTextObjectManager->inlineTextObject(cf);
    if (obj)
        obj->paint(*painter, paintDevice(), document(), rect, object, position, cf);
}

void KoTextDocumentLayout::positionInlineObject(QTextInlineObject item, int position, const QTextFormat &format)
{
    Q_ASSERT(format.isCharFormat());
    if (d->inlineTextObjectManager == 0)
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObject *obj = d->inlineTextObjectManager->inlineTextObject(cf);
    if (obj)
        obj->updatePosition(document(), item, position, cf);
}

void KoTextDocumentLayout::resizeInlineObject(QTextInlineObject item, int position, const QTextFormat &format)
{
    Q_ASSERT(format.isCharFormat());
    if (d->inlineTextObjectManager == 0)
        return;
    QTextCharFormat cf = format.toCharFormat();
    KoInlineObject *obj = d->inlineTextObjectManager->inlineTextObject(cf);
    if (obj) {
        obj->resize(document(), item, position, cf, paintDevice());
        m_state->registerInlineObject(item);
    }
}

void KoTextDocumentLayout::scheduleLayoutWithoutInterrupt()
{
    if (d->scheduled)
        return;
    d->scheduled = true;
    QTimer::singleShot(0, this, SLOT(relayoutPrivate()));
}

void KoTextDocumentLayout::scheduleLayout()
{
    if (! d->scheduled) {
        scheduleLayoutWithoutInterrupt();
        interruptLayout();
    }
}

void KoTextDocumentLayout::relayout()
{
    layout();
}

void KoTextDocumentLayout::interruptLayout()
{
    m_state->reset();
}

bool KoTextDocumentLayout::isInterrupted() const
{
    return m_state->isInterrupted();
}

void KoTextDocumentLayout::layout()
{
    d->scheduled = false;
//kDebug(32500) <<"KoTextDocumentLayout::layout";
    class End
    {
    public:
        End(LayoutState *state) {
            m_state = state;
        }
        ~End() {
            m_state->end();
        }
    private:
        LayoutState *m_state;
    };
    End ender(m_state); // poor mans finally{}

    if (! m_state->start())
        return;
    while (m_state->shape) {
        QTextLine line = m_state->layout->createLine();
        if (!line.isValid()) { // end of parag
            qreal posY = m_state->y();
            bool moreText = m_state->nextParag();
            if (m_state->shape && m_state->y() > posY)
                m_state->shape->update(QRectF(0, posY,
                                              m_state->shape->size().width(), m_state->y() - posY));

            if (! moreText) {
                emit finishedLayout();
                return; // done!
            }
            continue;
        }
        if (m_state->numColumns() > 0)
            line.setNumColumns(m_state->numColumns());
        else
            line.setLineWidth(m_state->width());
        line.setPosition(QPointF(m_state->x(), m_state->y()));
        m_state->addLine(line);
        if (m_state->shape == 0) { // shape is full!
            emit finishedLayout();
            return; // done!
        }
    }
}

QList<KoShape*> KoTextDocumentLayout::shapes() const
{
    return d->shapes;
}

KoShape* KoTextDocumentLayout::shapeForPosition(int position) const
{
    foreach(KoShape *shape, shapes()) {
        KoTextShapeData *data = qobject_cast<KoTextShapeData*>(shape->userData());
        if (data == 0)
            continue;
        if (data->position() <= position && (data->endPosition() == -1 || data->endPosition() > position))
            return shape;
    }
    return 0;
}

void KoTextDocumentLayout::setResizeMethod(KoTextDocument::ResizeMethod method)
{
    if (d->resizeMethod == method)
        return;
    d->resizeMethod = method;
    scheduleLayout();
}

KoTextDocument::ResizeMethod KoTextDocumentLayout::resizeMethod() const
{
    return d->resizeMethod;
}

#include <KoTextDocumentLayout.moc>
