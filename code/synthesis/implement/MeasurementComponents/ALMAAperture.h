// -*- C++ -*-
//# ALMAAperture.h: Definition of the ALMAAperture class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//# Copyright (C) 2011 by ESO (in the framework of the ALMA collaboration)
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
//# $Id$
//
#ifndef SYNTHESIS_ALMAAPERTURE_H
#define SYNTHESIS_ALMAAPERTURE_H

#include <images/Images/ImageInterface.h>
#include <synthesis/MeasurementComponents/ATerm.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Images/AntennaResponses.h>
//
//---------------------------------------------------------------------
//---------------------------------------------------------------------
// TEMPS The following #defines should REALLY GO!
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//
#define CONVSIZE (1024*2)
#define CONVWTSIZEFACTOR sqrt(2.0)
#define OVERSAMPLING 10
#define THRESHOLD 1E-3

namespace casa { //# NAMESPACE CASA - BEGIN
  template<class T> class ImageInterface;
  template<class T> class Matrix;
  class VisBuffer;
  class ALMAAperture : public ATerm
  {
  public:
    ALMAAperture();

    ~ALMAAperture() {};

    ALMAAperture& operator=(const ALMAAperture& other);
    //
    // Overload these functions.  They are pure virtual in the base class (ATerm).
    //
    virtual String name() {return String("ALMA Aperture");};

    virtual void applySky(ImageInterface<Float>& outputImages,
			  const VisBuffer& vb, 
			  const Bool doSquint=True,
			  const Int& cfKey=0);
    virtual void applySky(ImageInterface<Complex>& outputImages,
			  const VisBuffer& vb, 
			  const Bool doSquint=True,
			  const Int& cfKey=0);

    virtual Vector<Int> vbRow2CFKeyMap(const VisBuffer& vb, Int& nUnique);

    virtual void setPolMap(const Vector<Int>& polMap) {polMap_p.resize(0);polMap_p=polMap;};
    virtual void getPolMap(Vector<Int>& polMap) {polMap.resize(0);polMap=polMap_p;};
    virtual Int getConvSize() {return CONVSIZE;};
    virtual Int getOversampling() {return OVERSAMPLING;}
    virtual Float getConvWeightSizeFactor() {return CONVWTSIZEFACTOR;};
    virtual Float getSupportThreshold() {return THRESHOLD;};

    // tell the antenna type number for each antenna in the antenna table
    Vector<Int> antTypeMap(const VisBuffer& vb);

    // derive type number from first two characters in antenna name, 
    // return -1 if not recognised 
    static Int antTypeFromName(const String& name);

    void destroyAntResp(){ delete aR_p;};

  protected:
    Int getVisParams(const VisBuffer& vb);
    Int makePBPolnCoords(const VisBuffer&vb,
			 const Int& convSize,
			 const Int& convSampling,
			 const CoordinateSystem& skyCoord,
			 const Int& skyNx, const Int& skyNy,
			 CoordinateSystem& feedCoord);

  private:
    static AntennaResponses* aR_p; // shared between all instances of this class
    Vector<Int> polMap_p;
    Bool haveCannedResponses_p; // true if there are precalculated response images available
    Vector<Int> antTypeMap_p; // maps antenna id to antenna type
    SimpleOrderedMap<Int, Int > pairTypeToCFKeyMap_p; // maps antenna pair type to CFKey 
  };
};
#endif
