/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004  Alexander Dymo <cloudtemple@mskat.net>

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
 * Boston, MA 02110-1301, USA.
*/

#include "timeedit.h"

#include <q3datetimeedit.h>
#include <q3rangecontrol.h>
#include <QObject>
#include <QPainter>
#include <QLayout>
#include <QVariant>
#include <QDateTime>
#include <QHBoxLayout>

#include <klocale.h>
#include <kglobal.h>

using namespace KoProperty;

TimeEdit::TimeEdit(Property *property, QWidget *parent)
        : Widget(property, parent)
{
    QHBoxLayout *l = new QHBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    m_edit = new Q3TimeEdit(this);
    m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_edit->setMinimumHeight(5);
    l->addWidget(m_edit);

    setLeavesTheSpaceForRevertButton(true);
    connect(m_edit, SIGNAL(valueChanged(const QTime&)), this, SLOT(slotValueChanged(const QTime&)));
}

TimeEdit::~TimeEdit()
{}

QVariant
TimeEdit::value() const
{
    return m_edit->time();
}

void
TimeEdit::setValue(const QVariant &value, bool emitChange)
{
    m_edit->blockSignals(true);
    m_edit->setTime(value.toTime());
    m_edit->blockSignals(false);
    if (emitChange)
        emit valueChanged(this);
}

void
TimeEdit::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
// p->eraseRect(r);
    Widget::drawViewer(p, cg, r, value.toDate().toString(Qt::LocalDate));
// p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, value.toDate().toString(Qt::LocalDate));
}

void
TimeEdit::slotValueChanged(const QTime&)
{
    emit valueChanged(this);
}

void
TimeEdit::setReadOnlyInternal(bool readOnly)
{
    setVisibleFlag(!readOnly);
}

#include "timeedit.moc"
