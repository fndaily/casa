//# tSubImage.cc: Test program for class SubImage
//# Copyright (C) 1998,1999,2000,2001,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: tSubImage.cc 20567 2009-04-09 23:12:39Z gervandiepen $

#ifndef IMAGEANALYSIS_IMAGEFACTORY_H
#define IMAGEANALYSIS_IMAGEFACTORY_H

#include <imageanalysis/ImageTypedefs.h>

#include <casa/Arrays/Array.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/namespace.h>
#include <lattices/Lattices/LatticeBase.h>
#include <utility>
#include <vector>

using namespace std;

namespace casa {

class CoordinateSystem;
class IPosition;
class Record;
template <class T> class TempImage;
template <class T> class Vector;

class ImageFactory {
	// <summary>
	// Static methods for creating images 
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// </etymology>

	// <synopsis>
	// </synopsis>

public:

	enum ComplexToFloatFunction {
		REAL,
		IMAG
	};

	ImageFactory() = delete;

    ~ImageFactory() {};

    // Create a TempImage if outfile is empty, otherwise a PagedImage.
    // If log is True, log useful messages, quiet if False. Created image
    // has all pixel values set to zero and is unmasked.
    template <class T> static SPIIT createImage(
        const String& outfile,
        const CoordinateSystem& cSys, const IPosition& shape,
        Bool log, Bool overwrite,
        const std::vector<std::pair<LogOrigin, String> > *const &msgs
    );

    static String className() { static const String s = "ImageFactory"; return s; }

    // create an image with the specified shape and specified coordinate system.
    // If outfile is blank, the returned object is a TempImage, PagedImage otherwise.
    // If csys is empty,
    // a default coordiante system is attached to the image, and if linear
    // is True, it has linear coordinates in place of the direction coordinate.

    static SPIIF floatImageFromShape(
    	const String& outfile, const Vector<Int>& shape,
    	const Record& csys, Bool linear=True,
    	Bool overwrite=False, Bool verbose=True,
        const std::vector<std::pair<LogOrigin, String> > *const &msgs=0
    );

    static SPIIC complexImageFromShape(
    	const String& outfile, const Vector<Int>& shape,
    	const Record& csys, Bool linear=True,
    	Bool overwrite=False, Bool verbose=True,
        const std::vector<std::pair<LogOrigin, String> > *const &msgs=0
    );

    // only the pointer of the correct data type will be valid, the other
    // will be null.
    static pair<SPIIF, SPIIC> fromImage(
        const String& outfile, const String& infile,
        const Record& region, const String& mask,
        Bool dropdeg=False,
        Bool overwrite=False
    );

    template <class T> static SPIIT imageFromArray(
    	const String& outfile, const Array<T>& pixels,
    	const Record& csys, Bool linear=False,
    	Bool overwrite=False, Bool verbose=True,
    	const vector<std::pair<LogOrigin, String> > *const &msgs=0
    );

    static SPIIF fromASCII(
        const String& outfile, const String& infile,
        const IPosition& shape, const String& sep, const Record& csys,
        const Bool linear, const Bool overwrite
    );

    // Create a float-valued image from a complex-valued image. All metadata is copied
    // and pixel values are initialized according to <src>func</src>.
    static SHARED_PTR<TempImage<Float> > floatFromComplex(
    	SPCIIC complexImage, ComplexToFloatFunction func
    );

    // Create a complex-valued image from a float-valued image (real part)
    // and float-valued array (imaginary part). All metadata is copied from the
    // real image and pixel values are initialized to real + i*complex
    static SHARED_PTR<TempImage<Complex> > complexFromFloat(
    	SPCIIF realPart, const Array<Float>& imagPart
    );

    // exactly one of the pointers will not be null, indicating the
    // pixel data type
    static pair<SPIIF, SPIIC> fromFile(const String& filename);

    static SPIIF fromFITS(
        const String& outfile, const String& fitsfile,
        const Int whichrep, const Int whichhdu,
        const Bool zeroBlanks, const Bool overwrite
    );

    static pair<SPIIF, SPIIC> fromRecord(
        const RecordInterface& rec, const String& imagename=""
    );

    // open a canonical image
    static SPIIF testImage(
        const String& outfile, const Bool overwrite,
        const String& imagetype="2d"
    );

    static void toFITS(
    	SPCIIF image, const String& outfile, Bool velocity,
		Bool optical, Int bitpix, Double minpix, Double maxpix,
		const Record& region, const String& mask,
		Bool overwrite=False, Bool dropdeg=False, Bool deglast=False,
		Bool dropstokes=False, Bool stokeslast=False,
		Bool wavelength=False, Bool airWavelength=False,
		const String& origin="", Bool stretch=False, Bool history=True
    );

private:

	template <class T> static SPIIT _fromShape(
		const String& outfile, const Vector<Int>& shape,
		const Record& csys, Bool linear,
		Bool overwrite, Bool verbose,
        const std::vector<std::pair<LogOrigin, String> > *const &msgs
	);

	template <class T> static SPIIT _fromRecord(
	    const RecordInterface& rec, const String& name
	);

	static void _centerRefPix(
		CoordinateSystem& csys, const IPosition& shape
	);

	static void _checkInfile(const String& infile);

    // Convert a Record to a CoordinateSystem
    static CoordinateSystem* _makeCoordinateSystem(
        const casa::Record& cSys,
        const casa::IPosition& shape
    );

    static void _checkOutfile(const String& outfile, Bool overwrite);

    static pair<SPIIF, SPIIC> _fromLatticeBase(unique_ptr<LatticeBase>& latt);

};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageFactory.tcc>
#endif

#endif
