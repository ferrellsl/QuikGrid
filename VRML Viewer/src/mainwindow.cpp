/**************************************************************************\
 *
 *  This file is part of the Coin VRML Viewer.
 *  Copyright (C) 1998-2007 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using Coin with software that can not be combined with the GNU
 *  GPL, and for taking advantage of the additional benefits of our
 *  support services, please contact Systems in Motion about acquiring
 *  a Coin Professional Edition License.
 *
 *  See http://www.coin3d.org/ for more information.
 *
 *  Systems in Motion, Postboks 1283, Pirsenteret, 7462 Trondheim, NORWAY.
 *  http://www.sim.no/  sales@sim.no  coin-support@coin3d.org
 *
\**************************************************************************/


/*!
 * \class MainWindow
 * \brief MainWindow extends QMainWindow and contains most of the functionality of the program
 * 
 * This class is the class that contains the functionality. It is meant to be used with a QApplication 
 * starter program: 
 * \verbatim
 * #include <QApplication>
 *
 * #include "mainwindow.h"
 * 
 * 	int main(int argc, char *argv[])
 * 	{
 *	QApplication app(argc, argv);
 *	MainWindow window;
 * 
 *	window.show();
 *	return app.exec();
 *	}
 * \endverbatim
 */

#include "mainwindow.h"

/*!
 * Constructor.
 */
MainWindow::MainWindow()
{
    QWidget *w = new QWidget;
    
	createSoQt();

	setCentralWidget(w);

	vbox = new QVBoxLayout();
    vbox->setMargin(3);

	viewer->getWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    vbox->addWidget(viewer->getWidget());

    w->setLayout(vbox);

    createActions();
    createMenus();

	setExaminerViewerAct->setCheckable(true);
	setPlaneViewerAct->setCheckable(true);
	setFlyViewerAct->setCheckable(true);
	setExaminerViewerAct->setChecked(true);
	setPlaneViewerAct->setChecked(false);	
	setFlyViewerAct->setChecked(false);

	wmAct->setCheckable(true);
	wmAct->setChecked(true);
	fsAct->setCheckable(true);
	fsAct->setChecked(false);

    statusBar()->showMessage(tr("A context menu is available by right-clicking"));
 
	QString about_version = "VRML Viewer: ";
	about_version.append(VRML_VER);

	setWindowTitle(about_version);
    setMinimumSize(160, 160);
    resize(640, 480);
}

/*!
 * \brief Sets up the connection between QT and Coin, abit unfinished atm.
 */
void 
MainWindow::createSoQt()
{
	//Initializing SoQt and SoDB
	SoQt::init("VRML Viewer");
	SoDB::init();

	SoGroup *root = new SoGroup;
	root->ref();

	//Camera and light testing
	SoOrthographicCamera *orthoViewAll = new SoOrthographicCamera;
	root->addChild(orthoViewAll);
	root->addChild(new SoDirectionalLight);

	// Choose a font.
	SoFont *myFont = new SoFont;
	myFont->name.setValue("Arial");
	myFont->size.setValue(0.5f);
	root->addChild(myFont);
	SoDB::cleanup();
	// We'll color the font blue
	SoMaterial *myMaterial = new SoMaterial;
	SoMaterialBinding *myBinding = new SoMaterialBinding;
	myMaterial->diffuseColor.set1Value(0,SbColor(0.0f,0.0f,1.0f));
	myMaterial->diffuseColor.set1Value(1,SbColor(0.1f,0.1f,0.1f));
	myBinding->value = SoMaterialBinding::PER_PART;
	root->addChild(myMaterial);
	root->addChild(myBinding);

	// Add Text3 for VRML Viewer, transformed to proper location.
	SoSeparator *vrmlStartSep = new SoSeparator;
	SoTransform *vrmlStartTransform = new SoTransform;
	SoText3 *vrmlStartText = new SoText3;
	vrmlStartTransform->translation.setValue(0.25f,0.0f,1.25);

	vrmlStartText->parts = SoText3::ALL;
	vrmlStartText->string = "VRML Viewer";
	root->addChild(vrmlStartSep);
	vrmlStartSep->addChild(vrmlStartTransform);
	vrmlStartSep->addChild(vrmlStartText);

	// viewer = new SoQtRenderArea(this,"helo", true, true, true);
	// viewer = new SoQtFullViewer(this, "helo", true, SoQtFullViewer::BUILD_NONE, SoQtViewer::BROWSER, false);
	viewer = new SoQtExaminerViewer(this, "Viewer", TRUE, SoQtFullViewer::BUILD_NONE, SoQtViewer::BROWSER);

	// Camera and light adding
	SbViewportRegion myRegion(viewer->getSize());
	orthoViewAll->viewAll(root, myRegion);

	SbVec3f initialPos;
	initialPos = orthoViewAll->position.getValue();
	float x, y, z;
	initialPos.getValue(x,y,z);
	orthoViewAll->position.setValue(x, y, z+z/2);

	viewer->setSceneGraph(root);
	viewer->setViewing(false);
	viewer->show();
	viewer->setTitle("3D Text");
	viewer->setBackgroundColor(SbColor(0.0f, 0.0f, 0.0f));
}

/*!
 * \brief Creates the context menu events.
 */
void
MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(refreshAct);
    menu.addAction(fsAct);
    menu.addAction(wmAct);
    menu.exec(event->globalPos());
}

/*!
 * \brief Opens a filedialog and saves a screenshot.
 *
 * FIXME: This one isnt working completely
 * Known bug: The problem is that the QFileDialog returns a QString. The writeToFile function does
 * not understand this type of object, and I'm not able to find out how to convert it into SbString and SbName
 */
void 
MainWindow::snapshotFile()
{
	/*
	SbViewportRegion vpregion = viewer->getSceneManager()->getViewportRegion();
	SoOffscreenRenderer myRenderer(vpregion);
	SoNode *root = viewer->getSceneManager()->getSceneGraph();
	SbBool ok = myRenderer.render(root);

	QString fileToSave = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Images (*.png *.gif *.jpg *.jpeg *.tiff *.eps)"));

	if(fileToSave.isEmpty()){
		QMessageBox::about(this, tr("Notice"), tr("No file was selected, screenshot not taken."));	
		return;
	}

	QString extension = fileToSave;
	extension.remove(0,(extension.lastIndexOf('.',-1,Qt::CaseInsensitive)+1));
	QMessageBox::about(this, tr("Debugging"), fileToSave);
	QMessageBox::about(this, tr("Debugging"), extension);

	//Compiler error here....
	bool tmp = myRenderer.writeToFile(fileToSave.toAscii(), extension.toAscii()); 

	QString str = "";
	if(tmp){
		str.append("True");
	}
	else{
		QMessageBox::about(this, tr("Error"), tr("Could not create screenshot."));	
		str.append("False");
	}
	statusBar()->showMessage(str);
	*/
}

/*!
 * \brief Opens a filedialog and opens the model the user wants, if its a valid one.
 */
void 
MainWindow::open()
{
	fileInViewer = QFileDialog::getOpenFileName(this);
	if (!fileInViewer.isEmpty()){
		SoSeparator *root = new SoSeparator;
		root->ref();

		SoBaseColor *col = new SoBaseColor;
		col->rgb = SbColor(1, 1, 0);
		root->addChild(col);

		statusBar()->showMessage(fileInViewer);

		SoInput in;
		if ( in.openFile( fileInViewer.toAscii() ) ){
			SoSeparator * readFile = SoDB::readAll(&in);
			if (readFile){
				root->addChild(readFile);
				readFile->unref();
			}
			else{
				QMessageBox::about(this, tr("Error"), tr("Problem reading the file, perhaps not a supported format ?"));		
			}
		}
		else{
			QMessageBox::about(this, tr("Error"), tr("Could not open the file."));		
		}
		
		viewer->setViewing(true);
		viewer->setSceneGraph(root);	
		root->unref();

		setCurrentFile(fileInViewer);
		repaint();
	}
	else{
		QMessageBox::about(this, tr("Notice"), tr("No file specified, no model loaded."));		
	}
}

/*!
 * \brief Opens a file, used with the recent file selector.
 */
void 
MainWindow::loadFile(const QString &fileName)
{
    fileInViewer = fileName;
	QFile file(fileName);

	SoSeparator *root = new SoSeparator;
	root->ref();

	SoInput in;
	if ( in.openFile( fileName.toAscii() ) ){
		SoSeparator * readFile = SoDB::readAll(&in);
		if (readFile){
			root->addChild(readFile);
			readFile->unref();
		}
	}

	viewer->setViewing(true);
	viewer->setSceneGraph(root);	
	root->unref();

	setCurrentFile(fileName);

	repaint();
}

/*!
 * \brief Calls loadFile() with the selected recent file.
 */
void 
MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        loadFile(action->data().toString());
}

/*!
 * \brief Opens a dialogbox with version info.
 */
void 
MainWindow::about()
{
    statusBar()->showMessage(tr("Invoked Help|About"));
	QString about_version = "VRML Version: ";
	about_version.append(VRML_VER);

    QMessageBox::about(this, tr("About VRML"), about_version);
}

/*!
 * \brief Sets the viewer to SoQtExaminerViewer.
 */
void
MainWindow::setExaminerViewer()
{
	setExaminerViewerAct->setChecked(true);
	setPlaneViewerAct->setChecked(false);	
	setFlyViewerAct->setChecked(false);
	vbox->removeWidget(viewer->getWidget());
	viewer = new SoQtExaminerViewer(this, "Viewer", TRUE, SoQtFullViewer::BUILD_NONE, SoQtViewer::BROWSER);
	viewer->getWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	vbox->addWidget(viewer->getWidget());
	contextRefresh();
}
 
/*!
 * \brief Sets the viewer to SoQtPlaneViewer.
 */
void
MainWindow::setPlaneViewer()
{
	setExaminerViewerAct->setChecked(false);
	setPlaneViewerAct->setChecked(true);	
	setFlyViewerAct->setChecked(false);
	vbox->removeWidget(viewer->getWidget());
	viewer = new SoQtPlaneViewer(this, "Viewer", TRUE, SoQtFullViewer::BUILD_NONE, SoQtViewer::BROWSER);
	viewer->getWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	vbox->addWidget(viewer->getWidget());
	contextRefresh();
}

/*!
 * \brief Sets the viewer to SoQtFlyViewer
 */
void
MainWindow::setFlyViewer()
{
	setExaminerViewerAct->setChecked(false);
	setPlaneViewerAct->setChecked(false);	
	setFlyViewerAct->setChecked(true);
	vbox->removeWidget(viewer->getWidget());
	viewer = new SoQtFlyViewer(this, "Viewer", TRUE, SoQtFullViewer::BUILD_NONE, SoQtViewer::BROWSER);
	viewer->getWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	vbox->addWidget(viewer->getWidget());
	contextRefresh();
}

/*!
 * \brief Refreshes the current model in the viewer.
 */
void
MainWindow::contextRefresh()
{
	if (!fileInViewer.isEmpty()){
		SoSeparator *root = new SoSeparator;
		root->ref();

		SoBaseColor *col = new SoBaseColor;
		col->rgb = SbColor(1, 1, 0);
		root->addChild(col);

		SoInput in;
		if (in.openFile(fileInViewer.toAscii())){
			SoSeparator * readFile = SoDB::readAll(&in);
			if (readFile){
				root->addChild(readFile);
			}
		}

		viewer->setSceneGraph(root);	
		root->unref();
		repaint();	
	}
}

/*!
 * \brief Sets the program to fullscreen.
 */
void 
MainWindow::setFS()
{
	viewer->setFullScreen(true);
	wmAct->setChecked(false);
	fsAct->setChecked(true);
	statusBar()->showMessage(tr("Fullscreen"));
}

/*!
 * \brief Sets the program to normal windowed mode.
 */
void
MainWindow::setWM()
{
	viewer->setFullScreen(false);
	wmAct->setChecked(true);
	fsAct->setChecked(false);
	resize(640, 480);
	statusBar()->showMessage(tr("Windowed mode"));
}

/*!
 * \brief Internally used for setting the current file in the buffer.
 */
void
MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
	
	QSettings settings("VRML Viewer", "Recent Files");
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("recentFileList", files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()){
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
}

/*!
 * \brief Updates the recent 5 files seen.
 */
void
MainWindow::updateRecentFileActions()
{
    QSettings settings("VRML Viewer", "Recent Files");
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i){
        QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);

    separatorAct->setVisible(numRecentFiles > 0);
}

/*!
 * \brief Strips the name of the file in the buffer.
 */
QString
MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

/*!
 * \brief Sets up the connections with mouseclicks. 
 */
void
MainWindow::createActions()
{
    newAct = new QAction(tr("&Snapshot"), this);
    newAct->setShortcut(tr("F12"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(snapshotFile()));

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

	reloadAct = new QAction(tr("&Reload model"), this);
    reloadAct->setShortcut(tr("Ctrl+R"));
    connect(reloadAct, SIGNAL(triggered()), this, SLOT(contextRefresh()));


    for (int i = 0; i < MaxRecentFiles; ++i){
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),
                 this, SLOT(openRecentFile()));
    }

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	//Viewer changing
	setExaminerViewerAct = new QAction(tr("&Examiner"), this);
	connect(setExaminerViewerAct, SIGNAL(triggered()), this, SLOT(setExaminerViewer()));
 
	setPlaneViewerAct = new QAction(tr("&Plane"), this);
	connect(setPlaneViewerAct, SIGNAL(triggered()), this, SLOT(setPlaneViewer()));
 
	setFlyViewerAct = new QAction(tr("&Fly"), this);
	connect(setFlyViewerAct, SIGNAL(triggered()), this, SLOT(setFlyViewer()));

	//Right click menu
	refreshAct = new QAction(tr("Refresh"), this);
	connect(refreshAct, SIGNAL(triggered()), this, SLOT(contextRefresh()));
	
	fsAct = new QAction(tr("Fullscreen"), this);
	connect(fsAct, SIGNAL(triggered()), this, SLOT(setFS()));

	wmAct = new QAction(tr("Windowed mode"), this);
	connect(wmAct, SIGNAL(triggered()), this, SLOT(setWM()));
}

/*!
 * \brief Creates the top menu.
 */
void 
MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(reloadAct);
    fileMenu->addAction(openAct);
    fileMenu->addSeparator();
	separatorAct = fileMenu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i)
        fileMenu->addAction(recentFileActs[i]);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);
	updateRecentFileActions();

    viewMenu = menuBar()->addMenu(tr("&View"));

    sfxMenu = menuBar()->addMenu(tr("&SpecialFX"));

    viewerMenu = menuBar()->addMenu(tr("&Viewer"));
	viewerMenu->addAction(setExaminerViewerAct);
	viewerMenu->addAction(setPlaneViewerAct);
	viewerMenu->addAction(setFlyViewerAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
}