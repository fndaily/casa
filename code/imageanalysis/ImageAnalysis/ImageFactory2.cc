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
//# $Id: $

#include <imageanalysis/ImageAnalysis/ImageFactory.h>

#include <casa/OS/EnvVar.h>
#include <images/Images/ImageFITSConverter.h>
#include <images/Images/ImageUtilities.h>
#include <imageanalysis/IO/CasaImageOpener.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>

namespace casa {

SPIIF ImageFactory::floatImageFromShape(
		const String& outfile, const Vector<Int>& shape,
		const Record& csys, Bool linear,
		Bool overwrite, Bool verbose,
		const vector<std::pair<LogOrigin, String> > *const &msgs
) {
	return _fromShape<Float>(
			outfile, shape, csys, linear,
			overwrite, verbose, msgs
	);
}

SPIIC ImageFactory::complexImageFromShape(
		const String& outfile, const Vector<Int>& shape,
		const Record& csys, Bool linear,
		Bool overwrite, Bool verbose,
		const vector<std::pair<LogOrigin, String> > *const &msgs
) {
	return _fromShape<Complex>(
			outfile, shape, csys, linear,
			overwrite, verbose, msgs
	);
}


SPIIF ImageFactory::fromASCII(
    const String& outfile, const String& infile,
    const IPosition& shape, const String& sep, const Record& csys,
    const Bool linear, const Bool overwrite
) {
    Path filePath(infile);
    auto fileName = filePath.expandedName();

    ifstream inFile(fileName.c_str());
    ThrowIf(!inFile, "Cannot open " + infile);

    auto n = shape.product();
    auto nx = shape[0];
    Vector<Float> a(n, 0.0);
    int idx = 0;
    string line;
    unique_ptr<string[]> managed(new string[2 * nx]);
    auto line2 = managed.get();
    uInt iline = 0;
    uInt nl = 1;
    while (nl > 0) {
        getline(inFile, line, '\n');
        nl = split(line, line2, 2 * nx, sep);
        if (nl > 0) {
            ThrowIf(
                nl != nx,
                "Length of line " + String::toString(iline)
                + " is " + String::toString(nl) + " but should be "
                + String::toString(nx)
            );
            for (uInt i = 0; i < nx; i++) {
                a[idx + i] = atof(line2[i].c_str());
            }
            idx += nx;
            iline += 1;
        }
    }
    Vector<Float> vec(n);
    for (uInt i = 0; i < n; ++i) {
        vec[i] = a[i];
    }
    Array<Float> pixels(vec.reform(IPosition(shape)));
    return imageFromArray(outfile, pixels, csys, linear, overwrite);
}

pair<SPIIF, SPIIC> ImageFactory::fromImage(
    const String& outfile, const String& infile,
    const Record& region, const String& mask, Bool dropdeg,
    Bool overwrite
) {
    _checkInfile(infile);
    unique_ptr<LatticeBase> latt(CasaImageOpener::openImage(infile));
    ThrowIf (! latt, "Unable to open lattice");
    auto imagePair = _fromLatticeBase(latt);
    LogIO mylog;
    mylog << LogOrigin("ImageFactory", __func__);
    if (imagePair.first) {
        imagePair.first = SubImageFactory<Float>::createImage(
                *imagePair.first, outfile, region,
                mask, dropdeg, overwrite, False, False
        );
        ThrowIf(
           ! imagePair.first,
           "Failed to create PagedImage"
        );
        mylog << LogIO::NORMAL << "Created image '" << outfile
            << "' of shape " << imagePair.first->shape() << LogIO::POST;
    }
    else {
        imagePair.second = SubImageFactory<Complex>::createImage(
            *imagePair.second, outfile, region,
            mask, dropdeg, overwrite, False, False
        );
        ThrowIf(
            ! imagePair.second,
            "Failed to create PagedImage"
        );
        mylog << LogIO::NORMAL << "Created image '" << outfile
            << "' of shape " << imagePair.second->shape() << LogIO::POST;

    }
    return imagePair;
}


pair<SPIIF, SPIIC> ImageFactory::fromRecord(
    const RecordInterface& rec, const String& name
) {
    auto mytype = rec.type(rec.fieldNumber("imagearray"));
    pair<SPIIF, SPIIC> imagePair;
    if (isReal(mytype)) {
        imagePair.first = _fromRecord<Float>(rec, name);
    }
    else {
        imagePair.second = _fromRecord<Complex>(rec, name);
    }
    return imagePair;
}

void ImageFactory::_centerRefPix(
	CoordinateSystem& csys, const IPosition& shape
) {
	Int after = -1;
	Int iS = csys.findCoordinate(Coordinate::STOKES, after);
	Int sP = -1;
	if (iS >= 0) {
		Vector<Int> pixelAxes = csys.pixelAxes(iS);
		sP = pixelAxes(0);
	}
	Vector<Double> refPix = csys.referencePixel();
	for (Int i = 0; i < Int(refPix.nelements()); i++) {
		if (i != sP)
			refPix(i) = Double(shape(i) / 2);
	}
	csys.setReferencePixel(refPix);
}

CoordinateSystem* ImageFactory::_makeCoordinateSystem(
    const Record& coordinates, const IPosition& shape
) {
    std::unique_ptr<CoordinateSystem> pCS;
    if (coordinates.nfields() == 1) { 
        // must be a record as an element
        Record tmp(coordinates.asRecord(RecordFieldId(0)));
        pCS.reset(CoordinateSystem::restore(tmp, ""));
    } 
    else {
        pCS.reset(CoordinateSystem::restore(coordinates, ""));
    }
    // Fix up any body longitude ranges...
    String errMsg;
    ThrowIf(
        ! CoordinateUtil::cylindricalFix(*pCS, errMsg, shape),
        errMsg
    ); 
    return pCS.release();
}

SHARED_PTR<TempImage<Complex> > ImageFactory::complexFromFloat(
	SPCIIF realPart, const Array<Float>& imagPart
) {
	SHARED_PTR<TempImage<Complex> > newImage(
		new TempImage<Complex>(
			TiledShape(realPart->shape()),
			realPart->coordinates()
		)
	);
	{
		Array<Bool> mymask = realPart->getMask();
		if (realPart->hasPixelMask()) {
			mymask = mymask && realPart->pixelMask().get();
		}
		if (! allTrue(mymask)) {
			newImage->attachMask(ArrayLattice<Bool>(mymask));
		}
	}
	ImageUtilities::copyMiscellaneous(*newImage, *realPart);
	newImage->put(makeComplex(realPart->get(), imagPart));
	return newImage;
}

SHARED_PTR<TempImage<Float> > ImageFactory::floatFromComplex(
	SPCIIC complexImage, ComplexToFloatFunction function
) {
	SHARED_PTR<TempImage<Float> > newImage(
		new TempImage<Float>(
			TiledShape(complexImage->shape()),
			complexImage->coordinates()
		)
	);
	{
		Array<Bool> mymask = complexImage->getMask();
		if (complexImage->hasPixelMask()) {
			mymask = mymask && complexImage->pixelMask().get();
		}
		if (! allTrue(mymask)) {
			newImage->attachMask(ArrayLattice<Bool>(mymask));
		}
	}
	ImageUtilities::copyMiscellaneous(*newImage, *complexImage);
	switch (function) {
	case REAL:
		newImage->put(real(complexImage->get()));
		break;
	case IMAG:
		newImage->put(imag(complexImage->get()));
		break;
	default:
		ThrowCc("Logic Error: Unhandled function");
	}
	return newImage;
}

pair<SPIIF, SPIIC> ImageFactory::fromFile(const String& infile) {
    _checkInfile(infile);
    unique_ptr<LatticeBase> latt(CasaImageOpener::openImage(infile));
    ThrowIf (! latt, "Unable to open lattice");
    return _fromLatticeBase(latt);
}

pair<SPIIF, SPIIC> ImageFactory::_fromLatticeBase(unique_ptr<LatticeBase>& latt) {
    DataType dataType = latt->dataType();
    pair<SPIIF, SPIIC> ret(nullptr, nullptr);
    if (isReal(dataType)) {
        if (dataType != TpFloat) {
            ostringstream os;
            os << dataType;
            LogIO log;
            log << LogOrigin(className(), __func__);
            log << LogIO::WARN << "Converting " << os.str() << " precision pixel values "
                << "to float precision in CASA image" << LogIO::POST;
        }
        return pair<SPIIF, SPIIC>(
            SPIIF(dynamic_cast<ImageInterface<Float> *>(latt.release())),
            SPIIC(nullptr)
        );
    }
    else if (isComplex(dataType)) {
        if (dataType != TpComplex) {
            ostringstream os;
            os << dataType;
            LogIO log;
            log << LogOrigin(className(), __func__);
            log << LogIO::WARN << "Converting " << os.str() << " precision pixel values "
                << "to complex float precision in CASA image" << LogIO::POST;
        }
        return pair<SPIIF, SPIIC>(
            SPIIF(nullptr),
            SPIIC(dynamic_cast<ImageInterface<Complex> *>(latt.release()))
        );
    }
    ostringstream os;
    os << dataType;
    throw AipsError("unsupported image data type " + os.str());
}

void ImageFactory::_checkInfile(const String& infile) {
    ThrowIf(
        infile.empty(), "File name is empty"
    );
    File thefile(infile);
    ThrowIf(
        ! thefile.exists(),
        "File " + infile + " does not exist."
    );
}

SPIIF ImageFactory::fromFITS(
    const String& outfile, const String& fitsfile,
    const Int whichrep, const Int whichhdu,
    const Bool zeroBlanks, const Bool overwrite
) {
    _checkOutfile(outfile, overwrite);
    ThrowIf(
        whichrep < 0,
        "The Coordinate Representation index must be non-negative"
    );
    ImageInterface<Float> *x = nullptr;
    String error;
    Bool rval = ImageFITSConverter::FITSToImage(
        x, error, outfile, fitsfile, whichrep, whichhdu,
        HostInfo::memoryFree() / 1024, overwrite, zeroBlanks
    );
    SPIIF pOut(x);
    ThrowIf(! rval || ! pOut, error);
    return pOut;
}

void ImageFactory::toFITS(
	SPCIIF image, const String& outfile, Bool velocity, Bool optical,
	Int bitpix, Double minpix, Double maxpix,
	const Record& region, const String& mask,
	Bool overwrite, Bool dropdeg, Bool deglast,
	Bool dropStokes, Bool stokeslast,
	Bool wavelength, Bool airWavelength,
	const String& origin, Bool stretch, Bool history
) {
	LogIO log;
	log << LogOrigin("ImageFactory", __func__);
	_checkOutfile(outfile, overwrite);
	// The SubImage that goes to the FITSCOnverter no longer will know
	// the name of the parent mask, so spit it out here
	if (image->isMasked()) {
		log << LogIO::NORMAL << "Applying mask of name '"
			<< image->getDefaultMask() << "'" << LogIO::POST;
	}
	IPosition keepAxes;
	if (! dropdeg) {
		if (dropStokes) {
			const auto& cSys = image->coordinates();
			if (
				cSys.hasPolarizationCoordinate()
				&& cSys.nCoordinates() > 1
			) {
				// Stokes axis exists and its not the only one
				auto cNames = cSys.worldAxisNames();
				keepAxes = IPosition(cNames.size() - 1);
				uInt j = 0;
				for (uInt i = 0; i < cNames.size(); ++i) {
					if (cNames(i) != "Stokes") { // not Stokes?
						keepAxes(j) = i; // keep it
						j++;
					}
				}
			}
		}
	}
	AxesSpecifier axesSpecifier;
	if (dropdeg) {
		axesSpecifier = AxesSpecifier(False);
	}
	else if (! keepAxes.empty()) {
		axesSpecifier = AxesSpecifier(keepAxes);
	}
	auto subImage = SubImageFactory<Float>::createSubImageRO(
		*image, region, mask, &log, axesSpecifier, stretch
	);
	// FIXME remove when the casacore interface has been updated to const
	SPIIF myclone(subImage->cloneII());
	String error;
	ThrowIf (
		! ImageFITSConverter::ImageToFITS(
			error, *myclone, outfile,
			HostInfo::memoryFree() / 1024,
			velocity, optical, bitpix, minpix,
			maxpix, overwrite, deglast,
			False, //  verbose default
			stokeslast,	wavelength,
			airWavelength, // for airWavelength=True
			origin, history
		), error
	);
}

SPIIF ImageFactory::testImage(
    const String& outfile, const Bool overwrite,
    const String& imagetype
) {
    String var = EnvironmentVariable::get("CASAPATH");
    if (var.empty()) {
        var = EnvironmentVariable::get("AIPSPATH");
    }
    ThrowIf(
        var.empty(),
        "Neither CASAPATH nor AIPSPATH is set, so cannot locate data directory"
    );
    String fields[4];
    Int num = split(var, fields, 4, String(" "));
    ThrowIf (num <= 0, "Bad CASAPATH/AIPSPATH value: " + var);
    String fitsfile;
    if (imagetype.contains("cube")) {
        fitsfile = fields[0] + "/data/demo/Images/test_imageFloat.fits";
    }
    else if (imagetype.contains("2d")) {
        fitsfile = fields[0] + "/data/demo/Images/imagetestimage.fits";
    }
    else {
        ThrowCc("imageType must be either \"cube\" or \"2d\"");
    }
    return fromFITS(
        outfile, fitsfile, 0, 0, False, overwrite
    );
}

void ImageFactory::_checkOutfile(const String& outfile, Bool overwrite) {
    if (! overwrite && ! outfile.empty()) {
        NewFile validfile;
        String errmsg;
        ThrowIf(
            ! validfile.valueOK(outfile, errmsg), errmsg
        );
    }
}

}

