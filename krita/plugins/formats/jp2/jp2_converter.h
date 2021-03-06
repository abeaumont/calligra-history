/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _JP2_CONVERTER_H_
#define _JP2_CONVERTER_H_

#include <stdio.h>

#include <QObject>

#include <kio/job.h>

#include "kis_types.h"
class KisDoc2;
class KisUndoAdapter;

struct JP2ConvertOptions {
  int rate;
  int numberresolution;
};

/**
 * Image import/export plugins can use these results to report about success or failure.
 */
enum KisImageBuilder_Result {
    KisImageBuilder_RESULT_FAILURE = -400,
    KisImageBuilder_RESULT_NOT_EXIST = -300,
    KisImageBuilder_RESULT_NOT_LOCAL = -200,
    KisImageBuilder_RESULT_BAD_FETCH = -100,
    KisImageBuilder_RESULT_INVALID_ARG = -50,
    KisImageBuilder_RESULT_OK = 0,
    KisImageBuilder_RESULT_PROGRESS = 1,
    KisImageBuilder_RESULT_EMPTY = 100,
    KisImageBuilder_RESULT_BUSY = 150,
    KisImageBuilder_RESULT_NO_URI = 200,
    KisImageBuilder_RESULT_UNSUPPORTED = 300,
    KisImageBuilder_RESULT_INTR = 400,
    KisImageBuilder_RESULT_PATH = 500,
    KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE = 600
};

class jp2Converter : public QObject
{
    Q_OBJECT
private:
    enum {
        J2K_CFMT = 0,
        JP2_CFMT = 1,
        JPT_CFMT = 2
    };
public:
    jp2Converter(KisDoc2 *doc, KisUndoAdapter *adapter);
    virtual ~jp2Converter();
public:
    KisImageBuilder_Result buildImage(const KUrl& uri);
    KisImageBuilder_Result buildFile(const KUrl& uri, KisPaintLayerSP layer, const JP2ConvertOptions& options);
    /**
     * Retrieve the constructed image
     */
    KisImageWSP getImage();
private:
    KisImageBuilder_Result decode(const KUrl& uri);
public slots:
    virtual void cancel();
private:
    int getFileFormat(const KUrl& uri) const;
private:
    KisImageWSP m_image;
    KisDoc2 *m_doc;
    KisUndoAdapter *m_adapter;
    bool m_stop;
    KIO::TransferJob *m_job;
};

#endif
