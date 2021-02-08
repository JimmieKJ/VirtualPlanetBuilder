/* -*-c++-*- VirtualPlanetBuilder - Copyright (C) 1998-2007 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#ifndef SHAPEFILEPROCESSOR_H
#define SHAPEFILEPROCESSOR_H 1

#include "ObjectProcessor.h"
#include <cpl_conv.h>
#include <gdal.h>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <ogr_feature.h>
#include <ogr_geometry.h>

class ShapeFileProcessor : public ObjectProcessor
{
public:

    ShapeFileProcessor() {}
	~ShapeFileProcessor();
    virtual bool processor(osg::Node* model, const std::string& dsmFilePath, const std::string& demFilePath);
private:
	GDALDataset* _processordsmFilePath(const std::string& dsmFilePath);
private:
	GDALDataset* m_dsmDS;
};




#endif
