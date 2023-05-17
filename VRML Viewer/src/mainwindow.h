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
 * \brief Doc in mainwindow.cpp
 * 
 * This file sets up version number and includes those libs that are needed.
 * Version number is found in the defined constant VRML_VER.
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#ifndef VRML_VER
#define VRML_VER "1.0"

#include <iostream>
#include <string>

#include <QMainWindow>
#include <QtGui>
#include <QList>

#include <Inventor/Qt/SoQt.h>
#include <Inventor/Qt/viewers/SoQtExaminerViewer.h>
#include <Inventor/Qt/viewers/SoQtPlaneViewer.h>
#include <Inventor/Qt/viewers/SoQtFlyViewer.h>
#include <Inventor/SoOffscreenRenderer.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoMaterialBinding.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoFont.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoText3.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>

using namespace std;

class QAction;
class QActionGroup;
class QLabel;
class QMenu;
class QTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void snapshotFile();
    void open();
	void openRecentFile();
    void about();

	void setExaminerViewer();
	void setPlaneViewer();
	void setFlyViewer();

	void contextRefresh();
	void setFS();
	void setWM();

private:
    void createActions();
    void createMenus();
	void createSoQt();
	void loadFile(const QString &fileName);
	void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();

    QString strippedName(const QString &fullFileName);
	QString curFile;

	QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *sfxMenu;
    QMenu *helpMenu;
	QMenu *viewerMenu;
	 
    QActionGroup *alignmentGroup;
    QAction *newAct;
    QAction *openAct;
    QAction *reloadAct;
    QAction *exitAct;
    QAction *aboutAct;
	QAction *setExaminerViewerAct;
	QAction *setPlaneViewerAct;
	QAction *setFlyViewerAct;
	QAction *separatorAct;


	QAction *refreshAct;
	QAction *fsAct;
	QAction *wmAct;

    enum { MaxRecentFiles = 5 };
    QAction *recentFileActs[MaxRecentFiles];

	SoQtFullViewer *viewer;
	QString fileInViewer;
	QBoxLayout *vbox;
};

#endif
#endif