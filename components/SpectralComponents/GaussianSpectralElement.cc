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

//# Includes
#include <components/SpectralComponents/GaussianSpectralElement.h>

#include <casa/BasicSL/Constants.h>

#include <casa/iostream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//# Constants
const Double GaussianSpectralElement::SigmaToFWHM = sqrt(8.0*C::ln2);

//# Constructors
GaussianSpectralElement::GaussianSpectralElement()
: SpectralElement() {
	Vector<Double> param(3);
	param(0) = 1.0;
	param(1) = 0.0;
	param(2) = 2*sqrt(C::ln2)/C::pi;
	_construct(SpectralElement::GAUSSIAN, param);
}

GaussianSpectralElement::GaussianSpectralElement(
	const Double ampl,
	const Double center, const Double sigma
) : SpectralElement() {
	if (ampl == 0) {
		throw AipsError("Gaussian amplitude cannot equal 0");
	}
	Vector<Double> param(3);
	param(0) = ampl;
	param(1) = center;
	param(2) =  sigma > 0 ? sigma : -sigma;
	_construct(SpectralElement::GAUSSIAN, param);
}

GaussianSpectralElement::GaussianSpectralElement(
	const Vector<Double> &param
) : SpectralElement() {
    if (param.nelements() != 3) {
    	throw AipsError(
    		"GaussianSpectralElement: GAUSSIAN must have "
    		"3 parameters"
    	);
    }
	if (param[0] == 0) {
		throw AipsError("Gaussian amplitude cannot equal 0");
	}
	Vector<Double> p = param.copy();
	if (p[2] < 0) {
		p[2] = -p[2];
	}
	_construct(SpectralElement::GAUSSIAN, p);
}

GaussianSpectralElement::GaussianSpectralElement(
	const GaussianSpectralElement &other
) : SpectralElement(other) {}

GaussianSpectralElement::~GaussianSpectralElement() {}


SpectralElement* GaussianSpectralElement::clone() const {
	return new GaussianSpectralElement(*this);
}

GaussianSpectralElement& GaussianSpectralElement::operator=(
	const GaussianSpectralElement &other
) {
	if (this != &other) {
		SpectralElement::operator=(other);
	}
	return *this;
}

Double GaussianSpectralElement::operator()(const Double x) const {
	Vector<Double> p = get();
    return p(0)*exp(-0.5 * (x-p(1))*(x-p(1)) / p(2)/p(2));
}

Double GaussianSpectralElement::getAmpl() const {
  return get()[0];
}

Double GaussianSpectralElement::getCenter() const {
  return get()[1];
}

Double GaussianSpectralElement::getSigma() const {
	return get()[2];
}

Double GaussianSpectralElement::getFWHM() const {
	return sigmaToFWHM(get()[2]);
}

Double GaussianSpectralElement::getAmplErr() const {
	return getError()[0];
}

Double GaussianSpectralElement::getCenterErr() const {
	return getError()[1];
}

Double GaussianSpectralElement::getSigmaErr() const {
	return getError()[2];
}

Double GaussianSpectralElement::getFWHMErr() const {
	return sigmaToFWHM(getError()[2]);
}

void GaussianSpectralElement::setAmpl(Double ampl) {
	if (ampl == 0) {
		throw AipsError("Gaussian amplitude cannot equal 0");
	}
	Vector<Double> p = get();
	p(0) = ampl;
	_set(p);
	Vector<Double> err = getError();
	err[0] = 0;
	setError(err);
} 

void GaussianSpectralElement::setCenter(Double center) {
	Vector<Double> p = get();
	p[1] = center;
	_set(p);
	Vector<Double> err = getError();
	err[1] = 0;
	setError(err);
}

void GaussianSpectralElement::setSigma(Double sigma) {
	if (sigma < 0) {
		sigma = -sigma;
	}
	Vector<Double> p = get();
	p[2] = sigma;
	_set(p);
	Vector<Double> err = getError();
	err(2) = 0;
	setError(err);
}

void GaussianSpectralElement::setFWHM(Double fwhm) {
	setSigma(sigmaFromFWHM(fwhm));
}

void GaussianSpectralElement::fixAmpl(const Bool isFixed) {
	Vector<Bool> myFixed = fixed();
	myFixed[0] = isFixed;
	fix(myFixed);
}

void GaussianSpectralElement::fixCenter(const Bool isFixed) {
	Vector<Bool> myFixed = fixed();
	myFixed[1] = isFixed;
	fix(myFixed);
}

void GaussianSpectralElement::fixSigma(const Bool isFixed) {
	Vector<Bool> myFixed = fixed();
	myFixed[2] = isFixed;
	fix(myFixed);
}

void GaussianSpectralElement::fixByString(const String& s) {
	String fix(s);
	fix.downcase();
	if (fix.contains("p")) {
		fixAmpl(True);
	}
	if (fix.contains("c")) {
		fixCenter(True);
	}
	if (fix.contains("f")) {
		fixFWHM(True);
	}
}


void GaussianSpectralElement::fixFWHM(const Bool fix) {
	fixSigma(fix);
}

Bool GaussianSpectralElement::fixedAmpl() const {
	return fixed()[0];
}

Bool GaussianSpectralElement::fixedCenter() const {
	return fixed()[1];
}

Bool GaussianSpectralElement::fixedSigma() const {
	return fixed()[2];
}

Bool GaussianSpectralElement::fixedFWHM() const {
	return fixed()[2];
}

void GaussianSpectralElement::set(const Vector<Double>& params) {
	if (params.nelements() != 3) {
		throw AipsError(
			"GaussianSpectralElement: GAUSSIAN must have "
			"3 parameters"
		);
	}
	if (params[0] == 0) {
		throw AipsError("Gaussian amplitude cannot equal 0");
	}
	Vector<Double> p = params.copy();
	if (p[2] < 0) {
		p[2] = -p[2];
	}
 	_set(p);
}

Bool GaussianSpectralElement::toRecord(
	RecordInterface& out
) const {
	out.define(RecordFieldId("type"), fromType(getType()));
	Vector<Double> ptmp(get().copy());
	Vector<Double> etmp(getError().copy());
	ptmp(2) = sigmaToFWHM(ptmp(2));
	etmp(2) = sigmaToFWHM(etmp(2));
	out.define(RecordFieldId("parameters"), ptmp);
	out.define(RecordFieldId("errors"), etmp);
	return True;
}

ostream &operator<<(ostream &os, const GaussianSpectralElement &elem) {
	os << SpectralElement::fromType((elem.getType())) << " element: " << endl;
    os << "  Amplitude: " << elem.getAmpl() << ", " << elem.getAmplErr();
    if (elem.fixedAmpl()) os << " (fixed)";
    os << endl << "  Center:    " << elem.getCenter() << ", " << elem.getCenterErr();
    if (elem.fixedCenter()) os << " (fixed)";
    os << endl << "  Sigma:     " << elem.getSigma() << ", " << elem.getSigmaErr();
    if (elem.fixedSigma()) os << " (fixed)";
    os << endl;
    return os;
}

Double GaussianSpectralElement::sigmaToFWHM (const Double sigma) {
   return SigmaToFWHM * sigma;
}

Double GaussianSpectralElement::sigmaFromFWHM(const Double fwhm) {
  return fwhm / SigmaToFWHM;
}

} //# NAMESPACE CASA - END

