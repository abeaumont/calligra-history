/* This file is part of the KDE project
   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KSPREAD_SHEET_VIEW
#define KSPREAD_SHEET_VIEW

#include <QObject>
#include <QPoint>
#include <QPointF>
#include <QRect>

#include "kspread_export.h"

class QPainter;
class QRect;
class QRectF;
class QSize;
class QSizeF;

class KoViewConverter;

namespace KSpread
{
class CellView;
class Region;
class Sheet;
class CanvasBase;

/**
 * \ingroup Painting
 * The SheetView controls the painting of the sheets' cells.
 * It caches a set of CellViews.
 */
class KSPREAD_EXPORT SheetView : public QObject
{
    Q_OBJECT

    friend class CellView;

public:
    /**
     * Constructor.
     */
    explicit SheetView(const Sheet* sheet);

    /**
     * Destructor.
     */
    ~SheetView();

    /**
     * \return the Sheet
     */
    const Sheet* sheet() const;

    /**
     * Sets the KoViewConverter used by this SheetView.
     */
    void setViewConverter(const KoViewConverter* viewConverter);

    /**
     * \return the view in which the Sheet is painted
     */
    const KoViewConverter* viewConverter() const;

    /**
     * Looks up a CellView for the position \p col , \p row in the cache.
     * If no CellView exists yet, one is created and inserted into the cache.
     *
     * \return the CellView for the position
     */
#ifdef KSPREAD_MT
    CellView cellView(int col, int row);
#else
    const CellView& cellView(int col, int row);
#endif

    /**
     * Set the cell range, that should be painted to \p rect .
     * It also adjusts the cache size linear to the size of \p rect .
     */
    void setPaintCellRange(const QRect& rect);

    /**
     * Invalidates all cached CellViews in \p region .
     */
    virtual void invalidateRegion(const Region& region);

    /**
     * Invalidates all CellViews, the cached and the default.
     */
    virtual void invalidate();

    /**
     * Paints the cells.
     */
    virtual void paintCells(QPainter& painter, const QRectF& paintRect, const QPointF& topLeft, CanvasBase* canvas = 0, const QRect& visibleRect = QRect());

public Q_SLOTS:
    void updateAccessedCellRange(const QPoint& location = QPoint());

Q_SIGNALS:
    void visibleSizeChanged(const QSizeF&);

protected:
    virtual CellView* createDefaultCellView();
    virtual CellView* createCellView(int col, int row);
private:
    /**
     * Helper method for invalidateRegion().
     * Invalidates all cached CellViews in \p range .
     * \internal
     */
    void invalidateRange(const QRect& range);

    /**
     * Marks other CellViews in \p range as obscured by the CellView at \p position .
     * Used by CellView.
     */
    void obscureCells(const QRect& range, const QPoint& position);

    /**
     * Returns the default CellView.
     * Used by CellView.
     */
#ifdef KSPREAD_MT
    CellView defaultCellView() const;
#else
    const CellView& defaultCellView() const;
#endif

    Q_DISABLE_COPY(SheetView)

    class Private;
    Private * const d;
};

} // namespace KSpread

#endif // KSPREAD_SHEET_VIEW
