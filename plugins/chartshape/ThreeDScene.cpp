/* This file is part of the KDE project

   Copyright 2007 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2009 Inge Wallin    <inge@lysator.liu.se>

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


// Own
#include "ThreeDScene.h"


using namespace KChart;

class ThreeDScene::Private
{
public:
    Private();
    ~Private();

    int foo;
};

ThreeDScene::Private::Private()
{
}

ThreeDScene::Private::~Private()
{
}


using namespace KChart;

ThreeDScene::ThreeDScene()
    : d( new Private )
{
}

ThreeDScene::~ThreeDScene()
{
    delete d;
}

