/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qstring.h>
#include <qfile.h>

#include <kgenericfactory.h>
#include <koDocument.h>
#include <koFilterChain.h>

#include <half.h>
#include <ImfRgbaFile.h>
//#include <ImfStringAttribute.h>
//#include <ImfMatrixAttribute.h>
//#include <ImfArray.h>
//#include <drawImage.h>

#include <iostream>

#include "kis_types.h"
#include "kis_openexr_import.h"
#include "kis_doc.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_annotation.h"
#include "kis_colorspace_registry.h"
#include "kis_iterators_pixel.h"
#include "kis_strategy_colorspace.h"
#include "kis_paint_device.h"
#include "kis_strategy_colorspace_rgb_f32.h"

using namespace std;
using namespace Imf;
using namespace Imath;

typedef KGenericFactory<KisOpenEXRImport, KoFilter> KisOpenEXRImportFactory;
K_EXPORT_COMPONENT_FACTORY(libkrita_openexr_import, KisOpenEXRImportFactory("kofficefilters"))

KisOpenEXRImport::KisOpenEXRImport(KoFilter *, const char *, const QStringList&) : KoFilter()
{
}

KisOpenEXRImport::~KisOpenEXRImport()
{
}

KoFilter::ConversionStatus KisOpenEXRImport::convert(const QCString& from, const QCString& to)
{
	if (from != "image/x-exr" || to != "application/x-krita") {
		return KoFilter::NotImplemented;
	}

	kdDebug() << "\n\n\nKrita importing from OpenEXR\n";

	KisDoc * doc = dynamic_cast<KisDoc*>(m_chain -> outputDocument());
	if (!doc) {
		return KoFilter::CreationError;
	}

	doc -> prepareForImport();

	QString filename = m_chain -> inputFile();

	if (filename.isEmpty()) {
		return KoFilter::FileNotFound;
	}

	RgbaInputFile file(QFile::encodeName(filename));
	Box2i dataWindow = file.dataWindow();
	Box2i displayWindow = file.dataWindow();

	kdDebug() << "Data window: " << QRect(dataWindow.min.x, dataWindow.min.y, dataWindow.max.x - dataWindow.min.x + 1, dataWindow.max.y - dataWindow.min.y + 1) << endl;
	kdDebug() << "Display window: " << QRect(displayWindow.min.x, displayWindow.min.y, displayWindow.max.x - displayWindow.min.x + 1, displayWindow.max.y - displayWindow.min.y + 1) << endl;

	int imageWidth = displayWindow.max.x - displayWindow.min.x + 1;
	int imageHeight = displayWindow.max.y - displayWindow.min.y + 1;

	QString imageName = "Imported from OpenEXR";

	int dataWidth  = dataWindow.max.x - dataWindow.min.x + 1;
	int dataHeight = dataWindow.max.y - dataWindow.min.y + 1;

	KisStrategyColorSpaceRGBF32SP cs = static_cast<KisStrategyColorSpaceRGBF32 *>((KisColorSpaceRegistry::instance() -> get(KisID("RGBAF32", ""))));
	     
	if (cs == 0) {
		return KoFilter::InternalError;
	}

	doc -> undoAdapter() -> setUndo(false);

	KisImageSP image = new KisImage(doc, imageWidth, imageHeight, cs, imageName);

	if (image == 0) {
		return KoFilter::CreationError;
	}

	KisLayerSP layer = image -> layerAdd(image -> nextLayerName(), OPACITY_OPAQUE);

	if (layer == 0) {
		return KoFilter::CreationError;
	}

	Rgba pixels[dataWidth];

	for (int y = 0; y < dataHeight; ++y) {

		file.setFrameBuffer(pixels - dataWindow.min.x - (dataWindow.min.y + y) * dataWidth, 1, dataWidth);
		file.readPixels(dataWindow.min.y + y);

		KisHLineIterator it = layer -> createHLineIterator(dataWindow.min.x, dataWindow.min.y + y, dataWidth, true);
		Rgba *rgba = pixels;

		while (!it.isDone()) {

			// XXX: For now unmultiply the alpha, though compositing will be faster if we
			// keep it premultiplied.
			float unmultipliedRed = rgba -> r;
			float unmultipliedGreen = rgba -> g;
			float unmultipliedBlue = rgba -> b;

			if (rgba -> a >= HALF_EPSILON) {
				unmultipliedRed /= rgba -> a;
				unmultipliedGreen /= rgba -> a;
				unmultipliedBlue /= rgba -> a;
			}

			cs -> setPixel(it.rawData(), unmultipliedRed, unmultipliedGreen, unmultipliedBlue, rgba -> a);
			++it;
			++rgba;
		}
	}

	doc -> setCurrentImage(image);
	doc -> undoAdapter() -> setUndo(true);
	doc -> setModified(false);

	return KoFilter::OK;
}

#include "kis_openexr_import.moc"

