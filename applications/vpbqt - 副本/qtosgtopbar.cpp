#include "qtosgtopbar.h"
#include "ShapeFileProcessor.h"

#include <iostream>
#include <QToolButton>
#include <QLabel>
#include <vpb/Commandline>
#include <vpb/TaskManager>
#include <vpb/System>
#include <vpb/FileUtils>
#include <vpb/DatabaseBuilder>
#include <vpb/Version>
#include <ogr_spatialref.h>
#include <signal.h>
#include <vpb/System>
#include <osgDB/ReadFile>
#include <Windows.h>
#include <QFileInfo>
#include <QDir>

QtOSGTopBar::QtOSGTopBar(int widgetWidth,QWidget *parent):
    QWidget(parent),
    m_widgetWidth(widgetWidth),
	m_tabtoolbar(NULL)
{

}



QtOSGTopBar::~QtOSGTopBar()
{

}

void QtOSGTopBar::_createToolbtnOnWidget(QWidget *wid, int x, int y, int w, int h, QString iconStr, QString objName, QAction *act)
{
	if (!m_tabtoolbar) {
		m_tabtoolbar = new QToolBar(this);
	}
	QToolButton *tbtn = new QToolButton(wid);
	tbtn->setGeometry(x, y, w, h);
	tbtn->setMinimumHeight(h);
	tbtn->setMinimumWidth(w);
	tbtn->setObjectName(objName);
	tbtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
	tbtn->setDefaultAction(act);

	tbtn->setAutoRepeat(false);
	tbtn->setIconSize(QSize(w, h));
	tbtn->setToolTip("");
	connect(tbtn, SIGNAL(released()), this, SLOT(slot_hidetooltip()));
	tbtn->setStyleSheet("QToolButton{background:url(" + iconStr + ") center no-repeat;text-align:left;} "
		"QToolButton::hover{background-color:#555555;}"
		"QToolButton::checked{background-color:#666666;}");
	m_tabtoolbar->addWidget(tbtn);
	//return tbtn;
}

void QtOSGTopBar::_addLabelOnWidget(QWidget *wid, int x, int y, int w, int h,QString textname)
{
	QLabel *tlabel = new QLabel(wid);
	tlabel->setGeometry(x, y, w, h);
	tlabel->setObjectName(textname);
	tlabel->setText(textname);
	QFont font1;
	font1.setFamily("Tahoma"); // Tahoma
	font1.setPixelSize(10);
	tlabel->setFont(font1);
	//return tlabel;
}

void QtOSGTopBar::slot_addDSM()
{

}

void QtOSGTopBar::slot_addTexture()
{

}

void QtOSGTopBar::slot_addShpFile()
{

}

void QtOSGTopBar::slot_addModel()
{

}

std::string QtOSGTopBar::removeSpcSymbol(std::string strSource)//É¾³ý·ûºÅ
{

	const char *str = strSource.c_str();
	char buf[MAX_PATH] = { 0 };
	int j = 0;

	for (int i = 0; i < strlen(str); i++)
	{
		if (str[i] > 0 && str[i] < 0x7F)
		{
			if (str[i] >= '0' && str[i] <= '9')
			{
				buf[j] = str[i]; j++;
			}

			if (str[i] >= 'A' && str[i] <= 'Z')
			{
				buf[j] = str[i]; j++;
			}

			if (str[i] >= 'a' && str[i] <= 'z')
			{
				buf[j] = str[i]; j++;
			}
			continue;
		}

		if ((unsigned char)str[i] >= 0xB0 && (unsigned char)str[i] <= 0xF7)
		{
			if ((unsigned char)str[i + 1] >= 0xA0 && (unsigned char)str[i + 1] <= 0xFF)
			{
				buf[j] = str[i]; j++; i++;
				buf[j] = str[i]; j++;
				continue;
			}
		}
		i++;
	}
	return buf;
}

bool QtOSGTopBar::areCoordinateSystemEquivalent(const osg::CoordinateSystemNode* lhs, const osg::CoordinateSystemNode* rhs)
{
	// if ptr's equal the return true
	if (lhs == rhs) return true;

	// if one CS is NULL then true false
	if (!lhs || !rhs)
	{
		//log(osg::INFO, "areCoordinateSystemEquivalent lhs=%s  rhs=%s return true", lhs, rhs);
		return false;
	}

	//log(osg::INFO, "areCoordinateSystemEquivalent lhs=%s rhs=%s", lhs->getCoordinateSystem().c_str(), rhs->getCoordinateSystem().c_str());

	// use compare on ProjectionRef strings.
	if (lhs->getCoordinateSystem() == rhs->getCoordinateSystem()) return true;

	// set up LHS SpatialReference
	char* projection_string = strdup(lhs->getCoordinateSystem().c_str());
	char* importString = projection_string;
	OGRSpatialReference lhsSR;
	lhsSR.importFromWkt(&importString);

	free(projection_string);

	// set up RHS SpatialReference
	projection_string = strdup(rhs->getCoordinateSystem().c_str());
	importString = projection_string;
	OGRSpatialReference rhsSR;
	rhsSR.importFromWkt(&importString);

	free(projection_string);

	int result = lhsSR.IsSame(&rhsSR);
	//result = 1;
	if (result == 0)
	{
		if (rhsSR.IsProjected() && lhsSR.IsProjected())
		{
			std::string rhsAttrVal = rhsSR.GetAttrValue("PROJCS");
			std::string lhsAttrVal = lhsSR.GetAttrValue("PROJCS");
			std::string tempRHSVal = removeSpcSymbol(rhsAttrVal);
			std::string tempLHSVal = removeSpcSymbol(lhsAttrVal);
			if (tempLHSVal == tempRHSVal)
			{
				result = 1;
			}
		}
		else if ((!rhsSR.IsGeocentric()) && (!lhsSR.IsProjected()))
		{
			std::string rhsAttrVal = rhsSR.GetAttrValue("GEOGCS");
			std::string lhsAttrVal = lhsSR.GetAttrValue("GEOGCS");
			std::string tempRHSVal = removeSpcSymbol(rhsAttrVal);
			std::string tempLHSVal = removeSpcSymbol(lhsAttrVal);
			if (tempLHSVal == tempRHSVal)
			{
				result = 1;
			}
		}

	}
#if 0
	int result2 = lhsSR.IsSameGeogCS(&rhsSR);

	log(osg::NOTICE) << "areCoordinateSystemEquivalent " << std::endl
		<< "LHS = " << lhs->getCoordinateSystem() << std::endl
		<< "RHS = " << rhs->getCoordinateSystem() << std::endl
		<< "result = " << result << "  result2 = " << result2);
#endif
		return result ? true : false;
}

bool QtOSGTopBar::_copyFileToOtherName(const std::string& sourceFileName,const std::string& dstFileName)
{
	bool bsuccess = false;
	QString sorFileName = QString::fromLocal8Bit(sourceFileName.c_str());
	QString dstFileNameQ = QString::fromLocal8Bit(dstFileName.c_str());
	QFile file(sorFileName);
	//QFileInfo info(sorFileName);
	//QString dstPath = info.absoluteFilePath + QString("\\");
	//QDir directory;
	//directory.cd(dstPath);
	//dstPath += QString::fromLocal8Bit(dstFileName.c_str());
	file.copy(dstFileNameQ);
	file.close();

	QFile checkPath(dstFileNameQ);
	if (checkPath.open(QFile::ReadOnly))
	{
		bsuccess = true;
	}
	checkPath.close();
	return bsuccess;
}
void QtOSGTopBar::slot_computeDEMAndHeightOfShp()
{
	std::string dsmFilePath = "G:\\data\\building\\HuaiLai\\HuaiLai-dsm.tif";
	std::string demFilePath = "G:\\data\\building\\HuaiLai\\HuaiLai-dem.tif";
	std::string shpFilePath = "G:\\data\\building\\HuaiLai\\wgs\\temp3.shp";
	osg::ref_ptr<vpb::GeospatialDataset> dsmGeoDataset = vpb::System::instance()->openGeospatialDataset(dsmFilePath, vpb::READ_ONLY);
	//osg::ref_ptr<vpb::GeospatialDataset> shpGeoDataset = vpb::System::instance()->openShpGeospatialDataset(shpFilePath, vpb::READ_ONLY);
	osg::ref_ptr<osg::CoordinateSystemNode> dsmCoorNode = new osg::CoordinateSystemNode("WKT", dsmGeoDataset->GetProjectionRef());
	//osg::ref_ptr<osg::CoordinateSystemNode> shpCoorNode = new osg::CoordinateSystemNode("WKT", shpGeoDataset->GetProjectionRef());
	osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options;
	options->setOptionString("double");
	osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(shpFilePath, options.get());
	osgTerrain::Locator* locator = dynamic_cast<osgTerrain::Locator*>(model->getUserData());
	if (locator)
	{
		osg::ref_ptr<osg::CoordinateSystemNode> shpCoorNode = new osg::CoordinateSystemNode("WKT", locator->getCoordinateSystem());
		if (areCoordinateSystemEquivalent(dsmCoorNode, shpCoorNode))
		{
			emit sig_addNodeToScene(model);
			if (_copyFileToOtherName(dsmFilePath, demFilePath)) {
				ShapeFileProcessor* processor = new ShapeFileProcessor();
				processor->processor(model, dsmFilePath, demFilePath);
			}
		}
		else {
			vpb::System::instance()->getTaskManager()->log(osg::NOTICE,
				"slot_computeDEMAndHeightOfShp are not Coordinate System Equivalent");
		}
	}
}


void QtOSGTopBar::slot_convertDSMWithModelToOSGB()
{
	osg::Timer_t startTick = osg::Timer::instance()->tick();
	osg::ref_ptr<vpb::TaskManager> taskManager = vpb::System::instance()->getTaskManager();
	taskManager->setSignalAction(SIGABRT, vpb::TaskManager::TERMINATE_RUNNING_TASKS_THEN_EXIT);
	taskManager->setSignalAction(SIGINT, vpb::TaskManager::TERMINATE_RUNNING_TASKS_THEN_EXIT);
	taskManager->setSignalAction(SIGTERM, vpb::TaskManager::TERMINATE_RUNNING_TASKS_THEN_EXIT);
	//std::string runPath="D:\\work\\OSGExamples\\VirtualPlanetBuilder-master\\build\\bin\\Debug";
	//vpb::chdir(runPath.c_str());
	//taskManager->setRunPath(runPath);
	int argc = 7;
	char* argvArr[7];
	char* dsmFilePath = "G:\\data\\building\\HuaiLai\\HuaiLai-dsm.tif";
	char* textureFilePath = "G:\\data\\building\\HuaiLai\\HuaiLai-ortho.tif";
	char* outOSGBFilePath = "G:\\data\\building\\HuaiLai\\dsm.osgb";
	argvArr[0] = "vpbqtd.exe";
	argvArr[1] = "-d";
	argvArr[2] = dsmFilePath;
	argvArr[3] = "-t";
	argvArr[4] = textureFilePath;
	//argvArr[5] = "--building";
	//argvArr[6] = "G:\data\building\HuaiLai\gis\temp.shp";
	argvArr[5] = "-o";
	argvArr[6] = outOSGBFilePath;
	
	osg::ArgumentParser arguments(&argc, argvArr);
	taskManager->read(arguments);
	arguments.reportRemainingOptionsAsUnrecognized();

	// report any errors if they have occured when parsing the program aguments.
	if (arguments.errors())
	{
		arguments.writeErrorMessages(std::cout);
		taskManager->exit(SIGTERM);
		return ;
	}
	std::string buildProblems = taskManager->checkBuildValidity();
	std::string tasksOutputFileName;
	if (buildProblems.empty())
	{
		{
			if (!taskManager->hasTasks())
			{
				std::string sourceFileName = taskManager->getBuildName() + std::string("_master.source");
				tasksOutputFileName = taskManager->getBuildName() + std::string("_master.tasks");

				taskManager->setSourceFileName(sourceFileName);
				if (!taskManager->generateTasksFromSource())
				{
					// nothing to do.
					taskManager->exit(SIGTERM);
					return ;
				}

				taskManager->writeSource(sourceFileName);
				taskManager->writeTasks(tasksOutputFileName, true);

				taskManager->log(osg::NOTICE, "Generated tasks file = %s", tasksOutputFileName.c_str());

				vpb::DatabaseBuilder* db = dynamic_cast<vpb::DatabaseBuilder*>(taskManager->getSource()->getTerrainTechnique());
				vpb::BuildOptions* buildOptions = (db && db->getBuildOptions()) ? db->getBuildOptions() : 0;

				if (buildOptions)
				{
					std::stringstream sstr;
					sstr << buildOptions->getDirectory() << buildOptions->getDestinationTileBaseName() << buildOptions->getDestinationTileExtension() << "." << buildOptions->getRevisionNumber() << ".source";

					taskManager->writeSource(sstr.str());


					taskManager->log(osg::NOTICE, "Revsion source = %s", sstr.str().c_str());
				}
			}
			// make sure the OS writes changes to disk
			vpb::sync();
			if (taskManager->hasMachines())
			{
				if (!taskManager->run())
				{
					//result = 1;
					taskManager->log(osg::NOTICE, "taskManager->run() return false");
				}
			}
			else
			{
				taskManager->log(osg::NOTICE, "Cannot run build without machines assigned, please pass in a machines definition file via --machines <file>.");
			}
		}
	}
	else
	{
		taskManager->log(osg::NOTICE, "Build configuration invalid : %s", buildProblems.c_str());
	}

	double duration = osg::Timer::instance()->delta_s(startTick, osg::Timer::instance()->tick());
	taskManager->log(osg::NOTICE, "Total elapsed time = %f", duration);

	emit sig_addNodeToSceneByFilePath(outOSGBFilePath);
}

void QtOSGTopBar::slot_splitOSGBWithShpFile()
{

}

void QtOSGTopBar::_initFileAction()
{
	int height = 40;
	int width = 70;
	int x = 0, y = 0;
	int botLabDat = 15;
	int labToToolbtnHight = 6;
	int labWidth = 70;
	m_addDSMAction = new QAction(this);
	connect(m_addDSMAction, SIGNAL(triggered()), this, SLOT(slot_addDSM()));
	_createToolbtnOnWidget(this, x, y, width, height, "images/dsm.png", "dsm_toolbutton", m_addDSMAction);
	_addLabelOnWidget(this, x + botLabDat, y + height + labToToolbtnHight, labWidth, 15, tr("Open DSM"));
	x += 70;
	m_addTextureAction = new QAction(this);
	connect(m_addTextureAction, SIGNAL(triggered()), this, SLOT(slot_addTexture()));
	_createToolbtnOnWidget(this, x, y, width, height, "images/TIFF.png", "texture_toolbutton", m_addTextureAction);
	_addLabelOnWidget(this, x + botLabDat-5, y + height + labToToolbtnHight, labWidth, 15, tr("Open Texture"));
	x += 70;
	m_addShpFileAction = new QAction(this);
	connect(m_addShpFileAction, SIGNAL(triggered()), this, SLOT(slot_addShpFile()));
	_createToolbtnOnWidget(this, x, y, width, height, "images/shp.png", "shpfile_toolbutton", m_addShpFileAction);
	_addLabelOnWidget(this, x + botLabDat, y + height + labToToolbtnHight, labWidth, 15, tr("Open Shp"));
	x += 70;
	m_addModelFileAction = new QAction(this);
	connect(m_addModelFileAction, SIGNAL(triggered()), this, SLOT(slot_addModel()));
	_createToolbtnOnWidget(this, x, y, width, height, "images/model.png", "model_toolbutton", m_addModelFileAction);
	_addLabelOnWidget(this, x + botLabDat-5, y + height + labToToolbtnHight, labWidth, 15, tr("Open Model"));
}

void QtOSGTopBar::_initComputeAction()
{
	int height = 40;
	int width = 70;
	int x = 0, y = 0;
	int botLabDat = 10;
	int labToToolbtnHight = 6;
	int labWidth = 70;

	m_computeDEMAndHeightOfShpAction = new QAction(this);
	connect(m_computeDEMAndHeightOfShpAction, SIGNAL(triggered()), this, SLOT(slot_computeDEMAndHeightOfShp()));
	_createToolbtnOnWidget(this, x, y, width, height, "images/dsmTodem.png", "dsm_toolbutton", m_computeDEMAndHeightOfShpAction);
	_addLabelOnWidget(this, x + botLabDat-5, y + height + labToToolbtnHight, labWidth, 15, tr("Compute DEM"));
	x += 70;
	m_convertDSMWithModelToOSGBAction = new QAction(this);
	connect(m_convertDSMWithModelToOSGBAction, SIGNAL(triggered()), this, SLOT(slot_convertDSMWithModelToOSGB()));
	_createToolbtnOnWidget(this, x, y, width, height, "images/convertToOSGB.png", "texture_toolbutton", m_convertDSMWithModelToOSGBAction);
	_addLabelOnWidget(this, x + botLabDat, y + height + labToToolbtnHight, labWidth, 15, tr("Convert OSGB"));
	x += 70;
	m_splitOSGBWithShpFileAction = new QAction(this);
	connect(m_splitOSGBWithShpFileAction, SIGNAL(triggered()), this, SLOT(slot_splitOSGBWithShpFile()));
	_createToolbtnOnWidget(this, x, y, width, height, "images/SplitDSM.png", "shpfile_toolbutton", m_splitOSGBWithShpFileAction);
	_addLabelOnWidget(this, x + botLabDat+5, y + height + labToToolbtnHight, labWidth, 15, tr("Split DSM"));
	
}