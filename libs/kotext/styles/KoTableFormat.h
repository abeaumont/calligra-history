/* This file is part of the KDE project
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.com>
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
#ifndef KOTABLEFORMAT_H
#define KOTABLEFORMAT_H

#include "kotext_export.h"

#include <QSharedDataPointer>
#include <QMap>

class KoTableFormatPrivate;

class QVariant;
class QString;
class QBrush;

/**
 * The KoTableFormat class describes a format for a table component such
 * as a row or a column. It is the base class for KoTableColumnFormat and
 * KoTableRowFormat.
 *
 * It is not a style, but a format. Very much like the implicitly shared
 * QTextFormat in Qt.
 *
 * \sa KoTableColumnFormat, KoTableRowFormat
 */
class KOTEXT_EXPORT KoTableFormat
{
public:
    /// Creates a new format of type \c InvalidFormat.
    KoTableFormat();
    /// Creates a format with the same attributes as \a rhs.
    KoTableFormat(const KoTableFormat &rhs);
    /// Assigns \a rhs to this format and returns a reference to this format.
    KoTableFormat& operator=(const KoTableFormat &rhs);
    /// Destroys this format.
    ~KoTableFormat();

    /// Get property \a propertyId as a QVariant.
    QVariant property(int propertyId) const;
    /// Set property \a propertyId to \a value.
    void setProperty(int propertyId, const QVariant &value);
    /// Clear property \a propertyId.
    void clearProperty(int propertyId);
    /// Returns true if this format has property \a propertyId, otherwise false.
    bool hasProperty(int propertyId) const;
    /// Returns a map with all properties of this format.
    QMap<int, QVariant> properties() const;

    /// Get bool property \a propertyId.
    bool boolProperty(int propertyId) const;
    /// Get int property \a propertyId.
    int intProperty(int propertyId) const;
    /// Get double property \a propertyId.
    qreal doubleProperty(int propertyId) const;
    /// Get string property \a propertyId.
    QString stringProperty(int propertyId) const;
    /// Get brush property \a propertyId.
    QBrush brushProperty(int propertyId) const;

private:
    QSharedDataPointer<KoTableFormatPrivate> d; // Shared data pointer.
};

#endif // KOTABLEFORMAT_H
