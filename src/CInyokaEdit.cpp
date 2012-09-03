/**
 * \file CInyokaEdit.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2011-2012 The InyokaEdit developers
 *
 * This file is part of InyokaEdit.
 *
 * InyokaEdit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * InyokaEdit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with InyokaEdit.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \section DESCRIPTION
 * Main application generation (gui, object creation etc.).
 */

#include <QtGui>
#include <QtWebKit/QWebView>
#include <QWebFrame>

#include "CInyokaEdit.h"
#include "ui_CInyokaEdit.h"

bool bDEBUG = false;  // Don't change this value! Use "--debug" command line option instead.

CInyokaEdit::CInyokaEdit( QApplication *ptrApp, QDir userAppDir, QWidget *parent ) :
    QMainWindow(parent),
    m_pUi( new Ui::CInyokaEdit ),
    m_pApp( ptrApp ),
    m_UserAppDir (userAppDir)
{
    qDebug() << "Start" << Q_FUNC_INFO;

    bool bOpenFileAfterStart = false;

    m_pUi->setupUi(this);

    // Check for command line arguments
    if ( m_pApp->argc() >= 2 )
    {
        QString sTmp = m_pApp->argv()[1];

        if ( "-v" == sTmp || "--version" == sTmp )
        {
            std::cout << m_pApp->argv()[0] << "\t v" << m_pApp->applicationVersion().toStdString() << std::endl;
            exit(0);
        }
        else if ( "--debug" == sTmp )
        {
            bDEBUG = true;
        }
        else
        {
            bOpenFileAfterStart = true;
        }
    }

    // Create folder for downloaded article images
    m_tmpPreviewImgDir = m_UserAppDir.absolutePath() + "/tmpImages";
    if ( !m_tmpPreviewImgDir.exists() )
    {
        m_tmpPreviewImgDir.mkpath(m_tmpPreviewImgDir.absolutePath());  // Create folder including possible parent directories (mkPATH)!
    }

    // Create all objects (after definition of StylesAndImagesDir AND m_tmpPreviewImgDir)
    this->createObjects();

    // Setup gui, menus, actions, toolbar...
    this->setupEditor();
    this->createActions();
    this->createMenus();
    this->createToolBars();

    // Download style files if preview/styles/imgages folders doesn't exist (/home/user/.InyokaEdit)
    if ( !m_UserAppDir.exists() ||
         !QDir(m_UserAppDir.absolutePath() + "/img").exists() ||
         !QDir(m_UserAppDir.absolutePath() + "/styles").exists() ||
         !QDir(m_UserAppDir.absolutePath() + "/Wiki").exists() )
    {
        m_UserAppDir.mkdir( m_UserAppDir.absolutePath() );  // Create folder because user may not start download. Folder is needed for preview.
#if !defined _WIN32
        m_pDownloadModule->loadInyokaStyles();
#endif
    }

    if ( true == bOpenFileAfterStart )
    {
        m_pFileOperations->loadFile( m_pApp->argv()[1], true );
    }

    qDebug() << "End" << Q_FUNC_INFO;
}

CInyokaEdit::~CInyokaEdit()
{
    if ( m_pUi != NULL )
    {
        delete m_pUi;
    }
    m_pUi = NULL;
    qDebug() << "Closing " << m_pApp->applicationName();
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CInyokaEdit::createObjects()
{
    qDebug() << "Start" << Q_FUNC_INFO;

    m_findDialog = new FindDialog(this);  // Has to be create before readSettings
    m_findReplaceDialog = new FindReplaceDialog(this);

    m_pSettings = new CSettings( m_UserAppDir,
                                 m_pApp->applicationName(),
                                 *m_findDialog, *m_findReplaceDialog );
    // Load settings from config file
    m_pSettings->readSettings();

    m_pTemplates = new CTemplates( m_pApp->applicationName(),
                                   m_pApp->applicationDirPath(),
                                   m_pSettings->getTemplateLanguage() );  // Has to be created before parser

    m_pDownloadModule = new CDownload( this,
                                       m_pApp->applicationName(),
                                       m_pApp->applicationDirPath(),
                                       m_UserAppDir );

    m_pEditor = new CTextEditor( m_pUi,
                                 m_pSettings->getCodeCompletion(),
                                 m_pTemplates->getListTplMacrosALL(),
                                 m_pSettings->getAutoSave(),
                                 m_UserAppDir.absolutePath(),
                                 this );

//  if ( true == m_pSettings->getPreviewAlongside() )
//  {
    m_pEditor->installEventFilter(this);
//  }

    m_pFileOperations = new CFileOperations( this,
                                             m_pEditor,
                                             m_pSettings,
                                             m_pApp->applicationName() );  

    m_pParser = new CParser( m_pEditor->document(),
                             m_UserAppDir,
                             m_tmpPreviewImgDir,
                             m_pSettings,
                             m_pTemplates );

    m_pHighlighter = new CHighlighter( m_pTemplates,
                                       m_pEditor->document() );

    /**
     * \todo Add tabs for editing multiple documents.
     */
    //myTabwidgetDocuments = new QTabWidget;
    //if ( m_bLogging ) { std::clog << "Created myTabwidgetDocuments" << std::endl; }
    m_pTabwidgetRawPreview = new QTabWidget;

    m_pWebview = new QWebView( this );
    m_pWebview->installEventFilter( this );

    m_pInsertSyntaxElement = new CInsertSyntaxElement( m_pTemplates->getTransTemplate(),
                                                       m_pTemplates->getTransImage(),
                                                       m_pTemplates->getTransTOC() );

    m_pTableTemplate = new CTableTemplate( m_pEditor,
										   m_UserAppDir,
										   m_tmpPreviewImgDir,
                                           m_pSettings,
                                           m_pTemplates );

    m_pPreviewTimer = new QTimer(this);

    qDebug() << "End" << Q_FUNC_INFO;
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CInyokaEdit::setupEditor()
{
    qDebug() << "Start" << Q_FUNC_INFO;

    // Application icon
    this->setWindowIcon( QIcon(":/images/" + m_pApp->applicationName().toLower() + "_64x64.png") );

    m_pEditor->setFont( m_pSettings->getEditorFont() );

    // Find/replace dialogs
    m_findDialog->setTextEdit(m_pEditor);
    m_findReplaceDialog->setTextEdit(m_pEditor);

    connect( m_pFileOperations, SIGNAL(setMenuLastOpenedEnabled(bool)),
             m_pUi->fileMenuLastOpened, SLOT(setEnabled(bool)) );

    /*
    setCentralWidget(myTabwidgetDocuments);
    myTabwidgetDocuments->setTabPosition(QTabWidget::North);
    myTabwidgetDocuments->setTabsClosable(true);
    myTabwidgetDocuments->setMovable(true);
    myTabwidgetDocuments->setDocumentMode(true);

    myTabwidgetDocuments->addTab(m_pTabwidgetRawPreview, tr("Untitled"));
    */

    m_pFrameLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    m_pFrameLayout->addWidget( m_pWebview );
    m_pWebviewFrame = new QFrame;
    m_pWebviewFrame->setLayout( m_pFrameLayout );
    m_pWebviewFrame->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );

    if ( true == m_pSettings->getPreviewAlongside() && true == m_pSettings->getPreviewInEditor() )
    {
        m_pWidgetSplitter = new QSplitter;
        m_pWidgetSplitter->addWidget( m_pEditor );
        //m_pWidgetSplitter->addWidget( m_pWebview );
        m_pWidgetSplitter->addWidget( m_pWebviewFrame );

        connect( m_pFileOperations, SIGNAL(loadedFile()),
                 this, SLOT(previewInyokaPage()) );

        setCentralWidget( m_pWidgetSplitter );
        m_pWidgetSplitter->restoreState( m_pSettings->getSplitterState() );

        // Show an empty website after start
        this->previewInyokaPage();
    }
    else
    {
        setCentralWidget( m_pTabwidgetRawPreview );
        m_pTabwidgetRawPreview->setTabPosition(QTabWidget::West);
        m_pTabwidgetRawPreview->addTab(m_pEditor, tr("Raw format"));

        m_pTabwidgetRawPreview->addTab(m_pWebviewFrame, tr("Preview"));
        if ( false == m_pSettings->getPreviewInEditor() )
        {
            m_pTabwidgetRawPreview->setTabEnabled(m_pTabwidgetRawPreview->indexOf(m_pWebviewFrame), false);
        }
    }

    connect( m_pWebview, SIGNAL(loadFinished(bool)),
             this, SLOT(loadPreviewFinished(bool)) );

    m_pFileOperations->setCurrentFile("");
    this->setUnifiedTitleAndToolBarOnMac(true);

    connect( m_pDownloadModule, SIGNAL(sendArticleText(QString, QString)),
             this, SLOT(displayArticleText(QString, QString)) );

    // Restore window and toolbar settings
    // Settings have to be restored after toolbars are created!
    this->restoreGeometry(m_pSettings->getWindowGeometry());
    this->restoreState(m_pSettings->getWindowState());  // Restore toolbar position etc.

    if ( false == m_pSettings->getPreviewAlongside() )
    {
        this->removeToolBar(m_pUi->browserBar);
    }

    m_pUi->aboutAct->setText( m_pUi->aboutAct->text() + " " + m_pApp->applicationName() );

    // Timed preview
    connect( m_pPreviewTimer, SIGNAL(timeout()),
             this, SLOT(previewInyokaPage()) );
    if( m_pSettings->getPreviewInEditor() && m_pSettings->getPreviewAlongside() &&
            m_pSettings->getTimedPreview() != 0 )
    {
        m_pPreviewTimer->start( m_pSettings->getTimedPreview() * 1000 );
    }

    qDebug() << "End" << Q_FUNC_INFO;
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

// Generate actions (buttons / menu entries)
void CInyokaEdit::createActions()
{
    qDebug() << "Start" << Q_FUNC_INFO;

    // File menu
    // New file

    m_pUi->newAct->setShortcuts(QKeySequence::New);
    connect( m_pUi->newAct, SIGNAL(triggered()),
             m_pFileOperations, SLOT(newFile()) );

    // Open file
    m_pUi->openAct->setShortcuts(QKeySequence::Open);
    connect( m_pUi->openAct, SIGNAL(triggered()),
             this, SLOT(openFile()) );

    // Clear recent files list
    m_pClearRecentFilesAct = new QAction(tr("Clear list"), this);
    connect( m_pClearRecentFilesAct, SIGNAL(triggered()),
             m_pFileOperations, SLOT(clearRecentFiles()) );

    // Save file
    m_pUi->saveAct->setShortcuts(QKeySequence::Save);
    connect( m_pUi->saveAct, SIGNAL(triggered()),
             m_pFileOperations, SLOT(save()) );

    // Save file as...
    m_pUi->saveAsAct->setShortcuts(QKeySequence::SaveAs);
    connect( m_pUi->saveAsAct, SIGNAL(triggered()),
             m_pFileOperations, SLOT(saveAs()) );

    // Print preview
    m_pUi->printPreviewAct->setShortcut(QKeySequence::Print);
    connect( m_pUi->printPreviewAct, SIGNAL(triggered()),
             m_pFileOperations, SLOT(printPreview()) );
    if ( false == m_pSettings->getPreviewAlongside() )
    {
        m_pUi->printPreviewAct->setEnabled(false);
    }

    // Exit application
    m_pUi->exitAct->setShortcuts(QKeySequence::Quit);
    connect( m_pUi->exitAct, SIGNAL(triggered()),
             this, SLOT(close()) );

    // ---------------------------------------------------------------------------------------------
    // EDIT MENU

    // Find
    m_pUi->searchAct->setShortcuts(QKeySequence::Find);
    connect( m_pUi->searchAct, SIGNAL(triggered()),
             m_findDialog, SLOT(show()) );

    // Replace
    m_pUi->replaceAct->setShortcuts(QKeySequence::Replace);
    connect( m_pUi->replaceAct, SIGNAL(triggered()),
             m_findReplaceDialog, SLOT(show()) );

    // Find next
    m_pUi->findNextAct->setShortcuts(QKeySequence::FindNext);
    connect( m_pUi->findNextAct, SIGNAL(triggered()),
             m_findDialog, SLOT(findNext()) );

    // Find previous
    m_pUi->findPreviousAct->setShortcuts(QKeySequence::FindPrevious);
    connect( m_pUi->findPreviousAct, SIGNAL(triggered()),
             m_findDialog, SLOT(findPrev()) );


    // Initialize cut / copy / redo / undo
    m_pUi->cutAct->setEnabled(false);
    m_pUi->copyAct->setEnabled(false);
    m_pUi->undoAct->setEnabled(false);
    m_pUi->redoAct->setEnabled(false);

    // ---------------------------------------------------------------------------------------------
    // TOOLS MENU

    // Spell checker
    m_pUi->spellCheckerAct->setShortcut(Qt::Key_F7);
    connect( m_pUi->spellCheckerAct, SIGNAL(triggered()),
             this, SLOT(checkSpelling()) );
#ifdef DISABLE_SPELLCHECKER
    m_pUi->spellCheckerAct->setVisible(false);
#endif

    // Download styles
    connect( m_pUi->DownloadInyokaStylesAct, SIGNAL(triggered()),
             m_pDownloadModule, SLOT(loadInyokaStyles()) );
#if defined _WIN32
    m_pUi->DownloadInyokaStylesAct->setDisabled( true );
#endif

    // Clear temp. image download folder
    connect( m_pUi->deleteTempImagesAct, SIGNAL(triggered()),
             this, SLOT(deleteTempImages()) );

    // ---------------------------------------------------------------------------------------------

    // Show html preview
    m_pUi->previewAct->setShortcut(Qt::CTRL + Qt::Key_P);
    connect( m_pUi->previewAct, SIGNAL(triggered()),
             this, SLOT(previewInyokaPage()) );

    // Click on tabs of widget - int = index of tab
    connect( m_pTabwidgetRawPreview, SIGNAL(currentChanged(int)),
             this, SLOT(previewInyokaPage(int)) );

    // Download Inyoka article
    connect( m_pUi->downloadArticleAct, SIGNAL(triggered()),
             this, SLOT(downloadArticle()) );

    // ---------------------------------------------------------------------------------------------

    // Insert headline
    m_pHeadlineBox = new QComboBox();
    m_pHeadlineBox->setStatusTip(tr("Insert a headline - 5 headline steps are supported"));

    // Insert sample
    m_pTextmacrosBox = new QComboBox();
    m_pTextmacrosBox->setStatusTip(tr("Insert text sample"));

    // Insert text format
    m_pTextformatBox = new QComboBox();
    m_pTextformatBox->setStatusTip(tr("Insert text format"));

    // ---------------------------------------------------------------------------------------------
    // INSERT SYNTAX ELEMENTS

    m_pSigMapSomeElements = new QSignalMapper(this);

    // Insert bold element
    m_pUi->boldAct->setShortcut(Qt::CTRL + Qt::Key_B);
    m_pSigMapSomeElements->setMapping(m_pUi->boldAct, "boldAct");
    connect( m_pUi->boldAct, SIGNAL(triggered()),
             m_pSigMapSomeElements, SLOT(map()) );

    // Insert italic element
    m_pUi->italicAct->setShortcut(Qt::CTRL + Qt::Key_I);
    m_pSigMapSomeElements->setMapping(m_pUi->italicAct, "italicAct");
    connect( m_pUi->italicAct, SIGNAL(triggered()),
             m_pSigMapSomeElements, SLOT(map()) );

    // Insert monotype element
    m_pSigMapSomeElements->setMapping(m_pUi->monotypeAct, "monotypeAct");
    connect( m_pUi->monotypeAct, SIGNAL(triggered()),
             m_pSigMapSomeElements, SLOT(map()) );

    // Insert wiki link
    m_pSigMapSomeElements->setMapping(m_pUi->wikilinkAct, "wikilinkAct");
    connect( m_pUi->wikilinkAct, SIGNAL(triggered()),
             m_pSigMapSomeElements, SLOT(map()) );

    // Insert extern link
    m_pSigMapSomeElements->setMapping(m_pUi->externalLinkAct, "externalLinkAct");
    connect( m_pUi->externalLinkAct, SIGNAL(triggered()),
             m_pSigMapSomeElements, SLOT(map()) );

    // Insert image
    m_pSigMapSomeElements->setMapping(m_pUi->imageAct, "imageAct");
    connect( m_pUi->imageAct, SIGNAL(triggered()),
             m_pSigMapSomeElements, SLOT(map()) );

    connect( m_pSigMapSomeElements, SIGNAL(mapped(QString)),
            this, SLOT(insertSomeSamples(QString)) );

    // Code block + syntax highlighting
    mySigMapCodeHighlight = new QSignalMapper(this);
    QStringList sListHighlightText, sListHighlightLang;
    sListHighlightText << tr("Raw text") << tr("Code without highlighting") << "Bash" << "C" <<
                          "C#" << "C++" << "CSS" << "D" << "Django / Jinja Templates" <<
                          "HTML" << "IRC Logs" << "Java" << "JavaScript" << "Perl" << "PHP" <<
                          "Python" << "Python Console Sessions" << "Python Tracebacks" <<
                          "Ruby" << "sources.list" << "SQL" << "XML";

    sListHighlightLang << ""  << "text" << "bash" << "c" <<
                          "csharp" << "cpp" << "css" << "d" << "html+django" <<
                          "html" << "irc" << "java" << "js" << "perl" << "html+php" <<
                          "python" << "pycon" << "pytb" <<
                          "ruby" << "sourceslist" << "sql" << "xml";

    QString sCodeTag("#!" + m_pTemplates->getTransCodeBlock().toLower() + " ");
    m_pCodePopup = new QToolButton;
    m_pCodePopup->setIcon(QIcon(":/images/code.png"));
    m_pCodePopup->setPopupMode( QToolButton::InstantPopup );
    m_pCodeStyles = new QMenu( m_pCodePopup );

    for ( int i = 0; i < sListHighlightText.size(); i++ ) {
        if (0 != i) { sListHighlightLang[i] = sCodeTag + sListHighlightLang[i]; }
        m_CodeHighlightActions << new QAction( sListHighlightText[i], this );
        mySigMapCodeHighlight->setMapping( m_CodeHighlightActions[i], sListHighlightLang[i] );
        connect( m_CodeHighlightActions[i], SIGNAL(triggered()),
                 mySigMapCodeHighlight, SLOT(map()) );
    }

    m_pCodeStyles->addActions( m_CodeHighlightActions );
    m_pCodePopup->setMenu(m_pCodeStyles);
    m_pUi->inyokaeditorBar->addWidget(m_pCodePopup);

    connect( mySigMapCodeHighlight, SIGNAL(mapped(QString)),
             this, SLOT(insertCodeblock(QString)) );

    connect( m_pUi->insertTableAct, SIGNAL(triggered()),
    		m_pTableTemplate, SLOT(newTable()) );

    // ---------------------------------------------------------------------------------------------
    // MARKUP TEMPLATES MENU

    m_pSigMapTemplates = new QSignalMapper(this);

    this->createXmlActions( m_pSigMapTemplates, "/templates/" + m_pSettings->getTemplateLanguage() + "/",
                            m_TplActions, m_pTemplates->getTPLs() );

    connect( m_pSigMapTemplates, SIGNAL(mapped(QString)),
             this, SLOT(insertMacro(QString)) );

    // ---------------------------------------------------------------------------------------------
    // INTERWIKI LINKS MENU

    m_pSigMapInterWikiLinks = new QSignalMapper(this);

    this->createXmlActions( m_pSigMapInterWikiLinks, "/iWikiLinks/",
                            m_iWikiLinksActions, m_pTemplates->getIWLs() );

    connect( m_pSigMapInterWikiLinks, SIGNAL(mapped(QString)),
             this, SLOT(insertInterwikiLink(QString)) );

    // ---------------------------------------------------------------------------------------------
    // ABOUT MENU

    // Show syntax overview
    connect( m_pUi->showSyntaxOverviewAct, SIGNAL(triggered()),
             this, SLOT(showSyntaxOverview()) );

    // Report a bug using apport
    connect( m_pUi->reportBugAct, SIGNAL(triggered()),
             this, SLOT(reportBug()) );

    // Open about windwow
    connect( m_pUi->aboutAct, SIGNAL(triggered()),
             this, SLOT(about()) );

    qDebug() << "End" << Q_FUNC_INFO;
}

// -----------------------------------------------------------------------------------------------

void CInyokaEdit::createXmlActions( QSignalMapper *SigMap, const QString sIconPath,
                                    QList<QList<QAction *> >& listActions, CXmlParser* pXmlMenu )
{
    QList <QAction *> emptyActionList;
    emptyActionList.clear();

    // No installation: Use app path
    QString sTmpPath = m_pApp->applicationDirPath() + sIconPath;
    // Path from normal installation
    if ( QFile::exists("/usr/share/" + m_pApp->applicationName().toLower() + sIconPath)
         && !bDEBUG )
    {
        sTmpPath = "/usr/share/" + m_pApp->applicationName().toLower() + sIconPath;
    }

    for ( int i = 0; i < pXmlMenu->getGrouplist().size(); i++ )
    {
        listActions.append( emptyActionList );
        for ( int j = 0; j < pXmlMenu->getElementNames()[i].size(); j++ )
        {
            listActions[i].append( new QAction( QIcon( sTmpPath + pXmlMenu->getElementIcons()[i][j] ),
                                                pXmlMenu->getElementNames()[i][j], this ) );

            SigMap->setMapping( listActions[i][j],
                                                 QString::number(i) + "," + QString::number(j) );
            connect( listActions[i][j], SIGNAL(triggered()),
                     SigMap, SLOT(map()) );
        }
    }
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

// Generate menus
void CInyokaEdit::createMenus()
{
    qDebug() << "Start" << Q_FUNC_INFO;

    QDir articleTemplateDir("");

    // File menu (new from template)
    if ( QFile::exists("/usr/share/" + m_pApp->applicationName().toLower() +
                       "/templates/" + m_pSettings->getTemplateLanguage() + "/articles")
         && !bDEBUG )
    {
        articleTemplateDir.setPath("/usr/share/" + m_pApp->applicationName().toLower() +
                                   "/templates/" + m_pSettings->getTemplateLanguage() + "/articles");
    }
    // No installation: Use app path
    else
    {
        articleTemplateDir.setPath( m_pApp->applicationDirPath() + "/templates/" + m_pSettings->getTemplateLanguage() + "/articles" );
    }

    unsigned short nTplFileCount = 0;
    if ( articleTemplateDir.exists() )
    {
        m_pSigMapOpenTemplate = new QSignalMapper(this);
        QFileInfoList fiListFiles = articleTemplateDir.entryInfoList( QDir::NoDotAndDotDot | QDir::Files );
        for ( int nFile = 0; nFile < fiListFiles.count(); nFile++ )
        {
            if ( "tpl" == fiListFiles.at(nFile).suffix()) {
                m_OpenTemplateFilesActions << new QAction( fiListFiles.at(nFile).baseName().replace("_", " "), this );
                m_pSigMapOpenTemplate->setMapping (m_OpenTemplateFilesActions.last(), fiListFiles.at(nFile).absoluteFilePath() );
                connect( m_OpenTemplateFilesActions.last(), SIGNAL(triggered()),
                         m_pSigMapOpenTemplate, SLOT(map()) );
                nTplFileCount++;
            }
        }
        m_pUi->fileMenuFromTemplate->addActions( m_OpenTemplateFilesActions );
        connect( m_pSigMapOpenTemplate, SIGNAL(mapped(QString)),
                 m_pFileOperations, SLOT(loadFile(QString)) );

        if ( 0 == nTplFileCount ){
            m_pUi->fileMenuFromTemplate->setDisabled(true);
        }
    }
    else
    {
        m_pUi->fileMenuFromTemplate->setDisabled(true);
    }

    // File menu (recent opened files)
    m_pUi->fileMenuLastOpened->addActions( m_pFileOperations->getLastOpenedFiles() );
    m_pUi->fileMenuLastOpened->addSeparator();
    m_pUi->fileMenuLastOpened->addAction( m_pClearRecentFilesAct );
    if ( 0 == m_pSettings->getRecentFiles().size() )
    {
        m_pUi->fileMenuLastOpened->setEnabled(false);
    }

    // Insert TPL menu
    m_pTplMenu = new QMenu( m_pTemplates->getTPLs()->getMenuName(), this );
    this->insertXmlMenu( m_pTplMenu, m_TplGroups, "/templates/" + m_pSettings->getTemplateLanguage() + "/",
                         m_TplActions, m_pTemplates->getTPLs(), m_pUi->toolsMenu->menuAction() );

    // Insert IWL menu
    m_piWikiMenu = new QMenu( m_pTemplates->getIWLs()->getMenuName(), this );
    this->insertXmlMenu( m_piWikiMenu, m_iWikiGroups, "/iWikiLinks/",
                         m_iWikiLinksActions, m_pTemplates->getIWLs(), m_pUi->toolsMenu->menuAction() );

    qDebug() << "End" << Q_FUNC_INFO;
}

// -----------------------------------------------------------------------------------------------

void CInyokaEdit::insertXmlMenu( QMenu* pMenu, QList<QMenu *> pMenuGroup, const QString sIconPath,
                                 QList<QList<QAction *> > listActions, CXmlParser* pXmlMenu, QAction* pPosition )
{
    qDebug() << "Start" << Q_FUNC_INFO;

    m_pUi->menuBar->insertMenu( pPosition, pMenu );

    // No installation: Use app path
    QString sTmpPath = m_pApp->applicationDirPath() + sIconPath;
    // Path from normal installation
    if ( QFile::exists("/usr/share/" + m_pApp->applicationName().toLower() + sIconPath)
         && !bDEBUG )
    {
        sTmpPath = "/usr/share/" + m_pApp->applicationName().toLower() + sIconPath;
    }

    for ( int i = 0; i < pXmlMenu->getGrouplist().size(); i++ )
    {
        pMenuGroup.append( pMenu->addMenu( QIcon(sTmpPath + pXmlMenu->getGroupIcons()[i]),
                                           pXmlMenu->getGrouplist()[i]) );

        pMenuGroup[i]->addActions( listActions[i] );
    }

    qDebug() << "End" << Q_FUNC_INFO;
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

// Generate tool bars
void CInyokaEdit::createToolBars()
{
    qDebug() << "Start" << Q_FUNC_INFO;

    // Tool bar for combo boxes (samples and macros)
    m_pUi->samplesmacrosBar->addWidget( m_pHeadlineBox );

    // Headline combo box
    QString sHeadline = tr("Headline");
    QString sHeadlineStep = tr("Step", "GUI: Headline combo box");
    m_pHeadlineBox->addItem(sHeadline);
    m_pHeadlineBox->insertSeparator(1);
    m_pHeadlineBox->addItem(sHeadline + ": " + sHeadlineStep + " 1");
    m_pHeadlineBox->addItem(sHeadline + ": " + sHeadlineStep + " 2");
    m_pHeadlineBox->addItem(sHeadline + ": " + sHeadlineStep + " 3");
    m_pHeadlineBox->addItem(sHeadline + ": " + sHeadlineStep + " 4");
    m_pHeadlineBox->addItem(sHeadline + ": " + sHeadlineStep + " 5");
    connect(m_pHeadlineBox, SIGNAL(activated(int)),
            this, SLOT(insertDropDownHeadline(int)));

    // Macros combo box
    m_pUi->samplesmacrosBar->addWidget( m_pTextmacrosBox );
    m_pTextmacrosBox->addItem( m_pTemplates->getDropTPLs()->getMenuName() );
    m_pTextmacrosBox->insertSeparator(1);
    if( m_pTemplates->getDropTPLs()->getElementNames().size() > 0 )
    {
        foreach( QString s, m_pTemplates->getDropTPLs()->getElementNames()[0] )
        {
            m_pTextmacrosBox->addItem(s);
        }
    }
    connect(m_pTextmacrosBox, SIGNAL(activated(int)),
            this, SLOT(insertDropDownTextmacro(int)));

    // Text format combo box
    m_pUi->samplesmacrosBar->addWidget( m_pTextformatBox );
    m_pTextformatBox->addItem(tr("Text format", "GUI: Text format combo box"));
    m_pTextformatBox->insertSeparator(1);
    m_pTextformatBox->addItem(tr("Folders", "GUI: Text format folders"));
    m_pTextformatBox->addItem(tr("Menu entries", "GUI: Text format menu entries"));
    m_pTextformatBox->addItem(tr("Files", "GUI: Text format files"));
    m_pTextformatBox->addItem(tr("Command", "GUI: Text format command"));
    connect(m_pTextformatBox, SIGNAL(activated(int)),
            this, SLOT(insertDropDownTextformat(int)));

    // Browser buttons
    connect( m_pUi->goBackBrowserAct, SIGNAL(triggered()),
             m_pWebview, SLOT(back()) );
    connect( m_pUi->goForwardBrowserAct, SIGNAL(triggered()),
            m_pWebview, SLOT(forward()) );
    connect( m_pUi->reloadBrowserAct, SIGNAL(triggered()),
             m_pWebview, SLOT(reload()) );
    connect( m_pWebview, SIGNAL(urlChanged(QUrl)),
             this, SLOT(clickedLink()) );

    qDebug() << "End" << Q_FUNC_INFO;
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CInyokaEdit::openFile()
{
    // Reset scroll position
    m_pWebview->page()->mainFrame()->setScrollPosition( QPoint(0,0) );

    m_pFileOperations->open();
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

// Call parser
void CInyokaEdit::previewInyokaPage( const int nIndex )
{
    // Call parser if iIndex == index of m_pWebview -> Click on tab preview
    // or if iIndex == 999 -> Default parameter value when calling the function
    // e.g. by clicking on button preview
    if ( m_pTabwidgetRawPreview->indexOf(m_pWebviewFrame) == nIndex || 999 == nIndex )
    {
        // Only disable buttons if preview is not shown alongside editor
        if ( false == m_pSettings->getPreviewAlongside() )
        {
            // Disable editor and insert samples/macros toolbars
            m_pUi->editMenu->setDisabled(true);
            m_pTplMenu->setDisabled(true);
            m_piWikiMenu->setDisabled(true);
            m_pUi->editToolBar->setDisabled(true);
            m_pUi->inyokaeditorBar->setDisabled(true);
            //this->removeToolBar(m_pUi->inyokaeditorBar);
            m_pUi->samplesmacrosBar->setDisabled(true);
            //this->removeToolBar(m_pUi->samplesmacrosBar);
            m_pUi->previewAct->setDisabled(true);
            this->addToolBar(m_pUi->browserBar);
            m_pUi->browserBar->show();

            m_pUi->printPreviewAct->setEnabled(true);
        }

        m_pWebview->history()->clear();  // Clear history (clicked links)

        QString sRetHTML;
        if ( "" == m_pFileOperations->getCurrentFile() ||
             tr("Untitled") == m_pFileOperations->getCurrentFile() )
        {
            sRetHTML = m_pParser->genOutput("");
        }
        else
        {
            QFileInfo fi( m_pFileOperations->getCurrentFile() );
            sRetHTML = m_pParser->genOutput( fi.fileName() );
        }

        // File for temporary html output
        QFile tmphtmlfile( m_UserAppDir.absolutePath() + "/tmpinyoka.html" );

        // No write permission
        if ( !tmphtmlfile.open(QFile::WriteOnly | QFile::Text) )
        {
            QMessageBox::warning(this, m_pApp->applicationName(), tr("Could not create temporary HTML file!"));
            return;
        }

        // Stream for output in file
        QTextStream tmpoutputstream(&tmphtmlfile);
        tmpoutputstream.setCodec("UTF-8");
        tmpoutputstream.setAutoDetectUnicode(true);

        // Write HTML code into output file
        tmpoutputstream << sRetHTML;
        tmphtmlfile.close();

        if ( false == m_pSettings->getPreviewInEditor() )
        {
            // Open html-file in system web browser
            QDesktopServices::openUrl( QUrl::fromLocalFile(QFileInfo(tmphtmlfile).absoluteFilePath()) );
        }
        else
        {
            // Store scroll position
            m_WebviewScrollPosition = m_pWebview->page()->mainFrame()->scrollPosition();

            m_pWebview->load( QUrl::fromLocalFile(QFileInfo(tmphtmlfile).absoluteFilePath()) );
        }
    }
    else
    {
        // Enable editor and insert samples/macros toolbars again
        m_pUi->editMenu->setEnabled(true);
        m_pTplMenu->setEnabled(true);
        m_piWikiMenu->setEnabled(true);
        m_pUi->editToolBar->setEnabled(true);
        m_pUi->inyokaeditorBar->setEnabled(true);
        //this->addToolBar(m_pUi->inyokaeditorBar);
        //m_pUi->inyokaeditorBar->show();
        m_pUi->samplesmacrosBar->setEnabled(true);
        //this->addToolBar(m_pUi->samplesmacrosBar);
        //m_pUi->samplesmacrosBar->show();
        m_pUi->previewAct->setEnabled(true);
        this->removeToolBar(m_pUi->browserBar);

        m_pUi->printPreviewAct->setEnabled(false);
    }
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// INSERT INYOKA ELEMENTS

// Headline (combobox in toolbar)
void CInyokaEdit::insertDropDownHeadline( const int nSelection )
{
    if ( nSelection > 1 )
    {
        QString sHeadline = tr("Headline");
        QString sHeadTag = "";

        // Generate headline tag
        for ( int i = 1; i < nSelection; i++ )
        {
            sHeadTag.append("=");
        }

        // Some text was selected
        if ( m_pEditor->textCursor().selectedText() != "" )
        {
            m_pEditor->insertPlainText( sHeadTag + " " + m_pEditor->textCursor().selectedText() + " " + sHeadTag );
        }
        // Select text sHeadline if no text was selected
        else
        {
            m_pEditor->insertPlainText( sHeadTag + " " + sHeadline + " " + sHeadTag );

            QTextCursor myTextCursor = m_pEditor->textCursor();
            myTextCursor.setPosition( m_pEditor->textCursor().position() - sHeadline.length() - nSelection );
            myTextCursor.setPosition( m_pEditor->textCursor().position() - nSelection, QTextCursor::KeepAnchor );
            m_pEditor->setTextCursor( myTextCursor );
        }
    }

    // Reset selection
    m_pHeadlineBox->setCurrentIndex(0);

    m_pEditor->setFocus();
}

// Macro (combobox in toolbar)
void CInyokaEdit::insertDropDownTextmacro( const int nSelection )
{
    if ( nSelection != 0 && nSelection != 1 )
    {
        QString sTmp;
        QString sName = m_pTemplates->getDropTPLs()->getElementUrls()[0][nSelection -2];
        sName.remove(".tpl");
        sName.remove(".macro");

        int nIndex = m_pTemplates->getListTplNamesALL().indexOf(sName);
        if( nIndex >= 0 )
        {
            QString sMacro = m_pTemplates->getListTplMacrosALL()[nIndex];
            sMacro.replace("\\n", "\n");
            int nPlaceholder1 = sMacro.indexOf("%%");
            int nPlaceholder2 = sMacro.lastIndexOf("%%");

            // No text selected
            if ( m_pEditor->textCursor().selectedText() == "" )
            {
                int nCurrentPos =  m_pEditor->textCursor().position();

                // Insert macro
                sMacro.remove("%%");  // Remove placeholder
                m_pEditor->insertPlainText( sMacro );

                // Select placeholder
                if( (nPlaceholder1 != nPlaceholder2) && nPlaceholder1 >= 0 && nPlaceholder2 >= 0 )
                {
                    QTextCursor textCursor = m_pEditor->textCursor();
                    textCursor.setPosition( nCurrentPos + nPlaceholder1 );
                    textCursor.setPosition( nCurrentPos + nPlaceholder2 -2, QTextCursor::KeepAnchor );
                    m_pEditor->setTextCursor( textCursor );
                }
            }
            // Some text is selected
            else
            {
                sTmp = sMacro;
                if( (nPlaceholder1 != nPlaceholder2) && nPlaceholder1 >= 0 && nPlaceholder2 >= 0 )
                {
                    sTmp.replace( nPlaceholder1, nPlaceholder2 - nPlaceholder1,
                                  m_pEditor->textCursor().selectedText() );
                    m_pEditor->insertPlainText( sTmp.remove("%%") );
                }
                // Problem with placeholder
                else
                {
                    m_pEditor->insertPlainText( sMacro.remove("%%") );
                }
            }
        }
        else
        {
            qWarning()  << "Unknown macro choosen:" << sName;
        }
    }

    m_pTextmacrosBox->setCurrentIndex(0);
    m_pEditor->setFocus();
}

// Text format (combobox in toolbar)
void CInyokaEdit::insertDropDownTextformat( const int nSelection )
{
    bool bSelected = false;
    QString sInsertedText = "";
    unsigned short iFormatLength = 0;

    // Some text was selected
    if ( m_pEditor->textCursor().selectedText() != "" )
    {
        bSelected = true;
    }

    if ( nSelection != 0 && nSelection != 1 )
    {
        // -1 because of separator (considered as "item")
        switch ( nSelection-1 )
        {
            default:
            case 1:  // Folders
                if ( bSelected )
                {
                    m_pEditor->insertPlainText( "'''" + m_pEditor->textCursor().selectedText() + "'''" );
                }
                else
                {
                    sInsertedText = tr("Folders", "Text format: Folders");
                    iFormatLength = 3;
                    m_pEditor->insertPlainText( "'''" + sInsertedText + "'''" );
                }
                break;
            case 2:  // Menus
                if ( bSelected )
                {
                    m_pEditor->insertPlainText( "''\"" + m_pEditor->textCursor().selectedText() + "\"''" );
                }
                else
                {
                    sInsertedText = tr("Menu -> sub menu -> menu entry", "Text format: Menu entries example");
                    iFormatLength = 3;
                    m_pEditor->insertPlainText( "''\"" + sInsertedText + "\"''" );
                }
                break;
            case 3:  // Files
                if ( bSelected )
                {
                    m_pEditor->insertPlainText( "'''" + m_pEditor->textCursor().selectedText() + "'''" );
                }
                else
                {
                    sInsertedText = tr("Files", "GUI: Text format: Files");
                    iFormatLength = 3;
                    m_pEditor->insertPlainText( "'''" + sInsertedText + "'''" );
                }
                break;
            case 4:  // Commands
                if ( bSelected )
                {
                    m_pEditor->insertPlainText( "`" + m_pEditor->textCursor().selectedText() + "`" );
                }
                else
                {
                    sInsertedText = tr("Command", "Text format: Command");
                    iFormatLength = 1;
                    m_pEditor->insertPlainText( "`" + sInsertedText + "`" );
                }
                break;
        }

        // Reset selection
        m_pTextformatBox->setCurrentIndex(0);

        if ( !bSelected )
        {
            QTextCursor myTextCursor = m_pEditor->textCursor();
            myTextCursor.setPosition( m_pEditor->textCursor().position() - sInsertedText.length() - iFormatLength );
            myTextCursor.setPosition( m_pEditor->textCursor().position() - iFormatLength, QTextCursor::KeepAnchor );
            m_pEditor->setTextCursor( myTextCursor );
        }

        m_pEditor->setFocus();
    }
}

// Insert text sample / syntax element
void CInyokaEdit::insertSomeSamples( const QString &sMenuEntry )
{
    m_pEditor->insertPlainText( QString::fromUtf8(m_pInsertSyntaxElement->getElementInyokaCode(sMenuEntry.toStdString(), m_pEditor->textCursor().selectedText().toStdString()).c_str()) );
    m_pEditor->setFocus();
}

// Insert macro
void CInyokaEdit::insertMacro( const QString &sMenuEntry )
{
    // Get indices for links
    QStringList slistTmp = sMenuEntry.split(",");
    QString sTmp;

    // Check if right number of indices found
    if ( slistTmp.size() == 2 )
    {
        QString sName = m_pTemplates->getTPLs()->getElementUrls()[slistTmp[0].toInt()][slistTmp[1].toInt()];
        sName.remove(".tpl");
        sName.remove(".macro");

        int nIndex = m_pTemplates->getListTplNamesALL().indexOf(sName);
        if( nIndex >= 0 )
        {
            QString sMacro = m_pTemplates->getListTplMacrosALL()[nIndex];
            sMacro.replace("\\n", "\n");
            int nPlaceholder1 = sMacro.indexOf("%%");
            int nPlaceholder2 = sMacro.lastIndexOf("%%");

            // No text selected
            if ( m_pEditor->textCursor().selectedText() == "" )
            {
                int nCurrentPos =  m_pEditor->textCursor().position();

                // Insert macro
                sMacro.remove("%%");  // Remove placeholder
                m_pEditor->insertPlainText( sMacro );

                // Select placeholder
                if( (nPlaceholder1 != nPlaceholder2) && nPlaceholder1 >= 0 && nPlaceholder2 >= 0 )
                {
                    QTextCursor textCursor = m_pEditor->textCursor();
                    textCursor.setPosition( nCurrentPos + nPlaceholder1 );
                    textCursor.setPosition( nCurrentPos + nPlaceholder2 -2, QTextCursor::KeepAnchor );
                    m_pEditor->setTextCursor( textCursor );
                }
            }
            // Some text is selected
            else
            {
                sTmp = sMacro;
                if( (nPlaceholder1 != nPlaceholder2) && nPlaceholder1 >= 0 && nPlaceholder2 >= 0 )
                {
                    sTmp.replace( nPlaceholder1, nPlaceholder2 - nPlaceholder1,
                                  m_pEditor->textCursor().selectedText() );
                    m_pEditor->insertPlainText( sTmp.remove("%%") );
                }
                // Problem with placeholder
                else
                {
                    m_pEditor->insertPlainText( sMacro.remove("%%") );
                }
            }
        }
        else
        {
            qWarning()  << "Unknown macro choosen:" << sName;
        }
    }
    // Problem with indices
    else
    {
        qWarning() << "Error while inserting template macro - TPL indice:" << sMenuEntry;
        QMessageBox::warning( this, m_pApp->applicationName(), "Error while inserting template macro: Template indice" );
    }

    m_pEditor->setFocus();
}

// Insert interwiki-link
void CInyokaEdit::insertInterwikiLink( const QString &sMenuEntry )
{
    // Get indices for links
    QStringList sTmp = sMenuEntry.split(",");

    // Check if right number of indices found
    if ( sTmp.size() == 2 )
    {
        // No text selected
        if ( m_pEditor->textCursor().selectedText() == "" )
        {
            QString sSitename = tr("Sitename", "InterWiki links: Common sitename");
            QString sText = tr("Text", "Interwiki links: Common text");

            // Insert InterWiki-Link
            m_pEditor->insertPlainText( "[" + m_pTemplates->getIWLs()->getElementTypes()[sTmp[0].toInt()][sTmp[1].toInt()] +
                                        ":" + sSitename + ":" + sText + "]" );

            // Select site name in InterWiki-Link
            QTextCursor textCursor = m_pEditor->textCursor();
            textCursor.setPosition( m_pEditor->textCursor().position() - sSitename.length() - sText.length() - 2);
            textCursor.setPosition( m_pEditor->textCursor().position() - sText.length() - 2, QTextCursor::KeepAnchor );
            m_pEditor->setTextCursor( textCursor );
        }
        // Some text is selected
        else
        {
            // Insert InterWiki-Link with selected text
            m_pEditor->insertPlainText( "[" + m_pTemplates->getIWLs()->getElementTypes()[sTmp[0].toInt()][sTmp[1].toInt()] +
                                        ":" + m_pEditor->textCursor().selectedText() + ":]" );
        }
    }
    // Problem with indices
    else
    {
        qWarning() << "Error while inserting InterWiki link - IWL indice:" << sMenuEntry;
        QMessageBox::warning( this, m_pApp->applicationName(), "Error while inserting InterWiki link: InterWiki indice" );
    }

    m_pEditor->setFocus();
}

// Insert code block
void CInyokaEdit::insertCodeblock( const QString &sCodeStyle )
{
    // No text selected
    if ( m_pEditor->textCursor().selectedText() == "" )
    {
        QString sCode("Code");

        // Insert code block
        m_pEditor->insertPlainText( "{{{" + sCodeStyle + "\n" + sCode + "\n}}}\n" );

        // Select the word "code"
        QTextCursor textCursor = m_pEditor->textCursor();
        textCursor.setPosition( m_pEditor->textCursor().position() - sCode.length() - 5);
        textCursor.setPosition( m_pEditor->textCursor().position() - 5, QTextCursor::KeepAnchor );
        m_pEditor->setTextCursor( textCursor );
    }
    // Some text is selected
    else
    {
        // Insert code block with selected text
        m_pEditor->insertPlainText( "{{{" + sCodeStyle + "\n" + m_pEditor->textCursor().selectedText() + "\n}}}\n" );
    }

    m_pEditor->setFocus();
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CInyokaEdit::downloadArticle()
{
    if ( m_pFileOperations->maybeSave() )
    {
        bool bSuccess = m_pDownloadModule->downloadArticle( m_tmpPreviewImgDir, m_pSettings->getInyokaUrl(), m_pSettings->getAutomaticImageDownload() );
        if ( bSuccess )
        {
            // Reset scroll position
            m_pWebview->page()->mainFrame()->setScrollPosition( QPoint(0,0) );

            this->previewInyokaPage();
        }
    }
}

void CInyokaEdit::displayArticleText( const QString &sArticleText, const QString &sSitename )
{
    m_pEditor->setPlainText(sArticleText);
    m_pFileOperations->setCurrentFile(sSitename);
    m_pEditor->document()->setModified(true);
    this->documentWasModified();
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

// Wait until loading has finished
void CInyokaEdit::loadPreviewFinished( bool bSuccess )
{
    if ( bSuccess )
    {
        m_pTabwidgetRawPreview->setCurrentIndex( m_pTabwidgetRawPreview->indexOf(m_pWebviewFrame) );
        // Enable / disbale back button
        if ( m_pWebview->history()->canGoBack() )
        {
            m_pUi->goBackBrowserAct->setEnabled(true);
        }
        else
        {
            m_pUi->goBackBrowserAct->setEnabled(false);
        }

        // Enable / disable forward button
        if ( m_pWebview->history()->canGoForward() )
        {
            m_pUi->goForwardBrowserAct->setEnabled(true);
        }
        else
        {
            m_pUi->goForwardBrowserAct->setEnabled(false);
        }

        // Restore scroll position
        m_pWebview->page()->mainFrame()->setScrollPosition( m_WebviewScrollPosition );
    }
    else
    {
        QMessageBox::warning( this, m_pApp->applicationName(), tr("Error while loading preview.") );
    }
}

// A link on preview page was clicked
void CInyokaEdit::clickedLink()
{
    // Disable forward / backward button
    m_pUi->goForwardBrowserAct->setEnabled(false);
    m_pUi->goBackBrowserAct->setEnabled(false);
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

bool CInyokaEdit::eventFilter( QObject *obj, QEvent *event )
{
    if ( obj == m_pEditor && event->type() == QEvent::KeyPress )
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        // ---------------------------------------------------------------------------
        // Bug fix for LP: #922808
        Qt::KeyboardModifiers keyMod = QApplication::keyboardModifiers();
        bool isSHIFT = keyMod.testFlag(Qt::ShiftModifier);
        bool isCTRL = keyMod.testFlag(Qt::ControlModifier);

        // CTRL + SHIFT + arrow right
        if ( keyEvent->key() == Qt::Key_Right &&
             isSHIFT && isCTRL )
        {
            QTextCursor cursor(m_pEditor->textCursor());
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
            m_pEditor->setTextCursor(cursor);
            return true;
        }
        // CTRL + arrow right
        else if ( keyEvent->key() == Qt::Key_Right &&
                  !isSHIFT && isCTRL )
        {
            m_pEditor->moveCursor(QTextCursor::Right);
            m_pEditor->moveCursor(QTextCursor::EndOfWord);
            return true;
        }

        // ----------------------------------------------
        // Bug fix for LP: #889321

        // CTRL + SHIFT arrow down
        else if ( keyEvent->key() == Qt::Key_Up &&
                  isSHIFT && isCTRL )
        {
            QTextCursor cursor(m_pEditor->textCursor());
            cursor.movePosition(QTextCursor::Up, QTextCursor::KeepAnchor);
            m_pEditor->setTextCursor(cursor);
            return true;
        }

        // CTRL + SHIFT arrow down
        else if ( keyEvent->key() == Qt::Key_Down &&
                  isSHIFT && isCTRL )
        {
            QTextCursor cursor(m_pEditor->textCursor());
            cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor);
            m_pEditor->setTextCursor(cursor);
            return true;
        }
        // ---------------------------------------------------------------------------
        // ---------------------------------------------------------------------------
        // CTRL + SHIFT + T (only for debugging)
        else if ( keyEvent->key() == Qt::Key_T &&
                  isSHIFT && isCTRL )
        {
            static bool bToggle = false;
            static QTextDocument docBackup("");

            if ( !bToggle ) {
                docBackup.setPlainText( m_pEditor->document()->toPlainText() );
                m_pParser->replaceTemplates( m_pEditor->document() );
            }
            else {
                m_pEditor->setText( docBackup.toPlainText() );
            }
            bToggle = !bToggle;
            return true;
        }
        // ---------------------------------------------------------------------------
        // ---------------------------------------------------------------------------

        // Reload preview at F5 or defined button if preview alongside
        else if ( (Qt::Key_F5 == keyEvent->key() || m_pSettings->getReloadPreviewKey() == keyEvent->key()) &&
                  (true == m_pSettings->getPreviewAlongside() && true == m_pSettings->getPreviewInEditor()) )
        {
            previewInyokaPage();
        }
    }

    // Forward / backward mouse button
    else if ( obj == m_pWebview && event->type() == QEvent::MouseButtonPress )
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if ( mouseEvent->button() == Qt::XButton1 )
        {
            m_pWebview->back();
        }
        else if ( mouseEvent->button() == Qt::XButton2 )
        {
            m_pWebview->forward();
        }
    }

    // Else
    return QObject::eventFilter( obj, event );
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

void CInyokaEdit::checkSpelling()
{
  #ifndef DISABLE_SPELLCHECKER
    QString sDictPath("");

    // Standard path for Hunspell (Linux only)
    if ( QDir("/usr/share/hunspell").exists() ) // && !bDEBUG )
    {
        sDictPath = "/usr/share/hunspell/" + m_pSettings->getSpellCheckerLanguage();
    }
    // Otherwise look for MySpell dictionary (Linx only)
    else if ( QDir("/usr/share/myspell/dicts").exists() ) // && !bDEBUG )
    {
        sDictPath = "/usr/share/myspell/dicts/" + m_pSettings->getSpellCheckerLanguage();
    }
    // Fallback and for Windows look in app dir
    else
    {
        sDictPath = m_pApp->applicationDirPath() + "/dicts/" + m_pSettings->getSpellCheckerLanguage();
    }

    if ( !QFile::exists(sDictPath + ".dic") || !QFile::exists(sDictPath + ".aff") )
    {
        qWarning() << "Spell checker dictionary file does not exist:" << sDictPath << "*.dic *.aff";
        QMessageBox::critical( this, m_pApp->applicationName(), "Error: Spell checker dictionary file does not exist!" );
        return;
    }

    QString sUserDict= m_UserAppDir.absolutePath() + "/userDict_" + m_pSettings->getSpellCheckerLanguage() + ".txt";
    if ( !QFile::exists(sUserDict) )
    {
        QFile userDictFile(sUserDict);
        if( userDictFile.open(QIODevice::WriteOnly) )
        {
            userDictFile.close();
        }
        else {
            QMessageBox::warning( 0, m_pApp->applicationName(), "User dictionary file could not be created." );
        }
    }
    CSpellChecker *spellChecker = new CSpellChecker( sDictPath, sUserDict, this );
    spellChecker->start(m_pEditor);

    if ( spellChecker != NULL )
    {
        delete spellChecker;
    }
    spellChecker = NULL;

    QMessageBox::information( this, m_pApp->applicationName(), tr("Spell check has finished.") );
  #endif
}


// -----------------------------------------------------------------------------------------------

// Delete images in temp. download folder (images downloaded with articles)
void CInyokaEdit::deleteTempImages()
{
    int nRet = QMessageBox::question( this, m_pApp->applicationName(), tr("Do you really want to delete all images downloaded with articles?"), QMessageBox::Yes | QMessageBox::No);

    if ( QMessageBox::Yes== nRet )
    {
        // Remove all files in current folder
        QFileInfoList fiListFiles = m_tmpPreviewImgDir.entryInfoList( QDir::NoDotAndDotDot | QDir::Files );
        for ( int nFile = 0; nFile < fiListFiles.count(); nFile++ )
        {
            if ( !m_tmpPreviewImgDir.remove( fiListFiles.at(nFile).fileName() ) )
            {
                // Problem while removing
                QMessageBox::warning( this, m_pApp->applicationName(), tr("Could not delete file: ") + fiListFiles.at(nFile).fileName() );
                return;
            }
        }
        QMessageBox::information( this, m_pApp->applicationName(), tr("Images successfully deleted.") );
    }
    else
    {
        return;
    }
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------

// Set modified flag for window
void CInyokaEdit::documentWasModified()
{
    m_pTabwidgetRawPreview->setCurrentIndex( m_pTabwidgetRawPreview->indexOf(m_pEditor) );
    this->setWindowModified( m_pEditor->document()->isModified() );
}

// -----------------------------------------------------------------------------------------------

void CInyokaEdit::showSyntaxOverview()
{
    QDialog* dialog = new QDialog( this, this->windowFlags() & ~Qt::WindowContextHelpButtonHint );
    QGridLayout* layout = new QGridLayout(dialog);
    QWebView* webview = new QWebView();
    QTextDocument* pTextDocument = new QTextDocument(this);

    QFile OverviewFile("");
    // Path from normal installation
    if ( QFile::exists("/usr/share/" + m_pApp->applicationName().toLower() + "/templates/" +
                       m_pSettings->getTemplateLanguage() + "/SyntaxOverview") && !bDEBUG )
    {
        OverviewFile.setFileName( "/usr/share/" + m_pApp->applicationName().toLower() + "/templates/" +
                                  m_pSettings->getTemplateLanguage() + "/SyntaxOverview" );
    }
    // No installation: Use app path
    else
    {
        OverviewFile.setFileName( m_pApp->applicationDirPath() + "/templates/" +
                                  m_pSettings->getTemplateLanguage() + "/SyntaxOverview" );
    }

    QTextStream in( &OverviewFile );
    if ( ! OverviewFile.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        QMessageBox::warning( 0, "Warning", tr("Could not open syntax overview file!") );
        qWarning() << "Could not open syntax overview file:" << OverviewFile.fileName();
        return;
    }
    pTextDocument->setPlainText( in.readAll() );
    OverviewFile.close();

    CParser* pParser = new CParser( pTextDocument,
                                    m_UserAppDir,
                                    m_tmpPreviewImgDir,
                                    m_pSettings,
                                    m_pTemplates );

    QString sRet = pParser->genOutput("");
    sRet.remove( QRegExp("<h1 class=\"pagetitle\">.*</h1>") );
    sRet.remove( QRegExp("<p class=\"meta\">.*</p>") );
    sRet.replace( "</style>", "#page table{margin:0px;}</style>");
    pTextDocument->setPlainText( sRet );

    layout->setMargin(2);
    layout->setSpacing(0);
    layout->addWidget(webview);
    dialog->setWindowTitle( tr("Syntax overview") );

    webview->setHtml( pTextDocument->toPlainText(), QUrl::fromLocalFile(m_UserAppDir.absolutePath() + "/") );
    dialog->show();
}

// -----------------------------------------------------------------------------------------------

// Report a bug
void CInyokaEdit::reportBug()
{
    // Ubuntu: Using Apport, if needed files exist
    if ( QFile::exists( "/usr/bin/ubuntu-bug" )
         && QFile::exists( "/etc/apport/crashdb.conf.d/inyokaedit-crashdb.conf" )
         && QFile::exists( "/usr/share/apport/package-hooks/source_inyokaedit.py" ) )
    {
        // Start apport
        QProcess procApport;
        procApport.start( "ubuntu-bug", QStringList() << m_pApp->applicationName().toLower() );

        if ( !procApport.waitForStarted() )
        {
            QMessageBox::critical( this, m_pApp->applicationName(), tr("Error while starting Apport.") );
            return;
        }
        if ( !procApport.waitForFinished() )
        {
            QMessageBox::critical( this, m_pApp->applicationName(), tr("Error while executing Apport.") );
            return;
        }
    }
    // Not Ubuntu or apport files not found: Load Launchpad bug tracker
    else
    {
        QDesktopServices::openUrl( QUrl("https://bugs.launchpad.net/inyokaedit") );
    }
}

// -----------------------------------------------------------------------------------------------

// About info box
void CInyokaEdit::about()
{
    QString sUserIcon("");
    if ( QFile::exists("/usr/share/" + m_pApp->applicationName().toLower() + "/iWikiLinks/user.png")
         && !bDEBUG )
    {
        sUserIcon = "/usr/share/" + m_pApp->applicationName().toLower() + "/iWikiLinks/user.png";
    }
    // No installation: Use app path
    else
    {
        sUserIcon =  m_pApp->applicationDirPath() + "/iWikiLinks/user.png";
    }

    QMessageBox::about(
                this, tr("About %1", "About dialog <sAppName>").arg( m_pApp->applicationName() ),
                tr("<p><b>%1</b> - Editor for Inyoka-based portals<br />"
                   "Version: %2</p>"
                   "<p>&copy; 2011-2012, The %3 developers<br />"
                   "Licence: <a href=\"http://www.gnu.org/licenses/gpl-3.0.html\">GNU General Public License Version 3</a></p>"
                   "<p>Special thanks to <img src=\"%4\" /> bubi97, <img src=\"%4\" /> Lasall, <img src=\"%4\" /> Shakesbier"
                   " and all testers from <a href=\"http://ubuntuusers.de\">ubuntuusers.de</a>.</p>"
                   "<p>This application uses icons from <a href=\"http://tango.freedesktop.org\">Tango project</a>.</p>",
                   "About dialog text, <sAppName>, <sAppVersion>, <sAppName>, <sUserIcon>")
                .arg(m_pApp->applicationName())
                .arg(m_pApp->applicationVersion())
                .arg(m_pApp->applicationName())
                .arg(sUserIcon) );
}

// -----------------------------------------------------------------------------------------------

// Close event (File -> Close or X)
void CInyokaEdit::closeEvent( QCloseEvent *event )
{
    if ( m_pFileOperations->maybeSave() )
    {
        if ( true == m_pSettings->getPreviewAlongside() && true == m_pSettings->getPreviewInEditor() )
        {
            m_pSettings->writeSettings( saveGeometry(), saveState(), m_pWidgetSplitter->saveState() );
        }
        else
        {
            m_pSettings->writeSettings( saveGeometry(), saveState() );
        }
        event->accept();
    }
    else
    {
        event->ignore();
    }
}
