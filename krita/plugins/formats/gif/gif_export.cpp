/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "gif_export.h"

#include <QCheckBox>
#include <QSlider>

#include <kapplication.h>
#include <kdialog.h>
#include <kpluginfactory.h>
#include <kmessagebox.h>

#include <KoFilterChain.h>
#include <KoColorSpaceConstants.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>

#include "gif_converter.h"

class KisExternalLayer;

K_PLUGIN_FACTORY(ExportFactory, registerPlugin<gifExport>();)
K_EXPORT_PLUGIN(ExportFactory("kofficefilters"))

gifExport::gifExport(QObject *parent, const QVariantList &) : KoFilter(parent)
{
}

gifExport::~gifExport()
{
}

KoFilter::ConversionStatus gifExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile <<"GIF export! From:" << from <<", To:" << to <<"";

    if (from != "application/x-krita")
        return KoFilter::NotImplemented;

    if (KMessageBox::warningContinueCancel(0,
                                       i18n("You are trying to save an image in the GIF format.\n\n"
                                            "The GIF file format does not support true color images.\n"
                                            "All Krita images are true color. There will be changes in "
                                            "color and (if present) animation effects."),
                                       i18n("Krita")) == KMessageBox::Cancel) {
        return KoFilter::UserCancelled;
    }

    KisDoc2 *output = dynamic_cast<KisDoc2*>(m_chain->inputDocument());
    QString filename = m_chain->outputFile();

    if (!output)
        return KoFilter::CreationError;


    if (filename.isEmpty()) return KoFilter::FileNotFound;

    KUrl url;
    url.setPath(filename);

    KisImageWSP img = output->image();
    Q_CHECK_PTR(img);

    GifConverter kpc(output, output->undoAdapter());

    KisImageBuilder_Result res;

    if ( (res = kpc.buildFile(url, img)) == KisImageBuilder_RESULT_OK) {
        dbgFile <<"success !";
        return KoFilter::OK;
    }
    dbgFile <<" Result =" << res;
    return KoFilter::InternalError;
}

#include <gif_export.moc>

