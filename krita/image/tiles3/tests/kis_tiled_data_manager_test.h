/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KIS_TILED_DATA_MANAGER_TEST_H
#define KIS_TILED_DATA_MANAGER_TEST_H

#include <QtTest/QtTest>

class KisTiledDataManager;

class KisTiledDataManagerTest : public QObject
{
    Q_OBJECT

private:
    bool checkHole(quint8* buffer, quint8 holeColor, QRect holeRect,
                   quint8 backgroundColor, QRect backgroundRect);

    bool checkTilesShared(KisTiledDataManager *srcDM,
                          KisTiledDataManager *dstDM,
                          bool takeOldSrc, bool takeOldDst,
                          QRect tilesRect);

    bool checkTilesNotShared(KisTiledDataManager *srcDM,
                             KisTiledDataManager *dstDM,
                             bool takeOldSrc, bool takeOldDst,
                             QRect tilesRect);

private slots:
    void testUnversionedBitBlt();
    void testVersionedBitBlt();
    void testBitBltRough();
    void testTransactions();
    void testPurgeHistory();
    void benchmarkReadOnlyTileLazy();
    void benchmarkSharedPointers();

    void stressTest();
};

#endif /* KIS_TILED_DATA_MANAGER_TEST_H */

