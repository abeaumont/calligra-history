/*
 This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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
 * Boston, MA 02110-1301, USA.*/

#ifndef KOUNDOSTACK_H
#define KOUNDOSTACK_H

#include <QMetaType>

#include <kundostack.h>

#include "kotext_export.h"

#include <KoDataCenter.h>

class KOTEXT_EXPORT KoUndoStack : public KUndoStack, public KoDataCenter
{
    Q_OBJECT
public:

    KoUndoStack(QObject *parent = 0);

private:

    virtual bool completeSaving(KoStore *store, KoXmlWriter *manifestWriter, KoShapeSavingContext *context);
    virtual bool completeLoading(KoStore *store);
};

Q_DECLARE_METATYPE(KoUndoStack*)

#endif // KOUNDOSTACK_H