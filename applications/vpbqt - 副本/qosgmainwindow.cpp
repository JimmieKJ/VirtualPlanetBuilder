#include "qosgmainwindow.h"
#include "qtosgtopbar.h"
#include <osgDB/FileNameUtils>
#include <QDesktopWidget>
#include <QLayout>

QOSGMainWindow::QOSGMainWindow(QWidget *parent) : QMainWindow(parent),m_leftSideBar(NULL)
{
	_initViewWidget();
	_initLeftSideWidget();
	_initToolTabWidget();
	//_createConnection();
	showMaximized();
	_createConnection();
}

void QOSGMainWindow::_initViewWidget()
{
#if QT_VERSION >= 0x050000
	// Qt5 is currently crashing and reporting "Cannot make QOpenGLContext current in a different thread" when the viewer is run multi-threaded, this is regression from Qt4
	osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::SingleThreaded;
#else
	osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::CullDrawThreadPerContext;
#endif
	m_viewWidget = new OSGViewWidget(this, Qt::Widget, threadingModel);
	
	setCentralWidget(m_viewWidget);
	m_viewWidget->_repaintView();
	//m_viewWidget->_loadFileNodeToSceneRoot("tempDSM", "G:/data/building/HuaiLai/HuaiLai-dsm.osgb");
	//m_viewWidget->_loadFileNodeToSceneRoot("tempObj", "G:/data/building/HuaiLai/gis/temp.shp");
	m_viewWidget->setGeometry(0, 0, 500, 400);
	m_viewWidget->_SetHome();
}

void QOSGMainWindow::_initLeftSideWidget()
{
	m_showhouseViewAct = new QAction(QIcon("images/startView_.png"), tr("start View"), this);
	connect(m_showhouseViewAct, SIGNAL(triggered(bool)), this, SLOT(slot_ShowHouseView(bool)));
	m_showxyViewAct = new QAction(tr("up View"), this);
	m_showxyViewAct->setIcon(QIcon("images/upView_.png"));
	connect(m_showxyViewAct, SIGNAL(triggered(bool)), this, SLOT(slot_ShowXyView(bool)));
	m_showxzViewAct = new QAction(tr("left View"), this);
	m_showxzViewAct->setIcon(QIcon("images/leftView_.png"));
	connect(m_showxzViewAct, SIGNAL(triggered(bool)), this, SLOT(slot_ShowXzView(bool)));
	m_showyzViewAct = new QAction(tr("Front view"), this);
	m_showyzViewAct->setIcon(QIcon("images/faceView_.png"));
	connect(m_showyzViewAct, SIGNAL(triggered(bool)), this, SLOT(slot_ShowYzView(bool)));
	_createLeftSideBar();
}

void QOSGMainWindow::_initToolTabWidget()
{
	m_toolTabDockWidget = new QDockWidget(this);
	QTabWidget *tabWidget = new QTabWidget(m_toolTabDockWidget);
	tabWidget->setObjectName("tabWidget_tab");
	QFont font;
	font.setFamily("Tahoma"); // Tahoma
	font.setPixelSize(14);
	tabWidget->setFont(font);

	QtOSGTopBar* openFileWidget = new QtOSGTopBar(m_OSGViewWidth);
	openFileWidget->setObjectName("openFileWidget");
	openFileWidget->_initFileAction();
	tabWidget->addTab(openFileWidget, "File");

	QtOSGTopBar* computeToolWidget = new QtOSGTopBar(m_OSGViewWidth);
	computeToolWidget->setObjectName("computeToolWidget");
	computeToolWidget->_initComputeAction();
	tabWidget->addTab(computeToolWidget, "Compute");
	connect(computeToolWidget, &QtOSGTopBar::sig_addNodeToScene, this, &QOSGMainWindow::slot_addNodeToScene);
	connect(computeToolWidget, &QtOSGTopBar::sig_addNodeToSceneByFilePath, this, &QOSGMainWindow::slot_addNodeToSceneByFilePath);

	QGridLayout *layout = new QGridLayout();
	layout->setObjectName("tabWidget_layout");
	layout->addWidget(tabWidget);
	int dock_height = 105;
	m_toolTabDockWidget->setObjectName("toolTabDockWidget");
	m_toolTabDockWidget->setMinimumHeight(dock_height);
	m_toolTabDockWidget->setMaximumHeight(dock_height);
	m_toolTabDockWidget->setMinimumWidth(m_OSGViewWidth);
	QWidget *titleWidget = new QWidget(m_toolTabDockWidget);
	m_toolTabDockWidget->setTitleBarWidget(titleWidget);
	m_toolTabDockWidget->titleBarWidget()->setVisible(false);
	m_toolTabDockWidget->setWidget(tabWidget);
	addDockWidget(Qt::TopDockWidgetArea, m_toolTabDockWidget);
}

void QOSGMainWindow::_createConnection()
{
	connect(m_viewWidget, &OSGViewWidget::sig_updateViewSize, this, &QOSGMainWindow::slot_updateViewSize);
}

void QOSGMainWindow::slot_addNodeToScene(osg::Node* node)
{
	m_viewWidget->_loadNodeToSceneRoot(node);
	m_viewWidget->_SetHome();
}

void QOSGMainWindow::slot_updateViewSize(int height_, int width_)
{
	m_OSGViewHeight = height_;
	m_OSGViewWidth = width_;
	_createLeftSideBar();
}

void QOSGMainWindow::slot_ShowHouseView(bool flag)
{
    m_viewWidget->_SetHome();
}

void QOSGMainWindow::slot_ShowXyView(bool flag)
{
    m_viewWidget->_ChangeViewToXY();
//    m_showxzViewAct->setChecked(false);
//    m_showyzViewAct->setChecked(false);
}

void QOSGMainWindow::slot_ShowXzView(bool flag)
{
    m_viewWidget->_ChangeViewToXZ();
//    m_showxyViewAct->setChecked(false);
//    m_showyzViewAct->setChecked(false);
}

void QOSGMainWindow::slot_ShowYzView(bool flag)
{
    m_viewWidget->_ChangeViewToYZ();
//    m_showxyViewAct->setChecked(false);
//    m_showxzViewAct->setChecked(false);
}

void QOSGMainWindow::slot_addNodeToSceneByFilePath(std::string nodePathStr)
{
	std::string modelID = osgDB::getSimpleFileName(nodePathStr);
	m_viewWidget->_loadFileNodeToSceneRoot(modelID, nodePathStr);
	m_viewWidget->_SetHome();
}

void QOSGMainWindow::_getScreenInfo()
{
    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->availableGeometry();
    m_screenWidth = screenRect.width();
    m_screenHeight = screenRect.height();
}

void QOSGMainWindow::_addViewWidget(int x,int y,int w,int h)
{
    #if QT_VERSION >= 0x050000
        // Qt5 is currently crashing and reporting "Cannot make QOpenGLContext current in a different thread" when the viewer is run multi-threaded, this is regression from Qt4
        osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::SingleThreaded;
    #else
        osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::CullDrawThreadPerContext;
    #endif
    OSGViewWidget* widget = new OSGViewWidget(this, Qt::Widget, threadingModel);
    widget->_repaintView();
    //widget->_addGridFloor(30,2,2.0,"grid",Eigen::Affine3f::Identity());
    widget->setGeometry(x,y,w,h);
    //widget->_addCylinder(20,20,"cylinderModel",Eigen::Affine3f::Identity(),255,0,0);
    layout()->addWidget(widget);
}

void QOSGMainWindow::_createLeftSideBar()
{
    if(!!m_leftSideBar)
    {
        QObjectList objS = m_leftSideBar->children();
        for(int i=0;i<objS.length();++i){
            delete objS[i];
        }
        delete m_leftSideBar;
    }
    m_leftSideBar = new QtOSGLeftSideBar(m_OSGViewHeight,this);
    int x_l = m_viewWidget->geometry().x();
    int y_l = m_viewWidget->geometry().y();
    m_leftSideBar->setGeometry(x_l,y_l,30, m_OSGViewHeight);
    m_leftSideBar->addAction(m_showhouseViewAct);
    m_leftSideBar->addAction(m_showxyViewAct);
    m_leftSideBar->addAction(m_showxzViewAct);
    m_leftSideBar->addAction(m_showyzViewAct);
    m_leftSideBar->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);
    m_leftSideBar->show();
}

QPushButton * QOSGMainWindow::_addButtonToWindow(QWidget* wid, int x, int y, int w, int h, QString iconStr, QString objName, QString textname)
{
    QPushButton *tbtn = new QPushButton(wid);
    tbtn->setGeometry(x,y,w,h);
    tbtn->setIcon(QIcon(iconStr));
    tbtn->setIconSize(QSize(w,h));
    tbtn->setObjectName(objName);
    tbtn->setText(textname);
    return tbtn;
}
