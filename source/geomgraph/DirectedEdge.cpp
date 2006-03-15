/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <cmath>
#include <cassert>
#include <string>

#include <geos/util/TopologyException.h>
#include <geos/geomgraph/Edge.h>
#include <geos/geomgraph/DirectedEdge.h>
#include <geos/geom/Location.h>
#include <geos/geomgraph/Label.h>
#include <geos/geomgraph/Position.h>

//using namespace std;
using namespace geos::geom;

#ifndef USE_INLINE
# include "geos/geomgraph/DirectedEdge.inl"
#endif

namespace geos {
namespace geomgraph { // geos.geomgraph

/* public static */
int
DirectedEdge::depthFactor(int currLocation, int nextLocation)
{
	if (currLocation==Location::EXTERIOR && nextLocation==Location::INTERIOR)
		return 1;
	else if (currLocation==Location::INTERIOR && nextLocation==Location::EXTERIOR)
		return -1;
	return 0;
}

#if 0
DirectedEdge::DirectedEdge():
	EdgeEnd(),
	isInResultVar(false),
	isVisitedVar(false),
	sym(NULL),
	next(NULL),
	nextMin(NULL),
	edgeRing(NULL),
	minEdgeRing(NULL)
{
	depth[0]=0;
	depth[1]=-999;
	depth[2]=-999;

}
#endif

DirectedEdge::DirectedEdge(Edge *newEdge, bool newIsForward):
	EdgeEnd(newEdge),
	isForwardVar(newIsForward),
	isInResultVar(false),
	isVisitedVar(false),
	sym(NULL),
	next(NULL),
	nextMin(NULL),
	edgeRing(NULL),
	minEdgeRing(NULL)
{
	depth[0]=0;
	depth[1]=-999;
	depth[2]=-999;

	assert(newEdge->getNumPoints() >= 2);

	if (isForwardVar) {
		init(edge->getCoordinate(0), edge->getCoordinate(1));
	} else {
		int n=edge->getNumPoints()-1;
		init(edge->getCoordinate(n), edge->getCoordinate(n-1));
	}
	computeDirectedLabel();
}


/*public*/
void
DirectedEdge::setDepth(int position, int newDepth)
{
	if (depth[position]!=-999) {
		if (depth[position]!=newDepth)
			throw util::TopologyException("assigned depths do not match", &getCoordinate());
			//Assert.isTrue(depth[position] == depthVal, "assigned depths do not match at " + getCoordinate());
	}
	depth[position]=newDepth;
}

/*public*/
int
DirectedEdge::getDepthDelta()
{
	int depthDelta=edge->getDepthDelta();
	if (!isForwardVar) depthDelta=-depthDelta;
	return depthDelta;
}

/*public*/
void
DirectedEdge::setVisitedEdge(bool newIsVisited)
{
	setVisited(newIsVisited);
	sym->setVisited(newIsVisited);
}


/*public*/
bool
DirectedEdge::isLineEdge()
{
	bool isLine=label->isLine(0) || label->isLine(1);
	bool isExteriorIfArea0=!label->isArea(0) || label->allPositionsEqual(0,Location::EXTERIOR);
	bool isExteriorIfArea1=!label->isArea(1) || label->allPositionsEqual(1,Location::EXTERIOR);
	return isLine && isExteriorIfArea0 && isExteriorIfArea1;
}

/*public*/
bool
DirectedEdge::isInteriorAreaEdge()
{
	bool isInteriorAreaEdge=true;
	for (int i=0; i<2; i++) {
		if (!(label->isArea(i)
			&& label->getLocation(i,Position::LEFT )==Location::INTERIOR
			&& label->getLocation(i,Position::RIGHT)==Location::INTERIOR)) {
				isInteriorAreaEdge=false;
		}
	}
	return isInteriorAreaEdge;
}

/*private*/
void
DirectedEdge::computeDirectedLabel()
{
	delete label;
	label=new Label(*(edge->getLabel()));
	if (!isForwardVar)
		label->flip();
}

/*public*/
void
DirectedEdge::setEdgeDepths(int position, int newDepth)
{
	// get the depth transition delta from R to L for this directed Edge
	int depthDelta=getEdge()->getDepthDelta();
	if (!isForwardVar) depthDelta=-depthDelta;
	// if moving from L to R instead of R to L must change sign of delta
	int directionFactor=1;
	if (position==Position::LEFT)
		directionFactor=-1;
	int oppositePos=Position::opposite(position);
	int delta=depthDelta*directionFactor;
	//TESTINGint delta = depthDelta * DirectedEdge.depthFactor(loc, oppositeLoc);
	int oppositeDepth=newDepth+delta;
	setDepth(position,newDepth);
	setDepth(oppositePos,oppositeDepth);
}

/*public*/
std::string
DirectedEdge::print()
{
	std::string out=EdgeEnd::print();
	out+=" ";
	out+=depth[Position::LEFT];
	out+="/";
	out+=depth[Position::RIGHT];
	out+=" (";
	out+=getDepthDelta();
	out+=")";
	if (isInResultVar) out+=" inResult";
	return out;
}

/*public*/
std::string
DirectedEdge::printEdge()
{
	//std::string out=print();
	std::string out("");
	if (isForwardVar)
		out += edge->print();
	else
		out += edge->printReverse();
	return out;
}

} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.15  2006/03/15 17:16:29  strk
 * streamlined headers inclusion
 *
 * Revision 1.14  2006/03/15 15:26:58  strk
 * Cleanups
 *
 * Revision 1.13  2006/03/14 15:31:39  strk
 * Cleaned up toString funx (more WKT friendly)
 *
 * Revision 1.12  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.11  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.10  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.9  2006/02/28 14:34:04  strk
 * Added many assertions and debugging output hunting for a bug in BufferOp
 *
 * Revision 1.8  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.7  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.6  2005/11/14 18:14:04  strk
 * Reduced heap allocations made by TopologyLocation and Label objects.
 * Enforced const-correctness on GraphComponent.
 * Cleanups.
 *
 **********************************************************************/

