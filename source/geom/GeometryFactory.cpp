/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.29  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.28  2003/10/15 16:39:03  strk
 * Made Edge::getCoordinates() return a 'const' value. Adapted code set.
 *
 * Revision 1.27  2003/10/14 15:58:51  strk
 * Useless vector<Geometry *> leaking allocations removed
 *
 * Revision 1.26  2003/10/11 01:56:08  strk
 *
 * Code base padded with 'const' keywords ;)
 *
 * Revision 1.25  2003/10/09 15:35:13  strk
 * added 'const' keyword to GeometryFactory constructor, Log on top of geom.h
 *
 * Revision 1.24  2003/10/09 10:14:06  strk
 * just a style change in top Log comment.
 *
 **********************************************************************/


#include "../headers/geom.h"
#include "../headers/util.h"
#include <typeinfo>

namespace geos {

GeometryFactory::GeometryFactory() {
	precisionModel=new PrecisionModel();
	SRID=0;
}

/**
* Create a geometry factory using given precision model and srid.
* Will make a copy of given PrecisionModel object, so callers
* can safely delete it afterwards.
*/
GeometryFactory::GeometryFactory(const PrecisionModel* pm, int newSRID){
    precisionModel=new PrecisionModel(*pm);
    SRID=newSRID;
}
GeometryFactory::~GeometryFactory(){
	delete precisionModel;
}
  
Point*
GeometryFactory::createPointFromInternalCoord(const Coordinate* coord,
		const Geometry *exemplar)
{
	Point *ret;
	const PrecisionModel *pm = exemplar->getPrecisionModel();
	Coordinate newcoord = *coord;
	pm->makePrecise(&newcoord);
	ret = new Point(newcoord,pm, exemplar->getSRID());
	return ret;
}


Geometry* GeometryFactory::toGeometry(Envelope* envelope,PrecisionModel* precisionModel,int SRID) {
	if (envelope->isNull()) {
		return new Point(*(new Coordinate()),precisionModel,SRID);
	}
	if (envelope->getMinX()==envelope->getMaxX() && envelope->getMinY()==envelope->getMaxY()) {
		return new Point(*(new Coordinate(envelope->getMinX(),envelope->getMinY())),precisionModel,SRID);
	}
	CoordinateList *cl=CoordinateListFactory::internalFactory->createCoordinateList();
	cl->add(*(new Coordinate(envelope->getMinX(), envelope->getMinY())));
	cl->add(*(new Coordinate(envelope->getMaxX(), envelope->getMinY())));
	cl->add(*(new Coordinate(envelope->getMaxX(), envelope->getMaxY())));
	cl->add(*(new Coordinate(envelope->getMinX(), envelope->getMaxY())));
	cl->add(*(new Coordinate(envelope->getMinX(), envelope->getMinY())));
	return new Polygon(new LinearRing(cl,precisionModel,SRID),precisionModel,SRID);
}

PrecisionModel* GeometryFactory::getPrecisionModel() const {
	return precisionModel;
}

Point* GeometryFactory::createPoint(const Coordinate& coordinate){
	return new Point(coordinate,precisionModel,SRID);
}

MultiLineString* GeometryFactory::createMultiLineString(vector<Geometry *> *lineStrings){
	return new MultiLineString(lineStrings,precisionModel,SRID);
}

GeometryCollection* GeometryFactory::createGeometryCollection(vector<Geometry *> *geometries){
	return new GeometryCollection(geometries,precisionModel,SRID);
}

MultiPolygon* GeometryFactory::createMultiPolygon(vector<Geometry *> *polygons){
	return new MultiPolygon(polygons,precisionModel,SRID);
}

LinearRing* GeometryFactory::createLinearRing(CoordinateList* coordinates) {
	if (coordinates->getSize()>0 && 
		!coordinates->getAt(0).equals2D(coordinates->getAt(coordinates->getSize() - 1))) {
			delete precisionModel;
			throw new IllegalArgumentException("LinearRing not closed");
	}
	return new LinearRing(coordinates, precisionModel, SRID);
}

MultiPoint* GeometryFactory::createMultiPoint(vector<Geometry *> *point) {
	return new MultiPoint(point,precisionModel,SRID);
}

MultiPoint* GeometryFactory::createMultiPoint(CoordinateList* coordinates) {
	vector<Geometry *> *pts=new vector<Geometry *>();
	for (int i=0; i<coordinates->getSize(); i++) {
		Point *pt=createPoint(coordinates->getAt(i));
		pts->push_back(pt);
	}
	delete coordinates;
	MultiPoint *mp = createMultiPoint(pts);
	delete pts;
	return mp;
}

Polygon* GeometryFactory::createPolygon(LinearRing *shell, vector<Geometry *> *holes) {
	return new Polygon(shell, holes, precisionModel, SRID);
}

LineString* GeometryFactory::createLineString(const CoordinateList* coordinates) {
	return new LineString(coordinates, precisionModel, SRID);
}

Geometry* GeometryFactory::buildGeometry(vector<Geometry *> *geoms) {
	string geomClass("NULL");
	bool isHeterogeneous=false;
	bool isCollection=geoms->size()>1;
	unsigned int i;
    
	for (i=0; i<geoms->size(); i++) {
		string partClass(typeid(*(*geoms)[i]).name());
		if (geomClass=="NULL") {
			geomClass=partClass;
		}
		if (partClass!=geomClass) {
			isHeterogeneous = true;
		}
	}
    // for the empty geometry, return an empty GeometryCollection
	if (geomClass=="NULL") {
		return createGeometryCollection(NULL);
	}
	if (isHeterogeneous) {
		return createGeometryCollection(geoms);
	}
	Geometry *geom0=(*geoms)[0];
	if (isCollection) {
		if (typeid(*geom0)==typeid(Polygon)) {
			return createMultiPolygon(geoms);
		} else if (typeid(*geom0)==typeid(LineString)) {
			return createMultiLineString(geoms);
		} else if (typeid(*geom0)==typeid(Point)) {
			return createMultiPoint(geoms);
		}
		Assert::shouldNeverReachHere();
	}
	if (typeid(*geom0)==typeid(Polygon))
		return (Polygon*) geom0;
	// LineString also handles LinearRings
	else if (typeid(*geom0)==typeid(LineString))
		return (LineString*) geom0;
	else if (typeid(*geom0)==typeid(Point))
		return (Point*) geom0;
	else if (typeid(*geom0)==typeid(MultiPoint))
		return (MultiPoint*) geom0;
	else if (typeid(*geom0)==typeid(MultiLineString))
		return (MultiLineString*) geom0;
	else if (typeid(*geom0)==typeid(MultiPolygon))
		return (MultiPolygon*) geom0;
	else if (typeid(*geom0)==typeid(GeometryCollection))
		return (GeometryCollection*) geom0;
	else 
		return geom0;
}

}

