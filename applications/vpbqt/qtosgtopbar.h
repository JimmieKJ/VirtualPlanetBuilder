#ifndef QTOSGTOPBAR_H
#define QTOSGTOPBAR_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QIcon>
#include <QAction>
#include <QToolBar>
#include <osg/CoordinateSystemNode>
class QtOSGTopBar: public QWidget
{
    Q_OBJECT
public:
    QtOSGTopBar(int widgetWidth, QWidget *parent = 0);
    ~QtOSGTopBar();
	void _initFileAction();
	void _initComputeAction();
public Q_SLOTS:
	void slot_addDSM();
	void slot_addTexture();
	void slot_addShpFile();
	void slot_addModel();
	void slot_computeDEMAndHeightOfShp();
	void slot_convertDSMWithModelToOSGB();
	void slot_splitOSGBWithShpFile();
Q_SIGNALS:
	void sig_addNodeToSceneByFilePath(std::string nodePathStr);
	void sig_addNodeToScene(osg::Node* node);
private:
	bool _copyFileToOtherName(const std::string& sourceFileName, const std::string& dstFileName);
	void _createToolbtnOnWidget(QWidget *wid, int x, int y, int w, int h, QString iconStr, QString objName, QAction *act);
	void _addLabelOnWidget(QWidget *wid, int x, int y, int w, int h, QString textname);
	bool areCoordinateSystemEquivalent(const osg::CoordinateSystemNode* lhs, const osg::CoordinateSystemNode* rhs);
	std::string removeSpcSymbol(std::string strSource);
private:
	int m_widgetWidth;
	QToolBar* m_tabtoolbar;
	QAction* m_addDSMAction;
	QAction* m_addTextureAction;
	QAction* m_addShpFileAction;
	QAction* m_addModelFileAction;
	QAction* m_computeDEMAndHeightOfShpAction;
	QAction* m_splitOSGBWithShpFileAction;
	QAction* m_convertDSMWithModelToOSGBAction;
};

#endif // QTOSGTOPBAR_H
