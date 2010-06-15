/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_colselng_color_selector.h"

#include <QHBoxLayout>

#include <KoTriangleColorSelector.h>

KisColSelNgColorSelector::KisColSelNgColorSelector(QWidget* parent) : QWidget(parent)
{
    QWidget* triangleSelector = new KoTriangleColorSelector(this);
    QHBoxLayout* layout = new QHBoxLayout(this);

    layout->addWidget(triangleSelector);

    setMinimumSize(100, 100);
}