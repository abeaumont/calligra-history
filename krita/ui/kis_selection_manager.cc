
/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  The outline algorith uses the limn algorithm of fontutils by
 *  Karl Berry <karl@cs.umb.edu> and Kathryn Hargreaves <letters@cs.umb.edu>
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

#include "kis_selection_manager.h"
#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QTimer>

#include <kaction.h>
#include <ktoggleaction.h>
#include <klocale.h>
#include <kstandardaction.h>
#include <kactioncollection.h>

#include "KoCanvasController.h"
#include "KoChannelInfo.h"
#include "KoIntegerMaths.h"
#include <KoDocument.h>
#include <KoMainWindow.h>
#include <KoDocumentEntry.h>
#include <KoViewConverter.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoLineBorder.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoToolProxy.h>

#include "kis_adjustment_layer.h"
#include "kis_node_manager.h"
#include "canvas/kis_canvas2.h"
#include "kis_config.h"
#include "kis_convolution_painter.h"
#include "kis_convolution_kernel.h"
#include "kis_debug.h"
#include "kis_doc2.h"
#include "kis_fill_painter.h"
#include "kis_group_layer.h"
#include "kis_image.h"
#include "kis_iterator_pixel_trait.h"
#include "kis_iterators_pixel.h"
#include "kis_layer.h"
#include "kis_statusbar.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_painter.h"
#include "kis_transaction.h"
#include "kis_selection.h"
#include "kis_types.h"
#include "kis_canvas_resource_provider.h"
#include "kis_undo_adapter.h"
#include "kis_pixel_selection.h"
#include "flake/kis_shape_selection.h"
#include "commands/kis_selection_commands.h"
#include "kis_selection_mask.h"
#include "flake/kis_shape_layer.h"
#include "kis_selection_decoration.h"
#include "canvas/kis_canvas_decoration.h"
#include "kis_node_commands_adapter.h"

#include "kis_clipboard.h"
#include "kis_view2.h"


KisSelectionManager::KisSelectionManager(KisView2 * view, KisDoc2 * doc)
        : m_view(view),
        m_doc(doc),
        m_adapter(new KisNodeCommandsAdapter(view)),
        m_copy(0),
        m_cut(0),
        m_paste(0),
        m_pasteNew(0),
        m_cutToNewLayer(0),
        m_selectAll(0),
        m_deselect(0),
        m_clear(0),
        m_reselect(0),
        m_invert(0),
        m_toNewLayer(0),
        m_smooth(0),
        m_load(0),
        m_save(0),
        m_fillForegroundColor(0),
        m_fillBackgroundColor(0),
        m_fillPattern(0),
        m_imageResizeToSelection(0)
{
    m_clipboard = KisClipboard::instance();

    KoSelection * selection = m_view->canvasBase()->globalShapeManager()->selection();
    Q_ASSERT(selection);
    connect(selection, SIGNAL(selectionChanged()), this, SLOT(shapeSelectionChanged()));

    KisSelectionDecoration* decoration = new KisSelectionDecoration(m_view);
    connect(this, SIGNAL(currentSelectionChanged()), decoration, SLOT(selectionChanged()));
    decoration->setVisible(true);
    m_view->canvasBase()->addDecoration(decoration);
}

KisSelectionManager::~KisSelectionManager()
{
    qDeleteAll(m_pluginActions);
}

void KisSelectionManager::setup(KActionCollection * collection)
{
    // XXX: setup shortcuts!

    m_cut = KStandardAction::cut(this, SLOT(cut()), collection);
    m_copy = KStandardAction::copy(this, SLOT(copy()), collection);
    m_paste = KStandardAction::paste(this, SLOT(paste()), collection);

    m_pasteNew  = new KAction(i18n("Paste into &New Image"), this);
    collection->addAction("paste_new", m_pasteNew);
    connect(m_pasteNew, SIGNAL(triggered()), this, SLOT(pasteNew()));

    m_pasteAt = new KAction(i18n("Paste at cursor"), this);
    collection->addAction("paste_at", m_pasteAt);
    connect(m_pasteAt, SIGNAL(triggered()), this, SLOT(pasteAt()));

    m_selectAll = collection->addAction(KStandardAction::SelectAll,  "select_all", this, SLOT(selectAll()));

    m_deselect = collection->addAction(KStandardAction::Deselect,  "deselect", this, SLOT(deselect()));

    m_clear = collection->addAction(KStandardAction::Clear,  "clear", this, SLOT(clear()));
    m_clear->setShortcut(QKeySequence((Qt::Key_Delete)));

    m_reselect  = new KAction(i18n("&Reselect"), this);
    collection->addAction("reselect", m_reselect);
    m_reselect->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_D));
    connect(m_reselect, SIGNAL(triggered()), this, SLOT(reselect()));

    m_invert  = new KAction(i18n("&Invert Selection"), this);
    collection->addAction("invert", m_invert);
    m_invert->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
    connect(m_invert, SIGNAL(triggered()), this, SLOT(invert()));

    m_toNewLayer  = new KAction(i18n("Copy Selection to New Layer"), this);
    collection->addAction("copy_selection_to_new_layer", m_toNewLayer);
    m_toNewLayer->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_J));
    connect(m_toNewLayer, SIGNAL(triggered()), this, SLOT(copySelectionToNewLayer()));

    m_cutToNewLayer  = new KAction(i18n("Cut Selection to New Layer"), this);
    collection->addAction("cut_selection_to_new_layer", m_cutToNewLayer);
    m_cutToNewLayer->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_J));
    connect(m_cutToNewLayer, SIGNAL(triggered()), this, SLOT(cutToNewLayer()));

    m_fillForegroundColor  = new KAction(i18n("Fill with Foreground Color"), this);
    collection->addAction("fill_selection_foreground_color", m_fillForegroundColor);
    m_fillForegroundColor->setShortcut(QKeySequence(Qt::ALT + Qt::Key_Backspace));
    connect(m_fillForegroundColor, SIGNAL(triggered()), this, SLOT(fillForegroundColor()));

    m_fillBackgroundColor  = new KAction(i18n("Fill with Background Color"), this);
    collection->addAction("fill_selection_background_color", m_fillBackgroundColor);
    m_fillBackgroundColor->setShortcut(QKeySequence(Qt::Key_Backspace));
    connect(m_fillBackgroundColor, SIGNAL(triggered()), this, SLOT(fillBackgroundColor()));

    m_fillPattern  = new KAction(i18n("Fill with Pattern"), this);
    collection->addAction("fill_selection_pattern", m_fillPattern);
    connect(m_fillPattern, SIGNAL(triggered()), this, SLOT(fillPattern()));

    m_toggleDisplaySelection  = new KToggleAction(i18n("Display Selection"), this);
    collection->addAction("toggle_display_selection", m_toggleDisplaySelection);
    m_toggleDisplaySelection->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_H));
    connect(m_toggleDisplaySelection, SIGNAL(triggered()), this, SLOT(toggleDisplaySelection()));

    m_toggleDisplaySelection->setCheckedState(KGuiItem(i18n("Hide Selection")));
    m_toggleDisplaySelection->setChecked(true);

    m_smooth  = new KAction(i18n("Smooth..."), this);
    collection->addAction("smooth", m_smooth);
    connect(m_smooth, SIGNAL(triggered()), this, SLOT(smooth()));

    m_imageResizeToSelection  = new KAction(i18n("Size Canvas to Size of Selection"), this);
    collection->addAction("resizeimagetoselection", m_imageResizeToSelection);
    connect(m_imageResizeToSelection, SIGNAL(triggered()), this, SLOT(imageResizeToSelection()));

//     m_load
//         = new KAction(i18n("Load..."),
//                   0, 0,
//                   this, SLOT(load()),
//                   collection, "load_selection");
//
//
//     m_save
//         = new KAction(i18n("Save As..."),
//                   0, 0,
//                   this, SLOT(save()),
//                   collection, "save_selection");

    QClipboard *cb = QApplication::clipboard();
    connect(cb, SIGNAL(dataChanged()), SLOT(clipboardDataChanged()));
    connect(m_view->canvasBase()->toolProxy(), SIGNAL(toolChanged(const QString&)), SLOT(clipboardDataChanged()));

}

void KisSelectionManager::clipboardDataChanged()
{
    updateGUI();
}


void KisSelectionManager::addSelectionAction(QAction * action)
{
    m_pluginActions.append(action);
}


void KisSelectionManager::updateGUI()
{
    Q_ASSERT(m_view);
    Q_ASSERT(m_clipboard);

    if (m_view == 0) {
        // "Eek, no parent!
        return;
    }

    if (m_clipboard == 0) {
        // Eek, no clipboard!
        return;
    }

    KisImageWSP image = m_view->image();
    KisLayerSP l;
    KisPaintDeviceSP dev;

    bool enable = false;

    if (image && m_view->activeDevice() && m_view->activeLayer()) {
        l = m_view->activeLayer();

        enable = l && !l->userLocked() && l->visible();
#if 0 // XXX_SELECTION (how are we going to handle deselect and
        // reselect now?
        if (l->inherits("KisAdjustmentLayer")
                if (dev && !adjLayer)
                    m_reselect->setEnabled(dev->selectionDeselected());
                    if (adjLayer) // There's no reselect for adjustment layers
                        m_reselect->setEnabled(false);
#endif
                    }

    m_clear->setEnabled(enable);
    m_cut->setEnabled(enable);
    m_fillForegroundColor->setEnabled(enable);
    m_fillBackgroundColor->setEnabled(enable);
    m_fillPattern->setEnabled(enable);

    m_cutToNewLayer->setEnabled(enable && l->selection());
    m_selectAll->setEnabled(!image.isNull());

    bool hasPixelSelection = enable && l->selection() && l->selection()->hasPixelSelection()
                             && !m_view->selection()->isDeselected();
    m_invert->setEnabled(hasPixelSelection);

    m_smooth->setEnabled(enable);
//    m_load->setEnabled(enable);
//    m_save->setEnabled(enable);


    if (m_view->selection() && !m_view->selection()->isDeselected())
        m_deselect->setEnabled(true);
    else
        m_deselect->setEnabled(false);

    if (m_view->selection() && m_view->selection()->isDeselected())
        m_reselect->setEnabled(true);
    else
        m_reselect->setEnabled(false);

    m_imageResizeToSelection->setEnabled(m_view->selection() && !m_view->selection()->isDeselected());
    if (!m_pluginActions.isEmpty()) {
        QListIterator<QAction *> i(m_pluginActions);

        while (i.hasNext()) {
            i.next()->setEnabled(!image.isNull());
        }
    }

    // You can copy from locked layers and paste the clip into a new layer, even when
    // the current layer is locked.

    enable = false;
    if (image && l) {
        enable = l->selection() && l->visible();


    }

    l = m_view->activeLayer();
    KisShapeLayer * shapeLayer = dynamic_cast<KisShapeLayer*>(l.data());
    bool shapePasteEnable = false;
    bool shapeCopyEnable = false;
    if (shapeLayer) {

        shapeCopyEnable = true;

        const QMimeData* data = QApplication::clipboard()->mimeData();
        if (data) {
            QStringList mimeTypes = m_view->canvasBase()->toolProxy()->supportedPasteMimeTypes();
            foreach(const QString & mimeType, mimeTypes) {
                if (data->hasFormat(mimeType)) {
                    shapePasteEnable = true;
                    break;
                }
            }
        }
    }

    m_copy->setEnabled(enable || shapeCopyEnable);
    m_paste->setEnabled(!image.isNull() && (m_clipboard->hasClip() || shapePasteEnable));
    m_pasteNew->setEnabled(!image.isNull() && m_clipboard->hasClip());
    m_toNewLayer->setEnabled(enable);

    //Handle the clear action disponibility
    
    if (m_view->canvasBase()->shapeManager()->selection()->count() > 0) {
        m_clear->setEnabled(true);
    }
    else if (shapeLayer && m_view->canvasBase()->shapeManager()->shapes().empty()){
        m_clear->setEnabled(false);
    }
    else {
        m_clear->setEnabled(true);
    }
        
    updateStatusBar();

}

void KisSelectionManager::updateStatusBar()
{
    if (m_view && m_view->statusBar()) {
        m_view->statusBar()->setSelection(m_view->image());
    }
}

void KisSelectionManager::selectionChanged()
{
    updateGUI();
    emit currentSelectionChanged();
}

void KisSelectionManager::cut()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer = m_view->activeLayer();
    if (!layer) return;

    if (!m_view->selection()) return;

    copy();

    KisSelectedTransaction transaction(i18n("Cut"), layer);

    layer->paintDevice()->clearSelection(m_view->selection());
    QRect rect = m_view->selection()->selectedRect();
    deselect();

    transaction.commit(m_view->image()->undoAdapter());

    layer->setDirty(rect);
}

void KisSelectionManager::copy()
{
    KisLayerSP layer = m_view->activeLayer();
    if (!layer) return;

    KisShapeLayer * shapeLayer = dynamic_cast<KisShapeLayer*>(layer.data());
    if (shapeLayer) {
        m_view->canvasBase()->toolProxy()->copy();
    } 
    else {

        KisImageWSP image = m_view->image();
        if (!image) return;

        KisPaintDeviceSP dev = m_view->activeDevice();
        if (!dev) return;

        KisSelectionSP selection = m_view->selection();

        QRect r = (selection) ? selection->selectedExactRect() : image->bounds();

        KisPaintDeviceSP clip = new KisPaintDevice(dev->colorSpace());
        Q_CHECK_PTR(clip);

        const KoColorSpace * cs = clip->colorSpace();

        // TODO if the source is linked... copy from all linked layers?!?

        // Copy image data
        KisPainter gc;
        gc.begin(clip);
        gc.setCompositeOp(COMPOSITE_COPY);
        gc.bitBlt(0, 0, dev, r.x(), r.y(), r.width(), r.height());
        gc.end();

        if (selection) {
            // Apply selection mask.

            KisHLineIteratorPixel layerIt = clip->createHLineIterator(0, 0, r.width());
            KisHLineConstIteratorPixel selectionIt = selection->createHLineIterator(r.x(), r.y(), r.width());

            for (qint32 y = 0; y < r.height(); y++) {

                while (!layerIt.isDone()) {

                    cs->applyAlphaU8Mask(layerIt.rawData(), selectionIt.rawData(), 1);


                    ++layerIt;
                    ++selectionIt;
                }
                layerIt.nextRow();
                selectionIt.nextRow();
            }
        }

        m_clipboard->setClip(clip, r.topLeft());
    }

    selectionChanged();
}


void KisSelectionManager::paste()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    //figure out where to position the clip
    // XXX: Fix this for internal points & zoom! (BSAR)
    QWidget * w = m_view->canvas();
    QPoint center = QPoint(w->width() / 2, w->height() / 2);
    QPoint bottomright = QPoint(w->width(), w->height());
    if (bottomright.x() > image->width())
        center.setX(image->width() / 2);
    if (bottomright.y() > image->height())
        center.setY(image->height() / 2);

    const KoCanvasBase* canvasBase = m_view->canvasBase();
    const KoViewConverter* viewConverter = m_view->canvasBase()->viewConverter();

    KisPaintDeviceSP clip = m_clipboard->clip(
        QPoint(
            viewConverter->viewToDocumentX(canvasBase->canvasController()->canvasOffsetX()) + center.x(),
            viewConverter->viewToDocumentY(canvasBase->canvasController()->canvasOffsetY()) + center.y()));

    if (clip) {
        KisPaintLayer *layer = new KisPaintLayer(image.data(), image->nextLayerName() + i18n("(pasted)"), OPACITY_OPAQUE_U8, clip);
        Q_CHECK_PTR(layer);

        if (m_view->activeLayer()) {
            m_adapter->addNode(layer , m_view->activeLayer()->parent(), m_view->activeLayer().data());
        } else {
            m_adapter->addNode(layer , image->rootLayer(), 0);
        }
        layer->setDirty();
        m_view->nodeManager()->activateNode(layer);

    } else
        m_view->canvasBase()->toolProxy()->paste();
}

void KisSelectionManager::pasteAt()
{
    //XXX
}

void KisSelectionManager::pasteNew()
{
    KisPaintDeviceSP clip = m_clipboard->clip(QPoint(0,0));
    if (!clip) return;

    QRect r = clip->exactBounds();
    if (r.width() < 1 && r.height() < 1) {
        // Don't paste empty clips
        return;
    }

    const QByteArray mimetype = KoDocument::readNativeFormatMimeType();
    KoDocumentEntry entry = KoDocumentEntry::queryByMimeType(mimetype);

    KisDoc2* doc = dynamic_cast<KisDoc2*>(entry.createDoc());
    if (!doc) return;

    Q_ASSERT(doc->undoAdapter() != 0);

    KisImageWSP image = new KisImage(doc->undoAdapter(), r.width(), r.height(),
                                   KoColorSpaceRegistry::instance()->colorSpace(clip->colorSpace()->colorModelId().id(), clip->colorSpace()->colorDepthId().id(), clip->colorSpace()->profile()), "Pasted");    // TODO should be translated ?
    KisPaintLayerSP layer = new KisPaintLayer(image.data(), clip->objectName(), OPACITY_OPAQUE_U8, clip->colorSpace());

    KisPainter p(layer->paintDevice());
    p.setCompositeOp(COMPOSITE_COPY);
    p.bitBlt(0, 0, clip, r.x(), r.y(), r.width(), r.height());
    p.end();

    image->addNode(layer.data(), image->rootLayer());
    doc->setCurrentImage(image);

    KoMainWindow *win = new KoMainWindow(doc->componentData());
    win->show();
    win->setRootDocument(doc);
}

void KisSelectionManager::selectAll()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisUndoAdapter *undoAdapter = m_view->image()->undoAdapter();
    undoAdapter->beginMacro(i18n("Select All"));

    if (!image->globalSelection()) {
        QUndoCommand *globalSelectionCommand =
            new KisSetGlobalSelectionCommand(image, 0, 0);

        undoAdapter->addCommand(globalSelectionCommand);
    }

    KisSelectionSP selection = image->globalSelection();

    KisSelectionTransaction transaction(QString(), image, selection);
    selection->getOrCreatePixelSelection()->select(image->bounds());
    transaction.commit(undoAdapter);

    undoAdapter->endMacro();

    m_view->selectionManager()->selectionChanged();
}

void KisSelectionManager::deselect()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisSelectionSP sel = m_view->selection();
    if (!sel) return;

    KisLayerSP layer = m_view->activeLayer();
    if (!layer) return;

    if (image->globalSelection()) {
        KisDeselectGlobalSelectionCommand* cmd = new KisDeselectGlobalSelectionCommand(image);
        m_view->document()->addCommand(cmd);
    }
}


void KisSelectionManager::clear()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisPaintDeviceSP dev = m_view->activeDevice();

    if(m_view->canvasBase()->shapeManager()->selection()->count()){
        deleteSelection();
    }else if(dev){
        
        KisSelectionSP sel = m_view->selection();

        KisTransaction transaction(i18n("Clear"), dev);

        if (sel){
            dev->clearSelection(sel);
        }else{
            dev->clear();
            dev->setDirty();
        }

        updateGUI();

        transaction.commit(image->undoAdapter());
        dev->setDirty(image->bounds());
    }
}

void KisSelectionManager::deleteSelection()
{
    if (m_view->canvasBase()->shapeManager()->selection()){
        m_view->canvasBase()->toolProxy()->deleteSelection();  
    }
    updateGUI();
}

void KisSelectionManager::fill(const KoColor& color, bool fillWithPattern, const QString& transactionText)
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisPaintDeviceSP dev = m_view->activeDevice();
    if (!dev) return;

    KisSelectionSP selection = m_view->selection();

    KisPaintDeviceSP filled = new KisPaintDevice(dev->colorSpace());

    KisFillPainter painter(filled);

    if (fillWithPattern) {
        painter.fillRect(0, 0, image->width(), image->height(),
                         m_view->resourceProvider()->currentPattern());
    } else {
        painter.fillRect(0, 0, image->width(), image->height(), color);
    }

    painter.end();

    KisPainter painter2(dev, selection);

    painter2.beginTransaction(transactionText);
    painter2.bitBlt(0, 0, filled, 0, 0, image->width(), image->height());
    painter2.endTransaction(image->undoAdapter());

    dev->setDirty();
}

void KisSelectionManager::fillForegroundColor()
{
    fill(m_view->resourceProvider()->fgColor(), false, i18n("Fill with Foreground Color"));
}

void KisSelectionManager::fillBackgroundColor()
{
    fill(m_view->resourceProvider()->bgColor(), false, i18n("Fill with Background Color"));
}

void KisSelectionManager::fillPattern()
{
    fill(KoColor(), true, i18n("Fill with Pattern"));
}

void KisSelectionManager::reselect()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer = m_view->activeLayer();
    if (!layer) return;

    if (image->globalSelection()) {
        KisReselectGlobalSelectionCommand* cmd = new KisReselectGlobalSelectionCommand(image);
        image->undoAdapter()->addCommand(cmd);
    }
}


void KisSelectionManager::invert()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisSelectionSP selection = m_view->selection();
    if (!selection) return;

    KisPixelSelectionSP pixelSelection = selection->getOrCreatePixelSelection();

    KisSelectionTransaction transaction(i18n("Invert"), image, selection);
    pixelSelection->invert();
    transaction.commit(image->undoAdapter());

    pixelSelection->setDirty(image->bounds());
    selectionChanged();
}

void KisSelectionManager::copySelectionToNewLayer()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisLayerSP layer = m_view->activeLayer();
    if (!layer) return;

    copy();
    paste();
}

void KisSelectionManager::cutToNewLayer()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisPaintDeviceSP dev = m_view->activeDevice();
    if (!dev) return;

    cut();
    paste();
}

void KisSelectionManager::toggleDisplaySelection()
{
    KisCanvasDecoration* decoration = m_view->canvasBase()->decoration("selection");
    if (decoration)
        decoration->toggleVisibility();
}

bool KisSelectionManager::displaySelection()
{
    return m_toggleDisplaySelection->isChecked();
}

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

void KisSelectionManager::grow(qint32 xradius, qint32 yradius)
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    if (!m_view->selection()) return;
    KisPixelSelectionSP selection = m_view->selection()->getOrCreatePixelSelection();

    //determine the layerSize
    QRect layerSize = image->bounds();

    /*
      Any bugs in this function are probably also in thin_region
    */

    quint8  **buf;  // caches the region's pixel data
    quint8  **max;  // caches the largest values for each column

    if (xradius <= 0 || yradius <= 0)
        return;

    KisSelectionTransaction transaction(i18n("Grow Selection"), image, m_view->selection());

    max = new quint8* [layerSize.width() + 2 * xradius];
    buf = new quint8* [yradius + 1];
    for (qint32 i = 0; i < yradius + 1; i++) {
        buf[i] = new quint8[layerSize.width()];
    }
    quint8* buffer = new quint8[(layerSize.width() + 2 * xradius) *(yradius + 1)];
    for (qint32 i = 0; i < layerSize.width() + 2 * xradius; i++) {
        if (i < xradius)
            max[i] = buffer;
        else if (i < layerSize.width() + xradius)
            max[i] = &buffer[(yradius + 1) * (i - xradius)];
        else
            max[i] = &buffer[(yradius + 1) * (layerSize.width() + xradius - 1)];

        for (qint32 j = 0; j < xradius + 1; j++)
            max[i][j] = 0;
    }
    /* offset the max pointer by xradius so the range of the array
       is [-xradius] to [region->w + xradius] */
    max += xradius;

    quint8* out = new quint8[ layerSize.width()];  // holds the new scan line we are computing

    qint32* circ = new qint32[ 2 * xradius + 1 ]; // holds the y coords of the filter's mask
    computeBorder(circ, xradius, yradius);

    /* offset the circ pointer by xradius so the range of the array
       is [-xradius] to [xradius] */
    circ += xradius;

    memset(buf[0], 0, layerSize.width());
    for (qint32 i = 0; i < yradius && i < layerSize.height(); i++) { // load top of image
        selection->readBytes(buf[i + 1], layerSize.x(), layerSize.y() + i, layerSize.width(), 1);
    }

    for (qint32 x = 0; x < layerSize.width() ; x++) { // set up max for top of image
        max[x][0] = 0;         // buf[0][x] is always 0
        max[x][1] = buf[1][x]; // MAX (buf[1][x], max[x][0]) always = buf[1][x]
        for (qint32 j = 2; j < yradius + 1; j++) {
            max[x][j] = MAX(buf[j][x], max[x][j-1]);
        }
    }

    for (qint32 y = 0; y < layerSize.height(); y++) {
        rotatePointers(buf, yradius + 1);
        if (y < layerSize.height() - (yradius))
            selection->readBytes(buf[yradius], layerSize.x(), layerSize.y() + y + yradius, layerSize.width(), 1);
        else
            memset(buf[yradius], 0, layerSize.width());
        for (qint32 x = 0; x < layerSize.width(); x++) { /* update max array */
            for (qint32 i = yradius; i > 0; i--) {
                max[x][i] = MAX(MAX(max[x][i - 1], buf[i - 1][x]), buf[i][x]);
            }
            max[x][0] = buf[0][x];
        }
        qint32 last_max = max[0][circ[-1]];
        qint32 last_index = 1;
        for (qint32 x = 0; x < layerSize.width(); x++) { /* render scan line */
            last_index--;
            if (last_index >= 0) {
                if (last_max == 255)
                    out[x] = 255;
                else {
                    last_max = 0;
                    for (qint32 i = xradius; i >= 0; i--)
                        if (last_max < max[x + i][circ[i]]) {
                            last_max = max[x + i][circ[i]];
                            last_index = i;
                        }
                    out[x] = last_max;
                }
            } else {
                last_index = xradius;
                last_max = max[x + xradius][circ[xradius]];
                for (qint32 i = xradius - 1; i >= -xradius; i--)
                    if (last_max < max[x + i][circ[i]]) {
                        last_max = max[x + i][circ[i]];
                        last_index = i;
                    }
                out[x] = last_max;
            }
        }
        selection->writeBytes(out, layerSize.x(), layerSize.y() + y, layerSize.width(), 1);
    }
    /* undo the offsets to the pointers so we can free the malloced memmory */
    circ -= xradius;
    max -= xradius;
    //XXXX: replace delete by delete[] where it is necessary to avoid memory leaks!
    delete[] circ;
    delete[] buffer;
    delete[] max;
    for (qint32 i = 0; i < yradius + 1; i++)
        delete[] buf[i];
    delete[] buf;
    delete[] out;

    transaction.commit(image->undoAdapter());

    m_view->selection()->setDirty(image->bounds());
    selectionChanged();
}

void KisSelectionManager::shrink(qint32 xradius, qint32 yradius, bool edge_lock)
{

    KisImageWSP image = m_view->image();
    if (!image) return;

    if (!m_view->selection()) return;
    KisPixelSelectionSP selection = m_view->selection()->getOrCreatePixelSelection();

    KisSelectionTransaction transaction(i18n("Shrink Selection"), image, m_view->selection());

    //determine the layerSize
    QRect layerSize = image->bounds();
    /*
      pretty much the same as fatten_region only different
      blame all bugs in this function on jaycox@gimp.org
    */
    /* If edge_lock is true  we assume that pixels outside the region
       we are passed are identical to the edge pixels.
       If edge_lock is false, we assume that pixels outside the region are 0
    */
    quint8  **buf;  // caches the the region's pixels
    quint8  **max;  // caches the smallest values for each column
    qint32    last_max, last_index;

    if (xradius <= 0 || yradius <= 0)
        return;

    max = new quint8* [layerSize.width() + 2 * xradius];
    buf = new quint8* [yradius + 1];
    for (qint32 i = 0; i < yradius + 1; i++) {
        buf[i] = new quint8[layerSize.width()];
    }

    qint32 buffer_size = (layerSize.width() + 2 * xradius + 1) * (yradius + 1);
    quint8* buffer = new quint8[buffer_size];

    if (edge_lock)
        memset(buffer, 255, buffer_size);
    else
        memset(buffer, 0, buffer_size);

    for (qint32 i = 0; i < layerSize.width() + 2 * xradius; i++) {
        if (i < xradius)
            if (edge_lock)
                max[i] = buffer;
            else
                max[i] = &buffer[(yradius + 1) * (layerSize.width() + xradius)];
        else if (i < layerSize.width() + xradius)
            max[i] = &buffer[(yradius + 1) * (i - xradius)];
        else if (edge_lock)
            max[i] = &buffer[(yradius + 1) * (layerSize.width() + xradius - 1)];
        else
            max[i] = &buffer[(yradius + 1) * (layerSize.width() + xradius)];
    }
    if (!edge_lock)
        for (qint32 j = 0 ; j < xradius + 1; j++) max[0][j] = 0;

    // offset the max pointer by xradius so the range of the array is [-xradius] to [region->w + xradius]
    max += xradius;

    quint8* out = new quint8[layerSize.width()]; // holds the new scan line we are computing

    qint32* circ = new qint32[2 * xradius + 1]; // holds the y coords of the filter's mask

    computeBorder(circ, xradius, yradius);

    // offset the circ pointer by xradius so the range of the array is [-xradius] to [xradius]
    circ += xradius;

    for (qint32 i = 0; i < yradius && i < layerSize.height(); i++) // load top of image
        selection->readBytes(buf[i + 1], layerSize.x(), layerSize.y() + i, layerSize.width(), 1);

    if (edge_lock)
        memcpy(buf[0], buf[1], layerSize.width());
    else
        memset(buf[0], 0, layerSize.width());


    for (qint32 x = 0; x < layerSize.width(); x++) { // set up max for top of image
        max[x][0] = buf[0][x];
        for (qint32 j = 1; j < yradius + 1; j++)
            max[x][j] = MIN(buf[j][x], max[x][j-1]);
    }

    for (qint32 y = 0; y < layerSize.height(); y++) {
        rotatePointers(buf, yradius + 1);
        if (y < layerSize.height() - yradius)
            selection->readBytes(buf[yradius], layerSize.x(), layerSize.y() + y + yradius, layerSize.width(), 1);
        else if (edge_lock)
            memcpy(buf[yradius], buf[yradius - 1], layerSize.width());
        else
            memset(buf[yradius], 0, layerSize.width());

        for (qint32 x = 0 ; x < layerSize.width(); x++) { // update max array
            for (qint32 i = yradius; i > 0; i--) {
                max[x][i] = MIN(MIN(max[x][i - 1], buf[i - 1][x]), buf[i][x]);
            }
            max[x][0] = buf[0][x];
        }
        last_max =  max[0][circ[-1]];
        last_index = 0;

        for (qint32 x = 0 ; x < layerSize.width(); x++) { // render scan line
            last_index--;
            if (last_index >= 0) {
                if (last_max == 0)
                    out[x] = 0;
                else {
                    last_max = 255;
                    for (qint32 i = xradius; i >= 0; i--)
                        if (last_max > max[x + i][circ[i]]) {
                            last_max = max[x + i][circ[i]];
                            last_index = i;
                        }
                    out[x] = last_max;
                }
            } else {
                last_index = xradius;
                last_max = max[x + xradius][circ[xradius]];
                for (qint32 i = xradius - 1; i >= -xradius; i--)
                    if (last_max > max[x + i][circ[i]]) {
                        last_max = max[x + i][circ[i]];
                        last_index = i;
                    }
                out[x] = last_max;
            }
        }
        selection->writeBytes(out, layerSize.x(), layerSize.y() + y, layerSize.width(), 1);
    }

    // undo the offsets to the pointers so we can free the malloced memmory
    circ -= xradius;
    max -= xradius;
    //free the memmory
    //XXXX: replace delete by delete[] where it is necessary to avoid memory leaks!
    delete[] circ;
    delete[] buffer;
    delete[] max;
    for (qint32 i = 0; i < yradius + 1; i++)
        delete buf[i];
    delete[] buf;
    delete[] out;

    transaction.commit(image->undoAdapter());

    m_view->selection()->setDirty(image->bounds());
    selectionChanged();
}

//Simple convolution filter to smooth a mask (1bpp)

void KisSelectionManager::smooth()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    if (!m_view->selection()) return;
    if (!m_view->activeLayer()) return;
    KisPixelSelectionSP selection = m_view->selection()->getOrCreatePixelSelection();

    //determine the layerSize
    QRect layerSize = m_view->activeLayer()->exactBounds();

    quint8      *buf[3];

    qint32 width = layerSize.width();

    for (qint32 i = 0; i < 3; i++) buf[i] = new quint8[width + 2];

    quint8* out = new quint8[width];

    // load top of image
    selection->readBytes(buf[0] + 1, layerSize.x(), layerSize.y(), width, 1);

    buf[0][0]         = buf[0][1];
    buf[0][width + 1] = buf[0][width];

    memcpy(buf[1], buf[0], width + 2);

    for (qint32 y = 0; y < layerSize.height(); y++) {
        if (y + 1 < layerSize.height()) {
            selection->readBytes(buf[2] + 1, layerSize.x(), layerSize.y() + y + 1, width, 1);

            buf[2][0]         = buf[2][1];
            buf[2][width + 1] = buf[2][width];
        } else {
            memcpy(buf[2], buf[1], width + 2);
        }

        for (qint32 x = 0 ; x < width; x++) {
            qint32 value = (buf[0][x] + buf[0][x+1] + buf[0][x+2] +
                            buf[1][x] + buf[2][x+1] + buf[1][x+2] +
                            buf[2][x] + buf[1][x+1] + buf[2][x+2]);

            out[x] = value / 9;
        }

        selection->writeBytes(out, layerSize.x(), layerSize.y() + y, width, 1);

        rotatePointers(buf, 3);
    }

    for (qint32 i = 0; i < 3; i++)
        delete[] buf[i];

    delete[] out;
#if 0
    dev->setDirty(image->bounds());
#endif
}

// Erode (radius 1 pixel) a mask (1bpp)

void KisSelectionManager::erode()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisSelectionSP selection = m_view->selection();
    if (!selection) return;

    KisLayerSP layer = m_view->activeLayer();
    //determine the layerSize
    QRect layerSize = layer->exactBounds();

    quint8* buf[3];


    qint32 width = layerSize.width();

    for (qint32 i = 0; i < 3; i++)
        buf[i] = new quint8[width + 2];

    quint8* out = new quint8[width];

    // load top of image
    selection->readBytes(buf[0] + 1, layerSize.x(), layerSize.y(), width, 1);

    buf[0][0]         = buf[0][1];
    buf[0][width + 1] = buf[0][width];

    memcpy(buf[1], buf[0], width + 2);

    for (qint32 y = 0; y < layerSize.height(); y++) {
        if (y + 1 < layerSize.height()) {
            selection->readBytes(buf[2] + 1, layerSize.x(), layerSize.y() + y + 1, width, 1);

            buf[2][0]         = buf[2][1];
            buf[2][width + 1] = buf[2][width];
        } else {
            memcpy(buf[2], buf[1], width + 2);
        }

        for (qint32 x = 0 ; x < width; x++) {
            qint32 min = 255;

            if (buf[0][x+1] < min) min = buf[0][x+1];
            if (buf[1][x]   < min) min = buf[1][x];
            if (buf[1][x+1] < min) min = buf[1][x+1];
            if (buf[1][x+2] < min) min = buf[1][x+2];
            if (buf[2][x+1] < min) min = buf[2][x+1];

            out[x] = min;
        }

        selection->writeBytes(out, layerSize.x(), layerSize.y() + y, width, 1);

        rotatePointers(buf, 3);
    }

    for (qint32 i = 0; i < 3; i++)
        delete[] buf[i];

    delete[] out;
#if 0
    dev->setDirty();
#endif
}

// dilate (radius 1 pixel) a mask (1bpp)

void KisSelectionManager::dilate()
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    KisSelectionSP selection = m_view->selection();
    if (!selection) return;

    KisLayerSP layer = m_view->activeLayer();
    if (!layer) return;
    //determine the layerSize
    QRect layerSize = layer->exactBounds();

    quint8* buf[3];

    qint32 width = layerSize.width();

    for (qint32 i = 0; i < 3; i++)
        buf[i] = new quint8[width + 2];

    quint8* out = new quint8[width];

    // load top of image
    selection->readBytes(buf[0] + 1, layerSize.x(), layerSize.y(), width, 1);

    buf[0][0]         = buf[0][1];
    buf[0][width + 1] = buf[0][width];

    memcpy(buf[1], buf[0], width + 2);

    for (qint32 y = 0; y < layerSize.height(); y++) {
        if (y + 1 < layerSize.height()) {
            selection->readBytes(buf[2] + 1, layerSize.x(), layerSize.y() + y + 1, width, 1);

            buf[2][0]         = buf[2][1];
            buf[2][width + 1] = buf[2][width];
        } else {
            memcpy(buf[2], buf[1], width + 2);
        }

        for (qint32 x = 0 ; x < width; x++) {
            qint32 max = 0;

            if (buf[0][x+1] > max) max = buf[0][x+1];
            if (buf[1][x]   > max) max = buf[1][x];
            if (buf[1][x+1] > max) max = buf[1][x+1];
            if (buf[1][x+2] > max) max = buf[1][x+2];
            if (buf[2][x+1] > max) max = buf[2][x+1];

            out[x] = max;
        }

        selection->writeBytes(out, layerSize.x(), layerSize.y() + y, width, 1);

        rotatePointers(buf, 3);
    }

    for (qint32 i = 0; i < 3; i++)
        delete[] buf[i];

    delete[] out;

    layer->setDirty();
}

void KisSelectionManager::border(qint32 xradius, qint32 yradius)
{
    KisImageWSP image = m_view->image();
    if (!image) return;

    if (!m_view->selection()) return;
    KisPixelSelectionSP selection = m_view->selection()->getOrCreatePixelSelection();

    //determine the layerSize
    QRect layerSize = image->bounds();

    KisSelectionTransaction transaction(i18n("Border Selection"), image, m_view->selection());

    quint8  *buf[3];
    quint8 **density;
    quint8 **transition;

    if (xradius == 1 && yradius == 1) { // optimize this case specifically
        quint8* source[3];

        for (qint32 i = 0; i < 3; i++)
            source[i] = new quint8[layerSize.width()];

        quint8* transition = new quint8[layerSize.width()];

        selection->readBytes(source[0], layerSize.x(), layerSize.y(), layerSize.width(), 1);
        memcpy(source[1], source[0], layerSize.width());
        if (layerSize.height() > 1)
            selection->readBytes(source[2], layerSize.x(), layerSize.y() + 1, layerSize.width(), 1);
        else
            memcpy(source[2], source[1], layerSize.width());

        computeTransition(transition, source, layerSize.width());
        selection->writeBytes(transition, layerSize.x(), layerSize.y(), layerSize.width(), 1);

        for (qint32 y = 1; y < layerSize.height(); y++) {
            rotatePointers(source, 3);
            if (y + 1 < layerSize.height())
                selection->readBytes(source[2], layerSize.x(), layerSize.y() + y + 1, layerSize.width(), 1);
            else
                memcpy(source[2], source[1], layerSize.width());
            computeTransition(transition, source, layerSize.width());
            selection->writeBytes(transition, layerSize.x(), layerSize.y() + y, layerSize.width(), 1);
        }

        for (qint32 i = 0; i < 3; i++)
            delete[] source[i];
        delete[] transition;

        transaction.commit(image->undoAdapter());

        m_view->selection()->setDirty(image->bounds());
        selectionChanged();
        return;
    }

    qint32* max = new qint32[layerSize.width() + 2 * xradius];
    for (qint32 i = 0; i < (layerSize.width() + 2 * xradius); i++)
        max[i] = yradius + 2;
    max += xradius;

    for (qint32 i = 0; i < 3; i++)
        buf[i] = new quint8[layerSize.width()];

    transition = new quint8*[yradius + 1];
    for (qint32 i = 0; i < yradius + 1; i++) {
        transition[i] = new quint8[layerSize.width() + 2 * xradius];
        memset(transition[i], 0, layerSize.width() + 2 * xradius);
        transition[i] += xradius;
    }
    quint8* out = new quint8[layerSize.width()];
    density = new quint8*[2 * xradius + 1];
    density += xradius;

    for (qint32 x = 0; x < (xradius + 1); x++) { // allocate density[][]
        density[ x]  = new quint8[2 * yradius + 1];
        density[ x] += yradius;
        density[-x]  = density[x];
    }
    for (qint32 x = 0; x < (xradius + 1); x++) { // compute density[][]
        double tmpx, tmpy, dist;
        quint8 a;

        if (x > 0)
            tmpx = x - 0.5;
        else if (x < 0)
            tmpx = x + 0.5;
        else
            tmpx = 0.0;

        for (qint32 y = 0; y < (yradius + 1); y++) {
            if (y > 0)
                tmpy = y - 0.5;
            else if (y < 0)
                tmpy = y + 0.5;
            else
                tmpy = 0.0;
            dist = ((tmpy * tmpy) / (yradius * yradius) +
                    (tmpx * tmpx) / (xradius * xradius));
            if (dist < 1.0)
                a = (quint8)(255 * (1.0 - sqrt(dist)));
            else
                a = 0;
            density[ x][ y] = a;
            density[ x][-y] = a;
            density[-x][ y] = a;
            density[-x][-y] = a;
        }
    }
    selection->readBytes(buf[0], layerSize.x(), layerSize.y(), layerSize.width(), 1);
    memcpy(buf[1], buf[0], layerSize.width());
    if (layerSize.height() > 1)
        selection->readBytes(buf[2], layerSize.x(), layerSize.y() + 1, layerSize.width(), 1);
    else
        memcpy(buf[2], buf[1], layerSize.width());
    computeTransition(transition[1], buf, layerSize.width());

    for (qint32 y = 1; y < yradius && y + 1 < layerSize.height(); y++) { // set up top of image
        rotatePointers(buf, 3);
        selection->readBytes(buf[2], layerSize.x(), layerSize.y() + y + 1, layerSize.width(), 1);
        computeTransition(transition[y + 1], buf, layerSize.width());
    }
    for (qint32 x = 0; x < layerSize.width(); x++) { // set up max[] for top of image
        max[x] = -(yradius + 7);
        for (qint32 j = 1; j < yradius + 1; j++)
            if (transition[j][x]) {
                max[x] = j;
                break;
            }
    }
    for (qint32 y = 0; y < layerSize.height(); y++) { // main calculation loop
        rotatePointers(buf, 3);
        rotatePointers(transition, yradius + 1);
        if (y < layerSize.height() - (yradius + 1)) {
            selection->readBytes(buf[2], layerSize.x(), layerSize.y() + y + yradius + 1, layerSize.width(), 1);
            computeTransition(transition[yradius], buf, layerSize.width());
        } else
            memcpy(transition[yradius], transition[yradius - 1], layerSize.width());

        for (qint32 x = 0; x < layerSize.width(); x++) { // update max array
            if (max[x] < 1) {
                if (max[x] <= -yradius) {
                    if (transition[yradius][x])
                        max[x] = yradius;
                    else
                        max[x]--;
                } else if (transition[-max[x]][x])
                    max[x] = -max[x];
                else if (transition[-max[x] + 1][x])
                    max[x] = -max[x] + 1;
                else
                    max[x]--;
            } else
                max[x]--;
            if (max[x] < -yradius - 1)
                max[x] = -yradius - 1;
        }
        quint8 last_max =  max[0][density[-1]];
        qint32 last_index = 1;
        for (qint32 x = 0 ; x < layerSize.width(); x++) { // render scan line
            last_index--;
            if (last_index >= 0) {
                last_max = 0;
                for (qint32 i = xradius; i >= 0; i--)
                    if (max[x + i] <= yradius && max[x + i] >= -yradius && density[i][max[x+i]] > last_max) {
                        last_max = density[i][max[x + i]];
                        last_index = i;
                    }
                out[x] = last_max;
            } else {
                last_max = 0;
                for (qint32 i = xradius; i >= -xradius; i--)
                    if (max[x + i] <= yradius && max[x + i] >= -yradius && density[i][max[x + i]] > last_max) {
                        last_max = density[i][max[x + i]];
                        last_index = i;
                    }
                out[x] = last_max;
            }
            if (last_max == 0) {
                qint32 i;
                for (i = x + 1; i < layerSize.width(); i++) {
                    if (max[i] >= -yradius)
                        break;
                }
                if (i - x > xradius) {
                    for (; x < i - xradius; x++)
                        out[x] = 0;
                    x--;
                }
                last_index = xradius;
            }
        }
        selection->writeBytes(out, layerSize.x(), layerSize.y() + y, layerSize.width(), 1);
    }
    delete [] out;

    for (qint32 i = 0; i < 3; i++)
        delete buf[i];

    max -= xradius;
    delete[] max;

    for (qint32 i = 0; i < yradius + 1; i++) {
        transition[i] -= xradius;
        delete transition[i];
    }
    delete[] transition;

    for (qint32 i = 0; i < xradius + 1 ; i++) {
        density[i] -= yradius;
        delete density[i];
    }
    density -= xradius;
    delete[] density;

    transaction.commit(image->undoAdapter());

    m_view->selection()->setDirty(image->bounds());
    selectionChanged();
}

void KisSelectionManager::feather(qint32 radius)
{
    KisImageWSP image = m_view->image();
    if (!image) return;
    if (!m_view->selection())
        return;

    KisPixelSelectionSP selection = m_view->selection()->getOrCreatePixelSelection();
    KisSelectionTransaction transaction(i18n("Feather Selection"), image, m_view->selection());

    // compute horizontal kernel
    const uint kernelSize = radius * 2 + 1;
    Matrix<qreal, Dynamic, Dynamic> gaussianMatrix(1, kernelSize);

    const qreal multiplicand = 1 / (2 * M_PI * radius * radius);
    const qreal exponentMultiplicand = 1 / (2 * radius * radius);

    for (uint x = 0; x < kernelSize; x++)
    {
        uint xDistance = qAbs((int)radius - (int)x);
        gaussianMatrix(0, x) = multiplicand * exp( -(qreal)((xDistance * xDistance) + (radius * radius)) * exponentMultiplicand );
    }

    KisConvolutionKernelSP kernelHoriz = KisConvolutionKernel::fromMatrix(gaussianMatrix, 0, gaussianMatrix.sum());
    KisConvolutionKernelSP kernelVertical = KisConvolutionKernel::fromMatrix(gaussianMatrix.transpose(), 0, gaussianMatrix.sum());

    QRect rect = selection->selectedExactRect();
    // Make sure we've got enough space around the edges.
    rect.adjust(-kernelSize, -kernelSize, kernelSize, kernelSize);
    rect &= QRect(0, 0, image->width(), image->height());

    KisPaintDeviceSP interm = new KisPaintDevice(selection->colorSpace());
    KisConvolutionPainter horizPainter(interm);
    horizPainter.setChannelFlags(interm->colorSpace()->channelFlags(false, true, false, false));
    horizPainter.applyMatrix(kernelHoriz, selection, rect.topLeft(), rect.topLeft(), rect.size(), BORDER_AVOID);
    horizPainter.end();

    KisConvolutionPainter verticalPainter(selection);
    verticalPainter.setChannelFlags(selection->colorSpace()->channelFlags(false, true, false, false));
    verticalPainter.applyMatrix(kernelVertical, interm, rect.topLeft(), rect.topLeft(), rect.size(), BORDER_AVOID);
    verticalPainter.end();

    transaction.commit(image->undoAdapter());

    m_view->selection()->setDirty(image->bounds());
    selectionChanged();
}

#define RINT(x) floor ((x) + 0.5)

void KisSelectionManager::computeBorder(qint32  *circ, qint32  xradius, qint32  yradius)
{
    qint32 i;
    qint32 diameter = xradius * 2 + 1;
    double tmp;

    for (i = 0; i < diameter; i++) {
        if (i > xradius)
            tmp = (i - xradius) - 0.5;
        else if (i < xradius)
            tmp = (xradius - i) - 0.5;
        else
            tmp = 0.0;

        circ[i] = (qint32) RINT(yradius / (double) xradius * sqrt(xradius * xradius - tmp * tmp));
    }
}

void KisSelectionManager::rotatePointers(quint8  **p, quint32 n)
{
    quint32  i;
    quint8  *tmp;

    tmp = p[0];

    for (i = 0; i < n - 1; i++) p[i] = p[i + 1];

    p[i] = tmp;
}

void KisSelectionManager::computeTransition(quint8* transition, quint8** buf, qint32 width)
{
    qint32 x = 0;

    if (width == 1) {
        if (buf[1][x] > 127 && (buf[0][x] < 128 || buf[2][x] < 128))
            transition[x] = 255;
        else
            transition[x] = 0;
        return;
    }
    if (buf[1][x] > 127) {
        if (buf[0][x] < 128 || buf[0][x + 1] < 128 ||
                buf[1][x + 1] < 128 ||
                buf[2][x] < 128 || buf[2][x + 1] < 128)
            transition[x] = 255;
        else
            transition[x] = 0;
    } else
        transition[x] = 0;
    for (qint32 x = 1; x < width - 1; x++) {
        if (buf[1][x] >= 128) {
            if (buf[0][x - 1] < 128 || buf[0][x] < 128 || buf[0][x + 1] < 128 ||
                    buf[1][x - 1] < 128           ||          buf[1][x + 1] < 128 ||
                    buf[2][x - 1] < 128 || buf[2][x] < 128 || buf[2][x + 1] < 128)
                transition[x] = 255;
            else
                transition[x] = 0;
        } else
            transition[x] = 0;
    }
    if (buf[1][x] >= 128) {
        if (buf[0][x - 1] < 128 || buf[0][x] < 128 ||
                buf[1][x - 1] < 128 ||
                buf[2][x - 1] < 128 || buf[2][x] < 128)
            transition[x] = 255;
        else
            transition[x] = 0;
    } else
        transition[x] = 0;
}

void KisSelectionManager::shapeSelectionChanged()
{
    KoShapeManager* shapeManager = m_view->canvasBase()->globalShapeManager();

    KoSelection * selection = shapeManager->selection();
    QList<KoShape*> selectedShapes = selection->selectedShapes();

    KoLineBorder* border = new KoLineBorder(0, Qt::lightGray);
    foreach(KoShape* shape, shapeManager->shapes()) {
        if (dynamic_cast<KisShapeSelection*>(shape->parent())) {
            if (selectedShapes.contains(shape))
                shape->setBorder(border);
            else
                shape->setBorder(0);
        }
    }
}

void KisSelectionManager::imageResizeToSelection()
{
    KisSelectionSP selection = m_view->selection();
    KisUndoAdapter * undoAdapter = m_view->undoAdapter();

    KisImageWSP image = m_view->image();

    if (image && selection) {

        undoAdapter->beginMacro(i18n("Resize Image to Size of Selection"));
        image->resize(selection->selectedExactRect(), true);
        undoAdapter->endMacro();
    }
}

#include "kis_selection_manager.moc"
