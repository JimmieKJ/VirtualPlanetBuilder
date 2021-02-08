#include "osgviewwidget.h"
/* #include "paramtrackballdragger.h"
#include "paramaxisdragger.h" */
#include <QDebug>
#include <QGridLayout>
#include <osg/Math>
#include <osg/Vec4>
#include <osgViewer/View>
#include <osgViewer/ViewerEventHandlers>
#include <osg/CullFace>
#include <osgGA/MultiTouchTrackballManipulator>
#include <osg/ShapeDrawable>
#include <osg/Image>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/TexGen>
#include <osg/Material>
#include <QApplication>
#include <osg/ComputeBoundsVisitor>
#include <osgUtil/Simplifier>
#include <osgUtil/Optimizer>
#define METERSINUNIT 0.01

OSGViewWidget::OSGViewWidget(QWidget *parent, Qt::WindowFlags f, ThreadingModel threadingModel)
     : QGLWidget(parent),
        m_osgview(new osgViewer::View),
        m_osghudview(new osgViewer::View),
        m_bIsSelectiveActive(true),
        m_osgFigureRoot(new osg::Group)
{
    setThreadingModel(threadingModel);
    setKeyEventSetsDone(0);
    _initOsgViewWidget();

}

void OSGViewWidget::_initOsgViewWidget()
{
    QWidget* viewWidget = _addViewWidget( osg::ref_ptr<osg::Camera>(_CreateCamera(0,0,100,100,METERSINUNIT)),osg::ref_ptr<osg::Camera>(_CreateHUDCamera(0,0,100,100, METERSINUNIT)));
    QGridLayout* grid = new QGridLayout;
    /* m_keyhandler = new QOSGKeyboardEventHandler(boost::bind(&OSGViewWidget::_HandleOSGKeyDown, this, _1, _2),boost::bind(&OSGViewWidget::_HandleOSGKeyUp, this, _1, _2)); */
    /* m_osgview->addEventHandler(m_keyhandler); */
    m_osgSceneRoot = new osg::Group();
    grid->addWidget(viewWidget);
    grid->setContentsMargins(0, 0, 0, 0);
    setLayout(grid);
	
    //m_osgEnvModel = new QOSGEnvModel(m_osgSceneRoot);
    m_osgSceneRoot->addChild(m_osgFigureRoot);
    /* m_picker = new OSGPickHandler(boost::bind(&OSGViewWidget::_HandleRayPick, this, _1, _2, _3), boost::bind(&OSGViewWidget::_pickDraggerFunc,this,_1,_2,_3,_4)); */
    /* m_osgview->addEventHandler(m_picker); */
	osgUtil::Optimizer optimizer;
	optimizer.optimize(m_osgSceneRoot.get());
    //hud
    m_osgWorldAxis = new osg::MatrixTransform();
    m_osgWorldAxis->addChild(_CreateOSGXYZAxes(32, 2));
    if( !!m_osgCameraHUD ) {
        osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource();
        osg::ref_ptr<osg::Light> light(new osg::Light());
        // each light must have a unique number
        light->setLightNum(0);
        // we set the light's position via a PositionAttitudeTransform object
        light->setPosition(osg::Vec4(0.0, 0.0, 0.0, 1.0));
        light->setDiffuse(osg::Vec4(0, 0, 0, 1.0));
        light->setSpecular(osg::Vec4(0, 0, 0, 1.0));
        light->setAmbient( osg::Vec4(1, 1, 1, 1.0));
        lightSource->setLight(light.get());
        m_osgCameraHUD->addChild(lightSource.get());
        osg::Matrix m = m_osgCameraManipulator->getInverseMatrix();
        m.setTrans(this->width()/2 - 40, -this->height()/2 + 40, -50);
        m_osgWorldAxis->setMatrix(m);
        lightSource->addChild(m_osgWorldAxis.get());
    }
   /*  m_collsionCheck = new QOSGCollsionCheck(m_osgview,m_osgEnvModel);
    m_collsionWorld = m_collsionCheck->_initCollision(); */
    connect( &m_timer, SIGNAL(timeout()), this, SLOT(updateGL()) );
    m_timer.start( 10 );
    //_SetHome();
	
}

void OSGViewWidget::_addBoxModel(double length, double width, double height, const std::string &boxID)
{
	osg::ref_ptr<osg::MatrixTransform> boxTrans = new osg::MatrixTransform;
	osg::ref_ptr<osg::Group> boxData = new osg::Group;
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	osg::TessellationHints* hints = new osg::TessellationHints;
	hints->setDetailRatio(0.5f);
	geode->addDrawable(new osg::ShapeDrawable(new osg::Box(osg::Vec3(0, 0, 0), length, width, height), hints));

	boxData->addChild(geode);
	boxTrans->addChild(boxData);
	boxTrans->setName(boxID);
	
	m_osgSceneRoot->addChild(boxTrans);
}

void OSGViewWidget::_ChangeViewToXY()
{
    osg::Vec3d center = m_osgSceneRoot->getBound().center();
    osg::Matrix matCamera;
    matCamera.identity();
    matCamera.makeRotate( 0,osg::Vec3(1,0,0));
    matCamera.setTrans(center.x(), center.y(), center.z()+m_osgCameraManipulator->getDistance());
    _SetCameraTransform(matCamera);
}

void OSGViewWidget::_ChangeViewToXZ()
{
    osg::Vec3d center = m_osgSceneRoot->getBound().center();
    osg::Matrix matCamera;
    matCamera.identity();
    matCamera.makeRotate((osg::PI/2.0),osg::Vec3(1,0,0));
    matCamera.setTrans(center.x(), center.y()-m_osgCameraManipulator->getDistance(),center.z());
    _SetCameraTransform(matCamera);
}

void OSGViewWidget::_SetCameraTransform(const osg::Matrix& matCamera)
{
    // has to come after setting distance because internally orbit manipulator uses the distance to deduct view center
    m_osgCameraManipulator->setByMatrix((matCamera));
}

void OSGViewWidget::_loadFileNodeToSceneRoot( const std::string& modelId, const std::string& nodeName)
{
	osg::ref_ptr<osg::MatrixTransform> boxTrans = new osg::MatrixTransform;
	osg::ref_ptr<osg::Node> dataNode = osgDB::readNodeFile(nodeName);
	boxTrans->addChild(dataNode);
	boxTrans->setName(modelId);
	m_osgSceneRoot->addChild(boxTrans);
	//m_osgview->setSceneData(boxTrans);
}

void OSGViewWidget::_loadNodeToSceneRoot(osg::Node* node)
{
	//osg::ref_ptr<osg::MatrixTransform> boxTrans = new osg::MatrixTransform;
	//osg::ref_ptr<osg::Node> dataNode = osgDB::readNodeFile(nodeName);
	//boxTrans->addChild(dataNode);
	//boxTrans->setName(modelId);
	m_osgSceneRoot->addChild(node);
	//m_osgview->setSceneData(boxTrans);
}

void OSGViewWidget::_ChangeViewToYZ()
{
    osg::Vec3d center = m_osgSceneRoot->getBound().center();
    osg::Matrix matCamera;
    matCamera.identity();
    matCamera.makeRotate((osg::Quat((osg::PI /2.0),osg::Vec3(0,0,1)) * osg::Quat((osg::PI /2.0),osg::Vec3(1,0,0))));
    matCamera.setTrans(center.x(),center.y(),center.z());
    _SetCameraTransform(matCamera);
}

osg::Camera* OSGViewWidget::_CreateHUDCamera( int x, int y, int w, int h, double metersinunit)
{
    osg::ref_ptr<osg::Camera> camera(new osg::Camera());
    camera->setProjectionMatrix(osg::Matrix::ortho(-1,1,-1,1,1,10));

    // draw subgraph after main camera view.
    camera->setRenderOrder(osg::Camera::POST_RENDER);

    // only clear the depth buffer
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);

    // we don't want the camera to grab event focus from the viewers main camera(s).
    camera->setAllowEventFocus(false);

    // set the view matrix
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());
    camera->setCullingMode(camera->getCullingMode() & ~osg::CullSettings::SMALL_FEATURE_CULLING); // need this for allowing small points with zero bunding voluem to be displayed correctly
    return camera.release();
}




void OSGViewWidget::_createFrame()
{
//    m_osgWorldAxis = new osg::MatrixTransform();
//    m_osgWorldAxis->addChild(_CreateOSGXYZAxes(32.0, 2.0));
    m_osgWorldCenterAxisSwitch = new osg::Switch();
    m_osgWorldCenterAxis = new osg::MatrixTransform();
    m_osgWorldCenterAxis->addChild(osg::ref_ptr<osg::Group>(_CreateOSGXYZAxes(1.0, METERSINUNIT)));
    m_osgWorldCenterAxisSwitch->addChild(m_osgWorldCenterAxis);
    m_osgSceneRoot->addChild(m_osgWorldCenterAxisSwitch);
}

osg::Group* OSGViewWidget::_CreateOSGXYZAxes(double len, double axisthickness)
{
    osg::Vec4f colors[] = {
        osg::Vec4f(0,0,1,1),
        osg::Vec4f(0,1,0,1),
        osg::Vec4f(1,0,0,1)
    };
    osg::Quat rotations[] = {
        osg::Quat(0, osg::Vec3f(0,0,1)),
        osg::Quat(-(osg::PI /2.0), osg::Vec3f(1,0,0)),
        osg::Quat(osg::PI /2.0, osg::Vec3f(0,1,0))
    };

    osg::ref_ptr<osg::Group> proot = new osg::Group();

    // add 3 cylinder+cone axes
    for(int i = 0; i < 3; ++i) {
        osg::ref_ptr<osg::MatrixTransform> psep = new osg::MatrixTransform();
        //psep->setMatrix(osg::Matrix::translate(-16.0f,-16.0f,-16.0f));

        // set a diffuse color
        osg::ref_ptr<osg::StateSet> state = psep->getOrCreateStateSet();
        osg::ref_ptr<osg::Material> mat = new osg::Material;
        mat->setDiffuse(osg::Material::FRONT, colors[i]);
        mat->setAmbient(osg::Material::FRONT, colors[i]);
        state->setAttribute( mat );
//        osg::CullFace* cf = new osg::CullFace( osg::CullFace::BACK );
//        state->setAttribute( cf );
        osg::Matrix matrix;
        osg::ref_ptr<osg::MatrixTransform> protation = new osg::MatrixTransform();
        matrix.makeRotate(rotations[i]);
        protation->setMatrix(matrix);

        matrix.makeIdentity();
        osg::ref_ptr<osg::MatrixTransform> pcyltrans = new osg::MatrixTransform();
        matrix.setTrans(osg::Vec3f(0,0,0.5*len));
        pcyltrans->setMatrix(matrix);

        // make SoCylinder point towards z, not y
        osg::ref_ptr<osg::Cylinder> cy = new osg::Cylinder();
        cy->setRadius(axisthickness);
        cy->setHeight(len);
        osg::ref_ptr<osg::Geode> gcyl = new osg::Geode;
        osg::ref_ptr<osg::ShapeDrawable> sdcyl = new osg::ShapeDrawable(cy);
        sdcyl->setColor(colors[i]);
        gcyl->addDrawable(sdcyl.get());

        osg::ref_ptr<osg::Cone> cone = new osg::Cone();
        cone->setRadius(axisthickness*2);
        cone->setHeight(len*0.25);

        osg::ref_ptr<osg::Geode> gcone = new osg::Geode;
        osg::ref_ptr<osg::ShapeDrawable> sdcone = new osg::ShapeDrawable(cone);
        gcone->addDrawable(sdcone.get());
        sdcone->setColor(colors[i]);

        matrix.makeIdentity();
        osg::ref_ptr<osg::MatrixTransform> pconetrans = new osg::MatrixTransform();
        matrix.setTrans(osg::Vec3f(0,0,len));
        pconetrans->setMatrix(matrix);

        psep->addChild(protation);
        protation->addChild(pcyltrans);
        pcyltrans->addChild(gcyl.get());
        protation->addChild(pconetrans);
        pconetrans->addChild(gcone.get());
        proot->addChild(psep);
    }

    return proot.release();
}

void OSGViewWidget::_SetHome()
{
    //osg::BoundingSphere bs = m_osgSceneRoot->getBound();
    //m_osgview->getCameraManipulator()->setHomePosition(osg::Vec3d(0.6*bs.radius(),0,0.6*bs.radius()),osg::Vec3d(0,0,0),osg::Vec3d(0.0,0.0,1.0));
//    m_osgview->getCameraManipulator()->setHomePosition(osg::Vec3d(0.6*bs.radius(),-0.8*bs.radius(),-1*bs.radius()),bs.center(),osg::Vec3d(0,0,1));
    //m_osgview->home();
	m_osgCameraManipulator->computeHomePosition(m_osgview->getCamera());
	m_osgview->home();
//    bs = m_osgSceneRoot->getBound();
//    m_osgview->getCameraManipulator()->setHomePosition(osg::Vec3d(0.3*bs.radius(),1,-1*bs.radius()),bs.center(),osg::Vec3d(0.0,0.0,1.0));
//    m_osgview->home();
}

bool OSGViewWidget::_HandleOSGKeyDown(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{
    int key = ea.getKey();
//    if( !!_onKeyDown ) {
//        if( _onKeyDown(key) ) {
//            return true;
//        }
//    }

    /*if( key == 'f' || key == 'F') {
        m_osgCameraManipulator->_SetSeekMode(!m_osgCameraManipulator->_InSeekMode());
    }*/
	/*else if(key == osgGA::GUIEventAdapter::KEY_Alt_L){
        emit changeKeyMod(-3);
    }*//*else if(key == osgGA::GUIEventAdapter::KEY_Delete){
        emit changeToRotateMod(true);
        _deleteKinBodyFromEnv();
    }*/
    return false;
}

bool OSGViewWidget::_HandleOSGKeyUp(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    int key = ea.getKey();

    if( key == osgGA::GUIEventAdapter::KEY_Alt_L ) {
//        emit changeKeyMod(-2);
//        _ClearDragger();
//        m_draggerName.clear();
    }
    return true;
}

void OSGViewWidget::_repaintView()
{
    osg::ref_ptr<osg::Group> rootscene(new osg::Group());
    //  Normalize object normals
    rootscene->getOrCreateStateSet()->setMode(GL_NORMALIZE,osg::StateAttribute::ON);
    rootscene->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
    rootscene->addChild(m_osgSceneRoot);
    /* m_updateCallBace = new QOSGUpdateCallbackCheckCollsion(this->width(),this->height(),METERSINUNIT,m_collsionWorld,m_collsionCheck,boost::bind(&OSGViewWidget::_SetViewport, this, _1, _2, _3));
    rootscene->addUpdateCallback(m_updateCallBace); */
    m_osgview->setSceneData(rootscene);

}


void OSGViewWidget::_SetViewport(int width, int height, double metersinunit)
{
    m_osgview->getCamera()->setViewport(0,0,width,height);
    m_osghudview->getCamera()->setViewport(0,0,width,height);
    m_osghudview->getCamera()->setProjectionMatrix(osg::Matrix::ortho(-width/2, width/2, -height/2, height/2, METERSINUNIT/metersinunit, 100.0/metersinunit));

    osg::Matrix m = m_osgCameraManipulator->getInverseMatrix();
    m.setTrans(width/2 - 40, -height/2 + 40, -50);
    m_osgWorldAxis->setMatrix(m);

//    double textheight = (10.0/480.0)*height;
//    m_osgHudText->setPosition(osg::Vec3(-width/2+10, height/2-textheight, -50));
//    m_osgHudText->setCharacterSize(textheight);
}



void OSGViewWidget::_setSkyBox(const std::string &skyName)
{
    m_osgSkybox = new Skybox;
    _changeSkyBox(skyName);
    m_osgFigureRoot->addChild(m_osgSkybox);
}

void OSGViewWidget::_changeSkyBox(const std::string& bakName)
{

    QString appDir = QApplication::applicationDirPath();
    appDir+="/Skybox/%1/right.tga";
    std::string strBakPathNameR,strBakPathNameL,strBakPathNameD,strBakPathNameT,strBakPathNameB,strBakPathNameF;
    //strBakPathNameR = str(boost::format(appDir.toStdString().c_str())%bakName);
	strBakPathNameR = appDir.arg(QString(bakName.c_str())).toLocal8Bit().toStdString();
    appDir = QApplication::applicationDirPath();
    appDir+="/Skybox/%1/left.tga";
    strBakPathNameL = appDir.arg(QString(bakName.c_str())).toLocal8Bit().toStdString();
    appDir = QApplication::applicationDirPath();
    appDir+="/Skybox/%1/down.tga";
    strBakPathNameD = appDir.arg(QString(bakName.c_str())).toLocal8Bit().toStdString();
    appDir = QApplication::applicationDirPath();
    appDir+="/Skybox/%1/top.tga";
    strBakPathNameT = appDir.arg(QString(bakName.c_str())).toLocal8Bit().toStdString();
    appDir = QApplication::applicationDirPath();
    appDir+="/Skybox/%1/back.tga";
    strBakPathNameB = appDir.arg(QString(bakName.c_str())).toLocal8Bit().toStdString();
    appDir = QApplication::applicationDirPath();
    appDir+="/Skybox/%1/front.tga";
    strBakPathNameF = appDir.arg(QString(bakName.c_str())).toLocal8Bit().toStdString();

    _SetTextureCubeMap(strBakPathNameR,strBakPathNameL,strBakPathNameD,strBakPathNameT,strBakPathNameB,strBakPathNameF);
}

void OSGViewWidget::_SetTextureCubeMap(const std::string& posx, const std::string& negx, const std::string& posy,
                                     const std::string& negy, const std::string& posz, const std::string& negz)
{
    m_osgSkybox->setTextureCubeMap(posx, negx, posy, negy, posz, negz);
}

void OSGViewWidget::initializeGL()
{

}

void OSGViewWidget::paintGL()
{
	osg::Matrix m = m_osgCameraManipulator->getInverseMatrix();
	m.setTrans(m_OSGViewWidth / 2 - 40, -m_OSGViewHeight / 2 + 40, -50);
	m_osgWorldAxis->setMatrix(m);
    frame();
//    _SetViewport(this->width(),this->height(),METERSINUNIT);
//    bool lastColState = false;
//    m_collsionWorld->performDiscreteCollisionDetection();
//    m_collsionCheck->_excuteCollision(lastColState,m_collsionWorld);
}

void OSGViewWidget::resizeGL(int width, int height)
{
	_SetViewport(width, height, METERSINUNIT);
	emit sig_updateViewSize(height, width);
	m_OSGViewHeight = height;
	m_OSGViewWidth = width;
}



osg::Camera* OSGViewWidget::_CreateCamera( int x, int y, int w, int h, double metersinunit)
{
    osg::ref_ptr<osg::DisplaySettings> ds = osg::DisplaySettings::instance();
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->windowName = "";
    traits->windowDecoration = false;
    traits->x = x;
    traits->y = y;
    traits->width = w;
    traits->height = h;
    traits->doubleBuffer = true;
    traits->alpha = ds->getMinimumNumAlphaBits();
    traits->stencil = ds->getMinimumNumStencilBits();
    traits->sampleBuffers = 2/*ds->getMultiSamples()*/;//_numMultiSamples osg default was 0
    traits->samples = 4/*ds->getNumMultiSamples()*/;//_numMultiSamples osg default was 0
    osg::ref_ptr<osg::Camera> camera(new osg::Camera());
    camera->setGraphicsContext(new osgQt::GraphicsWindowQt(traits.get()));
    camera->setClearColor(osg::Vec4(0.95, 0.95, 0.95, 1.0));
    camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
    double fnear = METERSINUNIT/metersinunit;
    camera->setProjectionMatrixAsPerspective(45.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), fnear, 100.0/metersinunit);
    camera->setCullingMode(camera->getCullingMode() & ~osg::CullSettings::SMALL_FEATURE_CULLING); // need this for allowing small points with zero bunding voluem to be displayed correctly
    return camera.release();
}


QWidget *OSGViewWidget::_addViewWidget(osg::ref_ptr<osg::Camera> camera,osg::ref_ptr<osg::Camera> hudcamera)
{
    m_osgview->setCamera(camera.get());
    m_osghudview->setCamera( hudcamera.get() );
    m_osgview->addEventHandler(new osgViewer::StatsHandler);
    osgViewer::Viewer::Windows windows;
    this->getWindows(windows);
    m_osgCameraManipulator = new osgGA::TerrainManipulator();
    m_osgCameraManipulator->setWheelZoomFactor(0.2);
    m_osgview->setCameraManipulator(m_osgCameraManipulator.get());
    osgQt::GraphicsWindowQt* gw = dynamic_cast<osgQt::GraphicsWindowQt*>( camera->getGraphicsContext() );
    addView(m_osgview);
    addView( m_osghudview.get() );
    m_osgCameraHUD = new osg::MatrixTransform();
    hudcamera->addChild( m_osgCameraHUD.get() );
    m_osgCameraHUD->setMatrix(osg::Matrix::identity());
    hudcamera->setGraphicsContext(gw);
	int width = gw->getTraits()->width;
	int height = gw->getTraits()->height;
    hudcamera->setViewport(0,0, width, height);
    return gw ? gw->getGLWidget() : NULL;
}

