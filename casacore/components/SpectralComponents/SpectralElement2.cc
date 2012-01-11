//# SpectralElement.cc: Describes (a set of related) spectral lines
//# Copyright (C) 2001,2004
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
//# $Id: SpectralElement.cc 21024 2011-03-01 11:46:18Z gervandiepen $

#include <components/SpectralComponents/CompiledSpectralElement.h>
#include <components/SpectralComponents/GaussianSpectralElement.h>
#include <components/SpectralComponents/PolynomialSpectralElement.h>

#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

ostream &operator<<(ostream &os, const SpectralElement &elem) {
	switch (elem.getType()) {
	case SpectralElement::GAUSSIAN:
		os << *dynamic_cast<const GaussianSpectralElement*>(&elem);
		break;
	case SpectralElement::POLYNOMIAL:
		os << *dynamic_cast<const PolynomialSpectralElement*>(&elem);
		break;
	case SpectralElement::COMPILED:
		break;
		os << *dynamic_cast<const CompiledSpectralElement*>(&elem);
	default:
		throw AipsError("Logic Error: Unhandled spectral element type");
	}
    return os;
}



} //# NAMESPACE CASA - END


