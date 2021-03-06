#this file is automatically generated 
SET(APPLICATION_NAME vpbqt)
INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIR} ${OPENSCENEGRAPH_INCLUDE_DIRS} ${QT_INCLUDES} )
SET(TARGET_LIBRARIES_VARS GDAL_LIBRARY OSG_LIBRARY OSGTERRAIN_LIBRARY OSGVIEWER_LIBRARY OSGQT_LIBRARY OSGGA_LIBRARY)
SET(APPLICATION_DIR ${VirtualPlanetBuilder_SOURCE_DIR}/applications/${APPLICATION_NAME})
SET(APPLICATION_RESOURCE_DIR ${VirtualPlanetBuilder_SOURCE_DIR}/applications/${APPLICATION_NAME}/res)

SET(MAIN_WINDOW_H_CPP
	${APPLICATION_DIR}/main.cpp
	${APPLICATION_DIR}/osgviewwidget.cpp
	${APPLICATION_DIR}/osgviewwidget.h
	${APPLICATION_DIR}/qosgcommon.cpp
	${APPLICATION_DIR}/qosgcommon.h
	${APPLICATION_DIR}/qosgmainwindow.cpp
	${APPLICATION_DIR}/qosgmainwindow.h
	${APPLICATION_DIR}/qosgtrackball.cpp
	${APPLICATION_DIR}/qosgtrackball.h
	${APPLICATION_DIR}/qtosgleftsidebar.cpp
	${APPLICATION_DIR}/qtosgleftsidebar.h
	${APPLICATION_DIR}/osgskybox.cpp
	${APPLICATION_DIR}/osgskybox.h
	${APPLICATION_DIR}/qtosgtopbar.cpp
	${APPLICATION_DIR}/qtosgtopbar.h
	${APPLICATION_DIR}/ObjectProcessor.cpp
	${APPLICATION_DIR}/ObjectProcessor.h
	${APPLICATION_DIR}/ShapeFileProcessor.cpp
	${APPLICATION_DIR}/ShapeFileProcessor.h
	)
SOURCE_GROUP(vpbqt FILES ${MAIN_WINDOW_H_CPP})
SET(MAIN_WINDOW_RESOURCE
	${APPLICATION_RESOURCE_DIR}/vpbqt.qrc
	)
SOURCE_GROUP(Resource FILES ${MAIN_WINDOW_RESOURCE})
if (Qt5Widgets_FOUND)
	QT5_ADD_RESOURCES(MAIN_WINDOW_MOCS_RESOURCE
	${MAIN_WINDOW_RESOURCE}
	)
else()
	QT4_ADD_RESOURCES(MAIN_WINDOW_MOCS_RESOURCE
	${MAIN_WINDOW_RESOURCE}
	)
endif()
SET(MAIN_WINDOW_UI
	
	)

if (Qt5Widgets_FOUND)
	qt5_wrap_ui(UICS ${MAIN_WINDOW_UI})
else()
	qt4_wrap_ui(UICS ${MAIN_WINDOW_UI})
endif()

SOURCE_GROUP(vpbqt FILES ${MAIN_WINDOW_UI})

set(LIB_MOC_HDRS
	${APPLICATION_DIR}/osgviewwidget.h
	${APPLICATION_DIR}/qosgcommon.h
	${APPLICATION_DIR}/qosgmainwindow.h
	${APPLICATION_DIR}/qosgtrackball.h
	${APPLICATION_DIR}/qtosgleftsidebar.h
	${APPLICATION_DIR}/osgskybox.h
	${APPLICATION_DIR}/qtosgtopbar.h
	)

IF(Qt5Widgets_FOUND)
    SET(LIB_MOC_SRCS)
    FOREACH( LIB_MOC_HDR ${LIB_MOC_HDRS} )
        GET_FILENAME_COMPONENT( LIB_MOC_HDR_ABS ${LIB_MOC_HDR} ABSOLUTE )
        QT5_WRAP_CPP( LIB_MOC_SRC ${LIB_MOC_HDR} OPTIONS "-f${LIB_MOC_HDR_ABS}" )
        LIST( APPEND LIB_MOC_SRCS ${LIB_MOC_SRC} )
    ENDFOREACH()
ELSE()
    INCLUDE( ${QT_USE_FILE} )
    QT4_WRAP_CPP( LIB_MOC_SRCS ${LIB_MOC_HDRS} OPTIONS "-f" )
ENDIF()
set(CMAKEQT_SRCS "")

#add_executable(
#	${APPLICATION_NAME}
#	${LIB_MOC_HDRS}
#    ${LIB_MOC_SRCS}
#	${CMAKEQT_SRCS}
#	${MAIN_WINDOW_H_CPP}
#	${MAIN_WINDOW_MOCS_CPP}
#	${MAIN_WINDOW_UI}
#	${MAIN_WINDOW_RESOURCE}
#	${MAIN_WINDOW_MOCS_RESOURCE}
#	)
SET(TARGET_SRC ShapeFileProcessor.cpp ObjectProcessor.cpp qtosgtopbar.cpp osgviewwidget.cpp qosgcommon.cpp qosgmainwindow.cpp qosgtrackball.cpp qtosgleftsidebar.cpp osgskybox.cpp main.cpp ${LIB_MOC_SRCS})
SET(TARGET_H ObjectProcessor.h ShapeFileProcessor.h osgviewwidget.h qosgcommon.h qosgmainwindow.h qosgtrackball.h qtosgleftsidebar.h osgskybox.h qtosgtopbar.h)

#### end var setup  ###
SETUP_APPLICATION(${APPLICATION_NAME})

target_link_libraries(${TARGET_TARGETNAME} ${Qt5Widgets_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Core_LIBRARIES} ${Qt5OpenGL_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5Test_LIBRARIES} ${Qt5Xml_LIBRARIES})
