/* This file is part of the KDE project
   Copyright (C) 1999 Werner Trobin <wtrobin@carinthia.com>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <paragraph.h>
#include <paragraph.moc>

Paragraph::Paragraph(const Section * const parent, const unsigned char * const mainData,
              const FIB * const fib, const QArray<int> &rowMarks,
              const QArray<int> &cellMarks) : QObject(), m_parent(parent), 
              m_mainData(mainData), m_fib(fib), m_rowMarks(rowMarks), 
              m_cellMarks(cellMarks) {

    m_success=true;
    m_paragraph=QString::null;
}

Paragraph::~Paragraph() {
}
