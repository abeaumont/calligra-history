/* This file is part of the KDE project
   Copyright (C) 2003-2004 Jaroslaw Staniek <js@iidea.pl>

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

/*!
 Temporary moved from QKW KFileDialog implementation.
 TODO: move to KDElibs/win32 KFileDialog wrapper
*/

#include "KexiStartupFileDialog.h"

#include <kiconloader.h>
#include <kdebug.h>

#include <qobjectlist.h>
#include <qlineedit.h>

#include <win/win32_utils.h>

class KexiStartupFileDialogBasePrivate
{
	public:
		KexiStartupFileDialogBasePrivate()
		{}
		KFile::Mode mode;
		QString kde_filters;
		QStringList mimetypes;
};

KexiStartupFileDialogBase::KexiStartupFileDialogBase(
	const QString & dirName, const QString & filter, 
	QWidget * parent, const char * name, bool modal )
 : QFileDialog( dirName, filter, parent, name, modal )
 , d(new KexiStartupFileDialogBasePrivate())
{
	QString _dirName = dirName;
	//make default 'My Documents' folder
//TODO: store changes in the app's config file?
	if (_dirName.isEmpty()) {
		_dirName = getRegistryValue(HKEY_CURRENT_USER, 
			"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "Personal");
	}

	init(_dirName, filter, parent);

	//find "OK" button
	QObjectList *l = queryList( "QPushButton", "OK", false );
	m_okBtn = dynamic_cast<QPushButton*>(l->first());
	delete l;
	l = queryList( "QLineEdit", "name/filter editor", false );
	m_lineEdit = dynamic_cast<QLineEdit*>(l->first());
	delete l;

	adjustSize();
}

KexiStartupFileDialogBase::~KexiStartupFileDialogBase()
{
}

void KexiStartupFileDialogBase::init(const QString& startDir, const QString& filter, QWidget* widget)
{
//TODO    initStatic();
//TODO    d = new KFileDialogPrivate();

//(js)    d->boxLayout = 0;
//TODO    d->keepLocation = false;
//TODO    d->operationMode = Opening;
    setMode(KFile::File | KFile::ExistingOnly); //(js) default: open action
    setIcon( KGlobal::iconLoader()->loadIcon("fileopen", KIcon::Desktop) );
		setDir(QDir(startDir));
//TODO    d->hasDefaultFilter = false;
//TODO    d->hasView = false;
//(js)    d->mainWidget = new QWidget( this, "KFileDialog::mainWidget");
//(js)    setMainWidget( d->mainWidget );
//(js)    d->okButton = new KPushButton( KStdGuiItem::ok(), d->mainWidget );
//(js)    d->okButton->setDefault( true );
//(js)    d->cancelButton = new KPushButton(KStdGuiItem::cancel(), d->mainWidget);
//(js)    connect( d->okButton, SIGNAL( clicked() ), SLOT( slotOk() ));
//(js)    connect( d->cancelButton, SIGNAL( clicked() ), SLOT( slotCancel() ));
//(js)    d->customWidget = widget;
//(js)    d->autoSelectExtCheckBox = 0; // delayed loading
//TODO    d->autoSelectExtChecked = false;
//(js)    d->urlBar = 0; // delayed loading
//TODO    KConfig *config = KGlobal::config();
//TODO    KConfigGroupSaver cs( config, ConfigGroup );
//TODO    d->initializeSpeedbar = config->readBoolEntry( "Set speedbar defaults",
//TODO                                                   true );
//TODO    d->completionLock = false;

//TODO    QtMsgHandler oldHandler = qInstallMsgHandler( silenceQToolBar );
//TODO    toolbar = 0; //(js)
//(js)    toolbar = new KToolBar( d->mainWidget, "KFileDialog::toolbar", true);
//(js)    toolbar->setFlat(true);
//TODO    qInstallMsgHandler( oldHandler );

//(js)    d->pathCombo = new KURLComboBox( KURLComboBox::Directories, true,
//(js)                                     toolbar, "path combo" );
//(js)    QToolTip::add( d->pathCombo, i18n("Often used directories") );
//(js)    QWhatsThis::add( d->pathCombo, "<qt>" + i18n("Commonly used locations are listed here. "
//(js)                                                 "This includes standard locations, such as your home directory, as well as "
//(js)                                                 "locations that have been visited recently.") + autocompletionWhatsThisText);
/*
    KURL u;
    u.setPath( QDir::rootDirPath() );
    QString text = i18n("Root Directory: %1").arg( u.path() );
    d->pathCombo->addDefaultURL( u,
                                 KMimeType::pixmapForURL( u, 0, KIcon::Small ),
                                 text );

    u.setPath( QDir::homeDirPath() );
    text = i18n("Home Directory: %1").arg( u.path( +1 ) );
    d->pathCombo->addDefaultURL( u, KMimeType::pixmapForURL( u, 0, KIcon::Small ),
                                 text );

    KURL docPath;
    docPath.setPath( KGlobalSettings::documentPath() );
    if ( u.path(+1) != docPath.path(+1) ) {
        text = i18n("Documents: %1").arg( docPath.path( +1 ) );
        d->pathCombo->addDefaultURL( u,
                                     KMimeType::pixmapForURL( u, 0, KIcon::Small ),
                                     text );
    }

    u.setPath( KGlobalSettings::desktopPath() );
    text = i18n("Desktop: %1").arg( u.path( +1 ) );
    d->pathCombo->addDefaultURL( u,
                                 KMimeType::pixmapForURL( u, 0, KIcon::Small ),
                                 text );

    u.setPath( "/tmp" );

    d->url = getStartURL( startDir, d->fileClass );
    d->selection = d->url.url();

    // If local, check it exists. If not, go up until it exists.
    if ( d->url.isLocalFile() )
    {
        if ( !QFile::exists( d->url.path() ) )
        {
            d->url = d->url.upURL();
            QDir dir( d->url.path() );
            while ( !dir.exists() )
            {
                d->url = d->url.upURL();
                dir.setPath( d->url.path() );
            }
        }
    }

    ops = new KDirOperator(d->url, d->mainWidget, "KFileDialog::ops");
    ops->setOnlyDoubleClickSelectsFiles( true );
    connect(ops, SIGNAL(urlEntered(const KURL&)),
            SLOT(urlEntered(const KURL&)));
    connect(ops, SIGNAL(fileHighlighted(const KFileItem *)),
            SLOT(fileHighlighted(const KFileItem *)));
    connect(ops, SIGNAL(fileSelected(const KFileItem *)),
            SLOT(fileSelected(const KFileItem *)));
    connect(ops, SIGNAL(finishedLoading()),
            SLOT(slotLoadingFinished()));

    ops->setupMenu(KDirOperator::SortActions |
                   KDirOperator::FileActions |
                   KDirOperator::ViewActions);
    KActionCollection *coll = ops->actionCollection();

    // plug nav items into the toolbar
    coll->action( "up" )->plug( toolbar );
    coll->action( "up" )->setWhatsThis(i18n("<qt>Click this button to enter the parent directory.<p>"
                                            "For instance, if the current location is file:/home/%1 clicking this "
                                            "button will take you to file:/home.</qt>").arg(getlogin()));
    coll->action( "back" )->plug( toolbar );
    coll->action( "back" )->setWhatsThis(i18n("Click this button to move backwards one step in the browsing history."));
    coll->action( "forward" )->plug( toolbar );
    coll->action( "forward" )->setWhatsThis(i18n("Click this button to move forward one step in the browsing history."));
    coll->action( "reload" )->plug( toolbar );
    coll->action( "reload" )->setWhatsThis(i18n("Click this button to reload the contents of the current location."));
    coll->action( "mkdir" )->setShortcut(Key_F10);
    coll->action( "mkdir" )->plug( toolbar );
    coll->action( "mkdir" )->setWhatsThis(i18n("Click this button to create a new directory."));

    d->bookmarkHandler = new KFileBookmarkHandler( this );
    toolbar->insertButton(QString::fromLatin1("bookmark"),
                          (int)HOTLIST_BUTTON, true,
                          i18n("Bookmarks"));
    toolbar->getButton(HOTLIST_BUTTON)->setPopup( d->bookmarkHandler->menu(),
                                                  true);
    QWhatsThis::add(toolbar->getButton(HOTLIST_BUTTON),
                    i18n("<qt>This button allows you to bookmark specific locations. "
                         "Click on this button to open the bookmark menu where you may add, "
                         "edit or select a bookmark.<p>"
                         "These bookmarks are specific to the file dialog, but otherwise operate "
                         "like bookmarks elsewhere in KDE.</qt>"));
    connect( d->bookmarkHandler, SIGNAL( openURL( const QString& )),
             SLOT( enterURL( const QString& )));

    KToggleAction *showSidebarAction =
        new KToggleAction(i18n("Show Quick Access Navigation Panel"), Key_F9, coll,"toggleSpeedbar");
    connect( showSidebarAction, SIGNAL( toggled( bool ) ),
             SLOT( toggleSpeedbar( bool )) );

    KActionMenu *menu = new KActionMenu( i18n("Configure"), "configure", this, "extra menu" );
    menu->setWhatsThis(i18n("<qt>This is the configuration menu for the file dialog. "
                            "Various options can be accessed from this menu including: <ul>"
                            "<li>how files are sorted in the list</li>"
                            "<li>types of view, including icon and list</li>"
                            "<li>showing of hidden files</li>"
                            "<li>the Quick Access navigation panel</li>"
                            "<li>file previews</li>"
                            "<li>separating directories from files</li></ul></qt>"));
    menu->insert( coll->action( "sorting menu" ));
    menu->insert( coll->action( "separator" ));
    coll->action( "short view" )->setShortcut(Key_F6);
    menu->insert( coll->action( "short view" ));
    coll->action( "detailed view" )->setShortcut(Key_F7);
    menu->insert( coll->action( "detailed view" ));
    menu->insert( coll->action( "separator" ));
    coll->action( "show hidden" )->setShortcut(Key_F8);
    menu->insert( coll->action( "show hidden" ));
    menu->insert( showSidebarAction );
    coll->action( "preview" )->setShortcut(Key_F11);
    menu->insert( coll->action( "preview" ));
    coll->action( "separate dirs" )->setShortcut(Key_F12);
    menu->insert( coll->action( "separate dirs" ));

    menu->setDelayed( false );
    connect( menu->popupMenu(), SIGNAL( aboutToShow() ),
             ops, SLOT( updateSelectionDependentActions() ));
    menu->plug( toolbar );
*/
    /*
     * ugly little hack to have a 5 pixel space between the buttons
     * and the combo box
     */
/*    QWidget *spacerWidget = new QWidget(toolbar);
//(js)    spacerWidget->setMinimumWidth(spacingHint());
//(js)    spacerWidget->setMaximumWidth(spacingHint());
    d->m_pathComboIndex = toolbar->insertWidget(-1, -1, spacerWidget);
    toolbar->insertWidget(PATH_COMBO, 0, d->pathCombo);


    toolbar->setItemAutoSized (PATH_COMBO);
    toolbar->setIconText(KToolBar::IconOnly);
    toolbar->setBarPos(KToolBar::Top);
    toolbar->setMovingEnabled(false);
    toolbar->adjustSize();

    d->pathCombo->setCompletionObject( ops->dirCompletionObject(), false );

    connect( d->pathCombo, SIGNAL( urlActivated( const KURL&  )),
             this,  SLOT( enterURL( const KURL& ) ));
    connect( d->pathCombo, SIGNAL( returnPressed( const QString&  )),
             this,  SLOT( enterURL( const QString& ) ));
    connect( d->pathCombo, SIGNAL(textChanged( const QString& )),
             SLOT( pathComboChanged( const QString& ) ));
    connect( d->pathCombo, SIGNAL( completion( const QString& )),
             SLOT( dirCompletion( const QString& )));
    connect( d->pathCombo, SIGNAL( textRotation(KCompletionBase::KeyBindingType) ),
             d->pathCombo, SLOT( rotateText(KCompletionBase::KeyBindingType) ));

    QString whatsThisText;

    // the Location label/edit
    d->locationLabel = new QLabel(i18n("&Location:"), d->mainWidget);
    locationEdit = new KURLComboBox(KURLComboBox::Files, true,
                                    d->mainWidget, "LocationEdit");
    updateLocationWhatsThis ();
    d->locationLabel->setBuddy(locationEdit);

    // to get the completionbox-signals connected:
    locationEdit->setHandleSignals( true );
    (void) locationEdit->completionBox();

    locationEdit->setFocus();
//     locationEdit->setCompletionObject( new KURLCompletion() );
//     locationEdit->setAutoDeleteCompletionObject( true );
    locationEdit->setCompletionObject( ops->completionObject(), false );

    connect( locationEdit, SIGNAL( returnPressed() ),
             this, SLOT( slotOk()));
    connect(locationEdit, SIGNAL( activated( const QString&  )),
            this,  SLOT( locationActivated( const QString& ) ));
    connect( locationEdit, SIGNAL( completion( const QString& )),
             SLOT( fileCompletion( const QString& )));
    connect( locationEdit, SIGNAL( textRotation(KCompletionBase::KeyBindingType) ),
             locationEdit, SLOT( rotateText(KCompletionBase::KeyBindingType) ));

    // the Filter label/edit
    whatsThisText = i18n("<qt>This is the filter to apply to the file list. "
                         "File names that do not match the filter will not be shown.<p>"
                         "You may select from one of the preset filters in the "
                         "drop down menu, or you may enter a custom filter "
                         "directly into the text area.<p>"
                         "Wildcards such as * and ? are allowed.</qt>");
    d->filterLabel = new QLabel(i18n("&Filter:"), d->mainWidget);
    QWhatsThis::add(d->filterLabel, whatsThisText);
    filterWidget = new KFileFilterCombo(d->mainWidget,
                                        "KFileDialog::filterwidget");
    QWhatsThis::add(filterWidget, whatsThisText);
    setFilter(filter);
    d->filterLabel->setBuddy(filterWidget);
    connect(filterWidget, SIGNAL(filterChanged()), SLOT(slotFilterChanged()));

    // the Automatically Select Extension checkbox
    // (the text, visibility etc. is set in updateAutoSelectExtension(), which is called by readConfig())
    d->autoSelectExtCheckBox = new QCheckBox (d->mainWidget);
    connect(d->autoSelectExtCheckBox, SIGNAL(clicked()), SLOT(slotAutoSelectExtClicked()));

    initGUI(); // activate GM

    readRecentFiles( config );

    adjustSize();

    // we set the completionLock to avoid entering pathComboChanged() when
    // inserting the list of URLs into the combo.
    d->completionLock = true;
    ops->setViewConfig( config, ConfigGroup );
    readConfig( config, ConfigGroup );
    setSelection(d->selection);
    d->completionLock = false;
	*/
}

void KexiStartupFileDialogBase::clearFilter()
{
	d->kde_filters = "";//(js)
	QFileDialog::setFilter(""); //(js);
//todo    d->mimetypes.clear();
//todo    d->hasDefaultFilter = false;

	updateAutoSelectExtension ();
}

KFile::Mode KexiStartupFileDialogBase::mode() const
{
	return d->mode;
}

void KexiStartupFileDialogBase::setMode( KFile::Mode m )
{
    //(js) translate mode for QFileDialog
    d->mode = m;
    QFileDialog::Mode qm = (QFileDialog::Mode)0;
    if (m & KFile::File) qm = Mode(qm | QFileDialog::AnyFile);
    else if (m & KFile::Directory) qm = Mode(qm | QFileDialog::DirectoryOnly);
    if (m & KFile::Files) qm = Mode(qm | QFileDialog::ExistingFiles);
    if (m & KFile::ExistingOnly) qm = Mode(qm | QFileDialog::ExistingFile);
    
    QFileDialog::setMode( qm );
/*(js)    ops->setMode(m);
    if ( ops->dirOnlyMode() ) {
//(js)        filterWidget->setDefaultFilter( i18n("*|All Directories") );
    }
    else {
//(js)        filterWidget->setDefaultFilter( i18n("*|All Files") );
    }

    updateAutoSelectExtension ();*/
}

void KexiStartupFileDialogBase::setMode( unsigned int m )
{
    setMode(static_cast<KFile::Mode>( m ));
}

void KexiStartupFileDialogBase::setOperationMode( KFileDialog::OperationMode mode )
{
//    d->operationMode = mode;
  //  d->keepLocation = (mode == Saving);
    if (mode == KFileDialog::Saving) {
      setMode( KFile::File );
      setIcon( KGlobal::iconLoader()->loadIcon("filesave", KIcon::Desktop) );
    }
//(js)    filterWidget->setEditable( !d->hasDefaultFilter || mode != Saving );
//(js)    d->okButton->setGuiItem( (mode == Saving) ? KStdGuiItem::save() : KStdGuiItem::ok() );
//TODO    updateLocationWhatsThis ();
    updateAutoSelectExtension ();
}

QString KexiStartupFileDialogBase::currentFilter() const
{
	//(js)filterWidget->currentFilter();

	//we need to convert Qt filter format to KDE format
	//Qt format: "some text (*.first *.second)" or "All (*)"
	//KDE format: "*.first *.second" or "*"
	QString f = selectedFilter();
	if (f.find('(')!=-1)
		f = f.mid(f.find('(')+1);
	if (f.mid(f.find(')')!=-1))
		f = f.left(f.find(')'));
	return f;
}

void KexiStartupFileDialogBase::setFilter(const QString& filter)
{
	d->kde_filters = filter;
    int pos = d->kde_filters.find('/');

    // Check for an un-escaped '/', if found
    // interpret as a MIME filter.

    if (pos > 0 && filter[pos - 1] != '\\') {
        QStringList filters = QStringList::split( " ", d->kde_filters );
        setMimeFilter( filters );
        return;
    }

    // Strip the escape characters from
    // escaped '/' characters.

    QString copy (d->kde_filters);
    for (pos = 0; (pos = copy.find("\\/", pos)) != -1; ++pos)
        copy.remove(pos, 1);

	//<js>
	//we need to convert KDE filter format to Qt format
	//Qt format: "some text (*.first *.second)" or "All (*)"
	//KDE format: "*.first *.second" or "*"
	QStringList filters = QStringList::split("\n",d->kde_filters);
	QString current;
	QString converted; //finally - converted filter
	for (QStringList::ConstIterator it = filters.constBegin(); it!=filters.constEnd();++it) {
		current = *it;
		QString new_f;//filter part
		QString new_name;//filter name part
		int p = (*it).find('|');
		if (p!=-1) {
			new_f = current.left(p);
			new_name = current.mid(p+1);
		}else {
			new_f = current;
			new_name = current; //nothing better
		}
		//remove (.....) from name
		p=new_name.find('(');
		int p2 = new_name.findRev(')');
		QString new_name1, new_name2;
		if (p!=-1)
			new_name1 = new_name.left(p);
		if (p2!=-1)
			new_name2 = new_name.mid(p2+1);
		if (!new_name1.isEmpty() || !new_name2.isEmpty())
			new_name = new_name1.stripWhiteSpace() + " " + new_name2.stripWhiteSpace();
		new_name.replace('(',"");
		new_name.replace(')',"");
		new_name = new_name.stripWhiteSpace();

		if (!converted.isEmpty())
			converted += ";;";

		converted += (new_name + " (" + new_f + ")");
	}
	QFileDialog::setFilters(converted);
	//</js>
//(js)    ops->clearFilter();
//(js)    filterWidget->setFilter(copy);
//(js)    ops->setNameFilter(filterWidget->currentFilter());
//(js)    d->hasDefaultFilter = false;
//(js)    filterWidget->setEditable( true );

    updateAutoSelectExtension ();
}

void KexiStartupFileDialogBase::setMimeFilter( const QStringList& mimeTypes,
                                 const QString& defaultType )
{
    d->mimetypes = mimeTypes;
//(js)    filterWidget->setMimeFilter( mimeTypes, defaultType );

//(js)    QStringList types = QStringList::split(" ", filterWidget->currentFilter());
//(js)    types.append( QString::fromLatin1( "inode/directory" ));
//(js)    ops->clearFilter();
//(js)    ops->setMimeFilter( types );
//(js)    d->hasDefaultFilter = !defaultType.isEmpty();
//(js)    filterWidget->setEditable( !d->hasDefaultFilter ||
//(js)                               d->operationMode != Saving );

//TODO    updateAutoSelectExtension ();
}

