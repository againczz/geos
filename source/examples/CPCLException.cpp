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
 * Revision 1.3  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "CustomCoordinateListExample.h"
#include "../headers/util.h"

using namespace geos;

CPCLException::CPCLException(){
	GEOSException();
	setName("CPCLException");
}
CPCLException::CPCLException(string msg){
	setName("CPCLException");
	setMessage(msg);
}

CPCLException::~CPCLException(){}
