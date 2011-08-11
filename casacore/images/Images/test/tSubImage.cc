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

#include <images/Images/SubImage.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageAnalysis.h>
#include <images/Regions/ImageRegion.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/LCLELMask.h>
#include <lattices/Lattices/LCPixelSet.h>
#include <lattices/Lattices/LCPagedMask.h>
#include <casa/Arrays/AxesSpecifier.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>


#include <casa/namespace.h>
void testVectorROIter (const Lattice<Float>& sublat,
		       const Lattice<Float>& lattice,
		       const Slicer& slicer)
{
    Int nstep;
    const IPosition latticeShape(sublat.shape());
    const IPosition cursorShape(1,latticeShape(0));
    LatticeStepper step(latticeShape, cursorShape);
    RO_LatticeIterator<Float>  iter(sublat, step);
    LatticeStepper step2(lattice.shape(), cursorShape);
    step2.subSection (slicer.start(), slicer.end(), slicer.stride());
    RO_LatticeIterator<Float>  iter2(lattice, step2);
    for (iter.reset(); !iter.atEnd(); iter++, iter2++){
        AlwaysAssert(allEQ(iter.vectorCursor(),
			   iter2.vectorCursor()), AipsError);
    }
    nstep = iter.nsteps();
    AlwaysAssert(nstep == latticeShape.product()/latticeShape(0),
		 AipsError);
    IPosition expectedPos(latticeShape-1);
    AlwaysAssert(iter.endPosition() == expectedPos, AipsError);
    expectedPos(0) = 0;
    AlwaysAssert(iter.position() == expectedPos, AipsError);
}


void testRest()
{
    CoordinateSystem cSys = CoordinateUtil::defaultCoords2D();
    PagedImage<Float> pa(IPosition(2,10,10), cSys, "tSubImage_tmp.pa");
    AlwaysAssertExit (pa.isPaged());
    AlwaysAssertExit (pa.isPersistent());
    AlwaysAssertExit (pa.isWritable());
    AlwaysAssertExit (pa.name(True) == "tSubImage_tmp.pa");
    LCPagedMask lcmask(IPosition(2,10,10), "tSubImage_tmp.pa/mask");
    ImageRegion mask(lcmask);
    Slicer slicer(IPosition(2,1,1), IPosition(2,3,3));
    Slicer slfull(IPosition(2,0,0), IPosition(2,10,10));
    {
        // A SubImage as a Lattice copy (RO).
	SubImage<Float> sl(pa);
	AlwaysAssertExit (!sl.isMasked());
	AlwaysAssertExit (!sl.hasPixelMask());
	AlwaysAssertExit (sl.isPaged());
	AlwaysAssertExit (sl.isPersistent());
	AlwaysAssertExit (!sl.isWritable());
	// A copy of the SubImage.
	SubImage<Float> sl1(sl, True);
	AlwaysAssertExit (!sl1.isMasked());
	AlwaysAssertExit (!sl1.hasPixelMask());
	AlwaysAssertExit (sl1.isPaged());
	AlwaysAssertExit (sl1.isPersistent());
	AlwaysAssertExit (!sl1.isWritable());
	AlwaysAssertExit (sl1.name(True) == "tSubImage_tmp.pa");
    }
    {
        // A SubImage as a Lattice copy (RW).
	SubImage<Float> sl(pa, True);
	AlwaysAssertExit (!sl.isMasked());
	AlwaysAssertExit (!sl.hasPixelMask());
	AlwaysAssertExit (sl.isPaged());
	AlwaysAssertExit (sl.isPersistent());
	AlwaysAssertExit (sl.isWritable());
	// A RW copy of the SubImage.
	SubImage<Float> sl1(sl, True);
	AlwaysAssertExit (!sl1.isMasked());
	AlwaysAssertExit (!sl1.hasPixelMask());
	AlwaysAssertExit (sl1.isPaged());
	AlwaysAssertExit (sl1.isPersistent());
	AlwaysAssertExit (sl1.isWritable());
	// A RO copy of the SubImage.
	SubImage<Float> sl2(sl, False);
	AlwaysAssertExit (!sl2.isMasked());
	AlwaysAssertExit (!sl2.hasPixelMask());
	AlwaysAssertExit (sl2.isPaged());
	AlwaysAssertExit (sl2.isPersistent());
	AlwaysAssertExit (!sl2.isWritable());
	AlwaysAssertExit (sl2.name(True) == "tSubImage_tmp.pa");
    }
    {
        // A RO SubImage as a masked Lattice.
	SubImage<Float> sl(pa, mask);
	AlwaysAssertExit (sl.isMasked());
	AlwaysAssertExit (!sl.hasPixelMask());
	AlwaysAssertExit (sl.isPaged());
	AlwaysAssertExit (!sl.isPersistent());
	AlwaysAssertExit (!sl.isWritable());
	// A copy of the SubImage.
	SubImage<Float> sl1(sl, True);
	AlwaysAssertExit (sl1.isMasked());
	AlwaysAssertExit (!sl1.hasPixelMask());
	AlwaysAssertExit (sl1.isPaged());
	AlwaysAssertExit (!sl1.isPersistent());
	AlwaysAssertExit (!sl1.isWritable());
	AlwaysAssertExit (sl1.name(True) == "tSubImage_tmp.pa");
    }
    {
        // A RW SubImage as a masked Lattice.
	SubImage<Float> sl(pa, mask, True);
	AlwaysAssertExit (sl.isMasked());
	AlwaysAssertExit (!sl.hasPixelMask());
	AlwaysAssertExit (sl.isPaged());
	AlwaysAssertExit (!sl.isPersistent());
	AlwaysAssertExit (sl.isWritable());
	// A RW copy of the SubImage.
	SubImage<Float> sl1(sl, True);
	AlwaysAssertExit (sl1.isMasked());
	AlwaysAssertExit (!sl1.hasPixelMask());
	AlwaysAssertExit (sl1.isPaged());
	AlwaysAssertExit (!sl1.isPersistent());
	AlwaysAssertExit (sl1.isWritable());
	// A RO copy of the SubImage.
	SubImage<Float> sl2(sl, False);
	AlwaysAssertExit (sl2.isMasked());
	AlwaysAssertExit (!sl2.hasPixelMask());
	AlwaysAssertExit (sl2.isPaged());
	AlwaysAssertExit (!sl2.isPersistent());
	AlwaysAssertExit (!sl2.isWritable());
    }
    {
        // A small region of a lattice.
	SubImage<Float> sl(pa, slicer, True);
	AlwaysAssertExit (!sl.isMasked());
	AlwaysAssertExit (!sl.hasPixelMask());
	AlwaysAssertExit (sl.isPaged());
	AlwaysAssertExit (!sl.isPersistent());
	AlwaysAssertExit (sl.isWritable());
	// A RW copy of the SubImage.
	SubImage<Float> sl1(sl, True);
	AlwaysAssertExit (!sl1.isMasked());
	AlwaysAssertExit (!sl1.hasPixelMask());
	AlwaysAssertExit (sl1.isPaged());
	AlwaysAssertExit (!sl1.isPersistent());
	AlwaysAssertExit (sl1.isWritable());
	// A RO copy of the SubImage.
	SubImage<Float> sl2(sl, False);
	AlwaysAssertExit (!sl2.isMasked());
	AlwaysAssertExit (!sl2.hasPixelMask());
	AlwaysAssertExit (sl2.isPaged());
	AlwaysAssertExit (!sl2.isPersistent());
	AlwaysAssertExit (!sl2.isWritable());
    }
    {
        // A full region of a lattice.
	SubImage<Float> sl(pa, slfull, True);
	AlwaysAssertExit (!sl.isMasked());
	AlwaysAssertExit (!sl.hasPixelMask());
	AlwaysAssertExit (sl.isPaged());
	AlwaysAssertExit (sl.isPersistent());
	AlwaysAssertExit (sl.isWritable());
	// A RW copy of the SubImage.
	SubImage<Float> sl1(sl, True);
	AlwaysAssertExit (!sl1.isMasked());
	AlwaysAssertExit (!sl1.hasPixelMask());
	AlwaysAssertExit (sl1.isPaged());
	AlwaysAssertExit (sl1.isPersistent());
	AlwaysAssertExit (sl1.isWritable());
	// A RO copy of the SubImage.
	SubImage<Float> sl2(sl, False);
	AlwaysAssertExit (!sl2.isMasked());
	AlwaysAssertExit (!sl2.hasPixelMask());
	AlwaysAssertExit (sl2.isPaged());
	AlwaysAssertExit (sl2.isPersistent());
	AlwaysAssertExit (!sl2.isWritable());
    }
}

void testAxes()
{
    CoordinateSystem cSys = CoordinateUtil::defaultCoords3D();
    PagedImage<Float> pa(IPosition(3,10,11,12), cSys, "tSubImage_tmp.pa");
    LCPagedMask mask(IPosition(3,10,11,12), "tSubImage_tmp.pa/mask");
    Array<Float> arr(pa.shape());
    indgen(arr);
    pa.put (arr);
    Array<Bool> m(pa.shape());
    m = True;
    m(IPosition(3,0,0,0), IPosition(3,9,10,11), IPosition(3,2,1,1)) = False;
    mask.put (m);
    Array<Float> arrs1 = arr(IPosition(3,3,1,2), IPosition(3,8,1,9));
    Array<Float> arrsub = arrs1.reform(IPosition(2,6,8));
    Array<Bool> ms1 = m(IPosition(3,3,1,2), IPosition(3,8,1,9));
    Array<Bool> msub = ms1.reform(IPosition(2,6,8));
    // Make subimage with a removed axis 1.
    SubImage<Float> ml(pa, mask, True);
    Array<Bool> pixmask(IPosition(3,6,1,8));
    pixmask = True;
    pixmask (IPosition(3,0,0,0)) = !msub(IPosition(2,0,0));
    LCPixelSet pixset (pixmask, LCBox(IPosition(3,3,1,2),
				      IPosition(3,8,1,9),
				      m.shape()));
    SubImage<Float> sl(ml, pixset, True, AxesSpecifier(False));

    // Test its coordinate system.
    const CoordinateSystem& subcsys = sl.coordinates();
    AlwaysAssertExit (subcsys.nWorldAxes() == 2);
    AlwaysAssertExit (subcsys.nPixelAxes() == 2);
    AlwaysAssertExit (subcsys.worldAxisNames()(0) == cSys.worldAxisNames()(0));
    AlwaysAssertExit (subcsys.worldAxisNames()(1) == cSys.worldAxisNames()(2));

    IPosition ncs (pa.niceCursorShape());
    // Test if shape and niceCursorShape remove the axis.
    AlwaysAssertExit (sl.shape() == IPosition(2,6,8));
    AlwaysAssertExit (sl.niceCursorShape() == IPosition(2, min(6,ncs(0)),
							min(8,ncs(2))));
    // Test the getting functions.
    Array<Float> arrsl = sl.get();
    AlwaysAssertExit (allEQ (arrsl, arrsub));
    AlwaysAssertExit (sl.getAt(IPosition(2,1,3)) == arrsub(IPosition(2,1,3)));
    arrsub(IPosition(2,1,3)) += 10;
    // Test the put function and see if the result matches.
    sl.putAt(arrsub(IPosition(2,1,3)), IPosition(2,1,3));
    AlwaysAssertExit (sl.getAt(IPosition(2,1,3)) == arrsub(IPosition(2,1,3)));
    AlwaysAssertExit (allEQ (sl.get(), arrsub));
    arrsub(IPosition(2,1,3)) += 10;
    sl.put (arrsub);
    AlwaysAssertExit (allEQ (sl.get(), arrsub));
    AlwaysAssertExit (sl.getAt(IPosition(2,1,3)) == arrsub(IPosition(2,1,3)));
    // Now get and put a slice.
    Array<Float> arrsubsub = arrsub(IPosition(2,1,2),
				  IPosition(2,5,7),
				  IPosition(2,3,2));
    AlwaysAssertExit (allEQ (sl.getSlice(Slicer(IPosition(2,1,2),
						IPosition(2,5,7),
						IPosition(2,3,2),
						Slicer::endIsLast)),
			     arrsubsub));
    arrsubsub += Float(20);
    sl.putSlice (arrsubsub, IPosition(2,1,2), IPosition(2,3,2));
    AlwaysAssertExit (allEQ (sl.get(), arrsub));
    // Test the get mask functions.
    AlwaysAssertExit (allEQ (ml.getMask(), m));
    msub(IPosition(2,0,0)) = !msub(IPosition(2,0,0));
    AlwaysAssertExit (allEQ (sl.getMask(), msub));
    Array<Bool> msubsub = msub(IPosition(2,1,2),
			       IPosition(2,5,7),
			       IPosition(2,3,2));
    AlwaysAssertExit (allEQ (sl.getMaskSlice(Slicer(IPosition(2,1,2),
						    IPosition(2,5,7),
						    IPosition(2,3,2),
						    Slicer::endIsLast)),
			     msubsub));
}

void testExtend() {
    CoordinateSystem cSys = CoordinateUtil::defaultCoords4D();
    PagedImage<Float> pa(IPosition(4,10,11,4, 13), cSys, "tSubImage_tmp.pa");
    CoordinateSystem maskCsys = CoordinateUtil::defaultCoords3D();
    PagedImage<Float> maskImage(IPosition(3, 10, 11, 1), maskCsys, "mask_tmp.pa");
    maskImage.flush();
    Array<Float> arr(pa.shape());
    indgen(arr);
    pa.put (arr);
    Array<Float> maskarr(maskImage.shape());
    indgen(maskarr);
    maskImage.put (maskarr);
    Record empty;
    String maskExpr = "mask_tmp.pa > 10";
    SubImage<Float> myim2 = SubImage<Float>::createSubImage(
    	pa, empty, maskExpr, new LogIO(), False,
    	AxesSpecifier(), True
    );
    Array<Bool> got = myim2.getMask();
    LatticeExprNode x(ArrayLattice<Float>(maskImage.get()) > 10);
    LatticeExpr<Bool> kk(x);
    LCLELMask lel(kk);
    ExtendLattice<Bool> z(lel, pa.shape(), IPosition(1, 2), IPosition(1, 3));
    AlwaysAssert(got.shape().isEqual(z.shape()), AipsError);
    for (uInt i=0; i<z.shape().product(); i++) {
    	AlwaysAssert(z.get().data()[i] == got.data()[i], AipsError);
    }
}




int main ()
{
  try {
    {
      const IPosition latticeShape(4, 16, 12, 4, 32);
      CoordinateSystem cSys = CoordinateUtil::defaultCoords4D();
      Array<Float> arr(latticeShape);
      indgen(arr);
      PagedImage<Float> lattice(latticeShape, cSys, "tSubImage_tmp.pa");
      lattice.put (arr);
      Slicer slicer(IPosition(4,4,2,1,3), IPosition(4,14,10,3,23),
		    IPosition(4,2,3,1,4), Slicer::endIsLast);
      SubImage<Float> subimg (lattice, slicer, True);
      AlwaysAssertExit (subimg.isPaged());
      AlwaysAssertExit (!subimg.isPersistent());
      AlwaysAssertExit (!subimg.isMasked());
      AlwaysAssertExit (subimg.isWritable());
      AlwaysAssertExit (subimg.shape() == slicer.length());
      Array<Float> arr1, arr2;
      subimg.getSlice (arr1, IPosition(4,0), subimg.shape(),
		       IPosition(4,1));
      lattice.getSlice (arr2, slicer);
      AlwaysAssertExit (allEQ(arr1, arr2));
      AlwaysAssertExit (allEQ(arr1,
			      arr(slicer.start(), slicer.end(),
				  slicer.stride())));
      testVectorROIter (subimg, lattice, slicer);
    }
    // Test some other SubImage functions.
    testRest();
    // Test the axes removal..
    testAxes();
    // test extending mask
    testExtend();
  } catch (AipsError x) {
    cerr << "Caught exception: " << x.getMesg() << endl;
    cout << "FAIL" << endl;
    return 1;
  } 

  cout << "OK" << endl;
  return 0;
}
