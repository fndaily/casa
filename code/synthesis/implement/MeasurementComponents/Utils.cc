#include <msvis/MSVis/VisBuffer.h>
#include <casa/Logging/LogIO.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasTable.h>
#include <synthesis/MeasurementComponents/SkyModel.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <casa/Utilities/Assert.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <images/Images/PagedImage.h>
#include <casa/Containers/Record.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/TiledLineStepper.h> 
#include <lattices/Lattices/LatticeStepper.h> 
namespace casa{
  //
  //--------------------------------------------------------------------------------------------
  //  
  void storeImg(String& fileName,ImageInterface<Complex>& theImg)
  {
    ostringstream reName,imName;
    reName << "re" << fileName;
    imName << "im" << fileName;
    PagedImage<Complex> ctmp(theImg.shape(), theImg.coordinates(), fileName);
    LatticeExpr<Complex> le(theImg);
    ctmp.copyData(le);
    {
      PagedImage<Float> tmp(theImg.shape(), theImg.coordinates(), reName);
      LatticeExpr<Float> le(abs(theImg));
      tmp.copyData(le);
    }
    {
      PagedImage<Float> tmp(theImg.shape(), theImg.coordinates(), imName);
      LatticeExpr<Float> le(arg(theImg));
      tmp.copyData(le);
    }
  }
  
  void storeImg(String& fileName,ImageInterface<Float>& theImg)
  {
    PagedImage<Float> tmp(theImg.shape(), theImg.coordinates(), fileName);
    LatticeExpr<Float> le(theImg);
    tmp.copyData(le);
  }
  //
  //---------------------------------------------------------------------
  //
  // Get the time stamp for the for the current visibility integration.
  // Since VisTimeAverager() does not accumulate auto-correlations (it
  // should though!), and the VisBuffer can potentially have
  // auto-correlation placeholders, vb.time()(0) may not be correct (it
  // will be in fact zero when AC rows are present).  So look for the
  // first timestamp of a row corresponding to an unflagged
  // cross-correlation.
  //
  Double getCurrentTimeStamp(const VisBuffer& vb)
  {
    Int N=vb.nRow();
    for(Int i=0;i<N;i++)
      {
	if ((!vb.flagRow()(i)) && (vb.antenna1()(i) != vb.antenna2()(i)))
	  return vb.time()(i);
      }
    return 0.0;
  }
  //
  //---------------------------------------------------------------------
  // Compute the Parallactic Angle for the give VisBuffer
  //
  void getHADec(MeasurementSet& ms, const VisBuffer& vb, 
		Double& HA, Double& RA, Double& Dec)
  {
    MEpoch last;
    Double time = getCurrentTimeStamp(vb);
    
    Unit Second("s"), Day("d");
    MEpoch epoch(Quantity(time,Second),MEpoch::TAI);
    MPosition pos;
    MSObservationColumns msoc(ms.observation());
    String ObsName=msoc.telescopeName()(vb.arrayId());
    
    if (!MeasTable::Observatory(pos,ObsName))
      throw(AipsError("Observatory position for "+ ObsName + " not found"));
    
    MeasFrame frame(epoch,pos);
    MEpoch::Convert toLAST = MEpoch::Convert(MEpoch::Ref(MEpoch::TAI,frame),
					     MEpoch::Ref(MEpoch::LAST,frame));
    RA=vb.direction1()(0).getAngle().getValue()(0);    
    if (RA < 0.0) RA += M_PI*2.0;
    Dec=vb.direction1()(0).getAngle().getValue()(1);    

    last = toLAST(epoch);
    Double LST   = last.get(Day).getValue();
    LST -= floor(LST); // Extract the fractional day
    LST *= 2*C::pi;// Convert to Raidan

    cout << "LST = " << LST << " " << LST/(2*C::pi) << endl;
    
    HA = LST - RA;
  }
  //
  //---------------------------------------------------------------------
  // Compute the Parallactic Angle for the give VisBuffer
  //
  Double getPA(const VisBuffer& vb)
  {
    Double pa;
    Vector<Float> antPA = vb.feed_pa(getCurrentTimeStamp(vb));
    pa = sum(antPA)/(antPA.nelements()-1);
    return pa;
  }
  //
  //---------------------------------------------------------------------
  //
  // Make stokes axis, given the polarization types.
  //
  void makeStokesAxis(Int npol_p, Vector<String>& polType, Vector<Int>& whichStokes)
  {
    //    Vector<String> polType=msc.feed().polarizationType()(0);
    SkyModel::PolRep polRep_p;
    LogIO os(LogOrigin("Utils", "makeStokesAxis", WHERE));

    if (polType(0)!="X" && polType(0)!="Y" &&
	polType(0)!="R" && polType(0)!="L") 
      {
	os << "Warning: Unknown stokes types in feed table: ["
	   << polType(0) << ", " << polType(1) << "]" << endl
	   << "Results open to question!" << LogIO::POST;
      }
  
    if (polType(0)=="X" || polType(0)=="Y") 
      {
	polRep_p=SkyModel::LINEAR;
	os << "Preferred polarization representation is linear" << LogIO::POST;
      }
    else 
      {
	polRep_p=SkyModel::CIRCULAR;
	os << "Preferred polarization representation is circular" << LogIO::POST;
      }

    //    Vector<Int> whichStokes(npol_p);
    switch(npol_p) 
      {
      case 1:
	whichStokes.resize(1);
	whichStokes(0)=Stokes::I;
	os <<  "Image polarization = Stokes I" << LogIO::POST;
	break;
      case 2:
	whichStokes.resize(2);
	whichStokes(0)=Stokes::I;
	if (polRep_p==SkyModel::LINEAR) 
	  {
	    whichStokes(1)=Stokes::Q;
	    os <<  "Image polarization = Stokes I,Q" << LogIO::POST;
	  }
      else 
	{
	  whichStokes(1)=Stokes::V;
	  os <<  "Image polarization = Stokes I,V" << LogIO::POST;
	}
	break;
      case 3:
	whichStokes.resize(3);
	whichStokes(0)=Stokes::I;
	whichStokes(1)=Stokes::Q;
	whichStokes(1)=Stokes::U;
	os <<  "Image polarization = Stokes I,Q,U" << LogIO::POST;
	break;
      case 4:
	whichStokes.resize(4);
	whichStokes(0)=Stokes::I;
	whichStokes(1)=Stokes::Q;
	whichStokes(2)=Stokes::U;
	whichStokes(3)=Stokes::V;
	os <<  "Image polarization = Stokes I,Q,U,V" << LogIO::POST;
	break;
      default:
	os << LogIO::SEVERE << "Illegal number of Stokes parameters: " << npol_p
	   << LogIO::POST;
      };
  }
  //
  //--------------------------------------------------------------------------------------------
  //  
  Bool isVBNaN(const VisBuffer &vb,String& mesg)
  {
    IPosition ndx(3,0);
    Int N = vb.nRow();
    for(ndx(2)=0;ndx(2)<N;ndx(2)++)
      if (isnan(vb.modelVisCube()(ndx).real()) ||
	  isnan(vb.modelVisCube()(ndx).imag())
	  )
	{
	  ostringstream os;
	  os << ndx(2) << " " << vb.antenna1()(ndx(2)) << "-" << vb.antenna2()(ndx(2)) << " "
	     << vb.flagCube()(ndx) << " " << vb.flag()(0,ndx(2)) << " " << vb.weight()(ndx(2));
	  mesg = os.str().c_str();
	  return True;
	}
    return False;
  }
  //
  //--------------------------------------------------------------------------------------------
  //  
  /////////////////////////////////////////////////////////////////////////////
  //
  // IChangeDetector  - an interface class to detect changes in the VisBuffer
  //     Exact meaning of the "change" is defined in the derived classes
  //     (e.g. a change in the parallactic angle)
  
  // return True if a change occurs somewhere in the buffer
  Bool IChangeDetector::changed(const VisBuffer &vb) const throw(AipsError)
  {
     for (Int i=0;i<vb.nRow();++i)
          if (changed(vb,i)) return True;
     return False;
  }

  // return True if a change occurs somewhere in the buffer starting from row1
  // up to row2 (row2=-1 means up to the end of the buffer). The row number,
  // where the change occurs is returned in the row2 parameter
  Bool IChangeDetector::changedBuffer(const VisBuffer &vb, Int row1, 
		   Int &row2) const throw(AipsError)
  {
    if (row1<0) row1=0;
    Int jrow = row2;
    if (jrow < 0) jrow = vb.nRow()-1;
    DebugAssert(jrow<vb.nRow(),AipsError);
    
    // It is not important now to have a separate function for a "block"
    // operation. Because an appropriate caching is implemented inside
    // VisBuffer, changed(vb,row) can be called in the
    // loop without a perfomance penalty. This method returns the 
    // first row where the change occured rather than the last unchanged 
    // row as it was for BeamSkyJones::changedBuffer
      
    for (Int ii=row1;ii<=jrow;++ii)
         if (changed(vb,ii)) {
             row2 = ii;
             return True;
         }
    return False;
  }
  
  // a virtual destructor to make the compiler happy
  IChangeDetector::~IChangeDetector() throw(AipsError) {}
  
  //
  /////////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////////
  //
  // ParAngleChangeDetector - a class to detect a change in the parallatic 
  //                          angle
  //
  
  // set up the tolerance, which determines how much the position angle should
  // change to report the change by this class
  ParAngleChangeDetector::ParAngleChangeDetector(const Quantity &pa_tolerance) 
               throw(AipsError) : pa_tolerance_p(pa_tolerance.getValue("rad")),
		    last_pa_p(1000.) {}  // 1000 is >> 2pi, so it is changed
                                         // after construction
  
  // Set the value of the PA tolerance
  void ParAngleChangeDetector::setTolerance(const Quantity &pa_tolerance)
  {
    pa_tolerance_p = pa_tolerance.getValue("rad");
  }
  // reset to the state which exist just after construction
  void ParAngleChangeDetector::reset() throw(AipsError)
  {
      last_pa_p=1000.; // it is >> 2pi, which would force a changed state
  }
     
  // return parallactic angle tolerance
  Quantity ParAngleChangeDetector::getParAngleTolerance() const throw(AipsError)
  {
      return Quantity(pa_tolerance_p,"rad");
  }
  
  // return True if a change occurs in the given row since the last call 
  // of update
  Bool ParAngleChangeDetector::changed(const VisBuffer &vb, Int row) 
	        const throw(AipsError)
  {
     if (row<0) row=0;
     //     const Double feed1_pa=vb.feed1_pa()[row];
     Double feed1_pa=getPA(vb);
     if (abs(feed1_pa-last_pa_p) > pa_tolerance_p) 
       {
//  	 cout << "Utils: " << feed1_pa*57.295 << " " << last_pa_p*57.295 << " " << abs(feed1_pa-last_pa_p)*57.295 << " " << ttt*57.295 << " " << vb.time()(0)-4.51738e+09 << endl;
	 return True;
       }
     //     const Double feed2_pa=vb.feed2_pa()[row];
     Double feed2_pa = getPA(vb);
     if (abs(feed2_pa-last_pa_p) > pa_tolerance_p) 
       {
//  	 cout << "Utils: " << feed2_pa*57.295 << " " 
//  	      << last_pa_p*57.295 << " " 
//  	      << abs(feed2_pa-last_pa_p)*57.295 << " " << ttt*57.295 << vb.time()(0)-4.51738e+09 <<endl;
	 return True;
       }
     return False;
  }
  
  // start looking for a change from the given row of the VisBuffer
  void ParAngleChangeDetector::update(const VisBuffer &vb, Int row) 
	         throw(AipsError)
  {
     if (row<0) row=0;
     const Double feed1_pa=vb.feed1_pa()[row];
     const Double feed2_pa=vb.feed2_pa()[row];
     if (abs(feed1_pa-feed2_pa)>pa_tolerance_p) {
	 LogIO os;
	 os<<LogIO::WARN << LogOrigin("ParAngleChangeDetector","update") 
           <<"The parallactic angle is different at different stations"
	   <<LogIO::POST<<LogIO::WARN <<LogOrigin("ParAngleChangeDetector","update")
	   <<"The result may be incorrect. Continuing anyway."<<LogIO::POST;
     }
     last_pa_p=(feed1_pa+feed2_pa)/2.;
  }

  Int getPhaseCenter(MeasurementSet& ms, MDirection& dir0, Int whichField)
  {
    MSFieldColumns msfc(ms.field());
    if (whichField < 0)
      {
	MSRange msr(ms);
	//
	// An array of shape [2,1,1]!
	//
	Vector<Int> fieldId;
	fieldId = msr.range(MSS::FIELD_ID).asArrayInt(RecordFieldId(0));
	
	Array<Double> phaseDir = msfc.phaseDir().getColumn();
	
	if (fieldId.nelements() <= 0)
	  throw(AipsError("getPhaseCenter: No fields found in the selected MS."));
	
	IPosition ndx0(3,0,0,0),ndx1(3,1,0,0);
	
	Double maxRA, maxDec, RA,Dec,RA0,Dec0, dist0;
	maxRA = maxDec=0;
	for(uInt i=0;i<fieldId.nelements();i++)
	  {
	    RA   = phaseDir(IPosition(3,0,0,fieldId(i)));
	    Dec  = phaseDir(IPosition(3,1,0,fieldId(i)));
	    maxRA += RA; maxDec += Dec;
	  }
	RA0=maxRA/fieldId.nelements(); Dec0=maxDec/fieldId.nelements();
	
	dist0=10000;
	Int field0=0;
	for(uInt i=0;i<fieldId.nelements();i++)
	  {
	    RA   = RA0-phaseDir(IPosition(3,0,0,fieldId(i)));
	    Dec  = Dec0-phaseDir(IPosition(3,1,0,fieldId(i)));
	    Double dist=sqrt(RA*RA + Dec*Dec);
	    if (dist < dist0) {field0=fieldId(i);dist0=dist;};
	  }
	dir0=msfc.phaseDirMeasCol()(field0)(IPosition(1,0));
	//dir0=msfc.phaseDirMeasCol()(6)(IPosition(1,0));
	return field0;
      }
    else
      {
	dir0=msfc.phaseDirMeasCol()(whichField)(IPosition(1,0));
	return whichField;
      }
  }
  //
  //------------------------------------------------------------------
  //
  Bool findMaxAbsLattice(const ImageInterface<Float>& lattice,
			 Float& maxAbs,IPosition& posMaxAbs) 
  {
    posMaxAbs = IPosition(lattice.shape().nelements(), 0);
    maxAbs=0.0;

    const IPosition tileShape = lattice.niceCursorShape();
    TiledLineStepper ls(lattice.shape(), tileShape, 0);
    {
      RO_LatticeIterator<Float> li(lattice, ls);
      for(li.reset();!li.atEnd();li++)
	{
	  IPosition posMax=li.position();
	  IPosition posMin=li.position();
	  Float maxVal=0.0;
	  Float minVal=0.0;
	  
	  minMax(minVal, maxVal, posMin, posMax, li.cursor());
	  if((maxVal)>(maxAbs)) 
	    {
	      maxAbs=maxVal;
	      posMaxAbs=li.position();
	      posMaxAbs(0)=posMax(0);
	    }
	}
    }

    return True;
  };
  //
  //------------------------------------------------------------------
  //
  Bool findMaxAbsLattice(const ImageInterface<Float>& masklat,
			 const Lattice<Float>& lattice,
			 Float& maxAbs,IPosition& posMaxAbs, 
			 Bool flip)
  {
    
    AlwaysAssert(masklat.shape()==lattice.shape(), AipsError);

    Array<Float> msk;
  
    posMaxAbs = IPosition(lattice.shape().nelements(), 0);
    maxAbs=0.0;
    //maxAbs=-1.0e+10;
    const IPosition tileShape = lattice.niceCursorShape();
    TiledLineStepper ls(lattice.shape(), tileShape, 0);
    TiledLineStepper lsm(masklat.shape(), tileShape, 0);
    {
      RO_LatticeIterator<Float> li(lattice, ls);
      RO_LatticeIterator<Float> lim(masklat, lsm);
      for(li.reset(),lim.reset();!li.atEnd();li++,lim++) 
	{
	  IPosition posMax=li.position();
	  IPosition posMin=li.position();
	  Float maxVal=0.0;
	  Float minVal=0.0;
	  
	  msk = lim.cursor();
	  if(flip) msk = (Float)1.0 - msk;
	  
	  //minMaxMasked(minVal, maxVal, posMin, posMax, li.cursor(),lim.cursor());
	  minMaxMasked(minVal, maxVal, posMin, posMax, li.cursor(),msk);
	  
	  
	  if((maxVal)>(maxAbs)) 
	    {
	      maxAbs=maxVal;
	      posMaxAbs=li.position();
	      posMaxAbs(0)=posMax(0);
	    }
	}
    }

    return True;
  }
  
} // namespace casa
