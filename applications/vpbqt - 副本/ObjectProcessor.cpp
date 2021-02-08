/* -*-c++-*- VirtualPlanetBuilder - Copyright (C) 1998-2009 Robert Osfield
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


#include "ObjectProcessor.h"
#include <vpb/Destination>
#include <vpb/DataSet>



bool ObjectProcessor::processor(osg::Node* model, const std::string& dsmFilePath)
{
    vpb::log(osg::NOTICE,"ObjectProcessor::processor(%s)",model->getName().c_str());

    //destinationTile.addNodeToScene(model, true);
    
    return true;
}
