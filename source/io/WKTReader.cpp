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
 * Revision 1.17  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.16  2003/10/15 08:52:55  strk
 * Memory leaks fixed.
 *
 **********************************************************************/


#include "../headers/io.h"
#include "../headers/util.h"

namespace geos {

WKTReader::WKTReader(){
	geometryFactory=new GeometryFactory();
	precisionModel=new PrecisionModel();
}
WKTReader::WKTReader(GeometryFactory *gf){
	geometryFactory=gf;
	precisionModel=new PrecisionModel(*(gf->getPrecisionModel()));
}
WKTReader::~WKTReader(){
	delete geometryFactory;
	delete precisionModel;
}

Geometry* WKTReader::read(string wellKnownText){
	auto_ptr<StringTokenizer> tokenizer(new StringTokenizer(wellKnownText));
	try {
		StringTokenizer *st=tokenizer.release();
		Geometry *g=readGeometryTaggedText(st);
		delete st;
		return g;
	}
	catch (ParseException *e) {
		throw new ParseException(e->toString());
	}
}

CoordinateList* WKTReader::getCoordinates(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return CoordinateListFactory::internalFactory->createCoordinateList();
	}
	CoordinateList *coordinates=CoordinateListFactory::internalFactory->createCoordinateList();
	Coordinate* externalCoordinate=new Coordinate();
	Coordinate* internalCoordinate=new Coordinate();
	Coordinate *c;
	externalCoordinate->x=getNextNumber(tokenizer);
	externalCoordinate->y=getNextNumber(tokenizer);
	precisionModel->toInternal(*externalCoordinate,internalCoordinate);
	c=new Coordinate(*internalCoordinate);
	coordinates->add(*c);
	delete c;
	nextToken=getNextCloserOrComma(tokenizer);
	while (nextToken==",") {
		externalCoordinate->x=getNextNumber(tokenizer);
		externalCoordinate->y=getNextNumber(tokenizer);
		precisionModel->toInternal(*externalCoordinate,internalCoordinate);
		c=new Coordinate(*internalCoordinate);
		coordinates->add(*c);
		delete c;
		nextToken=getNextCloserOrComma(tokenizer);
	}
	delete externalCoordinate;
	delete internalCoordinate;
	return coordinates;
}

double WKTReader::getNextNumber(StringTokenizer *tokenizer) {
	int type=tokenizer->nextToken();
	switch(type){
		case StringTokenizer::TT_EOF:
			throw new ParseException("Expected number but encountered end of stream");
		case StringTokenizer::TT_EOL:
			throw new ParseException("Expected number but encountered end of line");
		case StringTokenizer::TT_NUMBER:
			return tokenizer->getNVal();
		case StringTokenizer::TT_WORD:
			throw new ParseException("Expected number but encountered word",tokenizer->getSVal());
		case '(':
			throw new ParseException("Expected number but encountered '('");
		case ')':
			throw new ParseException("Expected number but encountered ')'");
		case ',':
			throw new ParseException("Expected number but encountered ','");
	}
	Assert::shouldNeverReachHere("Encountered unexpected StreamTokenizer type");
	return 0;
}

string WKTReader::getNextEmptyOrOpener(StringTokenizer *tokenizer) {
	string nextWord=getNextWord(tokenizer);
	if (nextWord=="EMPTY" || nextWord=="(") {
		return nextWord;
	}
	throw new ParseException("Expected 'EMPTY' or '(' but encountered ",nextWord);
}

string WKTReader::getNextCloserOrComma(StringTokenizer *tokenizer) {
	string nextWord=getNextWord(tokenizer);
	if (nextWord=="," || nextWord==")") {
		return nextWord;
	}
	throw new ParseException("Expected ')' or ',' but encountered",nextWord);
}

string WKTReader::getNextCloser(StringTokenizer *tokenizer) {
	string nextWord=getNextWord(tokenizer);
	if (nextWord==")") {
		return nextWord;
	}
	throw new ParseException("Expected ')' but encountered",nextWord);
}

string WKTReader::getNextWord(StringTokenizer *tokenizer) {
	int type=tokenizer->nextToken();
	switch(type){
		case StringTokenizer::TT_EOF:
			throw new ParseException("Expected word but encountered end of stream");
		case StringTokenizer::TT_EOL:
			throw new ParseException("Expected word but encountered end of line");
		case StringTokenizer::TT_NUMBER:
			throw new ParseException("Expected word but encountered number", tokenizer->getNVal());
		case StringTokenizer::TT_WORD:
			return tokenizer->getSVal();
		case '(':
			return "(";
		case ')':
			return ")";
		case ',':
			return ",";
	}
	Assert::shouldNeverReachHere("Encountered unexpected StreamTokenizer type");
	return "";
}

Geometry* WKTReader::readGeometryTaggedText(StringTokenizer *tokenizer) {
	string type = getNextWord(tokenizer);
	if (type=="POINT") {
		return readPointText(tokenizer);
	} else if (type=="LINESTRING") {
		return readLineStringText(tokenizer);
	} else if (type=="POLYGON") {
		return readPolygonText(tokenizer);
	} else if (type=="MULTIPOINT") {
		return readMultiPointText(tokenizer);
	} else if (type=="MULTILINESTRING") {
		return readMultiLineStringText(tokenizer);
	} else if (type=="MULTIPOLYGON") {
		return readMultiPolygonText(tokenizer);
	} else if (type=="GEOMETRYCOLLECTION") {
		return readGeometryCollectionText(tokenizer);
	}
	throw new ParseException("Unknown type",type);
}

Point* WKTReader::readPointText(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return geometryFactory->createPoint(Coordinate::getNull());
	}
	double x=getNextNumber(tokenizer);
	double y=getNextNumber(tokenizer);
	Coordinate* externalCoordinate=new Coordinate(x, y);
	Coordinate* internalCoordinate=new Coordinate();
	precisionModel->toInternal(*externalCoordinate,internalCoordinate);
	getNextCloser(tokenizer);
	Point *pt=geometryFactory->createPoint(*internalCoordinate);
	delete externalCoordinate;
	delete internalCoordinate;
	return pt;
}

LineString* WKTReader::readLineStringText(StringTokenizer *tokenizer) {
	CoordinateList *coords = getCoordinates(tokenizer);
	LineString *ret = geometryFactory->createLineString(coords);
	delete coords;
	return ret;
}

LinearRing* WKTReader::readLinearRingText(StringTokenizer *tokenizer) {
	CoordinateList *coords = getCoordinates(tokenizer);
	LinearRing *ret = geometryFactory->createLinearRing(coords);
	delete coords;
	return ret;
}

MultiPoint* WKTReader::readMultiPointText(StringTokenizer *tokenizer) {
	CoordinateList *coords = getCoordinates(tokenizer);
	MultiPoint *ret = geometryFactory->createMultiPoint(coords);
	// This is INCONSISTENT. for every other calls to 
	// geometryFactory->createSOMTING(ARG) you can delete
	// ARG afterwards, for this single call you can not. --strk;
	// TODO: make createMultiPoint() copy given arg.
	//delete coords;
	return ret;
}

Polygon* WKTReader::readPolygonText(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return geometryFactory->createPolygon(NULL,NULL);
	}
	vector<Geometry *> *holes=new vector<Geometry *>();
	LinearRing *shell=readLinearRingText(tokenizer);
	nextToken=getNextCloserOrComma(tokenizer);
	while(nextToken==",") {
		LinearRing *hole=readLinearRingText(tokenizer);
		holes->push_back(hole);
		nextToken=getNextCloserOrComma(tokenizer);
	}
	return geometryFactory->createPolygon(shell,holes);
}

MultiLineString* WKTReader::readMultiLineStringText(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return geometryFactory->createMultiLineString(NULL);
	}
	vector<Geometry *> *lineStrings=new vector<Geometry *>();
	LineString *lineString=readLineStringText(tokenizer);
	lineStrings->push_back(lineString);
	nextToken=getNextCloserOrComma(tokenizer);
	while(nextToken==",") {
		LineString *lineString=readLineStringText(tokenizer);
		lineStrings->push_back(lineString);
		nextToken=getNextCloserOrComma(tokenizer);
	}
	MultiLineString *ret = geometryFactory->createMultiLineString(lineStrings);
	delete lineStrings;
	return ret;
}

MultiPolygon* WKTReader::readMultiPolygonText(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return geometryFactory->createMultiPolygon(NULL);
	}
	vector<Geometry *> *polygons=new vector<Geometry *>();
	Polygon *polygon=readPolygonText(tokenizer);
	polygons->push_back(polygon);
	nextToken=getNextCloserOrComma(tokenizer);
	while(nextToken==",") {
		Polygon *polygon=readPolygonText(tokenizer);
		polygons->push_back(polygon);
		nextToken=getNextCloserOrComma(tokenizer);
	}
	MultiPolygon *ret = geometryFactory->createMultiPolygon(polygons);
	delete polygons;
	return ret;
}

GeometryCollection* WKTReader::readGeometryCollectionText(StringTokenizer *tokenizer) {
	string nextToken=getNextEmptyOrOpener(tokenizer);
	if (nextToken=="EMPTY") {
		return geometryFactory->createGeometryCollection(NULL);
	}
	vector<Geometry *> *geoms=new vector<Geometry *>();
	Geometry *geom;
	geom=readGeometryTaggedText(tokenizer);
	geoms->push_back(geom);
	nextToken=getNextCloserOrComma(tokenizer);
	while(nextToken==",") {
		geom=readGeometryTaggedText(tokenizer);
		geoms->push_back(geom);
		nextToken=getNextCloserOrComma(tokenizer);
	}
	GeometryCollection *ret = geometryFactory->createGeometryCollection(geoms);
	delete geoms;
	return ret;
}
}

