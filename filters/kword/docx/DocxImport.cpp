/*
 * This file is part of Office 2007 Filters for KOffice
 * Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>
 * Copyright (C) 2003 David Faure <faure@kde.org>
 * Copyright (C) 2002, 2003, 2004 Nicolas GOUTTE <goutte@kde.org>
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "DocxImport.h"

#include <MsooXmlUtils.h>
#include <MsooXmlSchemas.h>
#include <MsooXmlContentTypes.h>
#include "DocxXmlDocumentReader.h"
#include "DocxXmlStylesReader.h"
#include "DocxXmlFontTableReader.h"

#include <QColor>
#include <QFile>
#include <QFont>
#include <QPen>
#include <QRegExp>
#include <QImage>

#include <kdeversion.h>
#include <KDebug>
#include <KZip>
#include <KGenericFactory>
#include <KMessageBox>

#include <KoOdfWriteStore.h>
#include <KoEmbeddedDocumentSaver.h>
#include <KoDocumentInfo.h>
#include <KoDocument.h>
#include <KoFilterChain.h>
#include <KoUnit.h>
#include <KoPageLayout.h>
#include <KoXmlWriter.h>

typedef KGenericFactory<DocxImport> DocxImportFactory;
K_EXPORT_COMPONENT_FACTORY( libdocximport, DocxImportFactory(  "kofficefilters" ) )

DocxImport::DocxImport( QObject* parent, const QStringList & )
  : MSOOXML::MsooXmlImport(QLatin1String("text"), parent)
{
}

DocxImport::~DocxImport()
{
}

bool DocxImport::acceptsSourceMimeType(const QByteArray& mime) const
{
    kDebug() << "Entering DOCX Import filter: from " << mime;
    return    mime == "application/vnd.openxmlformats-officedocument.wordprocessingml.document"
           || mime == "application/vnd.openxmlformats-officedocument.wordprocessingml.template";
}

bool DocxImport::acceptsDestinationMimeType(const QByteArray& mime) const
{
    kDebug() << "Entering DOCX Import filter: to " << mime;
    return mime == "application/vnd.oasis.opendocument.text";
}

KoFilter::ConversionStatus DocxImport::parseParts(KoOdfWriters *writers, MSOOXML::MsooXmlRelationships *relationships,
    QString& errorMessage)
{
    // more here...
    // 0. temporary styles

    //office:styles
    writers->mainStyles->addRawOdfDocumentStyles(
"    <!-- COPIED -->"
"\n    <style:default-style style:family=\"graphic\">"
"\n      <style:graphic-properties draw:shadow-offset-x=\"0.3cm\" draw:shadow-offset-y=\"0.3cm\" draw:start-line-spacing-horizontal=\"0.283cm\" draw:start-line-spacing-vertical=\"0.283cm\" draw:end-line-spacing-horizontal=\"0.283cm\" draw:end-line-spacing-vertical=\"0.283cm\" style:flow-with-text=\"false\"/>"
"\n      <style:paragraph-properties fo:line-height=\"115%\" style:text-autospace=\"ideograph-alpha\" style:line-break=\"strict\" style:writing-mode=\"lr-tb\" style:font-independent-line-spacing=\"false\">"
"\n        <style:tab-stops/>"
"\n      </style:paragraph-properties>"
"\n      <style:text-properties style:use-window-font-color=\"true\" fo:font-size=\"11pt\" fo:language=\"en\" fo:country=\"GB\" style:letter-kerning=\"true\" style:font-size-asian=\"11pt\" style:language-asian=\"en\" style:country-asian=\"US\" style:font-size-complex=\"11pt\" style:language-complex=\"ar\" style:country-complex=\"SA\"/>"
"\n    </style:default-style>"
"\n    <style:default-style style:family=\"paragraph\">"
"\n      <style:paragraph-properties fo:margin-top=\"0cm\" fo:margin-bottom=\"0.353cm\" fo:line-height=\"115%\" fo:hyphenation-ladder-count=\"no-limit\" style:text-autospace=\"ideograph-alpha\" style:punctuation-wrap=\"hanging\" style:line-break=\"strict\" style:tab-stop-distance=\"1.251cm\" style:writing-mode=\"page\"/>"
"\n      <style:text-properties style:use-window-font-color=\"true\" style:font-name=\"Calibri\" fo:font-size=\"11pt\" fo:language=\"en\" fo:country=\"GB\" style:letter-kerning=\"true\" style:font-name-asian=\"Arial1\" style:font-size-asian=\"11pt\" style:language-asian=\"en\" style:country-asian=\"US\" style:font-name-complex=\"F\" style:font-size-complex=\"11pt\" style:language-complex=\"ar\" style:country-complex=\"SA\" fo:hyphenate=\"false\" fo:hyphenation-remain-char-count=\"2\" fo:hyphenation-push-char-count=\"2\"/>"
"\n    </style:default-style>"
"\n    <style:default-style style:family=\"table\">"
"\n      <style:table-properties table:border-model=\"collapsing\"/>"
"\n    </style:default-style>"
"\n    <style:default-style style:family=\"table-row\">"
"\n      <style:table-row-properties fo:keep-together=\"auto\"/>"
"\n    </style:default-style>"
"\n    <style:style style:name=\"Standard\" style:family=\"paragraph\" style:default-outline-level=\"\" style:class=\"text\">"
"\n      <style:paragraph-properties fo:orphans=\"2\" fo:widows=\"2\" style:writing-mode=\"lr-tb\"/>"
"\n      <style:text-properties style:use-window-font-color=\"true\"/>"
"\n    </style:style>"
"\n    <style:style style:name=\"Heading\" style:family=\"paragraph\" style:parent-style-name=\"Standard\" style:next-style-name=\"Text_20_body\" style:class=\"text\">"
"\n      <style:paragraph-properties fo:margin-top=\"0.423cm\" fo:margin-bottom=\"0.212cm\" fo:keep-with-next=\"always\"/>"
"\n      <style:text-properties style:font-name=\"Arial\" fo:font-size=\"14pt\" style:font-name-asian=\"Arial1\" style:font-size-asian=\"14pt\" style:font-name-complex=\"Tahoma\" style:font-size-complex=\"14pt\"/>"
"\n    </style:style>"
"\n    <style:style style:name=\"Text_20_body\" style:display-name=\"Text body\" style:family=\"paragraph\" style:parent-style-name=\"Standard\" style:class=\"text\">"
"\n      <style:paragraph-properties fo:margin-top=\"0cm\" fo:margin-bottom=\"0.212cm\"/>"
"\n    </style:style>"
"\n    <style:style style:name=\"List\" style:family=\"paragraph\" style:parent-style-name=\"Text_20_body\" style:class=\"list\">"
"\n      <style:text-properties style:font-size-asian=\"12pt\" style:font-name-complex=\"Tahoma1\"/>"
"\n    </style:style>"
"\n    <style:style style:name=\"Caption\" style:family=\"paragraph\" style:parent-style-name=\"Standard\" style:class=\"extra\">"
"\n      <style:paragraph-properties fo:margin-top=\"0.212cm\" fo:margin-bottom=\"0.212cm\" text:number-lines=\"false\" text:line-number=\"0\"/>"
"\n      <style:text-properties fo:font-size=\"12pt\" fo:font-style=\"italic\" style:font-size-asian=\"12pt\" style:font-style-asian=\"italic\" style:font-name-complex=\"Tahoma1\" style:font-size-complex=\"12pt\" style:font-style-complex=\"italic\"/>"
"\n    </style:style>"
"\n    <style:style style:name=\"Index\" style:family=\"paragraph\" style:parent-style-name=\"Standard\" style:class=\"index\">"
"\n      <style:paragraph-properties text:number-lines=\"false\" text:line-number=\"0\"/>"
"\n      <style:text-properties style:font-size-asian=\"12pt\" style:font-name-complex=\"Tahoma1\"/>"
"\n    </style:style>"
"\n    <style:style style:name=\"Balloon_20_Text\" style:display-name=\"Balloon Text\" style:family=\"paragraph\" style:parent-style-name=\"Standard\"/>"
"\n    <style:style style:name=\"Document_20_Map\" style:display-name=\"Document Map\" style:family=\"paragraph\" style:parent-style-name=\"Standard\"/>"
"\n    <style:style style:name=\"Default_20_Paragraph_20_Font\" style:display-name=\"Default Paragraph Font\" style:family=\"text\"/>"
"\n    <style:style style:name=\"Balloon_20_Text_20_Char\" style:display-name=\"Balloon Text Char\" style:family=\"text\" style:parent-style-name=\"Default_20_Paragraph_20_Font\"/>"
"\n    <style:style style:name=\"Document_20_Map_20_Char\" style:display-name=\"Document Map Char\" style:family=\"text\" style:parent-style-name=\"Default_20_Paragraph_20_Font\"/>"
"\n    <style:style style:name=\"Graphics\" style:family=\"graphic\">"
"\n      <style:graphic-properties text:anchor-type=\"paragraph\" svg:x=\"0cm\" svg:y=\"0cm\" style:wrap=\"dynamic\" style:number-wrapped-paragraphs=\"no-limit\" style:wrap-contour=\"false\" style:vertical-pos=\"top\" style:vertical-rel=\"paragraph\" style:horizontal-pos=\"center\" style:horizontal-rel=\"paragraph\"/>"
"\n    </style:style>"
"\n    <text:outline-style style:name=\"Outline\">"
"\n      <text:outline-level-style text:level=\"1\" style:num-format=\"\">"
"\n        <style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">"
"\n          <style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"0.762cm\" fo:text-indent=\"-0.762cm\" fo:margin-left=\"0.762cm\"/>"
"\n        </style:list-level-properties>"
"\n      </text:outline-level-style>"
"\n      <text:outline-level-style text:level=\"2\" style:num-format=\"\">"
"\n        <style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">"
"\n          <style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.016cm\" fo:text-indent=\"-1.016cm\" fo:margin-left=\"1.016cm\"/>"
"\n        </style:list-level-properties>"
"\n      </text:outline-level-style>"
"\n      <text:outline-level-style text:level=\"3\" style:num-format=\"\">"
"\n        <style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">"
"\n          <style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.27cm\" fo:text-indent=\"-1.27cm\" fo:margin-left=\"1.27cm\"/>"
"\n        </style:list-level-properties>"
"\n      </text:outline-level-style>"
"\n      <text:outline-level-style text:level=\"4\" style:num-format=\"\">"
"\n        <style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">"
"\n          <style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.524cm\" fo:text-indent=\"-1.524cm\" fo:margin-left=\"1.524cm\"/>"
"\n        </style:list-level-properties>"
"\n      </text:outline-level-style>"
"\n      <text:outline-level-style text:level=\"5\" style:num-format=\"\">"
"\n        <style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">"
"\n          <style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.778cm\" fo:text-indent=\"-1.778cm\" fo:margin-left=\"1.778cm\"/>"
"\n        </style:list-level-properties>"
"\n      </text:outline-level-style>"
"\n      <text:outline-level-style text:level=\"6\" style:num-format=\"\">"
"\n        <style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">"
"\n          <style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.032cm\" fo:text-indent=\"-2.032cm\" fo:margin-left=\"2.032cm\"/>"
"\n        </style:list-level-properties>"
"\n      </text:outline-level-style>"
"\n      <text:outline-level-style text:level=\"7\" style:num-format=\"\">"
"\n        <style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">"
"\n          <style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.286cm\" fo:text-indent=\"-2.286cm\" fo:margin-left=\"2.286cm\"/>"
"\n        </style:list-level-properties>"
"\n      </text:outline-level-style>"
"\n      <text:outline-level-style text:level=\"8\" style:num-format=\"\">"
"\n        <style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">"
"\n          <style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.54cm\" fo:text-indent=\"-2.54cm\" fo:margin-left=\"2.54cm\"/>"
"\n        </style:list-level-properties>"
"\n      </text:outline-level-style>"
"\n      <text:outline-level-style text:level=\"9\" style:num-format=\"\">"
"\n        <style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">"
"\n          <style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.794cm\" fo:text-indent=\"-2.794cm\" fo:margin-left=\"2.794cm\"/>"
"\n        </style:list-level-properties>"
"\n      </text:outline-level-style>"
"\n      <text:outline-level-style text:level=\"10\" style:num-format=\"\">"
"\n        <style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">"
"\n          <style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"3.048cm\" fo:text-indent=\"-3.048cm\" fo:margin-left=\"3.048cm\"/>"
"\n        </style:list-level-properties>"
"\n      </text:outline-level-style>"
"\n    </text:outline-style>"
"\n    <text:notes-configuration text:note-class=\"footnote\" style:num-format=\"1\" text:start-value=\"0\" text:footnotes-position=\"page\" text:start-numbering-at=\"document\"/>"
"\n    <text:notes-configuration text:note-class=\"endnote\" style:num-format=\"i\" text:start-value=\"0\"/>"
"\n    <text:linenumbering-configuration text:number-lines=\"false\" text:offset=\"0.499cm\" style:num-format=\"1\" text:number-position=\"left\" text:increment=\"5\"/>"
"\n    <!-- /COPIED -->"
);

    writers->mainStyles->addRawOdfAutomaticStyles(
"\n    <!-- COPIED -->"
"\n    <style:page-layout style:name=\"Mpm1\">"
"\n      <style:page-layout-properties fo:page-width=\"20.999cm\" fo:page-height=\"29.699cm\" style:num-format=\"1\" style:print-orientation=\"portrait\" fo:margin-top=\"2.499cm\" fo:margin-bottom=\"2.499cm\" fo:margin-left=\"2.499cm\" fo:margin-right=\"2.499cm\" style:writing-mode=\"lr-tb\" style:layout-grid-color=\"#c0c0c0\" style:layout-grid-lines=\"24701\" style:layout-grid-base-height=\"0.423cm\" style:layout-grid-ruby-height=\"0cm\" style:layout-grid-mode=\"none\" style:layout-grid-ruby-below=\"false\" style:layout-grid-print=\"false\" style:layout-grid-display=\"false\" style:footnote-max-height=\"0cm\">"
"\n        <style:footnote-sep style:width=\"0.018cm\" style:distance-before-sep=\"0.101cm\" style:distance-after-sep=\"0.101cm\" style:adjustment=\"left\" style:rel-width=\"25%\" style:color=\"#000000\"/>"
"\n      </style:page-layout-properties>"
"\n      <style:header-style/>"
"\n      <style:footer-style/>"
"\n    </style:page-layout>"
"\n    <!-- /COPIED -->",
true // styles.xml
);

    // 1. parse font table
    {
        DocxXmlFontTableReaderContext context(*writers->mainStyles);
        DocxXmlFontTableReader fontTableReader(writers);
        KoFilter::ConversionStatus status = loadAndParseDocument(
            MSOOXML::ContentTypes::wordFontTable, &fontTableReader, writers, errorMessage, &context);
        if ( status != KoFilter::OK ) {
            return status;
        }
    }
    // 2. parse styles
    {
        DocxXmlStylesReader stylesReader(writers);
        KoFilter::ConversionStatus status = loadAndParseDocument(
            MSOOXML::ContentTypes::wordStyles, &stylesReader, writers, errorMessage);
        if ( status != KoFilter::OK ) {
            return status;
        }
    }
    // 3. parse document
    {
        //! @todo use m_contentTypes.values() when multiple paths are expected, e.g. for ContentTypes::wordHeader
        DocxXmlDocumentReaderContext context(
            *this, QLatin1String("word"), QLatin1String("document.xml"),
            *relationships);
        DocxXmlDocumentReader documentReader(writers);
        KoFilter::ConversionStatus status = loadAndParseDocument(
            MSOOXML::ContentTypes::wordDocument, &documentReader, writers, errorMessage, &context);
        if ( status != KoFilter::OK ) {
            return status;
        }
    }
    // more here...
    return KoFilter::OK;
}

#include "DocxImport.moc"
