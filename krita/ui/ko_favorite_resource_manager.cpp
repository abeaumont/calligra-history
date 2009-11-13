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

#include <QDebug>
#include <QStringList>
#include "ko_favorite_resource_manager.h"
#include "kis_popup_palette.h"
#include "flowlayout.h"
#include "kis_paintop_box.h"
#include "kis_palette_manager.h"
#include <kis_paintop_registry.h>
#include <KoToolManager.h>
#include <kis_paintop_preset.h>
#include <KoID.h>
#include <QDebug>
#include <QString>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <ui_wdgpaintoppresets.h>
#include <QColor>
#include "kis_view2.h"

const int KoFavoriteResourceManager::MAX_FAVORITE_BRUSHES;
const int KoFavoriteResourceManager::MAX_RECENT_COLORS;

KoFavoriteResourceManager::KoFavoriteResourceManager(KisPaintopBox *paintopBox)
        :m_favoriteBrushManager(0)
        ,m_popupPalette(0)
        ,m_paintopBox(paintopBox)
{

    //take favorite brushes from a file then append to QList
    KConfigGroup group(KGlobal::config(), "favoriteList");
    QStringList favoriteList = (group.readEntry("favoriteList")).split(",", QString::SkipEmptyParts);

    for (int pos = 0; pos < favoriteList.size(); pos++)
    {
        KisPaintOpPresetSP newBrush = m_paintopBox->paintOpPresetSP(new KoID(favoriteList[pos], favoriteList[pos]));
        KisFavoriteBrushData* newBrushData = new KisFavoriteBrushData(this, newBrush, new QIcon (m_paintopBox->paintopPixmap(newBrush->paintOp())));
        m_favoriteBrushesList.append(newBrushData);
    }
}

void KoFavoriteResourceManager::changeCurrentBrushLabel()
{
    if (m_favoriteBrushManager!=0)
        m_favoriteBrushManager->changeCurrentBrushLabel();
}

//Popup Palette
void KoFavoriteResourceManager::showPopupPalette()
{
    if (!m_popupPalette)
    {
        qDebug() << "no popup palette yet";
        m_popupPalette = new KisPopupPalette(this);
        m_popupPalette->show();
    } else {
        m_popupPalette->setVisible(!m_popupPalette->isVisible());
    }
}

void KoFavoriteResourceManager::showPaletteManager()
{
    KConfigGroup group(KGlobal::config(), "favoriteList");
    qDebug() << "[KoFavoriteResourceManager] Saved list : " << group.readEntry("favoriteList") << " | Size of list: " << this->favoriteBrushesTotal();

    if (!m_favoriteBrushManager)
    {
        m_favoriteBrushManager = new KisPaletteManager (this, m_paintopBox);

    }
    m_favoriteBrushManager->show();

}

//Favorite Brushes
int KoFavoriteResourceManager::addFavoriteBrush (KisPaintOpPresetSP newBrush)
{
    for (int pos = 0; pos < m_favoriteBrushesList.size(); pos ++)
    {
        if (newBrush->paintOp() == m_favoriteBrushesList.at(pos)->paintopPreset()->paintOp())
            return pos;
    }

    KisFavoriteBrushData* newBrushData = new KisFavoriteBrushData(this, newBrush, new QIcon (m_paintopBox->paintopPixmap(newBrush->paintOp())));
    m_favoriteBrushesList.append(newBrushData);

    if (m_popupPalette != 0)
    {
        m_popupPalette->setVisible(true);
        m_popupPalette->addFavoriteBrushButton(newBrushData);
    }
    this->saveFavoriteBrushes();

    return -1;
}

void KoFavoriteResourceManager::removeFavoriteBrush(int pos)
{
    KisFavoriteBrushData *brush = m_favoriteBrushesList.takeAt(pos);

    if (m_popupPalette != 0)
    {
        qDebug() << "popupPalette is not null";
        if (m_popupPalette->isVisible())
        {
            qDebug() << "popupPalette is Visible" ;
            m_popupPalette->removeFavoriteBrushButton(brush);
        } else {
            qDebug() << "popupPalette is not Visible" ;
            m_popupPalette->setVisible(true);
            m_popupPalette->removeFavoriteBrushButton(brush);
            m_popupPalette->setVisible(false);
        }
    }
    saveFavoriteBrushes();
}

bool KoFavoriteResourceManager::isFavoriteBrushesFull()
{
    return m_favoriteBrushesList.size() == KoFavoriteResourceManager::MAX_FAVORITE_BRUSHES;
}
int KoFavoriteResourceManager::favoriteBrushesTotal()
{
    return m_favoriteBrushesList.size();
}

KisFavoriteBrushData* KoFavoriteResourceManager::favoriteBrush(int pos)
{
    return m_favoriteBrushesList.at(pos);
}

void KoFavoriteResourceManager::changeCurrentPaintOp(KisPaintOpPresetSP brush)
{
    qDebug() << "[KoFavoriteResourceManager] Calling brush: " << brush->paintOp().id();
    m_paintopBox->setCurrentPaintop(brush);
    return;
}

//Recent Colors
int KoFavoriteResourceManager::isInRecentColor(QColor &newColor)
{
    for (int pos=0; pos < m_recentColorsData.size(); pos++)
    {
        if (newColor.rgb() == m_recentColorsData.at(pos)->color()->rgb())
            return pos;
    }

    return -1;
}

QQueue<KisRecentColorData*> * KoFavoriteResourceManager::recentColorsList()
{
    return &(m_recentColorsData);
}

void KoFavoriteResourceManager::addRecentColor(KisRecentColorData* newColor)
{

    int pos = isInRecentColor(*(newColor->color()));
    if (pos > -1)
    {
        m_recentColorsData.removeAt(pos);
    } else {
        if (m_recentColorsData.size() == KoFavoriteResourceManager::MAX_RECENT_COLORS)
        {
            KisRecentColorData *leastUsedColor = m_recentColorsData.dequeue();

            if (m_popupPalette != 0)
                m_popupPalette->removeRecentColorButton(leastUsedColor);

            delete leastUsedColor;
        }

        if (m_popupPalette != 0)
            m_popupPalette->addRecentColorButton(newColor);
    }

    m_recentColorsData.enqueue(newColor);
}

void KoFavoriteResourceManager::saveFavoriteBrushes()
{

    QString favoriteList = "";

    for (int pos = 0; pos < m_favoriteBrushesList.size(); pos++)
    {
        (favoriteList.append(m_favoriteBrushesList.at(pos)->paintopPreset()->paintOp().id())).append(",");
    }

    qDebug() << "[KoFavoriteResourceManager] Saving list: " << favoriteList;
    KConfigGroup group(KGlobal::config(), "favoriteList");    
    group.writeEntry("favoriteList", favoriteList);
    group.config()->sync();
}

KoFavoriteResourceManager::~KoFavoriteResourceManager()
{

    delete m_favoriteBrushManager;
    delete m_popupPalette;
}
#include "ko_favorite_resource_manager.moc"