#ifndef OSGVIEWWIDGET_H
#define OSGVIEWWIDGET_H

#include "qosgtrackball.h"
/* #include "qosgenvmodel.h"
#include "osgpick.h" */
#include "osgskybox.h"
/* #include "qosgkeyboardeventhandler.h"
#include "qosgselecteditemdraggercallback.h"
#include "qosgupdatecallbackcheckcollsion.h"
#include "qosgoctreebuilder.h" */
#include <QObject>
#include <QTimer>
#include <QMetaEnum>
#include <osgViewer/CompositeViewer>
#include <osgQt/GraphicsWindowQt>
#include <osg/Group>
#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osg/BlendEquation>
#include <osg/Switch>
#include <osgManipulator/Dragger>
#include <osgGA/TerrainManipulator>
/* #include <pcl/point_cloud.h>
#include <pcl/point_types.h> 

#include <eigen3/Eigen/Eigen>*/
#include <iostream>
class OSGViewWidget :  public QGLWidget,public osgViewer::CompositeViewer
{
    Q_OBJECT
public:


    OSGViewWidget(QWidget *parent = NULL, Qt::WindowFlags f = 0, osgViewer::ViewerBase::ThreadingModel threadingModel=osgViewer::CompositeViewer::SingleThreaded);
    void _SetViewport(int width, int height, double metersinunit);
//    osg::ref_ptr<osgGA::CameraManipulator> _GetCameraManipulator();
    /// add root node to scene
    void _repaintView();

    /// back to original location
    void _SetHome();


    void _setSkyBox(const std::string& skyName);

    void _ChangeViewToXZ();

    void _ChangeViewToXY();
	void _addBoxModel(double length, double width, double height, const std::string &boxID);
    void _ChangeViewToYZ();
	void _loadFileNodeToSceneRoot(const std::string& modelId, const std::string& nodeName);
	void _loadNodeToSceneRoot(osg::Node* node);
protected:
    void initializeGL();
signals:
	void sig_updateViewSize(int height_, int width_);
public slots:
    void paintGL();
    void resizeGL(int width, int height);
private:
    void _initOsgViewWidget();
    bool _HandleOSGKeyDown(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);
    bool _HandleOSGKeyUp(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);
    QWidget *_addViewWidget(osg::ref_ptr<osg::Camera> camera, osg::ref_ptr<osg::Camera> hudcamera);
    osg::Camera *_CreateCamera( int x, int y, int w, int h, double metersinunit);
//    osg::Image* _createImage(int width, int height, const osg::Vec3 &color );
    void _changeSkyBox(const std::string& bakName);
    /* osg::Matrix _getMatrixFromAffine3f(const Eigen::Affine3f &transfer); */
    void _createFrame();
    osg::Group *_CreateOSGXYZAxes(double len, double axisthickness);
    void _SetTextureCubeMap(const std::string& posx, const std::string& negx, const std::string& posy,
                                         const std::string& negy, const std::string& posz, const std::string& negz);

    osg::Camera *_CreateHUDCamera( int x, int y, int w, int h, double metersinunit);
    void _SetCameraTransform(const osg::Matrix& matCamera);

protected:
    QTimer m_timer;
private:
    osg::ref_ptr<osgViewer::View> m_osgview;
    osg::ref_ptr<osgViewer::View> m_osghudview;
    //osg::ref_ptr<QOSGTrackball> m_osgCameraManipulator;
	
	osg::ref_ptr<osgGA::TerrainManipulator> m_osgCameraManipulator;
    osg::ref_ptr<osgGA::GUIEventHandler> m_keyhandler;
    osg::ref_ptr<osg::Group> m_osgSceneRoot;
    /* QOSGEnvModel *m_osgEnvModel; */
    osg::ref_ptr<osg::MatrixTransform> m_osgWorldAxis; ///< the node that draws the rgb axes on the lower right corner
    osg::ref_ptr<osg::MatrixTransform> m_osgWorldCenterAxis; ///< this node that draw world center rgb axes
    osg::ref_ptr<osg::Switch> m_osgWorldCenterAxisSwitch;///< this node controll "m_osgWorldCenterAxis" show or not
    osg::ref_ptr<osg::Switch> m_envModelSwitch;
    /* osg::ref_ptr<OSGPickHandler> m_picker; */
    bool m_bIsSelectiveActive;
    osg::ref_ptr<osg::MatrixTransform> m_selectedItem;
    osg::ref_ptr<Skybox> m_osgSkybox;
    osg::ref_ptr<osg::Group> m_osgFigureRoot;
    osg::ref_ptr<osg::MatrixTransform> m_osgCameraHUD; ///< MatrixTransform node that gets displayed in the heads up display
    osg::ref_ptr<osg::MatrixTransform> m_draggerMatrix;
    osg::ref_ptr<osg::Group> m_osgSelectedNodeByDragger;
    std::vector<osg::ref_ptr<osgManipulator::Dragger> > m_draggers;
    osg::ref_ptr<osg::Group> m_osgDraggerRoot;
	int m_OSGViewHeight;
	int m_OSGViewWidth;
    /* QOSGUpdateCallbackCheckCollsion *m_updateCallBace; */
};

#endif // OSGVIEWWIDGET_H
