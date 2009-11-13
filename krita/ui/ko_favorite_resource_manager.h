/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <vla24@sfu.ca>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KO_FAVORITE_RESOURCE_MANAGER_H
#define KO_FAVORITE_RESOURCE_MANAGER_H

#include <QObject>
#include <kis_types.h>
#include <QQueue>
#include <QList>
#include "kis_favorite_brush_data.h"
#include "kis_recent_color_data.h"

class QString;
class QColor;
class KoID;
class KisPopupPalette;
class KisPaintopBox;
class KisPaletteManager;
class KisView2;

class KoFavoriteResourceManager : public QObject
{
    Q_OBJECT

public:

    KoFavoriteResourceManager(KisPaintopBox*);
    ~KoFavoriteResourceManager();

    static const int MAX_FAVORITE_BRUSHES = 10;
    static const int MAX_RECENT_COLORS = 10;

    void changeCurrentBrushLabel();
    /************************************Popup Palette************************************/

    void showPopupPalette();
    void showPaletteManager();
    void changeCurrentPaintOp(KisPaintOpPresetSP);

    /**********************************Favorite Brushes***********************************/

    /**Checks if newBrush is saved as a favorite brush.
    Returns -1 if the newBrush is not yet saved, then newBrush will be appended
    Returns the position of the brush on the list otherwise**/
    int addFavoriteBrush (KisPaintOpPresetSP);
    void removeFavoriteBrush(int);
    bool isFavoriteBrushesFull();
    int favoriteBrushesTotal();
    KisFavoriteBrushData* favoriteBrush(int);
    void saveFavoriteBrushes();


    /***********************************Recent Colors************************************/

    /**Checks if newColor is a recently used color.
    Returns -1 if the newColor is not used recently.
    Returns the position of the newColor on the list otherwise**/
    int isInRecentColor(QColor&);
    void addRecentColor(KisRecentColorData*);
    QQueue<KisRecentColorData*>* recentColorsList();
    

private:
    KisPaletteManager *m_favoriteBrushManager;
    KisPopupPalette* m_popupPalette;
    KisPaintopBox* m_paintopBox;

    QList<KisFavoriteBrushData*> m_favoriteBrushesList;

    /**The list of recently used colors**/
    QQueue<KisRecentColorData*> m_recentColorsData;
};

#endif // KIS_FAVORITE_BRUSH_DATA_H