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

#include "kis_transform_worker_test.h"

#include <qtest_kde.h>
#include <KoProgressUpdater.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "kis_types.h"
#include "kis_image.h"
#include "kis_filter_strategy.h"
#include "kis_paint_device.h"
#include "kis_transform_worker.h"
#include "testutil.h"
#include "kis_transaction.h"
#include "kis_random_accessor_ng.h"

void KisTransformWorkerTest::testCreation()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisFilterStrategy * filter = new KisBoxFilterStrategy();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    KisTransformWorker tw(dev, 1.0, 1.0,
                          1.0, 1.0,
                          0.0, 0.0,
                          1.5,
                          0, 0, updater, filter, true);
}

void KisTransformWorkerTest::testMirrorX()
{

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, "");
    KisTransformWorker::mirrorX(dev2);
    KisTransformWorker::mirrorX(dev2);
    KisTransformWorker::mirrorX(dev2);
    QImage result = dev2->convertToQImage(0, 0, 0, image.width(), image.height());
    image = image.mirrored(true, false);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        // They are the same, but should be mirrored
        image.save("mirror_test_1_source.png");
        result.save("mirror_test_1_result.png");
        QFAIL(QString("Failed to mirror the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

}

void KisTransformWorkerTest::testMirrorY()
{

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, "");
    KisTransformWorker::mirrorY(dev2);
    KisTransformWorker::mirrorY(dev2);
    KisTransformWorker::mirrorY(dev2);
    QImage result = dev2->convertToQImage(0, 0, 0, image.width(), image.height());
    image = image.mirrored(false, true  );

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        // They are the same, but should be mirrored
        image.save("mirror_test_2_source.png");
        result.save("mirror_test_2_result.png");
        QFAIL(QString("Failed to mirror the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

}

void KisTransformWorkerTest::testMirrorTransactionX()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, "");

    KisTransaction t("mirror", dev2);
    KisTransformWorker::mirrorX(dev2);
    t.end();

    QImage result = dev2->convertToQImage(0, 0, 0, image.width(), image.height());

    image = image.mirrored(true, false);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        // They are the same, but should be mirrored
        image.save("mirror_test_3_source.png");
        result.save("mirror_test_3_result.png");
        QFAIL(QString("Failed to mirror the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());

    }
}
void KisTransformWorkerTest::testMirrorTransactionY()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, "");

    KisTransaction t("mirror", dev2);
    KisTransformWorker::mirrorY(dev2);
    t.end();

    QImage result = dev2->convertToQImage(0, 0, 0, image.width(), image.height());

    image = image.mirrored(false, true);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        // They are the same, but should be mirrored
        image.save("mirror_test_4_source.png");
        result.save("mirror_test_4_result.png");
        QFAIL(QString("Failed to mirror the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisTransformWorkerTest::testScaleUp()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, "");

    KisFilterStrategy * filter = new KisBoxFilterStrategy();
    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 2.4, 2.4,
                          0.0, 0.0,
                          0.0, 0.0,
                          0.0,
                          0, 0, updater, filter, true);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() == qRound(image.width() * 2.4));
    QVERIFY(rc.height() == qRound(image.height() * 2.4));

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "test_scaleup_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("test_scaleup_source.png");
        result.save("test_scaleup_result.png");
        QFAIL(QString("Failed to scale the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}


void KisTransformWorkerTest::testXScaleUp()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, "");

    KisFilterStrategy * filter = new KisBoxFilterStrategy();
    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 2.0, 1.0,
                          0.0, 0.0,
                          0.0, 0.0,
                          0.0,
                          0, 0, updater, filter, true);
    tw.run();
    t.end();
    
    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() == image.width() * 2);
    QVERIFY(rc.height() == image.height());

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "scaleupx_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("test_x_scaleup_source.png");
        result.save("test_x_scaleup_result.png");
        QFAIL(QString("Failed to scale up the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisTransformWorkerTest::testYScaleUp()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, "");

    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 1.0, 2.0,
                          0.0, 0.0,
                          0.0, 0.0,
                          0.0,
                          0, 0, updater, filter, true);
    tw.run();
    t.end();
    
    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() == image.width());
    QVERIFY(rc.height() == image.height() * 2);

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "scaleupy_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("test_y_scaleup_source.png");
        result.save("test_y_scaleup_result.png");
        QFAIL(QString("Failed to scale up the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisTransformWorkerTest::testIdentity()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, "");
    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 1.0, 1.0,
                          0.0, 0.0,
                          0.0, 0.0,
                          0.0,
                          0, 0, updater, filter, true);
    tw.run();
    t.end();
    
    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() ==image.width());
    QVERIFY(rc.height() == image.height());

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("test_identity_source.png");
        result.save("test_identity_result.png");
        QFAIL(QString("Failed to apply identity transformation to image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisTransformWorkerTest::testScaleDown()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, "");
    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 0.123, 0.123,
                          0.0, 0.0,
                          0.0, 0.0,
                          0.0,
                          0, 0, updater, filter, true);
    tw.run();
    t.end();
    
    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() == qRound(image.width() * 0.123));
    QVERIFY(rc.height() == qRound(image.height() * 0.123));

//    KisTransaction t2("test", dev);
//    KisRandomAccessorSP ac = dev->createRandomAccessorNG(rc.x(), rc.y());
//    for(int x = rc.x(); x < rc.width(); ++x) {
//        for(int y = rc.y(); y < rc.height(); ++y) {
//            ac->moveTo(x, y);
//            cs->setOpacity(ac->rawData(), 0.5, 1);
//        }
//    }
//    t2.end();

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "test_scaledown_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("test_scaledown_source.png");
        result.save("test_scaledown_result.png");
        QFAIL(QString("Failed to scale down the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}


void KisTransformWorkerTest::testXScaleDown()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, "");

    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 0.123, 1.0,
                          0.0, 0.0,
                          0.0, 0.0,
                          0.0,
                          0, 0, updater, filter, true);
    tw.run();
    t.end();
    
    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() == qRound(image.width() * 0.123));
    QVERIFY(rc.height() == image.height() - 1); // the height is reduced by 1 because in the source image
                                                // at the bottom line most pixels (except 1 or 2) are
                                                // entirely transparent.
                                                // when scaling down the image by ~ 1/10, the few non-tranparent
                                                // pixels disappear when "mixed" with the transparent ones
                                                // around

//    KisTransaction t2("test", dev);
//    KisRandomAccessorSP ac = dev->createRandomAccessorNG(rc.x(), rc.y());
//    for(int x = rc.x(); x < rc.width(); ++x) {
//        for(int y = rc.y(); y < rc.height(); ++y) {
//            ac->moveTo(x, y);
//            cs->setOpacity(ac->rawData(), 0.5, 1);
//        }
//    }
//    t2.end();

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "scaledownx_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("scaledownx_source.png");
        result.save("scaledownx_result.png");
        QFAIL(QString("Failed to scale down the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisTransformWorkerTest::testYScaleDown()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, "");

    KisFilterStrategy * filter = new KisBoxFilterStrategy();
    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 1.0, 0.123,
                          0.0, 0.0,
                          0.0, 0.0,
                          0.0,
                          0, 0, updater, filter, true);
    tw.run();
    t.end();
    
    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() == image.width());
    QVERIFY(rc.height() == qRound(image.height() * 0.123));

//    KisTransaction t2("test", dev);
//    KisRandomAccessorSP ac = dev->createRandomAccessorNG(rc.x(), rc.y());
//    for(int x = rc.x(); x < rc.width(); ++x) {
//        for(int y = rc.y(); y < rc.height(); ++y) {
//            ac->moveTo(x, y);
//            cs->setOpacity(ac->rawData(), 0.5, 1);
//        }
//    }
//    t2.end();

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "scaledowny_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("scaledowny_source.png");
        result.save("scaledowny_result.png");
        QFAIL(QString("Failed to scale down the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisTransformWorkerTest::testXShear()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, "");

    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 1.0, 1.0,
                          1.0, 0.0,
                          300., 200.,
                          0.0,
                          0, 0, updater, filter, true);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() == 959);
    QVERIFY(rc.height() == image.height());

//    KisTransaction t2("test", dev);
//    KisRandomAccessorSP ac = dev->createRandomAccessorNG(rc.x(), rc.y());
//    for(int x = rc.x(); x < rc.width(); ++x) {
//        for(int y = rc.y(); y < rc.height(); ++y) {
//            ac->moveTo(x, y);
//            cs->setOpacity(ac->rawData(), 0.5, 1);
//        }
//    }
//    t2.end();

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "shearx_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("shearx_source.png");
        result.save("shearx_result.png");
        QFAIL(QString("Failed to shear the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}


void KisTransformWorkerTest::testYShear()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, "");

    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 1.0, 1.0,
                          0.0, 1.0,
                          300., 200.,
                          0.0,
                          0, 0, updater, filter, true);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() == image.width());
    QVERIFY(rc.height() == 959);

//    KisTransaction t2("test", dev);
//    KisRandomAccessorSP ac = dev->createRandomAccessorNG(rc.x(), rc.y());
//    for(int x = rc.x(); x < rc.width(); ++x) {
//        for(int y = rc.y(); y < rc.height(); ++y) {
//            ac->moveTo(x, y);
//            cs->setOpacity(ac->rawData(), 0.5, 1);
//        }
//    }
//    t2.end();

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "sheary_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("sheary_source.png");
        result.save("sheary_result.png");
        QFAIL(QString("Failed to shear the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

}


void KisTransformWorkerTest::testRotation()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, "");

    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisTransaction t("test", dev);
    KisTransformWorker tw(dev, 1.0, 1.0,
                          0.0, 0.0,
                          0.0, 0.0,
                          34,
                          0, 0, updater, filter, true);
    tw.run();
    t.end();

    QRect rc = dev->exactBounds();

    QVERIFY(rc.width() == 702);
    QVERIFY(rc.height() == 629);

//    KisTransaction t2("test", dev);
//    KisRandomAccessorSP ac = dev->createRandomAccessorNG(rc.x(), rc.y());
//    for(int x = rc.x(); x < rc.width(); ++x) {
//        for(int y = rc.y(); y < rc.height(); ++y) {
//            ac->moveTo(x, y);
//            cs->setOpacity(ac->rawData(), 0.5, 1);
//        }
//    }
//    t2.end();

    QImage result = dev->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    QPoint errpoint;
    image.load(QString(FILES_DATA_DIR) + QDir::separator() + "rotate_result.png");
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        image.save("rotate_source.png");
        result.save("rotate_result.png");
        QFAIL(QString("Failed to rotate the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

QTEST_KDEMAIN(KisTransformWorkerTest, GUI)
#include "kis_transform_worker_test.moc"
