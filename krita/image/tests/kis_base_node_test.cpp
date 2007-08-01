/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include <qtest_kde.h>
#include <limits.h>

#include "kis_base_node_test.h"

#include "kis_types.h"
#include "kis_global.h"
#include "kis_base_node.h"
#include "kis_merge_visitor.h"


class TestNode : public KisBaseNode
{
    bool accept( KisLayerVisitor & )
        {
            return false;
        }
};

void KisBaseNodeTest::testCreation()
{
    KisBaseNodeSP node = new TestNode();
    QVERIFY( node->name() == QString() );
    QVERIFY( node->name() == node->objectName() );
    QVERIFY( node->icon().isNull() );
    QVERIFY( node->visible() == true );
    QVERIFY( node->locked() == false );
    QVERIFY( node->x() == 0 );
    QVERIFY( node->y() == 0 );
}

void KisBaseNodeTest::testContract()
{
    KisBaseNodeSP node = new TestNode();

    node->setName( "bla" );
    QVERIFY( node->name()  == "bla" );
    QVERIFY( node->objectName() == "bla" );

    node->setObjectName( "zxc" );
    QVERIFY( node->name()  == "zxc" );
    QVERIFY( node->objectName() == "zxc" );

    node->setVisible( !node->visible() );
    QVERIFY( node->visible() == false );

    node->setLocked( !node->locked() );
    QVERIFY( node->locked() == true );

    KoDocumentSectionModel::PropertyList list = node->properties();
    QVERIFY( list.count() == 2 );
    QVERIFY( list.at( 0 ).state == node->visible() );
    QVERIFY( list.at( 1 ).state == node->locked() );

    QImage img = node->createThumbnail( 10, 10 );
    QCOMPARE( img.size(), QSize( 10, 10 ) );
    QVERIFY( img.pixel( 5, 5 ) == QColor( 0, 0, 0, 0 ).rgba() );

}




QTEST_KDEMAIN(KisBaseNodeTest, GUI)
#include "kis_base_node_test.moc"


