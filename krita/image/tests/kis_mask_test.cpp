/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_mask_test.h"

#include <qtest_kde.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_node.h"
#include "kis_mask.h"
#include "kis_selection.h"


class TestMask : public KisMask
{
public:
    using KisMask::apply;

    TestMask() : KisMask("TestMask") {
    }

    KisNodeSP clone() const {
        return new TestMask(*this);
    }

    bool allowAsChild(KisNodeSP) const {
        return false;
    }

};


void KisMaskTest::testCreation()
{
    TestMask test;
}

void KisMaskTest::testSelection()
{
    TestMask mask;
    QVERIFY(mask.selection()->isTotallyUnselected(QRect(0, 0, 1000, 1000)));
    QVERIFY(mask.exactBounds().width() == 0);
    QVERIFY(mask.extent().width() == 0);

    mask.select(QRect(0, 0, 1000, 1000));

    QCOMPARE(mask.exactBounds(), QRect(0, 0, 1000, 1000));
    QCOMPARE(mask.extent(), QRect(0, 0, 1024, 1024));

}

void KisMaskTest::testCropUpdateBySelection()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    /**
     * We do not use exact selection bounds for cropping,
     * so the rects should be covered by different tiles
     */
    QRect selectionRect(10, 10, 20, 20);
    QRect updateRect(64, 64, 20, 20);
    KisPaintDeviceSP projection = new KisPaintDevice(cs);

    TestMask mask;
    mask.select(selectionRect, MAX_SELECTED);

    mask.apply(projection, updateRect);
    // Here we crash! :)

    /**
     * If you see a crash, it means KisMask tried to update
     * the area that is outside its selection.
     * Please consider fixing KisMask::apply() first
     */
}

QTEST_KDEMAIN(KisMaskTest, GUI)
#include "kis_mask_test.moc"
