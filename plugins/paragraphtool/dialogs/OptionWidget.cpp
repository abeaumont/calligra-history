/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "OptionWidget.h"

OptionWidget::OptionWidget(QWidget *parent) : QWidget(parent)
{
    widget.setupUi(this);
    connect(widget.smoothCheckBox, SIGNAL(clicked(bool)), this, SIGNAL(smoothMovementChanged(bool)));
}

OptionWidget::~OptionWidget()
{}

void OptionWidget::setStyleName(const QString &styleName)
{
    widget.styleName->setText(styleName);
}

void OptionWidget::setSmoothMovement(bool smoothMovement)
{
    widget.smoothCheckBox->setChecked(smoothMovement);
}

