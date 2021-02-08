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


#include "ShapeFileProcessor.h"
#include <vpb/Destination>
#include <vpb/DataSet>
#include <vpb/HeightFieldMapper>
#include <vpb/ExtrudeVisitor>
#include <vpb/System>

#include <osg/NodeVisitor>
#include <osg/Array>
#include <osgUtil/ConvertVec>
#include <osgUtil/DrawElementTypeSimplifier>
#include <osgUtil/SmoothingVisitor>
#include <osgSim/ShapeAttribute>
#include <osg/Material>

#include <limits>
using namespace vpb;


class SetZeroToZVisitor : public osg::ArrayVisitor
{
public:

	void apply(osg::Vec3Array& array) { _getMinMaxXYInGeom<osg::Vec3Array>(array); }
	void apply(osg::Vec4Array& array) { _getMinMaxXYInGeom<osg::Vec4Array>(array); }

	void apply(osg::Vec3dArray& array) { _getMinMaxXYInGeom<osg::Vec3dArray>(array); }
	void apply(osg::Vec4dArray& array) { _getMinMaxXYInGeom<osg::Vec4dArray>(array); }


	template <typename ArrayType>
	void _getMinMaxXYInGeom(ArrayType & array)
	{
		int vCount = array.size();
		float minVX = array.at(0).x();
		float minVY = array.at(0).y();
		float maxVX = array.at(0).x();
		float maxVY = array.at(0).y();
		for (int vi = 0; vi < vCount; vi++)
		{
			float tempx = array.at(vi).x();
			float tempy = array.at(vi).y();
			if (tempx > maxVX)
			{
				maxVX = tempx;
			}
			if (tempx < minVX)
			{
				minVX = tempx;
			}
			if (tempy > maxVY)
			{
				maxVY = tempy;
			}
			if (tempy < minVY)
			{
				minVY = tempy;
			}
		}
		m_minMaxXYArray = osg::Vec4(minVX, minVY, maxVX, maxVY);
	}
	osg::Vec4 m_minMaxXYArray;
};

class ConvertDSMTODEMByShpFile : public osg::NodeVisitor
{
    public:
    
		enum DataType {
			kUnknown = 0,
			kByte = 1,
			kInt = 2,
			kUInt = 3,
			kFloat = 4,
			kDouble = 5,
			kShort = 6,
			kUByte = 7
		};
        
        ConvertDSMTODEMByShpFile(GDALDataset* ds, GDALDataset* demDataset) :
            _ds(ds),
			m_demDataset(demDataset)
        {
			
            /*_createdModel = new osg::Group;
            _nodeStack.push_back(_createdModel.get());
            _typeAttributeNameStack.push_back("NAME");
            _heightAttributeNameStack.push_back("HGT");*/
       }
        
        virtual void apply(osg::Node& node)
        {
            // ** if sub-graph overlap the HeightField
            if (!bTraversed &&overlap(node.getBound()))
            {
                // ** clone the node
                osg::ref_ptr<osg::Node> clonedNode = static_cast<osg::Node*>(node.clone(osg::CopyOp::SHALLOW_COPY));
                
                // ** if it's osg::Group type, clean children
                osg::Group * clonedGroup = clonedNode->asGroup();
                if (clonedGroup) clonedGroup->removeChild(0, clonedGroup->getNumChildren());
				traverse(node);
                //addAndTraverse(node, clonedNode.get());
            }
        }
		
		int _computeSimilarHeight(DataType dataTypeIn, int dataWidth,int dataHeight, void* demDataTemp)
		{
			long dataLength = dataWidth * dataHeight;
			float minVal = 0.0;
			int index = 1;
			switch (dataTypeIn) {
				case kUByte: {
					unsigned char* demData = ((unsigned char*)demDataTemp);
					{
						int i = 0;
						int j = 0;
						int ltindex = ((0 - 1) >= 0 && (0 - 1) >= 0) ? ((0 - 1) * dataWidth + (0 - 1)) : -1;
						int lcindex = ((i - 1) >= 0) ? (j * dataWidth + (i - 1)) : -1;
						int lbindex = (j + 1 < dataHeight && (i - 1) >= 0) ? ((j + 1) * dataWidth + (i - 1)) : -1;
						int ctindex = ((j - 1) >= 0) ? ((j - 1) * dataWidth + i) : -1;
						int index = j * dataWidth + i;
						int cbindex = ((j + 1) < dataHeight) ? ((j + 1) * dataWidth + i) : -1;
						int rtindex = ((j - 1) >= 0 && (i + 1) < dataWidth) ? ((j - 1) * dataWidth + i + 1) : -1;
						int rcindex = ((i + 1) < dataWidth) ? (j * dataWidth + i + 1) : -1;
						int rbindex = ((j + 1) < dataHeight && (i + 1) < dataWidth) ? ((j + 1) * dataWidth + i + 1) : -1;
						float val = demData[index];
						float ltvaldat = (ltindex >= 0 && ltindex < dataLength) ? (val - demData[ltindex]) : 0;
						float lcvaldat = (lcindex >= 0 && lcindex < dataLength) ? (val - demData[lcindex]) : 0;
						float lbvaldat = (lbindex >= 0 && lbindex < dataLength) ? (val - demData[lbindex]) : 0;
						float ctvaldat = (ctindex >= 0 && ctindex < dataLength) ? (val - demData[ctindex]) : 0;
						float cbvaldat = (cbindex >= 0 && cbindex < dataLength) ? (val - demData[cbindex]) : 0;
						float rtvaldat = (rtindex >= 0 && rtindex < dataLength) ? (val - demData[rtindex]) : 0;
						float rcvaldat = (rcindex >= 0 && rcindex < dataLength) ? (val - demData[rcindex]) : 0;
						float rbvaldat = (ltvaldat + lcvaldat + lbvaldat + ctvaldat + cbvaldat + rtvaldat + rcvaldat) / 7;
						minVal = rbvaldat;
						index = 1;
					}
					{
						int i = dataWidth - 1;
						int j = 0;
						int ltindex = ((j - 1) >= 0 && (i - 1) >= 0) ? ((j - 1) * dataWidth + (i - 1)) : -1;
						int lcindex = ((i - 1) >= 0) ? (j * dataWidth + (i - 1)) : -1;
						int lbindex = (j + 1 < dataHeight && (i - 1) >= 0) ? ((j + 1) * dataWidth + (i - 1)) : -1;
						int ctindex = ((j - 1) >= 0) ? ((j - 1) * dataWidth + i) : -1;
						int index = j * dataWidth + i;
						int cbindex = ((j + 1) < dataHeight) ? ((j + 1) * dataWidth + i) : -1;
						int rtindex = ((j - 1) >= 0 && (i + 1) < dataWidth) ? ((j - 1) * dataWidth + i + 1) : -1;
						int rcindex = ((i + 1) < dataWidth) ? (j * dataWidth + i + 1) : -1;
						int rbindex = ((j + 1) < dataHeight && (i + 1) < dataWidth) ? ((j + 1) * dataWidth + i + 1) : -1;
						float val = demData[index];
						float ltvaldat = (ltindex >= 0 && ltindex < dataLength) ? (val - demData[ltindex]) : 0;
						float lcvaldat = (lcindex >= 0 && lcindex < dataLength) ? (val - demData[lcindex]) : 0;
						float ctvaldat = (ctindex >= 0 && ctindex < dataLength) ? (val - demData[ctindex]) : 0;
						float cbvaldat = (cbindex >= 0 && cbindex < dataLength) ? (val - demData[cbindex]) : 0;
						float rtvaldat = (rtindex >= 0 && rtindex < dataLength) ? (val - demData[rtindex]) : 0;
						float rcvaldat = (rcindex >= 0 && rcindex < dataLength) ? (val - demData[rcindex]) : 0;
						float rbvaldat = (rbindex >= 0 && rbindex < dataLength) ? (val - demData[rbindex]) : 0;
						float lbvaldat = (ltvaldat + lcvaldat + rbvaldat + ctvaldat + cbvaldat + rtvaldat + rcvaldat) / 7;
						if (minVal > lbvaldat) {
							minVal = lbvaldat;
							index = 2;
						}
					}

					{
						int j = dataHeight - 1;
						int i = 0;
						int ltindex = ((j - 1) >= 0 && (i - 1) >= 0) ? ((j - 1) * dataWidth + (i - 1)) : -1;
						int lcindex = ((i - 1) >= 0) ? (j * dataWidth + (i - 1)) : -1;
						int lbindex = (j + 1 < dataHeight && (i - 1) >= 0) ? ((j + 1) * dataWidth + (i - 1)) : -1;
						int ctindex = ((j - 1) >= 0) ? ((j - 1) * dataWidth + i) : -1;
						int index = j * dataWidth + i;
						int cbindex = ((j + 1) < dataHeight) ? ((j + 1) * dataWidth + i) : -1;
						int rtindex = ((j - 1) >= 0 && (i + 1) < dataWidth) ? ((j - 1) * dataWidth + i + 1) : -1;
						int rcindex = ((i + 1) < dataWidth) ? (j * dataWidth + i + 1) : -1;
						int rbindex = ((j + 1) < dataHeight && (i + 1) < dataWidth) ? ((j + 1) * dataWidth + i + 1) : -1;
						float val = demData[index];
						float ltvaldat = (ltindex >= 0 && ltindex < dataLength) ? (val - demData[ltindex]) : 0;
						float lcvaldat = (lcindex >= 0 && lcindex < dataLength) ? (val - demData[lcindex]) : 0;
						float lbvaldat = (lbindex >= 0 && lbindex < dataLength) ? (val - demData[lbindex]) : 0;
						float ctvaldat = (ctindex >= 0 && ctindex < dataLength) ? (val - demData[ctindex]) : 0;
						float cbvaldat = (cbindex >= 0 && cbindex < dataLength) ? (val - demData[cbindex]) : 0;
						float rcvaldat = (rcindex >= 0 && rcindex < dataLength) ? (val - demData[rcindex]) : 0;
						float rbvaldat = (rbindex >= 0 && rbindex < dataLength) ? (val - demData[rbindex]) : 0;
						float rtvaldat = (ltvaldat + lcvaldat + rbvaldat + ctvaldat + cbvaldat + lbvaldat + rcvaldat) / 7;
						if (minVal > rtvaldat) {
							minVal = rtvaldat;
							index = 3;
						}
					}
					{
						int i = dataWidth - 1;
						int j = dataHeight - 1;
						int ltindex = ((j - 1) >= 0 && (i - 1) >= 0) ? ((j - 1) * dataWidth + (i - 1)) : -1;
						int lcindex = ((i - 1) >= 0) ? (j * dataWidth + (i - 1)) : -1;
						int lbindex = (j + 1 < dataHeight && (i - 1) >= 0) ? ((j + 1) * dataWidth + (i - 1)) : -1;
						int ctindex = ((j - 1) >= 0) ? ((j - 1) * dataWidth + i) : -1;
						int index = j * dataWidth + i;
						int cbindex = ((j + 1) < dataHeight) ? ((j + 1) * dataWidth + i) : -1;
						int rtindex = ((j - 1) >= 0 && (i + 1) < dataWidth) ? ((j - 1) * dataWidth + i + 1) : -1;
						int rcindex = ((i + 1) < dataWidth) ? (j * dataWidth + i + 1) : -1;
						int rbindex = ((j + 1) < dataHeight && (i + 1) < dataWidth) ? ((j + 1) * dataWidth + i + 1) : -1;
						float val = demData[index];
						float lcvaldat = (lcindex >= 0 && lcindex < dataLength) ? (val - demData[lcindex]) : 0;
						float lbvaldat = (lbindex >= 0 && lbindex < dataLength) ? (val - demData[lbindex]) : 0;
						float ctvaldat = (ctindex >= 0 && ctindex < dataLength) ? (val - demData[ctindex]) : 0;
						float cbvaldat = (cbindex >= 0 && cbindex < dataLength) ? (val - demData[cbindex]) : 0;
						float rtvaldat = (rtindex >= 0 && rtindex < dataLength) ? (val - demData[rtindex]) : 0;
						float rcvaldat = (rcindex >= 0 && rcindex < dataLength) ? (val - demData[rcindex]) : 0;
						float rbvaldat = (rbindex >= 0 && rbindex < dataLength) ? (val - demData[rbindex]) : 0;
						float ltvaldat = (rtvaldat + lcvaldat + rbvaldat + ctvaldat + cbvaldat + lbvaldat + rcvaldat) / 7;
						if (minVal > ltvaldat) {
							minVal = ltvaldat;
							index = 4;
						}
					}
					break;
				}
				case kFloat: {
					float* demData = (float*)demDataTemp;
					{
						int i = 0;
						int j = 0;
						int ltindex = ((0 - 1) >= 0 && (0 - 1) >= 0) ? ((0 - 1) * dataWidth + (0 - 1)) : -1;
						int lcindex = ((i - 1) >= 0) ? (j * dataWidth + (i - 1)) : -1;
						int lbindex = (j + 1 < dataHeight && (i - 1) >= 0) ? ((j + 1) * dataWidth + (i - 1)) : -1;
						int ctindex = ((j - 1) >= 0) ? ((j - 1) * dataWidth + i) : -1;
						int index = j * dataWidth + i;
						int cbindex = ((j + 1) < dataHeight) ? ((j + 1) * dataWidth + i) : -1;
						int rtindex = ((j - 1) >= 0 && (i + 1) < dataWidth) ? ((j - 1) * dataWidth + i + 1) : -1;
						int rcindex = ((i + 1) < dataWidth) ? (j * dataWidth + i + 1) : -1;
						int rbindex = ((j + 1) < dataHeight && (i + 1) < dataWidth) ? ((j + 1) * dataWidth + i + 1) : -1;
						float val = demData[index];
						float ltvaldat = (ltindex >= 0 && ltindex < dataLength) ? (val - demData[ltindex]) : 0;
						float lcvaldat = (lcindex >= 0 && lcindex < dataLength) ? (val - demData[lcindex]) : 0;
						float lbvaldat = (lbindex >= 0 && lbindex < dataLength) ? (val - demData[lbindex]) : 0;
						float ctvaldat = (ctindex >= 0 && ctindex < dataLength) ? (val - demData[ctindex]) : 0;
						float cbvaldat = (cbindex >= 0 && cbindex < dataLength) ? (val - demData[cbindex]) : 0;
						float rtvaldat = (rtindex >= 0 && rtindex < dataLength) ? (val - demData[rtindex]) : 0;
						float rcvaldat = (rcindex >= 0 && rcindex < dataLength) ? (val - demData[rcindex]) : 0;
						float rbvaldat = (ltvaldat + lcvaldat + lbvaldat + ctvaldat + cbvaldat + rtvaldat + rcvaldat) / 7;
						minVal = rbvaldat;
						index = 1;
					}
					{
						int i = dataWidth - 1;
						int j = 0;
						int ltindex = ((j - 1) >= 0 && (i - 1) >= 0) ? ((j - 1) * dataWidth + (i - 1)) : -1;
						int lcindex = ((i - 1) >= 0) ? (j * dataWidth + (i - 1)) : -1;
						int lbindex = (j + 1 < dataHeight && (i - 1) >= 0) ? ((j + 1) * dataWidth + (i - 1)) : -1;
						int ctindex = ((j - 1) >= 0) ? ((j - 1) * dataWidth + i) : -1;
						int index = j * dataWidth + i;
						int cbindex = ((j + 1) < dataHeight) ? ((j + 1) * dataWidth + i) : -1;
						int rtindex = ((j - 1) >= 0 && (i + 1) < dataWidth) ? ((j - 1) * dataWidth + i + 1) : -1;
						int rcindex = ((i + 1) < dataWidth) ? (j * dataWidth + i + 1) : -1;
						int rbindex = ((j + 1) < dataHeight && (i + 1) < dataWidth) ? ((j + 1) * dataWidth + i + 1) : -1;
						float val = demData[index];
						float ltvaldat = (ltindex >= 0 && ltindex < dataLength) ? (val - demData[ltindex]) : 0;
						float lcvaldat = (lcindex >= 0 && lcindex < dataLength) ? (val - demData[lcindex]) : 0;
						float ctvaldat = (ctindex >= 0 && ctindex < dataLength) ? (val - demData[ctindex]) : 0;
						float cbvaldat = (cbindex >= 0 && cbindex < dataLength) ? (val - demData[cbindex]) : 0;
						float rtvaldat = (rtindex >= 0 && rtindex < dataLength) ? (val - demData[rtindex]) : 0;
						float rcvaldat = (rcindex >= 0 && rcindex < dataLength) ? (val - demData[rcindex]) : 0;
						float rbvaldat = (rbindex >= 0 && rbindex < dataLength) ? (val - demData[rbindex]) : 0;
						float lbvaldat = (ltvaldat + lcvaldat + rbvaldat + ctvaldat + cbvaldat + rtvaldat + rcvaldat) / 7;
						if (minVal > lbvaldat) {
							minVal = lbvaldat;
							index = 2;
						}
					}

					{
						int j = dataHeight - 1;
						int i = 0;
						int ltindex = ((j - 1) >= 0 && (i - 1) >= 0) ? ((j - 1) * dataWidth + (i - 1)) : -1;
						int lcindex = ((i - 1) >= 0) ? (j * dataWidth + (i - 1)) : -1;
						int lbindex = (j + 1 < dataHeight && (i - 1) >= 0) ? ((j + 1) * dataWidth + (i - 1)) : -1;
						int ctindex = ((j - 1) >= 0) ? ((j - 1) * dataWidth + i) : -1;
						int index = j * dataWidth + i;
						int cbindex = ((j + 1) < dataHeight) ? ((j + 1) * dataWidth + i) : -1;
						int rtindex = ((j - 1) >= 0 && (i + 1) < dataWidth) ? ((j - 1) * dataWidth + i + 1) : -1;
						int rcindex = ((i + 1) < dataWidth) ? (j * dataWidth + i + 1) : -1;
						int rbindex = ((j + 1) < dataHeight && (i + 1) < dataWidth) ? ((j + 1) * dataWidth + i + 1) : -1;
						float val = demData[index];
						float ltvaldat = (ltindex >= 0 && ltindex < dataLength) ? (val - demData[ltindex]) : 0;
						float lcvaldat = (lcindex >= 0 && lcindex < dataLength) ? (val - demData[lcindex]) : 0;
						float lbvaldat = (lbindex >= 0 && lbindex < dataLength) ? (val - demData[lbindex]) : 0;
						float ctvaldat = (ctindex >= 0 && ctindex < dataLength) ? (val - demData[ctindex]) : 0;
						float cbvaldat = (cbindex >= 0 && cbindex < dataLength) ? (val - demData[cbindex]) : 0;
						float rcvaldat = (rcindex >= 0 && rcindex < dataLength) ? (val - demData[rcindex]) : 0;
						float rbvaldat = (rbindex >= 0 && rbindex < dataLength) ? (val - demData[rbindex]) : 0;
						float rtvaldat = (ltvaldat + lcvaldat + rbvaldat + ctvaldat + cbvaldat + lbvaldat + rcvaldat) / 7;
						if (minVal > rtvaldat) {
							minVal = rtvaldat;
							index = 3;
						}
					}
					{
						int i = dataWidth - 1;
						int j = dataHeight - 1;
						int ltindex = ((j - 1) >= 0 && (i - 1) >= 0) ? ((j - 1) * dataWidth + (i - 1)) : -1;
						int lcindex = ((i - 1) >= 0) ? (j * dataWidth + (i - 1)) : -1;
						int lbindex = (j + 1 < dataHeight && (i - 1) >= 0) ? ((j + 1) * dataWidth + (i - 1)) : -1;
						int ctindex = ((j - 1) >= 0) ? ((j - 1) * dataWidth + i) : -1;
						int index = j * dataWidth + i;
						int cbindex = ((j + 1) < dataHeight) ? ((j + 1) * dataWidth + i) : -1;
						int rtindex = ((j - 1) >= 0 && (i + 1) < dataWidth) ? ((j - 1) * dataWidth + i + 1) : -1;
						int rcindex = ((i + 1) < dataWidth) ? (j * dataWidth + i + 1) : -1;
						int rbindex = ((j + 1) < dataHeight && (i + 1) < dataWidth) ? ((j + 1) * dataWidth + i + 1) : -1;
						float val = demData[index];
						float lcvaldat = (lcindex >= 0 && lcindex < dataLength) ? (val - demData[lcindex]) : 0;
						float lbvaldat = (lbindex >= 0 && lbindex < dataLength) ? (val - demData[lbindex]) : 0;
						float ctvaldat = (ctindex >= 0 && ctindex < dataLength) ? (val - demData[ctindex]) : 0;
						float cbvaldat = (cbindex >= 0 && cbindex < dataLength) ? (val - demData[cbindex]) : 0;
						float rtvaldat = (rtindex >= 0 && rtindex < dataLength) ? (val - demData[rtindex]) : 0;
						float rcvaldat = (rcindex >= 0 && rcindex < dataLength) ? (val - demData[rcindex]) : 0;
						float rbvaldat = (rbindex >= 0 && rbindex < dataLength) ? (val - demData[rbindex]) : 0;
						float ltvaldat = (rtvaldat + lcvaldat + rbvaldat + ctvaldat + cbvaldat + lbvaldat + rcvaldat) / 7;
						if (minVal > ltvaldat) {
							minVal = ltvaldat;
							index = 4;
						}
					}
					break;
				}
			}
			return index;
		}

		void _computeDEMData(void* pDataIn, void* demDataTemp,DataType dataTypeIn, int dataWidth, int dataHeight,double& maxBudingHeight)
		{
			int typeIndex = _computeSimilarHeight(dataTypeIn, dataWidth, dataHeight, demDataTemp);
			switch (dataTypeIn) {
			case kUByte: {
				long dataLength = dataWidth * dataHeight;
				unsigned char* dsmData = ((unsigned char*)pDataIn);
				unsigned char* demData = ((unsigned char*)demDataTemp);
				
				for (int j = 0; j < dataHeight / 2; j++)
				{
					for (int i = 0; i < dataWidth / 2; i++)
					{
						int ltindex = ((j - 1) >= 0 && (i - 1) >= 0) ? ((j - 1) * dataWidth + (i - 1)) : -1;
						int lcindex = ((i - 1) >= 0) ? (j * dataWidth + (i - 1)) : -1;
						int lbindex = (j + 1 < dataHeight && (i - 1) >= 0) ? ((j + 1) * dataWidth + (i - 1)) : -1;
						int ctindex = ((j - 1) >= 0) ? ((j - 1) * dataWidth + i) : -1;
						int index = j * dataWidth + i;
						int cbindex = ((j + 1) < dataHeight) ? ((j + 1) * dataWidth + i) : -1;
						int rtindex = ((j - 1) >= 0 && (i + 1) < dataWidth) ? ((j - 1) * dataWidth + i + 1) : -1;
						int rcindex = ((i + 1) < dataWidth) ? (j * dataWidth + i + 1) : -1;
						int rbindex = ((j + 1) < dataHeight && (i + 1) < dataWidth) ? ((j + 1) * dataWidth + i + 1) : -1;
						unsigned char val = dsmData[index];
						char ltvaldat = (ltindex >= 0 && ltindex < dataLength) ? (val - dsmData[ltindex]) : 0;
						char lcvaldat = (lcindex >= 0 && lcindex < dataLength) ? (val - dsmData[lcindex]) : 0;
						char lbvaldat = (lbindex >= 0 && lbindex < dataLength) ? (val - dsmData[lbindex]) : 0;
						char ctvaldat = (ctindex >= 0 && ctindex < dataLength) ? (val - dsmData[ctindex]) : 0;
						char cbvaldat = (cbindex >= 0 && cbindex < dataLength) ? (val - dsmData[cbindex]) : 0;
						char rtvaldat = (rtindex >= 0 && rtindex < dataLength) ? (val - dsmData[rtindex]) : 0;
						char rcvaldat = (rcindex >= 0 && rcindex < dataLength) ? (val - dsmData[rcindex]) : 0;
						unsigned char rbvaldat = val - (ltvaldat + lcvaldat + lbvaldat + ctvaldat + cbvaldat + rtvaldat + rcvaldat) / 7;
						demData[rbindex] = rbvaldat;
					}
					for (int i = dataWidth - 1; i >= dataWidth / 2; i--)
					{
						int ltindex = ((j - 1) >= 0 && (i - 1) >= 0) ? ((j - 1) * dataWidth + (i - 1)) : -1;
						int lcindex = ((i - 1) >= 0) ? (j * dataWidth + (i - 1)) : -1;
						int lbindex = (j + 1 < dataHeight && (i - 1) >= 0) ? ((j + 1) * dataWidth + (i - 1)) : -1;
						int ctindex = ((j - 1) >= 0) ? ((j - 1) * dataWidth + i) : -1;
						int index = j * dataWidth + i;
						int cbindex = ((j + 1) < dataHeight) ? ((j + 1) * dataWidth + i) : -1;
						int rtindex = ((j - 1) >= 0 && (i + 1) < dataWidth) ? ((j - 1) * dataWidth + i + 1) : -1;
						int rcindex = ((i + 1) < dataWidth) ? (j * dataWidth + i + 1) : -1;
						int rbindex = ((j + 1) < dataHeight && (i + 1) < dataWidth) ? ((j + 1) * dataWidth + i + 1) : -1;
						unsigned char val = dsmData[index];
						char ltvaldat = (ltindex >= 0 && ltindex < dataLength) ? (val - dsmData[ltindex]) : 0;
						char lcvaldat = (lcindex >= 0 && lcindex < dataLength) ? (val - dsmData[lcindex]) : 0;
						char ctvaldat = (ctindex >= 0 && ctindex < dataLength) ? (val - dsmData[ctindex]) : 0;
						char cbvaldat = (cbindex >= 0 && cbindex < dataLength) ? (val - dsmData[cbindex]) : 0;
						char rtvaldat = (rtindex >= 0 && rtindex < dataLength) ? (val - dsmData[rtindex]) : 0;
						char rcvaldat = (rcindex >= 0 && rcindex < dataLength) ? (val - dsmData[rcindex]) : 0;
						char rbvaldat = (rbindex >= 0 && rbindex < dataLength) ? (val - dsmData[rbindex]) : 0;
						unsigned char lbvaldat = val - (ltvaldat + lcvaldat + rbvaldat + ctvaldat + cbvaldat + rtvaldat + rcvaldat) / 7;
						demData[lbindex] = lbvaldat;
					}
				}
				for (int j = dataHeight - 1; j >= dataHeight / 2; j++)
				{
					for (int i = 0; i < dataWidth / 2; i++)
					{
						int ltindex = ((j - 1) >= 0 && (i - 1) >= 0) ? ((j - 1) * dataWidth + (i - 1)) : -1;
						int lcindex = ((i - 1) >= 0) ? (j * dataWidth + (i - 1)) : -1;
						int lbindex = (j + 1 < dataHeight && (i - 1) >= 0) ? ((j + 1) * dataWidth + (i - 1)) : -1;
						int ctindex = ((j - 1) >= 0) ? ((j - 1) * dataWidth + i) : -1;
						int index = j * dataWidth + i;
						int cbindex = ((j + 1) < dataHeight) ? ((j + 1) * dataWidth + i) : -1;
						int rtindex = ((j - 1) >= 0 && (i + 1) < dataWidth) ? ((j - 1) * dataWidth + i + 1) : -1;
						int rcindex = ((i + 1) < dataWidth) ? (j * dataWidth + i + 1) : -1;
						int rbindex = ((j + 1) < dataHeight && (i + 1) < dataWidth) ? ((j + 1) * dataWidth + i + 1) : -1;
						unsigned char val = dsmData[index];
						char ltvaldat = (ltindex >= 0 && ltindex < dataLength) ? (val - dsmData[ltindex]) : 0;
						char lcvaldat = (lcindex >= 0 && lcindex < dataLength) ? (val - dsmData[lcindex]) : 0;
						char lbvaldat = (lbindex >= 0 && lbindex < dataLength) ? (val - dsmData[lbindex]) : 0;
						char ctvaldat = (ctindex >= 0 && ctindex < dataLength) ? (val - dsmData[ctindex]) : 0;
						char cbvaldat = (cbindex >= 0 && cbindex < dataLength) ? (val - dsmData[cbindex]) : 0;
						char rcvaldat = (rcindex >= 0 && rcindex < dataLength) ? (val - dsmData[rcindex]) : 0;
						char rbvaldat = (rbindex >= 0 && rbindex < dataLength) ? (val - dsmData[rbindex]) : 0;
						unsigned char rtvaldat = val - (ltvaldat + lcvaldat + rbvaldat + ctvaldat + cbvaldat + lbvaldat + rcvaldat) / 7;
						demData[rtindex] = rtvaldat;
					}
					for (int i = dataWidth - 1; i >= dataWidth / 2; i--)
					{
						int ltindex = ((j - 1) >= 0 && (i - 1) >= 0) ? ((j - 1) * dataWidth + (i - 1)) : -1;
						int lcindex = ((i - 1) >= 0) ? (j * dataWidth + (i - 1)) : -1;
						int lbindex = (j + 1 < dataHeight && (i - 1) >= 0) ? ((j + 1) * dataWidth + (i - 1)) : -1;
						int ctindex = ((j - 1) >= 0) ? ((j - 1) * dataWidth + i) : -1;
						int index = j * dataWidth + i;
						int cbindex = ((j + 1) < dataHeight) ? ((j + 1) * dataWidth + i) : -1;
						int rtindex = ((j - 1) >= 0 && (i + 1) < dataWidth) ? ((j - 1) * dataWidth + i + 1) : -1;
						int rcindex = ((i + 1) < dataWidth) ? (j * dataWidth + i + 1) : -1;
						int rbindex = ((j + 1) < dataHeight && (i + 1) < dataWidth) ? ((j + 1) * dataWidth + i + 1) : -1;
						unsigned char val = dsmData[index];
						//char ltvaldat = (ltindex >= 0 && ltindex < dataLength) ? (val - dsmData[ltindex]) : 0;
						char lcvaldat = (lcindex >= 0 && lcindex < dataLength) ? (val - dsmData[lcindex]) : 0;
						char lbvaldat = (lbindex >= 0 && lbindex < dataLength) ? (val - dsmData[lbindex]) : 0;
						char ctvaldat = (ctindex >= 0 && ctindex < dataLength) ? (val - dsmData[ctindex]) : 0;
						char cbvaldat = (cbindex >= 0 && cbindex < dataLength) ? (val - dsmData[cbindex]) : 0;
						char rtvaldat = (rtindex >= 0 && rtindex < dataLength) ? (val - dsmData[rtindex]) : 0;
						char rcvaldat = (rcindex >= 0 && rcindex < dataLength) ? (val - dsmData[rcindex]) : 0;
						char rbvaldat = (rbindex >= 0 && rbindex < dataLength) ? (val - dsmData[rbindex]) : 0;
						unsigned char ltvaldat = val - (rtvaldat + lcvaldat + rbvaldat + ctvaldat + cbvaldat + lbvaldat + rcvaldat) / 7;
						demData[ltindex] = ltvaldat;
					}
				}
				break;
			}
			case kInt: {
				
				break;
			}
			case kUInt: {
				break;
			}
			case kFloat: {
				long dataLength = dataWidth * dataHeight;
				float* dsmData = ((float*)pDataIn);
				float* demData = ((float*)demDataTemp);
				for (int j = 0; j < dataHeight / 2; j++)
				{
					for (int i = 0; i < dataWidth / 2; i++)
					{
						int ltindex = ((j - 1)>=0 && (i - 1)>=0)?((j - 1) * dataWidth + (i - 1)):-1;
						int lcindex = ((i-1)>=0) ? (j * dataWidth + (i - 1)) : -1;
						int lbindex = (j+1< dataHeight && (i - 1) >= 0)?((j + 1) * dataWidth + (i - 1)):-1;
						int ctindex = ((j - 1)>=0)?((j - 1) * dataWidth + i):-1;
						int index = j * dataWidth + i;
						int cbindex = ((j + 1) < dataHeight)?((j + 1) * dataWidth + i):-1;
						int rtindex = ((j - 1) >= 0&&(i+1)<dataWidth) ? ((j - 1) * dataWidth + i + 1):-1;
						int rcindex = ((i + 1) < dataWidth)?(j * dataWidth + i + 1) : -1;
						int rbindex = ((j + 1) < dataHeight&& (i + 1) < dataWidth) ? ((j + 1) * dataWidth + i + 1):-1;
						float val = demData[index];
						float ltvaldat = (ltindex >= 0 && ltindex < dataLength) ? (val - demData[ltindex]) : 0;
						float lcvaldat = (lcindex >= 0 && lcindex < dataLength) ? (val - demData[lcindex]) : 0;
						float lbvaldat = (lbindex >= 0 && lbindex < dataLength) ? (val - demData[lbindex]) : 0;
						float ctvaldat = (ctindex >= 0 && ctindex < dataLength) ? (val - demData[ctindex]) : 0;
						float cbvaldat = (cbindex >= 0 && cbindex < dataLength) ? (val - demData[cbindex]) : 0;
						float rtvaldat = (rtindex >= 0 && rtindex < dataLength) ? (val - demData[rtindex]) : 0;
						float rcvaldat = (rcindex >= 0 && rcindex < dataLength) ? (val - demData[rcindex]) : 0;
						float rbvaldat = val - (ltvaldat + lcvaldat + lbvaldat + ctvaldat + cbvaldat + rtvaldat + rcvaldat) / 7;
						float tempRBvalDat = dsmData[rbindex];
						if (maxBudingHeight < (tempRBvalDat - rbvaldat))
							maxBudingHeight = tempRBvalDat - rbvaldat;
						demData[rbindex] = rbvaldat;
						
					}
					for (int i = dataWidth - 1; i >= dataWidth / 2; i--)
					{
						int ltindex = ((j - 1) >= 0 && (i - 1) >= 0) ? ((j - 1) * dataWidth + (i - 1)) : -1;
						int lcindex = ((i - 1) >= 0) ? (j * dataWidth + (i - 1)) : -1;
						int lbindex = (j + 1 < dataHeight && (i - 1) >= 0) ? ((j + 1) * dataWidth + (i - 1)) : -1;
						int ctindex = ((j - 1) >= 0) ? ((j - 1) * dataWidth + i) : -1;
						int index = j * dataWidth + i;
						int cbindex = ((j + 1) < dataHeight) ? ((j + 1) * dataWidth + i) : -1;
						int rtindex = ((j - 1) >= 0 && (i + 1) < dataWidth) ? ((j - 1) * dataWidth + i + 1) : -1;
						int rcindex = ((i + 1) < dataWidth) ? (j * dataWidth + i + 1) : -1;
						int rbindex = ((j + 1) < dataHeight && (i + 1) < dataWidth) ? ((j + 1) * dataWidth + i + 1) : -1;
						float val = demData[index];
						float ltvaldat = (ltindex >= 0 && ltindex < dataLength) ? (val - demData[ltindex]) : 0;
						float lcvaldat = (lcindex >= 0 && lcindex < dataLength) ? (val - demData[lcindex]) : 0;
						float ctvaldat = (ctindex >= 0 && ctindex < dataLength) ? (val - demData[ctindex]) : 0;
						float cbvaldat = (cbindex >= 0 && cbindex < dataLength) ? (val - demData[cbindex]) : 0;
						float rtvaldat = (rtindex >= 0 && rtindex < dataLength) ? (val - demData[rtindex]) : 0;
						float rcvaldat = (rcindex >= 0 && rcindex < dataLength) ? (val - demData[rcindex]) : 0;
						float rbvaldat = (rbindex >= 0 && rbindex < dataLength) ? (val - demData[rbindex]) : 0;
						float lbvaldat = val - (ltvaldat + lcvaldat + rbvaldat + ctvaldat + cbvaldat + rtvaldat + rcvaldat) / 7;
						float tempRBvalDat = dsmData[lbindex];
						if (maxBudingHeight < (tempRBvalDat- lbvaldat))
							maxBudingHeight = tempRBvalDat - lbvaldat;
						demData[lbindex] = lbvaldat;
					}
				}
				for (int j = dataHeight - 1; j >= dataHeight / 2; j--)
				{
					for (int i = 0; i < dataWidth / 2; i++)
					{
						int ltindex = ((j - 1) >= 0 && (i - 1) >= 0) ? ((j - 1) * dataWidth + (i - 1)) : -1;
						int lcindex = ((i - 1) >= 0) ? (j * dataWidth + (i - 1)) : -1;
						int lbindex = (j + 1 < dataHeight && (i - 1) >= 0) ? ((j + 1) * dataWidth + (i - 1)) : -1;
						int ctindex = ((j - 1) >= 0) ? ((j - 1) * dataWidth + i) : -1;
						int index = j * dataWidth + i;
						int cbindex = ((j + 1) < dataHeight) ? ((j + 1) * dataWidth + i) : -1;
						int rtindex = ((j - 1) >= 0 && (i + 1) < dataWidth) ? ((j - 1) * dataWidth + i + 1) : -1;
						int rcindex = ((i + 1) < dataWidth) ? (j * dataWidth + i + 1) : -1;
						int rbindex = ((j + 1) < dataHeight && (i + 1) < dataWidth) ? ((j + 1) * dataWidth + i + 1) : -1;
						float val = demData[index];
						float ltvaldat = (ltindex >= 0 && ltindex < dataLength) ? (val - demData[ltindex]) : 0;
						float lcvaldat = (lcindex >= 0 && lcindex < dataLength) ? (val - demData[lcindex]) : 0;
						float lbvaldat = (lbindex >= 0 && lbindex < dataLength) ? (val - demData[lbindex]) : 0;
						float ctvaldat = (ctindex >= 0 && ctindex < dataLength) ? (val - demData[ctindex]) : 0;
						float cbvaldat = (cbindex >= 0 && cbindex < dataLength) ? (val - demData[cbindex]) : 0;
						float rcvaldat = (rcindex >= 0 && rcindex < dataLength) ? (val - demData[rcindex]) : 0;
						float rbvaldat = (rbindex >= 0 && rbindex < dataLength) ? (val - demData[rbindex]) : 0;
						float rtvaldat = val - (ltvaldat + lcvaldat + rbvaldat + ctvaldat + cbvaldat + lbvaldat + rcvaldat) / 7;
						float tempRBvalDat = dsmData[lbindex];
						if (maxBudingHeight < (tempRBvalDat - rtvaldat))
							maxBudingHeight = tempRBvalDat - rtvaldat;
						demData[rtindex] = rtvaldat;
					}
					for (int i = dataWidth - 1; i >= dataWidth / 2; i--)
					{
						int ltindex = ((j - 1) >= 0 && (i - 1) >= 0) ? ((j - 1) * dataWidth + (i - 1)) : -1;
						int lcindex = ((i - 1) >= 0) ? (j * dataWidth + (i - 1)) : -1;
						int lbindex = (j + 1 < dataHeight && (i - 1) >= 0) ? ((j + 1) * dataWidth + (i - 1)) : -1;
						int ctindex = ((j - 1) >= 0) ? ((j - 1) * dataWidth + i) : -1;
						int index = j * dataWidth + i;
						int cbindex = ((j + 1) < dataHeight) ? ((j + 1) * dataWidth + i) : -1;
						int rtindex = ((j - 1) >= 0 && (i + 1) < dataWidth) ? ((j - 1) * dataWidth + i + 1) : -1;
						int rcindex = ((i + 1) < dataWidth) ? (j * dataWidth + i + 1) : -1;
						int rbindex = ((j + 1) < dataHeight && (i + 1) < dataWidth) ? ((j + 1) * dataWidth + i + 1) : -1;
						float val = demData[index];
						float lcvaldat = (lcindex >= 0 && lcindex < dataLength) ? (val - demData[lcindex]) : 0;
						float lbvaldat = (lbindex >= 0 && lbindex < dataLength) ? (val - demData[lbindex]) : 0;
						float ctvaldat = (ctindex >= 0 && ctindex < dataLength) ? (val - demData[ctindex]) : 0;
						float cbvaldat = (cbindex >= 0 && cbindex < dataLength) ? (val - demData[cbindex]) : 0;
						float rtvaldat = (rtindex >= 0 && rtindex < dataLength) ? (val - demData[rtindex]) : 0;
						float rcvaldat = (rcindex >= 0 && rcindex < dataLength) ? (val - demData[rcindex]) : 0;
						float rbvaldat = (rbindex >= 0 && rbindex < dataLength) ? (val - demData[rbindex]) : 0;
						float ltvaldat = val - (rtvaldat + lcvaldat + rbvaldat + ctvaldat + cbvaldat + lbvaldat + rcvaldat) / 7;
						float tempRBvalDat = dsmData[lbindex];
						if (maxBudingHeight < (tempRBvalDat - ltvaldat))
							maxBudingHeight = tempRBvalDat - ltvaldat;
						demData[ltindex] = ltvaldat;
					}
				}
				break;
			}
			case kDouble: {
				break;
			}
			case kShort: {
				break;
			}
			case kUnknown:
			default:
				break;
			}
			
		}

		void* _getDemOriginValFromDsmData(void* pDataIn, DataType dataTypeIn, int dataWidth, int dataHeight)
		{
			void* res = NULL;
			switch (dataTypeIn) {
				case kUByte: {
					unsigned char* demVal = new unsigned char[dataWidth*dataHeight];
					unsigned char maxVal = (std::numeric_limits<unsigned char>::max)();
					for (int j = 0; j < dataHeight; j++) {
						for (int i = 0; i < dataWidth; i++) {
							int index = j * dataWidth + i;
							if (i < 2) {
								demVal[index] = ((unsigned char*)pDataIn)[index];
							}
							else if (i > dataWidth - 3) {
								demVal[index] = ((unsigned char*)pDataIn)[index];
							}
							else if (j < 2) {
								demVal[index] = ((unsigned char*)pDataIn)[index];
							}
							else if (j > dataHeight - 3) {
								demVal[index] = ((unsigned char*)pDataIn)[index];
							}
							else {
								demVal[index] = maxVal;
							}
						}
					}
					res = demVal;
					break;
				}
				case kInt: {
					int* demVal = new int[dataWidth*dataHeight];
					int maxVal = (std::numeric_limits<int>::max)();
					for (int j = 0; j < dataHeight; j++) {
						for (int i = 0; i < dataWidth; i++) {
							int index = j * dataWidth + i;
							if (i < 2) {
								demVal[index] = ((int*)pDataIn)[index];
							}
							else if (i > dataWidth - 3) {
								demVal[index] = ((int*)pDataIn)[index];
							}
							else if (j < 2) {
								demVal[index] = ((int*)pDataIn)[index];
							}
							else if (j > dataHeight - 3) {
								demVal[index] = ((int*)pDataIn)[index];
							}
							else {
								demVal[index] = maxVal;
							}
						}
					}
					res = demVal;
					break;
				}
				case kUInt: {
					unsigned int* demVal = new unsigned int[dataWidth*dataHeight];
					unsigned int maxVal = (std::numeric_limits<unsigned int>::max)();
					for (int j = 0; j < dataHeight; j++) {
						for (int i = 0; i < dataWidth; i++) {
							int index = j * dataWidth + i;
							if (i < 2) {
								demVal[index] = ((unsigned int*)pDataIn)[index];
							}
							else if (i > dataWidth - 3) {
								demVal[index] = ((unsigned int*)pDataIn)[index];
							}
							else if (j < 2) {
								demVal[index] = ((unsigned int*)pDataIn)[index];
							}
							else if (j > dataHeight - 3) {
								demVal[index] = ((unsigned int*)pDataIn)[index];
							}
							else {
								demVal[index] = maxVal;
							}
						}
					}
					res = demVal;
					break;
				}
				case kFloat: {
					float* demVal = new float[dataWidth*dataHeight];
					float maxVal = (std::numeric_limits<float>::max)();
					for (int j = 0; j < dataHeight; j++) {
						for (int i = 0; i < dataWidth; i++) {
							int index = j * dataWidth + i;
							if (i < 2) {
								demVal[index] = ((float*)pDataIn)[index];
							}
							else if (i > dataWidth - 3) {
								demVal[index] = ((float*)pDataIn)[index];
							}
							else if (j < 2) {
								demVal[index] = ((float*)pDataIn)[index];
							}
							else if (j > dataHeight - 3) {
								demVal[index] = ((float*)pDataIn)[index];
							}
							else {
								demVal[index] = maxVal;
							}
						}
					}
					res = demVal;
					break;
				}
				case kDouble: {
					double* demVal = new double[dataWidth*dataHeight];
					double maxVal = (std::numeric_limits<double>::max)();
					for (int j = 0; j < dataHeight; j++) {
						for (int i = 0; i < dataWidth; i++) {
							int index = j * dataWidth + i;
							if (i < 2) {
								demVal[index] = ((double*)pDataIn)[index];
							}
							else if (i > dataWidth - 3) {
								demVal[index] = ((double*)pDataIn)[index];
							}
							else if (j < 2) {
								demVal[index] = ((double*)pDataIn)[index];
							}
							else if (j > dataHeight - 3) {
								demVal[index] = ((double*)pDataIn)[index];
							}
							else {
								demVal[index] = maxVal;
							}
						}
					}
					res = demVal;
					break;
				}
				case kShort: {
					short* demVal = new short[dataWidth*dataHeight];
					short maxVal = (std::numeric_limits<short>::max)();
					for (int j = 0; j < dataHeight; j++) {
						for (int i = 0; i < dataWidth; i++) {
							int index = j * dataWidth + i;
							if (i < 2) {
								demVal[index] = ((short*)pDataIn)[index];
							}
							else if (i > dataWidth - 3) {
								demVal[index] = ((short*)pDataIn)[index];
							}
							else if (j < 2) {
								demVal[index] = ((short*)pDataIn)[index];
							}
							else if (j > dataHeight - 3) {
								demVal[index] = ((short*)pDataIn)[index];
							}
							else {
								demVal[index] = maxVal;
							}
						}
					}
					res = demVal;
					break;
				}
				case kUnknown:
				default:
					break;
						
			}
			return res;
		}

		void _getDSMDataByShpExtent(const osg::Vec4& minMaxXY, double& buildingHeight)
		{
			GDALRasterBand* gdalBand = _ds->GetRasterBand(1);
			GDALRasterBand* gdalDEMBand = m_demDataset->GetRasterBand(1);
			GDALDataType dataType = _ds->GetRasterBand(1)->GetRasterDataType();
			int dataWidth = 0;
			int dataHeight = 0;
			int dataStartX = 0;
			int dataStartY = 0;
			double minShpX = minMaxXY.x();
			double minShpY = minMaxXY.y();
			double maxShpX = minMaxXY.z();
			double maxShpY = minMaxXY.w();

			dataStartX = (minShpX - minDSMX) / _dsmTrans[1] - 2;
			dataStartY = (minShpY - minDSMY) / _dsmTrans[5] - 2;
			if (bReverseY)
			{
				dataStartY = (maxDSMY - maxShpY) / abs(_dsmTrans[5]) + 2;
			}
			dataWidth = (maxShpX - minShpX) / _dsmTrans[1] + 4;
			dataHeight = (maxShpY - minShpY) / abs(_dsmTrans[5]) + 4;
			switch (dataType) {
				case GDT_Byte: {
					unsigned char* pDataBuf = new unsigned char[dataWidth * dataHeight];
					CPLErr err = gdalBand->RasterIO(GF_Read, dataStartX, dataStartY, dataWidth, dataHeight, pDataBuf,
						dataWidth, dataHeight, GDT_Byte, 0, 0);
					if (err != CPLErr::CE_None) {
						std::string errMsg = "Read raster data file failed: " ;
						log(osg::NOTICE, "ConvertDSMTODEMByShpFile::_getDSMDataByShpExtent(%s)", errMsg);
						for (int i = 0; i < dataWidth * dataHeight; ++i) {
							pDataBuf[i] = (unsigned char)0;
						}
					}
					
					void* demData = _getDemOriginValFromDsmData(pDataBuf, DataType::kUByte, dataWidth, dataHeight);
					_computeDEMData(pDataBuf, demData, DataType::kUByte,dataWidth,dataHeight, buildingHeight);
					break;
				}
				case GDT_Int16: {
					short* pDataBuf = new short[dataWidth * dataHeight];
					CPLErr err = gdalBand->RasterIO(GF_Read, dataStartX, dataStartY, dataWidth, dataHeight, pDataBuf,
						dataWidth, dataHeight, GDT_Int16, 0, 0);
					if (err != CPLErr::CE_None) {
						std::string errMsg = "Read raster data file failed: ";
						log(osg::NOTICE, "ConvertDSMTODEMByShpFile::_getDSMDataByShpExtent(%s)", errMsg);
						for (int i = 0; i < dataWidth * dataHeight; ++i) {
							pDataBuf[i] = (short)0;
						}
					}
					void* demData = _getDemOriginValFromDsmData(pDataBuf, DataType::kShort, dataWidth, dataHeight);
					_computeDEMData(pDataBuf, demData, DataType::kShort, dataWidth, dataHeight, buildingHeight);

					break;
				}
				case GDT_Int32: {
					int* pDataBuf = new int[dataWidth * dataHeight];
					CPLErr err = gdalBand->RasterIO(GF_Read, dataStartX, dataStartY, dataWidth, dataHeight, pDataBuf,
						dataWidth, dataHeight, GDT_Int32, 0, 0);
					if (err != CPLErr::CE_None) {
						std::string errMsg = "Read raster data file failed: ";
						log(osg::NOTICE, "ConvertDSMTODEMByShpFile::_getDSMDataByShpExtent(%s)", errMsg);
						for (int i = 0; i < dataWidth * dataHeight; ++i) {
							pDataBuf[i] = 0;
						}
					}
					void* demData = _getDemOriginValFromDsmData(pDataBuf, DataType::kInt, dataWidth, dataHeight);
					_computeDEMData(pDataBuf, demData, DataType::kInt, dataWidth, dataHeight, buildingHeight);
					break;
				}
				case GDT_UInt32: {
					unsigned int* pDataBuf = new unsigned int[dataWidth * dataHeight];
					CPLErr err = gdalBand->RasterIO(GF_Read, dataStartX, dataStartY, dataWidth, dataHeight, pDataBuf,
						dataWidth, dataHeight, GDT_UInt32, 0, 0);
					if (err != CPLErr::CE_None) {
						std::string errMsg = "Read raster data file failed: ";
						log(osg::NOTICE, "ConvertDSMTODEMByShpFile::_getDSMDataByShpExtent(%s)", errMsg);
						for (int i = 0; i < dataWidth * dataHeight; ++i) {
							pDataBuf[i] = (unsigned int)0;
						}
					}
					void* demData = _getDemOriginValFromDsmData(pDataBuf, DataType::kUInt, dataWidth, dataHeight);
					_computeDEMData(pDataBuf, demData, DataType::kUInt, dataWidth, dataHeight, buildingHeight);
					break;
				}
				case GDT_Float64: {
					double* pDataBuf = new double[dataWidth * dataHeight];
					CPLErr err = gdalBand->RasterIO(GF_Read, dataStartX, dataStartY, dataWidth, dataHeight, pDataBuf,
						dataWidth, dataHeight, GDT_Float64, 0, 0);
					if (err != CPLErr::CE_None) {
						std::string errMsg = "Read raster data file failed: ";
						log(osg::NOTICE, "ConvertDSMTODEMByShpFile::_getDSMDataByShpExtent(%s)", errMsg);
						for (int i = 0; i < dataWidth * dataHeight; ++i) {
							pDataBuf[i] = 0.0;
						}
					}
					void* demData = _getDemOriginValFromDsmData(pDataBuf, DataType::kDouble, dataWidth, dataHeight);
					_computeDEMData(pDataBuf, demData, DataType::kDouble, dataWidth, dataHeight, buildingHeight);
					break;
				}
				case GDT_Float32: {
					float* pDataBuf = new float[dataWidth * dataHeight];
					CPLErr err = gdalBand->RasterIO(GF_Read, dataStartX, dataStartY, dataWidth, dataHeight, pDataBuf,
						dataWidth, dataHeight, GDT_Float32, 0, 0);
					if (err != CPLErr::CE_None) {
						std::string errMsg = "Read raster data file failed: ";
						log(osg::NOTICE, "ConvertDSMTODEMByShpFile::_getDSMDataByShpExtent(%s)", errMsg);
						for (int i = 0; i < dataWidth * dataHeight; ++i) {
							pDataBuf[i] = 0.0f;
						}
					}
					void* demData = _getDemOriginValFromDsmData(pDataBuf, DataType::kFloat, dataWidth, dataHeight);
					_computeDEMData(pDataBuf, demData, DataType::kFloat, dataWidth, dataHeight, buildingHeight);
					CPLErr errW = gdalDEMBand->RasterIO(GF_Write, dataStartX, dataStartY, dataWidth, dataHeight, demData,
						dataWidth, dataHeight, GDT_Float32, 0, 0);
					if (errW != CPLErr::CE_None) {
						std::string errMsg = "Write raster data file failed: ";
						log(osg::NOTICE, "ConvertDSMTODEMByShpFile::_getDSMDataByShpExtent(%s)", errMsg);
					}
					gdalDEMBand->FlushCache();//要保存至文件需要调用poBand->FlushCache()语句
					delete[] pDataBuf;
					delete[] demData;
					break;
				}
				default: {
					std::string errMsg = "data type not support error:  ";
					log(osg::NOTICE, "ConvertDSMTODEMByShpFile::_getDSMDataByShpExtent(%s)", errMsg);
					break;
				}
			}


			//tiff->addBand(geoBand);
		}


		
        virtual void apply(osg::Geode& node)
        {
            const osg::BoundingBox & bb = node.getBoundingBox();
            
            // ** if sub-graph overlap the HeightField
            if (!bTraversed && overlap(bb.xMin(), bb.yMin(), bb.xMax(), bb.yMax()))
            {
                unsigned int numDrawables = node.getNumDrawables();
				for (unsigned int i = 0; i < numDrawables; ++i)
				{
					// ** get the geometry
					osg::Drawable * drawable = node.getDrawable(i);
					if (drawable == NULL) continue;
					osg::Geometry * geom = drawable->asGeometry();
					if (geom == NULL) continue;
					if (geom->getPrimitiveSet(0)->getMode() != osg::PrimitiveSet::POLYGON) continue;

					SetZeroToZVisitor daif;
					geom->getVertexArray()->accept(daif);
					osg::Vec4 minMaxXY = daif.m_minMaxXYArray;
					double buildingHeight = 0.0;
					_getDSMDataByShpExtent(minMaxXY, buildingHeight);
					double dbb = buildingHeight + 111;
					//}
				}
				GDALClose(_ds);
				GDALClose(m_demDataset);
            }
        }

		double getGeoX(int pixelX) const
		{
			return _dsmTrans[0] + pixelX * _dsmTrans[1];
		}
		double getGeoY(int pixelY) const
		{
			return _dsmTrans[3] + pixelY * _dsmTrans[5];
		}
    
        bool overlap(double xMin, double yMin, double xMax, double yMax)
        {
			_ds->GetGeoTransform(_dsmTrans);
			_dsmWidth = _ds->GetRasterXSize();
			_dsmHeight = _ds->GetRasterYSize();
			minDSMX = getGeoX(0);
			maxDSMX = getGeoX(_dsmWidth);
			minDSMY = getGeoY(0);
			maxDSMY = getGeoY(_dsmHeight);
			if (minDSMY > maxDSMY)
			{
				bReverseY = true;
				double temp = minDSMY;
				minDSMY = maxDSMY;
				maxDSMY = temp;
			}
            if ((minDSMX > xMax) || (xMin > maxDSMX) || (minDSMY > yMax) || (yMin > maxDSMY))
                return false;
            else
                return true;
        }
        
        bool overlap(const osg::BoundingSphere & bs)
        {
            double xMin = bs.center().x() - bs.radius();
            double yMin = bs.center().y() - bs.radius();
            double xMax = bs.center().x() + bs.radius();
            double yMax = bs.center().y() + bs.radius();
            return overlap(xMin, yMin, xMax, yMax);
        }
        
        osg::Node * getCreatedModel() { return _createdModel.get(); }
        
    private:
        
        /*osg::HeightField & _hf;*/
		GDALDataset*  _ds;
		GDALDataset *m_demDataset;
		double _dsmTrans[6] = {0};
		int _dsmWidth = 0;
		int _dsmHeight = 0;
		
		bool bTraversed = false;
		bool bReverseY = false;
        osg::ref_ptr<osg::Node> _createdModel;
		double minDSMX;
		double maxDSMX;
		double minDSMY;
		double maxDSMY;
};



ShapeFileProcessor::~ShapeFileProcessor()
{

	//GDALClose(m_dsmDS);
}


bool ShapeFileProcessor::processor(osg::Node* model, const std::string& dsmFilePath, const std::string& demFilePath)
{
	bool bsuccess = false;
    log(osg::NOTICE,"ShapeFileProcessor::processor(%s)",model->getName().c_str());
	
	m_dsmDS = _processordsmFilePath(dsmFilePath);
	if (m_dsmDS)
	{
		GDALDataset *demDataset;   //GDAL数据集
		demDataset = (GDALDataset *)GDALOpen(demFilePath.c_str(), GA_Update);
		if (demDataset != NULL)
		{
			ConvertDSMTODEMByShpFile shapePlacer(m_dsmDS, demDataset);
			model->accept(shapePlacer);
			bsuccess = true;
		}
	}
	return bsuccess;
}



GDALDataset* ShapeFileProcessor::_processordsmFilePath(const std::string& dsmFilePath)
{
	GDALAllRegister();
	GDALDataset* poDS = nullptr;
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	
	poDS = (GDALDataset*)GDALOpenEx(dsmFilePath.c_str(), GDAL_OF_RASTER, nullptr, nullptr, nullptr);
	if (!poDS) {
		log(osg::NOTICE, "Open TIFF:(%s) error", dsmFilePath.c_str());
		return NULL;
	}
	//std::string projectStr = poDS->GetProjectionRef();
	//int width = poDS->GetRasterXSize();
	//int height = poDS->GetRasterYSize();
	int bandNum = poDS->GetRasterCount();

	if (bandNum != 1) {
		GDALClose(poDS);
		return NULL;
	}
	return poDS;
	
}


