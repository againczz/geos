/* =========================================================================
 * $Id$
 *
 * ruby.i
 * 
 * Copyright 2005 Charlie Savage, cfis@interserv.com
 *
 * Interface for a SWIG generated geos module.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 * ========================================================================= */

%{ 
#include <sstream>
%}

/* Next conflicts with a Ruby keyword */
%rename(next_) geos::GeometryCollectionIterator::next();

/* Need to deal with these ignores by renaming them */
%rename("build_geometry!") geos::GeometryFactory::buildGeometry(vector<Geometry * > *) const;
%rename("create_geometry_collection!") geos::GeometryFactory::createGeometryCollection(vector<Geometry * > *) const;
%rename("create_linear_ring!") geos::GeometryFactory::createLinearRing(CoordinateSequence *) const;
%rename("create_line_string!") geos::GeometryFactory::createLineString(CoordinateSequence *) const;
%rename("create_multi_line_string!") geos::GeometryFactory::createMultiLineString(vector<Geometry * > *) const;
%rename("create_multi_point!") geos::GeometryFactory::createMultiPoint(vector<Geometry * > *) const;
%rename("create_multi_polygon!") geos::GeometryFactory::createMultiPolygon(vector<Geometry * > *) const;
%rename("create_point!") geos::GeometryFactory::createPoint(CoordinateSequence *) const;
%rename("create_polygon!") geos::GeometryFactory::createPolygon(LinearRing *,vector<Geometry * > *) const;

/* All these classes do not follow the Ruby naming convention */
%rename("PlanarGraphComponent") geos::planarGraphComponent;
%rename("PlanarDirectedEdge") geos::planarDirectedEdge;
%rename("PlanarDirectedEdgeStar") geos::planarDirectedEdgeStar;
%rename("PlanarNode") geos::planarNode;
%rename("PlanarEdge") geos::planarEdge;
%rename("PlanarCoordLT") geos::planarCoordLT;
%rename("PlanarNodeMap") geos::planarNodeMap;
%rename("PlanarPlanarGraph") geos::planarPlanarGraph;


/* Ruby defines a macro called select in its win32.h header file.  However,
   the object geos::MonotoneChainSelectAction also defines a select method,
   thus causing compilation problems.  So #undef the macro. */
%runtime %{
#undef select
%}


namespace geos {

%extend Coordinate {
	std::string to_s()
	{
		std::ostringstream os;
		os << "<Coordinate ";
		os << self->toString();
		os << ">";
		return os.str();
	}
};

%extend CoordinateSequence {
	std::string to_s()
	{
		std::ostringstream os;
		os << "<CoordinateSequence ";
		os << self->toString();
		os << ">";
		return os.str();
	}
};

%extend Envelope {
	std::string to_s()
	{
		std::ostringstream os;
		os << "<Envelope ";
		os << self->toString();
		os << ">";
		return os.str();
	}
};


%extend Geometry {
  std::string to_s()
	{
		std::ostringstream os;
		os << "<Geometry ";
		os << self->toString();
		os << ">";
		return os.str();
	}
};
} // namespace geos


/* Geos uses vectors of pointers to pass around geometries.  These will be 
	 wrapped by SWIG - but we have to be careful.  The problem is
   if we put a C++ object into the vector that a Ruby object owns, when
   that Ruby object goes out of scope the C++ object will be freed.  Thus
   the pointer inside the vector becomes corrupt.  To stop this from happening
   we have to implement a Ruby mark function, which in turn requires turning
   on SWIG Ruby Object tracking for these containers.*/

// First declare mark functions for the containers
%markfunc std::vector<geos::Geometry *> "mark_GeometryVector";
%markfunc std::vector<geos::LineString *> "mark_LineStringVector";
%markfunc std::vector<geos::Polygon *> "mark_PolygonVector";

// Manually rename methods on vector to get around SWIG 1.3.29 bug
%rename(__len__) std::vector<geos::Geometry *>::size;
%rename("empty?") std::vector<geos::Geometry *>::empty;
%rename(push) std::vector<geos::Geometry *>::push_back;

%rename(__len__) std::vector<geos::LineString *>::size;
%rename("empty?") std::vector<geos::LineString *>::empty;
%rename(push) std::vector<geos::LineString *>::push_back;

%rename(__len__) std::vector<geos::Polygon *>::size;
%rename("empty?") std::vector<geos::Polygon *>::empty;
%rename(push) std::vector<geos::Polygon *>::push_back;

// Now define the containers
%template("GeometryVector") std::vector<geos::Geometry *>;
%template("LineStringVector") std::vector<geos::LineString *>;
%template("PolygonVector") std::vector<geos::Polygon *>;


// Now track the objects that go go into the containers, which
// is Geometry or any class inherited from Geometry
%trackobjects geos::Geometry;
%trackobjects geos::Point;
%trackobjects geos::LineString;
%trackobjects geos::LinearRing;
%trackobjects geos::Polygon;
%trackobjects geos::GeometryCollection;
%trackobjects geos::MultiPoint;
%trackobjects geos::MultiLineString;
%trackobjects geos::MultiPolygon;

// Last define the mark functions
%header %{
	static void mark_GeometryVector(void* ptr)
	{
	  typedef std::vector<geos::Geometry *> GeometryVector;
	  typedef GeometryVector::iterator GeometryVectorIter;
	  
	  GeometryVector *vec = reinterpret_cast<GeometryVector*>(ptr);
	  GeometryVectorIter iter = vec->begin();
	  GeometryVectorIter last = vec->end();
	  
		for(; iter != last; ++iter)
		{
			geos::Geometry *geometry = *iter;
			VALUE object = SWIG_RubyInstanceFor(geometry);
			if (object != Qnil)
			{
				rb_gc_mark(object);
			}
			else
			{
				// This should not happen
				rb_raise(rb_eRuntimeError, "Unknown object stored in vector");
			}
		}
	}

	static void mark_LineStringVector(void* ptr)
	{
	  typedef std::vector<geos::LineString *> LineStringVector;
	  typedef LineStringVector::iterator LineStringVectorIter;
	  
	  LineStringVector *vec = reinterpret_cast<LineStringVector*>(ptr);
	  LineStringVectorIter iter = vec->begin();
	  LineStringVectorIter last = vec->end();
	  
		for(; iter != last; ++iter)
		{
			geos::LineString *geometry = *iter;
			VALUE object = SWIG_RubyInstanceFor(geometry);
			if (object != Qnil)
			{
				rb_gc_mark(object);
			}
			else
			{
				// This should not happen
				rb_raise(rb_eRuntimeError, "Unknown object stored in vector");
			}
		}
	}

	static void mark_PolygonVector(void* ptr)
	{
	  typedef std::vector<geos::Polygon *> PolygonVector;
	  typedef PolygonVector::iterator PolygonVectorIter;
	  
	  PolygonVector *vec = reinterpret_cast<PolygonVector*>(ptr);
	  PolygonVectorIter iter = vec->begin();
	  PolygonVectorIter last = vec->end();
	  
		for(; iter != last; ++iter)
		{
			geos::Polygon *geometry = *iter;
			VALUE object = SWIG_RubyInstanceFor(geometry);
			if (object != Qnil)
			{
				rb_gc_mark(object);
			}
			else
			{
				// This should not happen
				rb_raise(rb_eRuntimeError, "Unknown object stored in vector");
			}
		}
	}
%}
