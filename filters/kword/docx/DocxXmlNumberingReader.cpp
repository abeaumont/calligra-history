/*
 * This file is part of Office 2007 Filters for KOffice
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Suresh Chande suresh.chande@nokia.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include "DocxXmlNumberingReader.h"
#include <MsooXmlSchemas.h>
#include <MsooXmlUtils.h>
#include <KoXmlWriter.h>
#include <KoGenStyles.h>
#include <limits.h>
#include <MsooXmlUnits.h>

#define MSOOXML_CURRENT_NS "w"
#define MSOOXML_CURRENT_CLASS MsooXmlNumberingReader
#define BIND_READ_CLASS MSOOXML_CURRENT_CLASS

#include <MsooXmlReader_p.h>

class DocxXmlNumberingReader::Private
{
public:
    Private() : counter(0) {
    }
    ~Private() {
    }
    QString pathAndFile;
    int counter;
};

DocxXmlNumberingReader::DocxXmlNumberingReader(KoOdfWriters *writers)
    : MSOOXML::MsooXmlReader(writers)
    , d(new Private)
{
    init();
}

DocxXmlNumberingReader::~DocxXmlNumberingReader()
{
    delete d;
}

void DocxXmlNumberingReader::init()
{
    d->counter = 0;
    m_currentListStyleProperties = 0;
}

KoFilter::ConversionStatus DocxXmlNumberingReader::read(MSOOXML::MsooXmlReaderContext* context)
{
    Q_UNUSED(context)
    kDebug() << "=============================";
    readNext();
    if (!isStartDocument()) {
        return KoFilter::WrongFormat;
    }
    //w:footnotes/w:endnotes
    readNext();
    kDebug() << *this << namespaceUri();
    if (!expectEl(QList<QByteArray>() << "w:numbering")) {
        return KoFilter::WrongFormat;
    }
    if (!expectNS(MSOOXML::Schemas::wordprocessingml)) {
        return KoFilter::WrongFormat;
    }
    QXmlStreamNamespaceDeclarations namespaces(namespaceDeclarations());
    /*    for (int i = 0; i < namespaces.count(); i++) {
        kDebug() << "NS prefix:" << namespaces[i].prefix() << "uri:" << namespaces[i].namespaceUri();
    }*/
    //! @todo find out whether the namespace returned by namespaceUri()
    //!       is exactly the same ref as the element of namespaceDeclarations()

    if (!namespaces.contains(QXmlStreamNamespaceDeclaration("w", MSOOXML::Schemas::wordprocessingml))) {
        raiseError(i18n("Namespace \"%1\" not found", MSOOXML::Schemas::wordprocessingml));
        return KoFilter::WrongFormat;
    }

    const QString qn(qualifiedName().toString());

    RETURN_IF_ERROR(read_numbering())

    if (!expectElEnd(qn)) {
        return KoFilter::WrongFormat;
    }
    kDebug() << "===========finished============";
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL abstractNum
//! w:abstractNum handler (Abstract Numbering)
/*!

 Parent elements:

 Child elements:
//! @todo: Handle all children
*/
KoFilter::ConversionStatus DocxXmlNumberingReader::read_abstractNum()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());
    TRY_READ_ATTR(abstractNumId)

    m_currentListStyle = KoGenStyle(KoGenStyle::ListStyle, "list");

    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            TRY_READ_IF(lvl)
        }
        BREAK_IF_END_OF(CURRENT_EL)
    }

    m_abstractListStyles[abstractNumId] = m_currentListStyle;

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL lvl
//! w:lvl handler (Level)
/*!

 Parent elements:

 Child elements:
//! @todo: Handle all children
*/
KoFilter::ConversionStatus DocxXmlNumberingReader::read_lvl()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());

    m_currentListStyleProperties = new KoListLevelProperties;
    m_currentListStyleProperties->setBulletCharacter(QChar());

    TRY_READ_ATTR(ilvl)
    if (!ilvl.isEmpty()) {
        m_currentListStyleProperties->setLevel(ilvl.toInt());
    }

    m_bulletCharacter = QString();
    m_bulletFont = QString();
    m_bulletStyle = false;

    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            TRY_READ_IF(start)
            ELSE_TRY_READ_IF(numFmt)
            ELSE_TRY_READ_IF(lvlText)
            ELSE_TRY_READ_IF(lvlJc)
            else if ( qualifiedName() == QLatin1String("w:pPr") ) {
                TRY_READ(pPr_numbering)
            }
            else if ( qualifiedName() == QLatin1String("w:rPr") ) {
                TRY_READ(rPr_numbering)
            }
        }
        BREAK_IF_END_OF(CURRENT_EL)
    }

    // For some symbol bullets MS2007 sets the bullet char to wingdings/symbol  but since
    // ODF does not support this, we replace those cases with default value '-'
    if (m_bulletStyle && !m_bulletCharacter.isEmpty()) {
        if (m_bulletFont == "Wingdings" || m_bulletFont == "Symbol") {
            m_currentListStyleProperties->setBulletCharacter('-');
        }
        else {
            m_currentListStyleProperties->setBulletCharacter(m_bulletCharacter.at(0));
        }
    }

    QBuffer listBuf;
    KoXmlWriter listStyleWriter(&listBuf);

    m_currentListStyleProperties->saveOdf(&listStyleWriter);
    const QString elementContents = QString::fromUtf8(listBuf.buffer(), listBuf.buffer().size());
    m_currentListStyle.addChildElement("list-style-properties", elementContents);

    delete m_currentListStyleProperties;
    m_currentListStyleProperties = 0;

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL numbering
//! w:numbering handler (Numbering)
/*!

 Parent elements:

 Child elements:
//! @todo: Handle all children
*/
KoFilter::ConversionStatus DocxXmlNumberingReader::read_numbering()
{
    READ_PROLOGUE

    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            TRY_READ_IF(abstractNum)
            ELSE_TRY_READ_IF(num)
        }
        BREAK_IF_END_OF(CURRENT_EL)
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL numFmt
//! w:numfmt handler (Number format)
/*!

 Parent elements:

 Child elements:
//! @todo: Handle all children
*/
KoFilter::ConversionStatus DocxXmlNumberingReader::read_numFmt()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());

    TRY_READ_ATTR(val)
    if (!val.isEmpty()) {
        if (val == "lowerRoman") {
            m_currentListStyleProperties->setStyle(KoListStyle::RomanLowerItem);
        }
        else if (val == "lowerLetter") {
            m_currentListStyleProperties->setStyle(KoListStyle::AlphaLowerItem);
        }
        else if (val == "decimal") {
            m_currentListStyleProperties->setStyle(KoListStyle::DecimalItem);
        }
        else if (val == "upperRoman") {
            m_currentListStyleProperties->setStyle(KoListStyle::UpperRomanItem);
        }
        else if (val == "upperLetter") {
            m_currentListStyleProperties->setStyle(KoListStyle::UpperAlphaItem);
        }
        else if (val == "bullet") {
            m_bulletStyle = true;
        }
        else if (val == "ordinal") {
            // in ooxml this means having 1st, 2nd etc. but currently there's no real support for it
            m_currentListStyleProperties->setStyle(KoListStyle::DecimalItem);
            m_currentListStyleProperties->setListItemSuffix(".");
        }
    }

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL start
//! w:start handler (Start)
/*!

 Parent elements:

 Child elements:
//! @todo: Handle all children
*/
KoFilter::ConversionStatus DocxXmlNumberingReader::read_start()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());

    TRY_READ_ATTR(val)
    if (!val.isEmpty()) {
        m_currentListStyleProperties->setStartValue(val.toInt());
    }

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL lvlText
//! w:lvlText handler (Level Text)
/*!

 Parent elements:

 Child elements:
//! @todo: Handle all children
*/
KoFilter::ConversionStatus DocxXmlNumberingReader::read_lvlText()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());

    TRY_READ_ATTR(val)
    if (!val.isEmpty()) {
        if (!m_bulletStyle) {
            m_currentListStyleProperties->setListItemSuffix(val.right(1));
        }
        else {
            m_bulletCharacter = val;
        }
    }

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL num
//! w:num handler (Number)
/*!

 Parent elements:

 Child elements:
//! @todo: Handle all children
*/
KoFilter::ConversionStatus DocxXmlNumberingReader::read_num()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());

    TRY_READ_ATTR(numId);

    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            TRY_READ_IF(abstractNumId)
        }
        BREAK_IF_END_OF(CURRENT_EL)
    }

    if (!numId.isEmpty()) {
        QString name = "NumStyle" + numId;
        KoGenStyles::InsertionFlags insertionFlags = KoGenStyles::DontAddNumberToName | KoGenStyles::AllowDuplicates;
        mainStyles->insert(m_currentListStyle, name, insertionFlags);
        // Maybe this should go to styles.xml?
        //mainStyles->markStyleForStylesXml(name);
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL rPr
//! w:rpr handler (Run Properties)
/*!

 Parent elements:

 Child elements:
//! @todo: Handle all children
*/
KoFilter::ConversionStatus DocxXmlNumberingReader::read_rPr_numbering()
{
    READ_PROLOGUE

    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (qualifiedName() == QLatin1String("w:rFonts")) {
                TRY_READ(rFonts_numbering)
            }
        }
        BREAK_IF_END_OF(CURRENT_EL)
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL pPr
//! w:ppr handler (Paragraph Properties)
/*!

 Parent elements:

 Child elements:
//! @todo: Handle all children
*/
KoFilter::ConversionStatus DocxXmlNumberingReader::read_pPr_numbering()
{
    READ_PROLOGUE

    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (qualifiedName() == QLatin1String("w:ind")) {
                TRY_READ(ind_numbering)
            }
        }
        BREAK_IF_END_OF(CURRENT_EL)
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL abstractNumId
//! w:abstractNumId handler (Abstract Number Id)
/*!

 Parent elements:

 Child elements:
//! @todo: Handle all children
*/
KoFilter::ConversionStatus DocxXmlNumberingReader::read_abstractNumId()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());

    TRY_READ_ATTR(val)
    if (!val.isEmpty()) {
        m_currentListStyle = m_abstractListStyles[val];
    }

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL lvlJc
//! w:lvlJc handler (Level justification)
/*!

 Parent elements:

 Child elements:
//! @todo: Handle all children
*/
KoFilter::ConversionStatus DocxXmlNumberingReader::read_lvlJc()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());

    TRY_READ_ATTR(val)
    if (!val.isEmpty()) {
        if (val == "left") {
            m_currentListStyleProperties->setAlignment(Qt::AlignLeft);
        }
        else if (val == "center") {
            m_currentListStyleProperties->setAlignment(Qt::AlignHCenter);
        }
        else if (val == "right") {
            m_currentListStyleProperties->setAlignment(Qt::AlignRight);
        }
    }

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL ind
//! w:ind handler (Indentation)
/*!

 Parent elements:

 Child elements:
//! @todo: Handle all children
*/
KoFilter::ConversionStatus DocxXmlNumberingReader::read_ind_numbering()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());

    TRY_READ_ATTR(left)

    bool ok = false;
    const qreal leftInd = qreal(TWIP_TO_POINT(left.toDouble(&ok)));
    if (ok) {
        m_currentListStyleProperties->setIndent(leftInd);
    }

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL rFonts
//! w:rFonts handler (Run Fonts)
/*!

 Parent elements:

 Child elements:
//! @todo: Handle all children
*/
KoFilter::ConversionStatus DocxXmlNumberingReader::read_rFonts_numbering()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());

    TRY_READ_ATTR(ascii)

    if (!ascii.isEmpty()) {
        m_bulletFont = ascii;
qDebug() << "Font is " << m_bulletFont;
    }

    readNext();
    READ_EPILOGUE
}
