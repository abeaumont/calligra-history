/* This file is part of the KDE project
   Copyright 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>

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
#ifndef TESTROWREPEATSTORAGE_H
#define TESTROWREPEATSTORAGE_H

#include <QObject>

namespace KSpread
{

class TestRowRepeatStorage : public QObject
{
    Q_OBJECT
private slots:
    void testEmptyStorage();
    void testSimpleSetRowRepeat();
    void testOverlappingRanges_data();
    void testOverlappingRanges();
    void testComplexSetRowRepeat();
    void testInsertRowsEmpty();
    void testInsertRowsBetween();
    void testInsertRowsMiddle();
    void testRemoveRowsEmpty();
    void testRemoveRowsBetween();
    void testRemoveRowsOverlap();
    void testInsertShiftDown1();
    void testInsertShiftDown2();
    void testInsertShiftDown3();
    void testInsertShiftDown4();
    void testRemoveShiftUp1();
    void testRemoveShiftUp2();
    void testRemoveShiftUp3();
    void testRemoveShiftUp4();
    void testInsertShiftRight();
    void testRemoveShiftLeft();
};

} // namespace KSpread

#endif // TESTROWREPEATSTORAGE_H
