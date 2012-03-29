//# AsciiRegionLine.h
//# Copyright (C) 1998,1999,2000,2001
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#


#ifndef IMAGES_ASCIIANNOTATIONFILELINE_H
#define IMAGES_ASCIIANNOTATIONFILELINE_H

#include <casa/aips.h>
#include <imageanalysis/Annotations/AnnotationBase.h>
#include <measures/Measures/Stokes.h>

namespace casa {

// <summary>
// Represents a line in an ascii region file
// </summary>
// <author>Dave Mehringer</author>
// <use visibility=export>
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// <prerequisite>

// </prerequisite>

// <etymology>
// Represents a line in an ascii region file
// </etymology>

// <synopsis>
// Represents a line in an ascii region file
// See the region file format proposal attached to CAS-2285 (https://bugs.nrao.edu/browse/CAS-2285)
// </synopsis>

class AsciiAnnotationFileLine {

public:
	// type type of line
	enum Type {
		// region/annotation description
		ANNOTATION,
		// comment line
		COMMENT,
		// line containing a set of parameters to be applied to all regions/annotations following it
		GLOBAL,
		UNKNOWN_TYPE
	};

	AsciiAnnotationFileLine();

	AsciiAnnotationFileLine(const String& comment);

	AsciiAnnotationFileLine(const AnnotationBase * const annotationBase);

	AsciiAnnotationFileLine(const map<AnnotationBase::Keyword, String>& globals);

	AsciiAnnotationFileLine& operator= (const AsciiAnnotationFileLine& other);

	// get the associated comment, if the line is of Type COMMENT
	String getComment() const;

	// get the associated global parameters, if the line is of type GLOBAL
	map<AnnotationBase::Keyword, String> getGloabalParams() const;

	// get the aossicated region/annotation, if the line is of type ANNOTATION
	const AnnotationBase* getAnnotationBase() const;

	// get the type of line this object corresponds to.
	Type getType() const;

	ostream& print(ostream& os) const;

private:
	Type _type;
	String _comment;
	const AnnotationBase  *_annotationBase;
	map<AnnotationBase::Keyword, String> _globals;

};

inline ostream &operator<<(ostream& os, const AsciiAnnotationFileLine& line) {
	return line.print(os);
};

}

#endif /* ASCIIREGIONLINE_H */
