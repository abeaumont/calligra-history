/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers

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

#include "karbon_view.h"

#include <qbuttongroup.h>
#include <qdockarea.h>
#include <qdragobject.h>
#include <qiconset.h>
#include <qpainter.h>
#include <qtoolbutton.h>
#include <qapplication.h>
#include <qclipboard.h>

#include <kaction.h>
#include <kcolordrag.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <koMainWindow.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include "kocontexthelp.h"

#include "vtoolbox.h"

#include "vstroke.h"

// Commands.
#include "vcleanupcmd.h"
#include "vdeletecmd.h"
#include "vfillcmd.h"
#include "vgroupcmd.h"
#include "vungroupcmd.h"
#include "vzordercmd.h"
#include "vstrokecmd.h"

// Dialogs.
#include "vconfiguredlg.h"
#include "vfilldlg.h"
#include "vstrokedlg.h"

// Dockers.
#include "vcolordocker.h"
#include "vdocumentdocker.h"
#include "vstrokedocker.h"
#include "vtooloptionsdocker.h"
#include "vtransformdocker.h"

// ToolBars
#include "vselecttoolbar.h"

// The rest.
#include "karbon_factory.h"
#include "karbon_part.h"
#include "karbon_view_iface.h"
#include "vselection.h"
#include "vtool.h"
#include "vtoolfactory.h"
#include "vgroup.h"
#include "vpath.h"
#include "vpainterfactory.h"
#include "vqpainter.h"
#include "vstrokefillpreview.h"
#include "vstatebutton.h"
#include "vcanvas.h"
#include "vlayer.h"
#include <kdeversion.h>

// Only for debugging.
#include <kdebug.h>
#include "vcomposite.h"
#include "vpath.h"
#include "vsegment.h"

#include "koUnitWidgets.h"


KarbonView::KarbonView( KarbonPart* p, QWidget* parent, const char* name )
		: KarbonViewBase( p, parent, name ), KXMLGUIBuilder( shell() )
{
	m_toolbox = 0L;
	m_currentTool = 0L;
	m_toolFactory = new VToolFactory( this );

	setInstance( KarbonFactory::instance() );
	setAcceptDrops( true );

	m_toolbox->setupTools();

	setClientBuilder( this );

	if( !p->isReadWrite() )
		setXMLFile( QString::fromLatin1( "karbon_readonly.rc" ) );
	else
		setXMLFile( QString::fromLatin1( "karbon.rc" ) );

	m_dcop = 0L;

	dcopObject(); // build it

	// tools:
	//m_tools.setAutoDelete( true );
	//m_selectTool = new VSelectTool( this );

	// set up status bar message
	m_status = new KStatusBarLabel( QString::null, 0, statusBar() );

	m_status->setAlignment( AlignLeft | AlignVCenter );

	m_status->setMinimumWidth( 300 );

	addStatusBarItem( m_status, 0 );

	initActions();

	m_strokeFillPreview = 0L;

	m_ColorManager = 0L;

	m_strokeDocker = 0L;

	if( shell() )
	{
		//Create Dockers
		m_ColorManager = new VColorDocker( part(), this );
		m_strokeDocker = new VStrokeDocker( part(), this );

		m_TransformDocker = new VTransformDocker( part(), this );

		//create toolbars
		m_selectToolBar = new VSelectToolBar( this, "selecttoolbar" );
		mainWindow()->addToolBar( m_selectToolBar );
	}

	setNumberOfRecentFiles( part()->maxRecentFiles() );

	reorganizeGUI();

	// widgets:
	m_canvas = new VCanvas( this, p );
	m_canvas->setGeometry( 0, 0, width(), height() );

	// set up factory
	m_painterFactory = new VPainterFactory;
	m_painterFactory->setPainter( canvasWidget()->pixmap(), width(), height() );
	m_painterFactory->setEditPainter( canvasWidget()->viewport(), width(), height() );

	zoomChanged();
}

KarbonView::~KarbonView()
{
	delete m_toolFactory;
	// dialogs:

	if( shell() )
	{
		delete( m_ColorManager );
		delete( m_strokeDocker );
		delete( m_TransformDocker );
	}

	// widgets:
	delete( m_status );

	delete( m_painterFactory );

	delete( m_canvas );

	delete( m_dcop );
}

void
KarbonView::registerTool( VTool *tool )
{
	if( !m_toolbox )
		m_toolbox = new VToolBox( (KarbonPart *)m_part, mainWindow(), "toolbox" );
	m_toolbox->registerTool( tool );
	m_currentTool = tool;
}

QWidget *
KarbonView::createContainer( QWidget *parent, int index, const QDomElement &element, int &id )
{
	if( element.attribute( "name" ) == "toolbox" )
	{
		if( !m_toolbox )
			m_toolbox = new VToolBox( (KarbonPart *)m_part, mainWindow(), "toolbox" );
		connect( m_toolbox, SIGNAL( activeToolChanged( VTool * ) ), this, SLOT( slotActiveToolChanged( VTool * ) ) );

		if( shell() )
		{
			m_strokeFillPreview = m_toolbox->strokeFillPreview();
			connect( m_strokeFillPreview, SIGNAL( strokeChanged( const VStroke & ) ), this, SLOT( selectionChanged() ) );
			connect( m_strokeFillPreview, SIGNAL( fillChanged( const VFill & ) ), this, SLOT( selectionChanged() ) );

			connect( m_strokeFillPreview, SIGNAL( strokeSelected() ), m_ColorManager, SLOT( setStrokeDocker() ) );
			connect( m_strokeFillPreview, SIGNAL( fillSelected( ) ), m_ColorManager, SLOT( setFillDocker() ) );
			selectionChanged();

			m_documentDocker = new VDocumentDocker( this );
			mainWindow()->addDockWindow( m_documentDocker, DockRight );
			m_toolOptionsDocker = new VToolOptionsDocker( this );
			m_toolOptionsDocker->show();
			//selectTool();
		}

		mainWindow()->moveDockWindow( m_toolbox, Qt::DockLeft, false, 0 );
		return m_toolbox;
	}

	return KXMLGUIBuilder::createContainer( parent, index, element, id );
}

void
KarbonView::removeContainer( QWidget *container, QWidget *parent,
							 QDomElement &element, int id )
{
	if( m_toolbox )
	{
		delete m_toolbox;
		delete m_toolOptionsDocker;
		delete m_documentDocker;
		m_toolbox = 0L;
		return ;
	}

	KXMLGUIBuilder::removeContainer( container, parent, element, id );
}


DCOPObject *
KarbonView::dcopObject()
{
	if( !m_dcop )
		m_dcop = new KarbonViewIface( this );

	return m_dcop;
}

void
KarbonView::updateReadWrite( bool /*rw*/ )
{}

QWidget*
KarbonView::canvas() const
{
	return m_canvas;
}

void
KarbonView::resizeEvent( QResizeEvent* /*event*/ )
{
	m_painterFactory->painter()->resize( width(), height() );
	m_painterFactory->editpainter()->resize( width(), height() );
	m_canvas->resize( width(), height() );
	//kdDebug() << "Moving to : " << m_canvas->contentsHeight() - height() << endl;
	m_canvas->setContentsPos( 0, m_canvas->contentsHeight() - height() );
	reorganizeGUI();
}

void
KarbonView::dragEnterEvent( QDragEnterEvent *event )
{
	event->accept( KColorDrag::canDecode( event ) );
}

void
KarbonView::dropEvent ( QDropEvent *e )
{
	//Accepts QColor - from Color Manager's KColorPatch
	QColor color;
	VColor realcolor;

	if( KColorDrag::decode( e, color ) )
	{
		float r = color.red() / 255.0;
		float g = color.green() / 255.0;
		float b = color.blue() / 255.0;

		realcolor.set( r, g, b );

		if( part() )
			part()->addCommand( new VFillCmd( &part()->document(), realcolor ), true );

		selectionChanged();
	}
}

void
KarbonView::setupPrinter( KPrinter& /*printer*/ )
{}

void
KarbonView::print( KPrinter &printer )
{
	VQPainter p( ( QPaintDevice * ) & printer, width(), height() );
	p.begin();
	p.setZoomFactor( 1.0 );

	// print the doc using QPainter at zoom level 1
	// TODO : better use eps export?
	// TODO : use real page layout stuff
	QPtrListIterator<VLayer> i = part()->document().layers();
	KoRect rect( 0, 0, width(), height() );

	for( ; i.current(); ++i )
		//if( i.current()->visible() )
		i.current()->draw( &p, &rect );

	p.end();
}

void
KarbonView::editCut()
{
	addSelectionToClipboard();
	// remove selection
	editDeleteSelection();
}

void
KarbonView::editCopy()
{
	addSelectionToClipboard();
}

void
KarbonView::addSelectionToClipboard() const
{
	QClipboard *clipboard = QApplication::clipboard();
	VObjectListIterator itr( part()->document().selection()->objects() );
	// build a xml fragment containing the selection as karbon xml
	QDomDocument doc( "clip" );
	QDomElement elem = doc.createElement( "clip" );
	QString result;
	QTextStream ts( &result, IO_WriteOnly );
	for( ; itr.current() ; ++itr )
		itr.current()->save( elem );
	ts << elem;
	// push to clipbaord
	clipboard->setText( result.latin1() );

}

void
KarbonView::editPaste()
{
	QClipboard *clipboard = QApplication::clipboard();
	QDomDocument doc( "clip" );
	doc.setContent( clipboard->text() );
	QDomElement clip = doc.documentElement();
	// Try to parse the clipboard data
	if( clip.tagName() == "clip" )
	{
		VObjectList selection;
		// Use group to assemble the xml contents
		// TODO : maybe not clone() so much
		VGroup grp( 0L );
		grp.load( clip );
		VObjectListIterator itr( grp.objects() );
		for( ; itr.current() ; ++itr )
			selection.append( itr.current()->clone() );

		part()->document().selection()->clear();

		// Calc new selection
		VObjectListIterator itr2( selection );

		for( ; itr2.current() ; ++itr2 )
		{
			part()->insertObject( itr2.current() );
			part()->document().selection()->append( itr2.current() );
		}

		part()->repaintAllViews();
	}
}

void
KarbonView::editSelectAll()
{
	part()->document().selection()->append();

	if( part()->document().selection()->objects().count() > 0 )
		part()->repaintAllViews();

	selectionChanged();
}

void
KarbonView::editDeselectAll()
{
	if( part()->document().selection()->objects().count() > 0 )
	{
		part()->document().selection()->clear();
		part()->repaintAllViews();
	}

	selectionChanged();
}

void
KarbonView::editDeleteSelection()
{
	kdDebug() << "*********" << endl;

	if( part()->document().selection()->objects().count() > 0 )
	{
		part()->addCommand(
			new VDeleteCmd( &part()->document() ),
			true );
	}
}

void
KarbonView::editPurgeHistory()
{
	// TODO: check for history size != 0

	if( KMessageBox::warningContinueCancel( this,
			i18n( "This action cannot be undone later. Do you really want to continue?" ),
			i18n( "Purge History" ),
			i18n( "C&ontinue" ),		// TODO: is there a constant for this?
			"edit_purge_history" ) )
	{
		// Use the VCleanUp command to remove "deleted"
		// objects from all layers.
		VCleanUpCmd cmd( &part()->document() );
		cmd.execute();

		part()->clearHistory();
	}
}

void
KarbonView::selectionBringToFront()
{
	part()->addCommand(
		new VZOrderCmd( &part()->document(), VZOrderCmd::bringToFront ), true );
}

void
KarbonView::selectionMoveUp()
{
	part()->addCommand(
		new VZOrderCmd( &part()->document(), VZOrderCmd::up ), true );
}

void
KarbonView::selectionMoveDown()
{
	part()->addCommand(
		new VZOrderCmd( &part()->document(), VZOrderCmd::down ), true );
}

void
KarbonView::selectionSendToBack()
{
	part()->addCommand(
		new VZOrderCmd( &part()->document(), VZOrderCmd::sendToBack ), true );
}

void
KarbonView::groupSelection()
{
	part()->addCommand( new VGroupCmd( &part()->document() ), true );
	selectionChanged();
}

void
KarbonView::ungroupSelection()
{
	part()->addCommand( new VUnGroupCmd( &part()->document() ), true );
	selectionChanged();
}

// TODO: remove this one someday:
void
KarbonView::dummyForTesting()
{
	kdDebug() << "KarbonView::dummyForTesting()" << endl;

	VPath p( 0L );
	p.moveTo( KoPoint( 235.61, 177.628 ) );
	p.curveTo( KoPoint( 499.791, 462.572 ), KoPoint( 82.4413, 334.715 ), KoPoint( 66.8293, 192.397 ) );

	KoPoint input( 152, 250 );

	VPath q( 0L );
	q.moveTo( input );

	KoPoint output =
		p.getLast()->pointAt(
			p.getLast()->nearestPointParam( input ) );

	q.lineTo( output );


	VComposite* comp = new VComposite( 0L );
	comp->combinePath( p );
	part()->document().append( comp );

	comp = new VComposite( 0L );
	comp->combinePath( q );
	part()->document().append( comp );

	part()->repaintAllViews();
}

void
KarbonView::objectTrafoTranslate()
{
	if( m_TransformDocker->isVisible() == false )
	{
		mainWindow()->addDockWindow( m_TransformDocker, DockRight );
		m_TransformDocker->setTab( Translate );
		m_TransformDocker->show();
	}
}

void
KarbonView::objectTrafoScale()
{
	if( m_TransformDocker->isVisible() == false )
	{
		mainWindow()->addDockWindow( m_TransformDocker, DockRight );
		m_TransformDocker->setTab( Scale );
		m_TransformDocker->show();
	}
}

void
KarbonView::objectTrafoRotate()
{
	if( m_TransformDocker->isVisible() == false )
	{
		mainWindow()->addDockWindow( m_TransformDocker, DockRight );
		m_TransformDocker->setTab( Rotate );
		m_TransformDocker->show();
	}
}

void
KarbonView::objectTrafoShear()
{
	if( m_TransformDocker->isVisible() == false )
	{
		mainWindow()->addDockWindow( m_TransformDocker, DockRight );
		m_TransformDocker->setTab( Shear );
		m_TransformDocker->show();
	}
}

void
KarbonView::slotActiveToolChanged( VTool *tool )
{
	m_currentTool->deactivate();

	m_currentTool = tool;

	m_currentTool->activateAll();

	m_canvas->repaintAll();
}

void
KarbonView::viewModeChanged()
{
	canvasWidget()->pixmap()->fill();

	if( m_viewAction->currentItem() == 1 )
		m_painterFactory->setWireframePainter( canvasWidget()->pixmap(), width(), height() );
	else
		m_painterFactory->setPainter( canvasWidget()->pixmap(), width(), height() );

	m_canvas->repaintAll();
}

void
KarbonView::zoomChanged()
{
	double centerX = double( m_canvas->contentsX() + m_canvas->visibleWidth() / 2 ) / double( m_canvas->contentsWidth() );
	double centerY = double( m_canvas->contentsY() + m_canvas->visibleHeight() / 2 ) / double( m_canvas->contentsHeight() );
	bool bOK;
	double zoomFactor = m_zoomAction->currentText().toDouble( &bOK ) / 100.0;

	// above 2000% probably doesnt make sense... (Rob)
	if( zoomFactor > 20 )
	{
		zoomFactor = 20;
		m_zoomAction->changeItem( m_zoomAction->currentItem(), " 2000%" );
	}

	setZoom( zoomFactor );
	// TODO : I guess we should define a document size member at this point...
	//kdDebug() << "part()->pageLayout().ptWidth :" << part()->pageLayout().ptWidth << endl;
	//kdDebug() << "part()->pageLayout().ptHeight :" << part()->pageLayout().ptHeight << endl;
	// TODO : the default shouldnt be necessary?

	if( int( part()->pageLayout().ptWidth ) == 0 || int( part()->pageLayout().ptHeight ) == 0 )
		m_canvas->resizeContents( int( 640 * zoomFactor ), int( 900 * zoomFactor ) );
	else
		m_canvas->resizeContents( int( ( part()->pageLayout().ptWidth + 40 ) * zoomFactor ),
								  int( ( part()->pageLayout().ptHeight + 80 ) * zoomFactor ) );

	VPainter *painter = painterFactory()->editpainter();
	painter->setZoomFactor( zoomFactor );

	m_canvas->setContentsPos( centerX * m_canvas->contentsWidth() - m_canvas->visibleWidth() / 2,
							  centerY * m_canvas->contentsHeight() - m_canvas->visibleHeight() / 2 );
	m_canvas->repaintAll();

	m_canvas->setFocus();

	emit zoomChanged( zoomFactor );
}

void
KarbonView::solidFillClicked()
{
	if( shell() && shell()->rootView() == this )
	{
		VFillDlg * dialog = new VFillDlg( part() );
		connect( dialog, SIGNAL( fillChanged( const VFill & ) ), this, SLOT( selectionChanged() ) );
		dialog->exec();
		delete dialog;
		disconnect( dialog, SIGNAL( fillChanged( const VFill & ) ), this, SLOT( selectionChanged() ) );
	}
}

void
KarbonView::strokeClicked()
{
	if( shell() && shell()->rootView() == this )
	{
		VStrokeDlg * dialog = new VStrokeDlg( part() );
		connect( dialog, SIGNAL( strokeChanged( const VStroke & ) ), this, SLOT( selectionChanged() ) );
		dialog->exec();
		delete dialog;
		disconnect( dialog, SIGNAL( strokeChanged( const VStroke & ) ), this, SLOT( selectionChanged() ) );
	}
}

void
KarbonView::slotStrokeChanged( const VStroke &c )
{
	part()->document().selection()->setStroke( c );

	part()->addCommand( new VStrokeCmd( &part()->document(), &c ), true );

	m_strokeFillPreview->update( *( part()->document().selection()->stroke() ),
								 *( part()->document().selection()->fill() ) );
}

void
KarbonView::slotFillChanged( const VFill &f )
{
	part()->document().selection()->setFill( f );

	part()->addCommand( new VFillCmd( &part()->document(), f ), true );

	m_strokeFillPreview->update( *( part()->document().selection()->stroke() ),
								 *( part()->document().selection()->fill() ) );
}

void
KarbonView::slotJoinStyleClicked()
{
	VObjectListIterator itr( part()->document().selection()->objects() );

	for( ; itr.current() ; ++itr )
	{
		VStroke stroke( *( itr.current()->stroke() ) );
		stroke.setParent( itr.current() );
		stroke.setLineJoin( ( VStroke::VLineJoin ) m_joinStyle->getState() );
		itr.current()->setStroke( stroke );
	}

	part()->repaintAllViews();
}

void
KarbonView::slotCapStyleClicked()
{
	VObjectListIterator itr( part()->document().selection()->objects() );

	for( ; itr.current() ; ++itr )
	{
		VStroke stroke( *( itr.current()->stroke() ) );
		stroke.setParent( itr.current() );
		stroke.setLineCap( ( VStroke::VLineCap ) m_capStyle->getState() );
		itr.current()->setStroke( stroke );
	}

	part()->repaintAllViews();
}

void
KarbonView::setLineWidth()
{
	setLineWidth( m_setLineWidth->value() );
	selectionChanged();
}

//necessary for dcop call !
void
KarbonView::setLineWidth( double val )
{
	part()->addCommand( new VStrokeCmd( &part()->document(), val ), true );
}

void
KarbonView::viewColorManager()
{
	if( m_ColorManager->isVisible() == false )
	{
		mainWindow()->addDockWindow( m_ColorManager, DockRight );
		m_ColorManager->show();
	}
}

void
KarbonView::viewToolOptions()
{
	if( m_toolOptionsDocker->isVisible() == false )
	{
		m_toolOptionsDocker->show();
	}
}

void
KarbonView::viewStrokeDocker()
{
	if( m_strokeDocker->isVisible() == false )
	{
		mainWindow()->addDockWindow( m_strokeDocker, DockRight );
		m_strokeDocker->show();
	}
}

void
KarbonView::initActions()
{

	// edit ----->
	KStdAction::cut( this,
					 SLOT( editCut() ), actionCollection(), "edit_cut" );
	KStdAction::copy( this,
					  SLOT( editCopy() ), actionCollection(), "edit_copy" );
	KStdAction::paste( this,
					   SLOT( editPaste() ), actionCollection(), "edit_paste" );
	KStdAction::selectAll( this,
						   SLOT( editSelectAll() ), actionCollection(), "edit_select_all" );
	new KAction(
		i18n( "&Deselect All" ), QKeySequence( "Ctrl+D" ), this,
		SLOT( editDeselectAll() ), actionCollection(), "edit_deselect_all" );
	new KAction(
		i18n( "D&elete" ), "editdelete", QKeySequence( "Shift+Del" ), this,
		SLOT( editDeleteSelection() ), actionCollection(), "edit_delete" );
	new KAction(
		i18n( "&History" ), 0, 0, this,
		SLOT( editPurgeHistory() ), actionCollection(), "edit_purge_history" );
	// edit <-----

	// object ----->
	new KAction(
		i18n( "Bring to &Front" ), 0, QKeySequence( "Shift+PgUp" ), this,
		SLOT( selectionBringToFront() ), actionCollection(), "object_move_totop" );
	new KAction(
		i18n( "&Raise" ), 0, QKeySequence( "Ctrl+PgUp" ), this,
		SLOT( selectionMoveUp() ), actionCollection(), "object_move_up" );
	new KAction(
		i18n( "&Lower" ), 0, QKeySequence( "Ctrl+PgDown" ), this,
		SLOT( selectionMoveDown() ), actionCollection(), "object_move_down" );
	new KAction(
		i18n( "Send to &Back" ), 0, QKeySequence( "Shift+PgDown" ), this,
		SLOT( selectionSendToBack() ), actionCollection(), "object_move_tobottom" );
	m_groupObjects = new KAction(
						 i18n( "&Group Objects" ), "14_group", QKeySequence( "Ctrl+G" ), this,
						 SLOT( groupSelection() ), actionCollection(), "selection_group" );
	m_ungroupObjects = new KAction(
						   i18n( "&Ungroup Objects" ), "14_ungroup", QKeySequence( "Ctrl+U" ), this,
						   SLOT( ungroupSelection() ), actionCollection(), "selection_ungroup" );
	new KAction(
		i18n( "&Translate" ), "14_translate", 0, this,
		SLOT( objectTrafoTranslate() ), actionCollection(), "object_trafo_translate" );
	new KAction(
		i18n( "&Rotate" ), "14_rotate", 0, this,
		SLOT( objectTrafoRotate() ), actionCollection(), "object_trafo_rotate" );
	new KAction(
		i18n( "S&hear" ), "14_shear", 0, this,
		SLOT( objectTrafoShear() ), actionCollection(), "object_trafo_shear" );
	// object <-----

	// text ----->
	// Disable the text, hopefully it will make a comback soon (Rob)
	/*m_setFontFamily = new KFontAction(
		i18n( "Set Font Family" ), 0, actionCollection(), "setFontFamily" );
	m_setFontFamily->setCurrentItem( 0 );*/

	//connect( m_setFontFamily,
	//	SIGNAL( activated( const QString& ) ),
	//	SLOT( setFontFamily( const QString& ) ) );

	/*m_setFontSize = new KFontSizeAction(
		i18n( "Set Font Size" ), 0, actionCollection(), "setFontSize" );
	m_setFontSize->setCurrentItem( 0 );*/

	//connect( m_setFontSize,
	//	SIGNAL( activated( const QString& ) ),
	//	SLOT( setFontSize( const QString& ) ) );

	/*m_setFontItalic = new KToggleAction(
		i18n( "&Italic" ), "text_italic", 0, actionCollection(), "setFontItalic" );
	m_setFontBold = new KToggleAction(
		i18n( "&Bold" ), "text_bold", 0, actionCollection(), "setFontBold" );*/

	//m_setTextColor = new TKSelectColorAction(
	//	i18n("Set Text Color"), TKSelectColorAction::TextColor,
	//	actionCollection(), "setTextColor" );
	//connect( m_setTextColor, SIGNAL( activated() ), SLOT( setTextColor() ) );
	// text <-----

	// view ----->
	m_viewAction = new KSelectAction(
					   i18n( "View &Mode" ), 0, this,
					   SLOT( viewModeChanged() ), actionCollection(), "view_mode" );

	m_zoomAction = new KSelectAction(
					   i18n( "&Zoom" ), 0, this,
					   SLOT( zoomChanged() ), actionCollection(), "view_zoom" );

	QStringList mstl;
	mstl
	<< i18n( "Normal" )
	<< i18n( "Wireframe" );
	m_viewAction->setItems( mstl );
	m_viewAction->setCurrentItem( 0 );
	m_viewAction->setEditable( false );

	QStringList stl;
	stl
	<< i18n( "   25%" )
	<< i18n( "   50%" )
	<< i18n( "  100%" )
	<< i18n( "  200%" )
	<< i18n( "  300%" )
	<< i18n( "  400%" )
	<< i18n( "  800%" );

	m_zoomAction->setItems( stl );
	m_zoomAction->setEditable( true );
	m_zoomAction->setCurrentItem( 2 );

	new KAction(
		i18n( "&Color Manager" ), "colorman", 0, this,
		SLOT( viewColorManager() ), actionCollection(), "view_color_manager" );
/*	new KAction(
		i18n( "&Layers Manager" ), "layersman", 0, this,
		SLOT( viewLayersDocker() ), actionCollection(), "view_layers_manager" );*/
	new KAction(
		i18n( "&Tool Options" ), "tooloptions", 0, this,
		SLOT( viewToolOptions() ), actionCollection(), "view_tool_options" );
	new KAction(
		i18n( "&Stroke" ), "strokedocker", 0, this,
		SLOT( viewStrokeDocker() ), actionCollection(), "view_stroke_docker" );
/*	new KAction(
		i18n( "H&istory" ), "historydocker", 0, this,
		SLOT( viewHistory() ), actionCollection(), "view_history_docker" );*/
	// view <-----

	// line width
	m_setLineWidth = new KoUnitDoubleSpinComboBox( this, 0.0, 1000.0, 0.5, 1.0, KoUnit::U_PT, 1 );
	new KWidgetAction( m_setLineWidth, i18n( "Set Line Width" ), 0, this, SLOT( setLineWidth() ), actionCollection(), "setLineWidth" );
	m_setLineWidth->insertItem( 0.5 );
	m_setLineWidth->insertItem( 1.0 );
	m_setLineWidth->insertItem( 2.0 );
	m_setLineWidth->insertItem( 5.0 );
	m_setLineWidth->insertItem( 10.0 );
	m_setLineWidth->insertItem( 20.0 );
	connect( m_setLineWidth, SIGNAL( valueChanged( double ) ), this, SLOT( setLineWidth() ) );

	// set up join style widget
	m_joinStyle = new VStateButton( this );
	m_joinStyle->addState( new QPixmap( DesktopIcon( "join_bevel" ) ) );
	m_joinStyle->addState( new QPixmap( DesktopIcon( "join_miter" ) ) );
	m_joinStyle->addState( new QPixmap( DesktopIcon( "join_round" ) ) );
	m_joinStyle->setState( 0 );

	new KWidgetAction( m_joinStyle, i18n( "Set Join Style" ), 0, this, SLOT( slotJoinStyleClicked() ), actionCollection(), "setJoinStyle" );

	connect( m_joinStyle, SIGNAL( clicked() ), this, SLOT( slotJoinStyleClicked() ) );

	// set up cap style widget
	m_capStyle = new VStateButton( this );
	m_capStyle->addState( new QPixmap( DesktopIcon( "cap_butt" ) ) );
	m_capStyle->addState( new QPixmap( DesktopIcon( "cap_square" ) ) );
	m_capStyle->addState( new QPixmap( DesktopIcon( "cap_round" ) ) );
	m_capStyle->setState( 0 );

	new KWidgetAction( m_capStyle, i18n( "Set Cap Style" ), 0, this, SLOT( slotCapStyleClicked() ), actionCollection(), "setCapStyle" );

	connect( m_capStyle, SIGNAL( clicked() ), this, SLOT( slotCapStyleClicked() ) );

	m_configureAction = new KAction(
							i18n( "Configure Karbon..." ), "configure", 0, this,
							SLOT( configure() ), actionCollection(), "configure" );

	m_contextHelpAction = new KoContextHelpAction( actionCollection(), this );
}

void
KarbonView::paintEverything( QPainter& /*p*/, const QRect& /*rect*/,
							 bool /*transparent*/ )
{
	kdDebug() << "view->paintEverything()" << endl;
}

bool
KarbonView::mouseEvent( QMouseEvent* event, const KoPoint &p )
{
	if( m_currentTool )
		return m_currentTool->mouseEvent( event, p );
	else
		return false;
}

bool
KarbonView::keyEvent( QEvent* event )
{
	if( m_currentTool )
		return m_currentTool->keyEvent( event );
	else
		return false;
}

void
KarbonView::reorganizeGUI()
{
	if( statusBar() )
	{
		if( part()->showStatusBar() )
			statusBar()->show();
		else
			statusBar()->hide();
	}
}

void
KarbonView::setNumberOfRecentFiles( int number )
{
	if( shell() )	// 0L when embedded into konq !
		shell()->setMaxRecentItems( number );
}

void
KarbonView::configure()
{
	VConfigureDlg dialog( this );
	dialog.exec();
}

void
KarbonView::selectionChanged()
{
	int count = part()->document().selection()->objects().count();

	if( count > 0 )
	{
		VGroup * group = dynamic_cast<VGroup *>( part()->document().selection()->objects().getFirst() );
		m_groupObjects->setEnabled( count > 1 );
		m_ungroupObjects->setEnabled( group && ( count == 1 ) );

		if( count == 1 )
		{
			m_strokeFillPreview->update( *part()->document().selection()->objects().getFirst()->stroke(),
										 *part()->document().selection()->objects().getFirst()->fill() );
			m_strokeDocker->setStroke( *( part()->document().selection()->objects().getFirst()->stroke() ) );
		}
		else
		{
			VStroke stroke;
			stroke.setType( VStroke::none );
			VFill fill;
			m_strokeFillPreview->update( stroke, fill );
		}

		part()->document().selection()->setStroke( *( part()->document().selection()->objects().getFirst()->stroke() ) );
		part()->document().selection()->setFill( *( part()->document().selection()->objects().getFirst()->fill() ) );
		m_setLineWidth->setEnabled( true );
		m_setLineWidth->updateValue( part()->document().selection()->objects().getFirst()->stroke()->lineWidth() );

		if( m_ColorManager->isStrokeDocker() )
		{
			VColor * c = new VColor ( part()->document().selection()->objects().getFirst()->stroke()->color() );
			m_ColorManager->setColor( c );
		}
		else
		{
			VColor *c = new VColor ( part()->document().selection()->objects().getFirst()->fill()->color() );
			m_ColorManager->setColor( c );
		}
	}
	else
	{
		m_strokeFillPreview->update( *( part()->document().selection()->stroke() ),
									 *( part()->document().selection()->fill() ) );
		m_setLineWidth->setEnabled( false );
		m_groupObjects->setEnabled( false );
		m_ungroupObjects->setEnabled( false );
	}
}

void KarbonView::setUnit( KoUnit::Unit /*_unit*/ )
{
	/*m_ellipseTool->refreshUnit();
	m_rectangleTool->refreshUnit();
	m_sinusTool->refreshUnit();
	m_spiralTool->refreshUnit();
	m_starTool->refreshUnit();
	m_roundRectTool->refreshUnit();
	m_polygonTool->refreshUnit();*/
}

#include "karbon_view.moc"

