//# StandardVisCal.cc: Implementation of Standard VisCal types
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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

#include <synthesis/MeasurementComponents/StandardVisCal.h>
#include <synthesis/MeasurementComponents/CalCorruptor.h>

#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisBuffAccumulator.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <synthesis/MeasurementEquations/VisEquation.h>
#include <scimath/Fitting/LSQFit.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <tables/Tables/ExprNode.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/GenSort.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/Memory.h>
#include <casa/System/Aipsrc.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

#define RI_DEBUG

// math.h ?

namespace casa { //# NAMESPACE CASA - BEGIN


// **********************************************************
//  PJones
//

PJones::PJones(VisSet& vs) :
  VisCal(vs), 
  VisMueller(vs),
  VisJones(vs),
  pjonestype_(Jones::Diagonal),
  pa_()
{
  if (prtlev()>2) cout << "P::P(vs)" << endl;
}

PJones::~PJones() {
  if (prtlev()>2) cout << "P::~P()" << endl;
}

// We need to locally form the Jones according to 
//  the correlations we have

void PJones::syncJones(const Bool& doInv) {

  // Circulars
  if (vb().corrType()(0)==5)
    pjonestype_=Jones::Diagonal;

  // Linears
  else if (vb().corrType()(0)==9)
    pjonestype_=Jones::General;

  VisJones::syncJones(doInv);

}


void PJones::calcPar() {

  if (prtlev()>6) cout << "      VC::calcPar()" << endl;

  // Get parallactic angle from the vb:
  pa().resize(nAnt());
  pa() = vb().feed_pa(currTime());

  // Initialize parameter arrays
  currCPar().resize(1,1,nAnt());
  currParOK().resize(1,1,nAnt());
  currParOK()=True;

  // Fill currCPar() with exp(i*pa)
  Float* a=pa().data();
  Complex* cp=currCPar().data();
  Double ang(0.0);
  for (Int iant=0;iant<nAnt();++iant,++a,++cp) {
    ang=Double(*a);
    (*cp) = Complex(cos(ang),sin(ang));  // as a complex number
  }
  //  cout << "ang = " << ang << endl;

  // Pars now valid, matrices not
  validateP();
  invalidateJ();

}

// Calculate a single Jones matrix by some means from parameters
void PJones::calcOneJones(Vector<Complex>& mat, Vector<Bool>& mOk,
			  const Vector<Complex>& par, const Vector<Bool>& pOk ) {

  if (prtlev()>10) cout << "       P::calcOneJones()" << endl;

  //  TBD: handle linears too

  // Circular version:
  if (pOk(0)) {

    switch (jonesType()) {
    case Jones::Diagonal: {
      mat(0)=conj(par(0));  // exp(-ia)
      mat(1)=par(0);        // exp(ia)
      mOk=True;
      break;
    }
    case Jones::General: {
      Float a=arg(par(0));
      mat(0)=mat(3)=cos(a);
      mat(1)=sin(a);
      mat(2)=-mat(1);
      mOk=True;
      break;
    }
    default:
      throw(AipsError("PJones doesn't know if it is Diag (Circ) or General (Lin)"));
      break;

    }



  }
}



// **********************************************************
//  TJones Implementations
//

TJones::TJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  SolvableVisJones(vs),    // immediate parent
  tcorruptor_p(NULL)
{
  if (prtlev()>2) cout << "T::T(vs)" << endl;
}
TJones::TJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  SolvableVisJones(nAnt),
  tcorruptor_p(NULL)
{
  if (prtlev()>2) cout << "T::T(nAnt)" << endl;
}

TJones::~TJones() {
  if (prtlev()>2) cout << "T::~T()" << endl;
}

void TJones::guessPar(VisBuffer& vb) {

  if (prtlev()>4) cout << "   T::guessPar(vb)" << endl;

  // Assumes:  1. corrs in canonical order
  //           2. vb has 1 channel (has been freq-averaged)

  // Make an antenna-based guess at T
  //  Correlation membership-dependence
  //  nCorr = 1: use icorr=0
  //  nCorr = 2: use icorr=[0,1]
  //  nCorr = 4: use icorr=[0,3]

  Int nCorr(1);
  Int nDataCorr(vb.visCube().shape()(0));
  Vector<Int> corridx(1,0);
  if (nDataCorr==2) {
    nCorr=2;
    corridx.resize(nCorr);
    corridx(0)=0;
    corridx(1)=1;
  } 
  else if (nDataCorr==4) {
    nCorr=2;
    corridx.resize(nCorr);
    corridx(0)=0;
    corridx(1)=3;
  }

  // Find out which ants are available
  // TBD: count nominal guessant rows, insist not much less than nAnt
  Vector<Bool> antok(nAnt(),False);
  Vector<Bool> rowok(vb.nRow(),False);
  for (Int irow=0;irow<vb.nRow();++irow) {
    // Is this row ok?
    rowok(irow)= (!vb.flagRow()(irow) &&
		  (vb.antenna1()(irow)!=vb.antenna2()(irow)) &&
		  nfalse(vb.flag().column(irow))> 0 );
    if (rowok(irow)) {
      antok(vb.antenna1()(irow))=True;
      antok(vb.antenna2()(irow))=True;
    }
  }

  // Assume refant is the target ant, for starters
  Int guessant(refant());

  // If no refant specified, or no data for refant
  //   base first guess on first good ant
  if (guessant<0 || !antok(guessant)) {
    guessant=0;
    while (!antok(guessant++));
  }

  AlwaysAssert(guessant>-1,AipsError);

  Cube<Complex>& V(vb.visCube());
  Float amp(0.0),ampave(0.0);
  Int namp(0);
  solveCPar()=Complex(0.0);
  for (Int irow=1;irow<vb.nRow();++irow) {  // why not row zero? copied from Calibrator

    if (rowok(irow)) {
      Int a1=vb.antenna1()(irow);
      Int a2=vb.antenna2()(irow);

      // If this row contains the guessant
      if (a1 == guessant || a2==guessant) {
      
	for (Int icorr=0;icorr<nCorr;icorr++) {
	  Complex& Vi(V(corridx(icorr),0,irow));
	  amp=abs(Vi);
	  if (amp>0.0f) {
	    if (a1 == guessant)
	      solveCPar()(0,0,a2)+=(conj(Vi)/amp/Float(nCorr));
	    //	      solveCPar()(0,0,a2)+=(conj(Vi)/Float(nCorr));
	    else
	      solveCPar()(0,0,a1)+=((Vi)/amp/Float(nCorr));
	    //	      solveCPar()(0,0,a1)+=((Vi)/Float(nCorr));
	    
	    ampave+=amp;
	    namp++;
	    //	cout << "          " << abs(Vi) << " " << arg(Vi)*180.0/C::pi << endl;
	  }
	}
      } // guessant in row
    } // rowok
  } // irow

  //  cout << "Guess:" << endl
  //   << "amp = " << amplitude(solveCPar())
  //     << endl;
 

  // Scale them by the mean amplitude
  ampave/=Float(namp);
  ampave=sqrt(ampave);

  //  cout << "ampave = " << ampave << endl;

  solveCPar()*=Complex(ampave);
  //  solveCPar()/=Complex(ampave);
  solveCPar()(0,0,guessant) = Complex(ampave);
  solveCPar()(LogicalArray(amplitude(solveCPar())==0.0f)) = Complex(ampave);
  solveParOK()=True;

  //  solveCPar()*=Complex(0.9);

  //  cout << "Guess:" << endl
  //       << "amp = " << amplitude(solveCPar())
  //       << "phase = " << phase(solveCPar())
  //       << endl;

}

// Fill the trivial DJ matrix elements
void TJones::initTrivDJ() {

  if (prtlev()>4) cout << "   T::initTrivDJ" << endl;

  // Must be trivial
  AlwaysAssert((trivialDJ()),AipsError);

  // This is the unit matrix
  //  TBD: could we use a Jones::Unit type instead?
  diffJElem().resize(IPosition(4,1,1,1,1));
  diffJElem()=Complex(1.0);

}

  





//============================================================



Int TJones::setupSim(VisSet& vs, const Record& simpar, Vector<Int>& nChunkPerSol, Vector<Double>& solTimes)
{

  LogIO os(LogOrigin("T", "setupSim()", WHERE));

  prtlev()=4; // debug

  if (prtlev()>2) cout << "   T::setupSim()" << endl;
  AlwaysAssert((isSimulated()),AipsError);
  // assume only one ms attached to the VI. need vi for mscolumns.
  VisIter& vi(vs.iter());
  

  Int nSim = sizeUpSim(vs,nChunkPerSol,solTimes);
  if (prtlev()>2) cout << " sized for Sim, " << nSim << " slots." << endl;

  // we can use the private access directly the corruptor
  // RIATM 
  tcorruptor_p = new TJonesCorruptor(nSim);
  //tcorruptor_p = new AtmosCorruptor(nSim);
  // but set the public one 
  corruptor_p = tcorruptor_p;
  if (prtlev()>4) cout << " TCorruptor created." << endl;

  corruptor_p->startTime()=min(solTimes);
  corruptor_p->stopTime()=max(solTimes);
  corruptor_p->prtlev()=prtlev();

  Int Seed(1234);
  if (simpar.isDefined("seed")) {    
    Seed=simpar.asInt("seed");
  }

  Float Beta(1.1); // exponent for generalized 1/f noise
  if (simpar.isDefined("beta")) {    
    Beta=simpar.asFloat("beta");
  }
  
  if (simpar.isDefined("mean_pwv"))
    tcorruptor_p->mean_pwv() = simpar.asFloat("mean_pwv");
  
  if (tcorruptor_p->mean_pwv()<=0)
    throw(AipsError("TCorruptor attempted initialization with undefined PWV"));

  Float Scale(.15); // scale of fluctuations rel to mean
  if (simpar.isDefined("delta_pwv") and simpar.isDefined("mean_pwv")) {    
    Scale=simpar.asFloat("delta_pwv")/simpar.asFloat("mean_pwv");
    if (Scale>.5) {
      Scale=.5;  
      os << LogIO::WARN << " decreasing PWV fluctuation magnitude to half of the mean PWV, " << simpar.asFloat("mean_pwv") << LogIO::POST;  
    }
  }


  // initialize spw etc information in Corruptor
  // the "initialize" function that's corruptor-specific calculates the 
  // corruption values
  // this preparation is just to tell the corruptor about its VC - maybe 
  // the logic should be changed so the corruptor doesn't have to know 

  const ROMSSpWindowColumns& spwcols = vi.msColumns().spectralWindow();

  if (prtlev()>3) cout << " SpwCols accessed:" << endl;
  if (prtlev()>3) cout << "   nSpw()= " << nSpw() << endl;
  if (prtlev()>3) cout << "   spwcols.nrow()= " << nSpw() << endl;

  AlwaysAssert(nSpw()==spwcols.nrow(),AipsError);

  // things will break if spw mapping, ie not in same order as in vs
  corruptor_p->nSpw()=nSpw();
  corruptor_p->nAnt()=nAnt();
  corruptor_p->currAnt()=0;
  corruptor_p->currSpw()=0;
  corruptor_p->fRefFreq().resize(nSpw());
  corruptor_p->fnChan().resize(nSpw());
  corruptor_p->fWidth().resize(nSpw());

  for (Int irow=0;irow<nSpw();++irow) { 
    corruptor_p->fRefFreq()[irow]=spwcols.refFrequency()(irow);
    corruptor_p->fnChan()[irow]=spwcols.numChan()(irow);
    corruptor_p->fWidth()[irow]=spwcols.totalBandwidth()(irow); 
    // totalBandwidthQuant ?  in other places its assumed to be in Hz
  }
  // see MSsummary.cc for more info/examples

  if (simpar.isDefined("mode")) {    
    if (prtlev()>2)
      cout << "initializing TCorruptor with mode " << simpar.asString("mode") << endl;

    if (simpar.asString("mode") == "test")
      tcorruptor_p->initialize();
    else if (simpar.asString("mode") == "individual") 
      tcorruptor_p->initialize(Seed,Beta,Scale);
    else if (simpar.asString("mode") == "screen") {
      const ROMSAntennaColumns& antcols(vi.msColumns().antenna());
      if (simpar.isDefined("windspeed")) {
	tcorruptor_p->windspeed()=simpar.asFloat("windspeed");
	tcorruptor_p->initialize(Seed,Beta,Scale,antcols);
      } else
	throw(AipsError("Unknown wind speed for TJonesCorruptor"));        
    } else 
      throw(AipsError("Unknown Mode for TJonesCorruptor"));        
  } else {
    throw(AipsError("No Mode specified for TJones corruptor."));
  }
  
  return nSim;
}






Bool TJones::simPar(VisIter& vi, const Int nChunks){

  LogIO os(LogOrigin("T", "simPar()", WHERE));

  if (prtlev()>4) cout << "   T::simPar()" << endl;

  AlwaysAssert((isSimulated()),AipsError);
  
  // sizeUpSim did solveCPar().resize(nPar(),nChanPar(),nAnt());

  // RI TODO T:simPar loop through correlations or use corridx ?

#ifndef RI_DEBUG
    try {
#endif
    
    Vector<Int> a1;
    Vector<Int> a2;
    Matrix<Bool> flags;

    solveCPar()=Complex(0.0);
    // n good VI elements averaged in each CPar() entry:
    IPosition cparshape=solveCPar().shape();
    Cube<Complex> nGood(cparshape);
    solveParOK()=False;

    if (corruptor_p->curr_slot()<1 and prtlev()>3) 
      cout << "  shape of solveCPar = " << cparshape << 
	"  and cs().shape(spw=" << corruptor_p->currSpw() << ") = " << 
	cs().shape(corruptor_p->currSpw()) << endl;

    Vector<Double> timevec;
    Double starttime,stoptime;
    starttime=vi.time(timevec)[0];

    for (Int ichunk=0;ichunk<nChunks;++ichunk) {
      Int spw(vi.spectralWindow());
      corruptor_p->currSpw()=spw;

      for (vi.origin(); vi.more(); vi++) {
        // these need to be updated whenever the VI scrolls forward
        vi.antenna1(a1);
        vi.antenna2(a2);
        vi.flag(flags);
        vi.time(timevec);
    
	for (Int irow=0;irow<vi.nRow();++irow) {

	  if (nfalse(flags.column(irow))> 0 ) {
	    if ( a1(irow)==a2(irow) ) {
	      // autocorrels should get 1. for multiplicative VC
	      solveCPar()(0,focusChan(),a1(irow))=1.0;
	    } else {

            if (corruptor_p->curr_time()!=timevec[irow]) {
              corruptor_p->curr_time()=timevec[irow];
              // find new slot if required
              Double dt(1e10),dt0(-1);
              dt0 = abs(corruptor_p->slot_time() - timevec[irow]);

              if (timevec[irow]<corruptor_p->curr_time()) {
                for (Int newslot=corruptor_p->curr_slot()-1;
                     newslot>=0;newslot--) {
                  dt=abs(corruptor_p->slot_time(newslot) - timevec[irow]);
                  if (dt<dt0) {
                    // use this check for on-depand corruptors to get new val.
                    corruptor_p->curr_slot()=newslot;
                    dt0 = dt;
                    if (prtlev()>4)
                      cout << "    G:simPar retreating to time " <<
                        timevec[irow] << endl;
                  }
                }
              } else {
                for (Int newslot=corruptor_p->curr_slot()+1;
                     newslot<corruptor_p->nSim();newslot++) {
                  dt=abs(corruptor_p->slot_time(newslot) - timevec[irow]);
                  if (dt<dt0) {
                    // use this check for on-depand corruptors to get new val.
                    corruptor_p->curr_slot()=newslot;
                    dt0 = dt;
                    if (prtlev()>4)
                      cout << "    G:simPar advancing to time " <<
                        timevec[irow] << endl;
                  }
                }
              }
            }

            corruptor_p->currAnt()=a1(irow);

	    // chan loop has to be outside of this because George has 
	    // hardcoded that a SVJ only has one channel.
	    if(tcorruptor_p->mode()=="test" or tcorruptor_p->mode()=="1d")
	      solveCPar()(0,focusChan(),a1(irow))=tcorruptor_p->gain(focusChan());
	    else if (tcorruptor_p->mode()=="2d") {
	      // RI_TODO modify x,y by tan(zenith angle)*(layer altitude)
	      Int ix(Int(tcorruptor_p->antx()[a1(irow)]));
	      Int iy(Int(tcorruptor_p->anty()[a1(irow)]));
	      if (prtlev()>5) 
		cout << " getting gain for antenna ix,iy = " << ix << "," << iy << endl;  
	      solveCPar()(0,focusChan(),a1(irow))=tcorruptor_p->gain(ix,iy,focusChan());
	    } else 
	      throw(AipsError("T: unknown corruptor mode "+tcorruptor_p->mode()));
	    } // if not autocorr
	  }// if not flagged
	}// row
      }// vi
      if (vi.moreChunks()) vi.nextChunk();
    } // chunk loop

    // RI TODO T::simPar  don't divide by zero  **
    for (Int i=0;i<cparshape(2);i++)
      solveCPar().xyPlane(i) =  solveCPar().xyPlane(i) / nGood.xyPlane(i);

    vi.time(timevec);
    Int tvsize;
    timevec.shape(tvsize);
    stoptime=timevec[tvsize-1];
    refTime() = 0.5*(starttime+stoptime);
    interval() = (stoptime-starttime);
    currField() = vi.fieldId();

#ifndef RI_DEBUG    
  } catch (AipsError x) {
    if (tcorruptor_p) delete tcorruptor_p;
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
#endif
  return True;
}













// **********************************************************
//  TfJones Implementations
//

TfJones::TfJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  TJones(vs)              // immediate parent
{
  if (prtlev()>2) cout << "Tf::Tf(vs)" << endl;
}

TfJones::TfJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  TJones(nAnt)
{
  if (prtlev()>2) cout << "Tf::Tf(nAnt)" << endl;
}

TfJones::~TfJones() {
  if (prtlev()>2) cout << "Tf::~Tf()" << endl;
}











// **********************************************************
//  GJones Implementations
//

GJones::GJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  SolvableVisJones(vs),    // immediate parent
  gcorruptor_p(NULL)
{
  if (prtlev()>2) cout << "G::G(vs)" << endl;
}

GJones::GJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  SolvableVisJones(nAnt),
  gcorruptor_p(NULL)
{
  if (prtlev()>2) cout << "G::G(nAnt)" << endl;
}

GJones::~GJones() {
  if (prtlev()>2) cout << "G::~G()" << endl;
}

void GJones::guessPar(VisBuffer& vb) {

  if (prtlev()>4) cout << "   G::guessPar(vb)" << endl;

  // Make a guess at antenna-based G
  //  Correlation membership-dependencexm
  //  assumes corrs in canonical order
  //  nCorr = 1: use icorr=0
  //  nCorr = 2: use icorr=[0,1]
  //  nCorr = 4: use icorr=[0,3]

  Int nCorr(2);
  Int nDataCorr(vb.visCube().shape()(0));
  Vector<Int> corridx(nCorr,0);
  if (nDataCorr==2) {
    corridx(0)=0;
    corridx(1)=1;
  } 
  else if (nDataCorr==4) {
    corridx(0)=0;
    corridx(1)=3;
  }

  Int guesschan(vb.visCube().shape()(1)-1);

  //  cout << "guesschan = " << guesschan << endl;
  //  cout << "nCorr = " << nCorr << endl;
  //  cout << "corridx = " << corridx << endl;


  // Find out which ants are available
  Vector<Int> antok(nAnt(),0);
  Vector<Bool> rowok(vb.nRow(),False);
  for (Int irow=0;irow<vb.nRow();++irow) {
    // Is this row ok
    rowok(irow)= (!vb.flagRow()(irow) &&
		  vb.antenna1()(irow)!=vb.antenna2()(irow) &&
		  nfalse(vb.flag().column(irow))> 0 );
    if (rowok(irow)) {
      antok(vb.antenna1()(irow))++;
      antok(vb.antenna2()(irow))++;
    }
  }

  // Assume refant is the target ant, for starters
  Int guessant(refant());
  //  Int guessant(-1);

  // If no refant specified, or no data for refant
  //   base first guess on first good ant
  if (guessant<0 || antok(guessant)<1) {
    guessant=0;
    while (antok(guessant)<1) guessant++;
  }

  //  cout << "antok = " << antok << endl;

  //  cout << "guessant = " << guessant << "  (" << currSpw() << ")" << endl;

  AlwaysAssert(guessant>-1,AipsError);

  Cube<Complex>& V(vb.visCube());
  Float amp(0.0),ampave(0.0);
  Int namp(0);
  solveCPar()=Complex(0.0);
  for (Int irow=1;irow<vb.nRow();++irow) {

    if (rowok(irow) && !vb.flag()(guesschan,irow)) {
      Int a1=vb.antenna1()(irow);
      Int a2=vb.antenna2()(irow);

      // If this row contains the guessant
      if (a1 == guessant || a2==guessant) {

	//	cout << a1 << " " << a2 << " " 
	//	     << vb.visCube().xyPlane(irow).column(guesschan) << " "
	//	     << amplitude(vb.visCube().xyPlane(irow).column(guesschan)) << " "
	//	     << endl;

	for (Int icorr=0;icorr<nCorr;icorr++) {
	  Complex& Vi(V(corridx(icorr),guesschan,irow));
	  amp=abs(Vi);
	  if (amp>0.0f) {
	    if (a1 == guessant)
	      solveCPar()(icorr,0,a2)=conj(Vi);
	    else
	      solveCPar()(icorr,0,a1)=(Vi);
	      
	    ampave+=amp;
	    namp++;
	  }
	}
      } // guessant
    } // rowok
  } // irow

  // Scale them by the mean amplitude

  if (namp>0) {
    ampave/=Float(namp);
    ampave=sqrt(ampave);
    //  solveCPar()*=Complex(ampave);
    solveCPar()/=Complex(ampave);
    solveCPar()(0,0,guessant)=solveCPar()(1,0,guessant)=Complex(ampave);
    solveCPar()(LogicalArray(amplitude(solveCPar())==0.0f)) = Complex(ampave);
  }
  else
    solveCPar()=Complex(0.3);

  solveParOK()=True;

  //For scalar data, Set "other" pol soln to zero
  if (nDataCorr == 1)
    solveCPar()(IPosition(3,1,0,0),IPosition(3,1,0,nAnt()-1))=Complex(0.0);

  //  cout << "Guess:" << endl;
  //  cout << "amplitude(solveCPar())   = " << amplitude(solveCPar()) << endl;
  //  cout << "phases       = " << phase(solveCPar())*180.0/C::pi << endl;
  //  cout << "solveParOK() = " << solveParOK() << endl;

}

// Fill the trivial DJ matrix elements
void GJones::initTrivDJ() {

  if (prtlev()>4) cout << "   G::initTrivDJ" << endl;

  // Must be trivial
  AlwaysAssert((trivialDJ()),AipsError);

  //  1 0     0 0
  //  0 0  &  0 1
  // 
  if (diffJElem().nelements()==0) {
    diffJElem().resize(IPosition(4,2,2,1,1));
    diffJElem()=0.0;
    diffJElem()(IPosition(4,0,0,0,0))=Complex(1.0);
    diffJElem()(IPosition(4,1,1,0,0))=Complex(1.0);
  }

}


Int GJones::setupSim(VisSet& vs, const Record& simpar, Vector<Int>& nChunkPerSol, Vector<Double>& solTimes)
{

  LogIO os(LogOrigin("G", "setupSim()", WHERE));
  //prtlev()=4; // debug

  if (prtlev()>2) cout << "   G::setupSim()" << endl;
  AlwaysAssert((isSimulated()),AipsError);
  VisIter& vi(vs.iter());
  
  Int nSim = sizeUpSim(vs,nChunkPerSol,solTimes);
  if (prtlev()>2) cout << " sized for Sim with " << nSim << " slots."<< endl;

  gcorruptor_p = new GJonesCorruptor(nSim);
  corruptor_p = gcorruptor_p;
  if (prtlev()>3) cout << " GCorruptor created." << endl;

  //Vector<Int> corrType; vi.corrType(corrType);
  //uInt nCorr = corrType.nelements();
  corruptor_p->nCorr()=vi.nCorr();
  cout << "nCorr= " << vi.nCorr() <<endl;
      
  corruptor_p->startTime()=min(solTimes);
  corruptor_p->stopTime()=max(solTimes);
  corruptor_p->prtlev()=prtlev();

  Int Seed(1234);
  if (simpar.isDefined("seed")) {    
    Seed=simpar.asInt("seed");
  }

  Float Beta(1.1); // exponent for generalized 1/f noise
  if (simpar.isDefined("beta")) {    
    Beta=simpar.asFloat("beta");
  }
  
  if (simpar.isDefined("tsys")) {
    gcorruptor_p->tsys() = simpar.asFloat("tsys");
  } 
  
  Float Scale(.15); // scale of fluctuations rel to mean
  if (simpar.isDefined("scale")) {
    Scale=simpar.asFloat("scale");
    if (Scale>.5) {
      Scale=.5;
      os << LogIO::WARN << " decreasing PWV fluctuation magnitude to half of the mean PWV, " << simpar.asFloat("mean_pwv") << LogIO::POST;  
    }
  }

  const ROMSSpWindowColumns& spwcols = vi.msColumns().spectralWindow();

  if (prtlev()>3) cout << " SpwCols accessed:" << endl;
  if (prtlev()>3) cout << "   nSpw()= " << nSpw() << endl;
  if (prtlev()>3) cout << "   spwcols.nrow()= " << nSpw() << endl;

  AlwaysAssert(nSpw()==spwcols.nrow(),AipsError);

  // things will break if spw mapping, ie not in same order as in vs
  corruptor_p->nSpw()=nSpw();
  corruptor_p->nAnt()=nAnt();
  corruptor_p->currAnt()=0;
  corruptor_p->currSpw()=0;
  corruptor_p->fRefFreq().resize(nSpw());
  corruptor_p->fnChan().resize(nSpw());
  corruptor_p->fWidth().resize(nSpw());

  for (Int irow=0;irow<nSpw();++irow) { 
    corruptor_p->fRefFreq()[irow]=spwcols.refFrequency()(irow);
    corruptor_p->fnChan()[irow]=spwcols.numChan()(irow);
    corruptor_p->fWidth()[irow]=spwcols.totalBandwidth()(irow); 
    // totalBandwidthQuant ?  in other places its assumed to be in Hz
  }
  // see MSsummary.cc for more info/examples

  gcorruptor_p->initialize(Seed,Beta,Scale);
  
  //if (prtlev()>2) cout << "   G::setupSim()  nSim = " << nSim << endl;
  return nSim;
}






Bool GJones::simPar(VisIter& vi, const Int nChunks){
  LogIO os(LogOrigin("G", "simPar()", WHERE));
  if (prtlev()>4) cout << "   G::simPar()" << endl;
  
  AlwaysAssert((isSimulated()),AipsError);
#ifndef RI_DEBUG
  try {
#endif
    Vector<Int> a1;
    Vector<Int> a2;
    Matrix<Bool> flags;

    solveCPar()=Complex(0.0);
    // n good VI elements averaged in each CPar() entry:
    IPosition cparshape=solveCPar().shape();
    Cube<Complex> nGood(cparshape);
    solveParOK()=False;
    
    Vector<Double> timevec;
    Double starttime,stoptime;
    starttime=vi.time(timevec)[0];
    
    for (Int ichunk=0;ichunk<nChunks;++ichunk) {
      Int spw(vi.spectralWindow());
      corruptor_p->currSpw()=spw; 

      for (vi.origin(); vi.more(); vi++) {
        // these need to be updated whenever the VI scrolls forward
        vi.antenna1(a1);
        vi.antenna2(a2);
        vi.flag(flags);
	vi.time(timevec);

	for (Int irow=0;irow<vi.nRow();++irow) {
      
	  if (nfalse(flags.column(irow))> 0 ) {
	    if ( a1(irow)==a2(irow) ) {
	      // autocorrels should get 1. for multiplicative VC
	      for (Int icorr=0;icorr<corruptor_p->nCorr();icorr++) 
		solveCPar().xyPlane(a1(irow)).row(icorr) = Complex(1.);	      
	      nGood.xyPlane(a1(irow)) = nGood.xyPlane(a1(irow)) + Complex(1.);
	    } else {
	
	    if (corruptor_p->curr_time()!=timevec[irow]) {	  
	      corruptor_p->curr_time()=timevec[irow];          
	      // find new slot if required
	      Double dt(1e10),dt0(-1);
	      dt0 = abs(corruptor_p->slot_time() - timevec[irow]);
	  
	      if (timevec[irow]<corruptor_p->curr_time()) {
		for (Int newslot=corruptor_p->curr_slot()-1;
		     newslot>=0;newslot--) {
		  dt=abs(corruptor_p->slot_time(newslot) - timevec[irow]);
		  if (dt<dt0) {
		    // use this check for on-depand corruptors to get new val.
		    corruptor_p->curr_slot()=newslot;
		    dt0 = dt;
		    if (prtlev()>4) 
		      cout << "    G:simPar retreating to time " << 
			timevec[irow] << endl;
		  }
		}
	      } else {          
		for (Int newslot=corruptor_p->curr_slot()+1;
		     newslot<corruptor_p->nSim();newslot++) {
		  dt=abs(corruptor_p->slot_time(newslot) - timevec[irow]);
		  if (dt<dt0) {
		    // use this check for on-demand corruptors to get new val.
		    corruptor_p->curr_slot()=newslot;
		    dt0 = dt;
		    if (prtlev()>4) 
		      cout << "    G:simPar advancing to time " << 
			timevec[irow] << endl;
		  }         
		}
	      }
	    }

	    corruptor_p->currAnt()=a1(irow);

	    // Gcorruptor::gain doesn't currently depend on chan/freq:
	    // RI TODO Gcorruptor needs freqDepPar and switch in gain()

	    if (freqDepPar()) { 
	      // then this is actually a B which is a Gf
	      for (Int icorr=0;icorr<corruptor_p->nCorr();icorr++) {
		solveCPar()(icorr,focusChan(),a1(irow)) = 
		  gcorruptor_p->gain(icorr,focusChan());
		nGood(icorr,focusChan(),a1(irow)) += Complex(1.);
		solveParOK()(icorr,focusChan(),a1(irow)) = True;
	      }
	    } else {
	      // do all channels:
	      for (Int icorr=0;icorr<corruptor_p->nCorr();icorr++) 
		solveCPar().xyPlane(a1(irow)).row(icorr) = 
		  gcorruptor_p->gain(icorr,0);
	      nGood.xyPlane(a1(irow)) = nGood.xyPlane(a1(irow)) + Complex(1.);
	      solveParOK().xyPlane(a1(irow)) = True;
	    }	    
	    } // if not autocorr
	  } // if not flagged 
	} // irow loop
      } // vi loop
      if (vi.moreChunks()) vi.nextChunk();
    } // chunk loop

    // RI TODO G::simPar  don't divide by zero  **
    for (Int i=0;i<cparshape(2);i++)
      solveCPar().xyPlane(i) =  solveCPar().xyPlane(i) / nGood.xyPlane(i);
    
    vi.time(timevec);
    Int tvsize;
    timevec.shape(tvsize);
    stoptime=timevec[tvsize-1];
    refTime() = 0.5*(starttime+stoptime);
    interval() = (stoptime-starttime);
    currField() = vi.fieldId();

#ifndef RI_DEBUG
  } catch (AipsError x) {
    if (gcorruptor_p) delete gcorruptor_p;
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
#endif
  return True;
}




GJonesCorruptor::GJonesCorruptor(const Int nSim) : 
  CalCorruptor(nSim),  // parent
  tsys_(0.0)
{}

GJonesCorruptor::~GJonesCorruptor() {
  if (prtlev()>2) cout << "GCorruptor::~GCorruptor()" << endl;
}



Matrix<Complex>* GJonesCorruptor::drift() { 
  if (currAnt()<=drift_p.nelements())
    return drift_p[currAnt()];
  else
    return NULL;
};
 

void GJonesCorruptor::initialize() {
  // for testing only
  if (prtlev()>2) cout << "GCorruptor::init [test]" << endl;
}

void GJonesCorruptor::initialize(const Int Seed, const Float Beta, const Float scale) {
  // individual delays for each antenna

  fBM* myfbm = new fBM(nSim());
  drift_p.resize(nAnt(),False,True);
  for (Int iant=0;iant<nAnt();++iant) {
    drift_p[iant] = new Matrix<Complex>(nCorr(),nSim());
    for (Int icorr=0;icorr<nCorr();++icorr){
      myfbm->initialize(Seed+iant+icorr,Beta); // (re)initialize
      Float pmean = mean(myfbm->data());
      Float rms = sqrt(mean( ((myfbm->data())-pmean)*((myfbm->data())-pmean) ));
      if (prtlev()>3 and currAnt()<2) {
	cout << "RMS fBM fluctuation for antenna " << iant 
	     << " = " << rms << " ( " << pmean << " ; beta = " << Beta << " ) " << endl;      
      }
      // amp
      Vector<Float> amp = (myfbm->data()) * scale/rms;
      for (Int i=0;i<nSim();++i)  {
	(*(drift_p[iant]))(icorr,i) = Complex(1.+amp[i],0);
      }
      
      // phase
      myfbm->initialize((Seed+iant+icorr)*100,Beta); // (re)initialize
      pmean = mean(myfbm->data());
      rms = sqrt(mean( ((myfbm->data())-pmean)*((myfbm->data())-pmean) ));
      Vector<Float> angle = (myfbm->data()) * scale/rms * 3.141592; // *2 ?
      for (Int i=0;i<nSim();++i)  {
	(*(drift_p[iant]))(icorr,i) *= exp(Complex(0,angle[i]));
      }

      currAnt()=iant;
    }
  }

  if (slot_times_.nelements()<=0) {
    slot_times_.resize(nSim());
    Double dtime( (stopTime()-startTime()) / Double(nSim()-1) );
    for (Int i=0;i<nSim();i++) 
      slot_time(i) = startTime() + (Double(i)+0.5) * dtime;
  }
  curr_slot()=0;      
  curr_time()=slot_time();  

  initialized()=True;
  if (prtlev()>2) cout << "GCorruptor::init" << endl;

}


Complex GJonesCorruptor::gain(const Int icorr,const Int ichan) {
  if (curr_slot()>=0 and curr_slot()<nSim() and icorr>=0 and icorr<nCorr()) {    
    if (currAnt()>drift_p.nelements())
      throw(AipsError("GJonesCorruptor internal error accessing drift()"));  
    Complex delta = (*drift_p[currAnt()])(icorr,curr_slot());    
    return delta;
  } else {
    cout << "GCorruptor::gain: slot " << curr_slot() << "out of range!" <<endl;
    return Complex(1.);
  }
}









// **********************************************************
//  BJones Implementations
//

BJones::BJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  GJones(vs),             // immediate parent
  maxchangap_p(0)
{
  if (prtlev()>2) cout << "B::B(vs)" << endl;
}

BJones::BJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  GJones(nAnt),
  maxchangap_p(0)
{
  if (prtlev()>2) cout << "B::B(nAnt)" << endl;
}

BJones::~BJones() {
  if (prtlev()>2) cout << "B::~B()" << endl;
}

void BJones::setSolve(const Record& solve) {

  // call parent to get general stuff
  GJones::setSolve(solve);

  // get max chan gap from user
  maxchangap_p=0;
  if (solve.isDefined("maxgap"))
    maxchangap_p=solve.asInt("maxgap");

}

void BJones::normalize() {

  logSink() << "Normalizing solutions per spw, pol, ant, time." 
	    << LogIO::POST;

  // Iteration axes (norm per spw, pol, ant, timestamp)
  IPosition itax(3,0,2,3);
  
  for (Int ispw=0;ispw<nSpw();++ispw)
    if (cs().nTime(ispw)>0) {
      ArrayIterator<Complex> soliter(cs().par(ispw),itax,False);
      ArrayIterator<Bool> okiter(cs().parOK(ispw),itax,False);
      while (!soliter.pastEnd()) {
	normSolnArray(soliter.array(),okiter.array(),True);
	soliter.next();
	okiter.next();
      }
      
    }

}

void BJones::globalPostSolveTinker() {

  // Call parent to do more general things
  SolvableVisJones::globalPostSolveTinker();

  // Fill gaps in channels, if necessary
  if (maxchangap_p>0)
    fillChanGaps();

}

void BJones::fillChanGaps() {

  logSink() << "Filling in flagged solution channels by interpolation." 
	    << LogIO::POST;

  // Iteration axes (norm per spw, pol, ant, timestamp)
  IPosition itax(3,0,2,3);

  for (Int ispw=0;ispw<nSpw();++ispw)
    // Only if there are any solutions, and there are more than 2 channels
    if (cs().nTime(ispw)>0 && cs().nChan(ispw)>2) {
      // Iterate over time, pol, and ant
      ArrayIterator<Complex> soliter(cs().par(ispw),itax,False);
      ArrayIterator<Bool> okiter(cs().parOK(ispw),itax,False);
      while (!soliter.pastEnd()) {
	fillChanGapArray(soliter.array(),okiter.array());
	soliter.next();
	okiter.next();
      }
    }
}

void BJones::fillChanGapArray(Array<Complex>& sol,
			      Array<Bool>& solOK) {

  // Make the arrays 1D
  Vector<Complex> solv(sol.reform(IPosition(1,sol.nelements())));
  Vector<Bool> solOKv(solOK.reform(IPosition(1,solOK.nelements())));

  Int nChan(solv.nelements());
  Bool done(False);
  Int ich(0), ch1(-1), ch2(-1);
  Int dch(1);
  Float a0, da, a, p0, dp, p, fch;

  // Advance to first unflagged channel
  while(!solOKv(ich) && ich<nChan) ++ich;

  // Found no unflagged channels, so signal escape
  if (ich==nChan) done=True;

  // done turns True if we reach nChan, and nothing more to do
  while (!done) {

    // Advance to next flagged channel
    while(solOKv(ich) && ich<nChan) ++ich;

    if (ich<nChan) {

      // Left boundary of gap
      ch1=ich-1;     // (NB: above logic prevents ch1 < 0)

      // Find right boundary:
      ch2=ich+1;
      while (!solOKv(ch2) && ch2<nChan) ++ch2;

      if (ch2<nChan) {
	
	// The span of the interpolation (in channels)
	dch=ch2-ch1;

	//cout << ch1 << " " << ch2 << " " << dch << endl;

	// Interpolate only if gap is narrower than maxchangap_p
	if (dch<=maxchangap_p+1) {

	  // calculate interp params
	  a0=abs(solv(ch1));
	  da=abs(solv(ch2))-a0;
	  p0=arg(solv(ch1));
	  dp=arg(solv(ch2))-p0;
	  if (dp>C::pi) dp-=(2*C::pi);
	  if (dp<-C::pi) dp+=(2*C::pi);	

	  //cout << a0 << " " << da << " " << p0 << " " << dp << endl;


	  // interpolate the intervening channels
	  while (ich<ch2) {
	    fch=Float(ich-ch1)/Float(dch);
	    a=a0 + da*fch;
	    p=p0 + dp*fch;

	    // cout << " " << ich << " " << a << " " << p << endl;

	    solv(ich)=a*Complex(cos(p),sin(p));
	    solOKv(ich)=True;
	    ++ich;
	  }

	  // Begin looking for new gap on next round
	  ++ich;
	  
	}
	else
	  // we skipped a gap, look beyond it on next round
	  ich=ch2+1;
      }
      else
	// Reached nChan looking for ch2
	done=True;
    }
    else
      // Reach nChan looking for gaps 
      done=True;
  } // done

}

// **********************************************************
//  DJones Implementations
//

DJones::DJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  SolvableVisJones(vs),   // immediate parent
  solvePol_(0)
{
  if (prtlev()>2) cout << "D::D(vs)" << endl;

}

DJones::DJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  SolvableVisJones(nAnt),
  solvePol_(0)
{
  if (prtlev()>2) cout << "D::D(nAnt)" << endl;

}

DJones::~DJones() {
  if (prtlev()>2) cout << "D::~D()" << endl;
}

void DJones::setApply(const Record& apply) {

  SolvableVisJones::setApply(apply);

  // Force calwt to False for now
  calWt()=False;

}


void DJones::setSolve(const Record& solvepar) {

  // Call parent
  SolvableVisJones::setSolve(solvepar);

  // Determine if we are solving for source pol or not
  if (solvepar.isDefined("type")) {
    String type = solvepar.asString("type");
    if (type.contains("QU")) {
      solvePol_=2;
      logSink() << "Will solve for source polarization (Q,U)" << LogIO::POST;
    }
    else if (type.contains("X")) {
      solvePol_=1;
      logSink() << "Will solve for source polarization position angle correction" << LogIO::POST;
    }
    else
      solvePol_=0;
  }

  logSink() << "Using only cross-hand data for instrumental polarization solution." << LogIO::POST;
  
  // For D insist preavg is meaningful (5 minutes or user-supplied)
  if (preavg()<0.0)
    preavg()=300.0;

}


void DJones::calcOneJones(Vector<Complex>& mat, Vector<Bool>& mOk,
			  const Vector<Complex>& par, const Vector<Bool>& pOk) {

  if (prtlev()>10) cout << "   D::calcOneJones(vb)" << endl;

  // Only if both pars are good, form matrix
  if (allEQ(pOk,True)) {

    // On-diag = 1
    mat(0)=mat(3)=Complex(1.0);
    // Off-diag = par
    mat(1)=par(0);
    mat(2)=par(1);

    mOk=True;

  }
  else {
    mat=Complex(0.0);
    mOk=False;
  }

}

void DJones::guessPar(VisBuffer& vb) {

  if (prtlev()>4) cout << "   D::guessPar(vb)" << endl;

  // TBD: Should we use a common wt for the X-hands??

  // First guess is zero D-terms
  solveCPar()=0.0;
  solveParOK()=True;

  if (jonesType()==Jones::GenLinear) {
    vb.weightMat().row(0)=0.0;
    vb.weightMat().row(3)=0.0;
  }

  if (solvePol()) {

    // solvePol() tells us how many source pol parameters
    srcPolPar().resize(solvePol());

    // The following assumes the MODEL_DATA has been
    //  corrupted by P 

    LSQFit fit(2,LSQComplex());
    Vector<Complex> ce(2);
    ce(0)=Complex(1.0);
    Complex d,md;
    Float wt,a(0.0);
    for (Int irow=0;irow<vb.nRow();++irow) {
      if (!vb.flagRow()(irow)  &&
	  vb.antenna1()(irow)!=vb.antenna2()(irow)) {
	for (Int ich=0;ich<vb.nChannel();++ich) {
	  if (!vb.flag()(ich,irow)) {
	    for (Int icorr=1;icorr<3;++icorr) {
	      md=vb.modelVisCube()(icorr,ich,irow);
	      if (icorr==2) md=conj(md);
	      a=abs(md);
	      if (a>0.0) {
		wt=Double(vb.weightMat()(icorr,irow));
		if (wt>0.0) {
		  d=vb.visCube()(icorr,ich,irow);
		  if (icorr==2) d=conj(d);
		  if (abs(d)>0.0) {

		    ce(1)=md;
		    fit.makeNorm(ce.data(),wt,d,LSQFit::COMPLEX);

		  } // abs(d)>0
		} // wt>0
	      } // a>0
	    } // icorr
	  } // !flag
	} // ich
      } // !flagRow
    } // row

    uInt rank;
    Bool ok = fit.invert(rank);

    Complex sol[2];
    if (ok)
      fit.solve(sol);
    else
      throw(AipsError("Source polarization solution is singular; try solving for D-terms only."));

    if (solvePol()==1 && a>0.0) 
      srcPolPar()(0)=Complex(arg(sol[1]));
    else if (solvePol()==2) {
      srcPolPar()(0)=Complex(real(sol[1]));
      srcPolPar()(1)=Complex(imag(sol[1]));
    }

    // Log source polarization solution
    reportSolvedQU();


  } // solvePol()?

}

void DJones::updatePar(const Vector<Complex> dCalPar,
		       const Vector<Complex> dSrcPar) {

  // Enforce no change in source parameters 
  //  before calling generic version
  Vector<Complex> dsrcpar(dSrcPar.shape());
  dsrcpar=Complex(0.0);
  SolvableVisJones::updatePar(dCalPar,dsrcpar);

}

void DJones::formSolveSNR() {

  solveParSNR()=0.0;

  for (Int iant=0;iant<nAnt();++iant)
    for (Int ipar=0;ipar<nPar();++ipar) {
      if (solveParOK()(ipar,0,iant) &&
	  solveParErr()(ipar,0,iant)>0.0f) {
	solveParSNR()(ipar,0,iant)=1.0f/solveParErr()(ipar,0,iant);
      }
      else
	// Ensure F if Err<=0  (OK?)
	solveParOK()(ipar,0,iant)=False;
    }
}

void DJones::globalPostSolveTinker() {

  // call parent
  SolvableVisJones::globalPostSolveTinker();

  // if not freqdep, report results to the logger
  logResults();

}

						       
void DJones::applyRefAnt() {

  if (refant()<0)
    throw(AipsError("No refant specified."));

  // Get the refant name from the nMS
  String refantName("none");
  MeasurementSet ms(msName());
  MSAntennaColumns msantcol(ms.antenna());
  refantName=msantcol.name()(refant());
  
  logSink() << "Applying refant: " << refantName
            << LogIO::POST;

  Vector<Int> altrefantlist(nAnt());
  indgen(altrefantlist);

  for (Int ispw=0;ispw<nSpw();++ispw) {

    currSpw()=ispw;

    if (cs().nTime(ispw)>0) {

      // References to ease access to solutions
      Array<Complex> sol(cs().par(ispw));
      Array<Bool> sok(cs().parOK(ispw));

      for (Int islot=0;islot<cs().nTime(ispw);++islot) {

	for (Int ich=0;ich<cs().nChan(ispw);++ich) {

	  IPosition ipr(4,0,ich,0,islot);

	  Complex refD1(0.0),refD2(0.0);

	  // Assume user's refant
	  ipr(2)=refant();

	  // If user's refant unavailable, so find another
	  if (!cs().parOK(ispw)(ipr)) {
	    ipr(2)=0;
	    while (ipr(2)<cs().nElem() &&
		   !cs().parOK(ispw)(ipr)) ++ipr(2);
	  }

	  // Found a refant, so use it
	  if (ipr(2)<nAnt()) {

	    refD1=cs().par(ispw)(ipr);
	    refD2=conj(refD1);

	    for (Int iant=0;iant<cs().nElem();++iant) {
	      ipr(2)=iant;
	      ipr(0)=0;
	      cs().par(ispw)(ipr)-=refD1;
	      ipr(0)=1;
	      cs().par(ispw)(ipr)+=refD2;
	    }
	  } // found refant
	} // ich
      } // islot
    } // nTime(ispw)>0
  } // ispw

}

void DJones::logResults() {

  // Don't bother, if the Ds are channelized
  if (freqDepPar()) return;

  Vector<String> rl(2);
  rl(0)="R: ";
  rl(1)="L: ";

  const ROMSAntennaColumns ac(MeasurementSet(msName()).antenna());
  Vector<String> antNames(ac.name().getColumn());

  Vector<uInt> ord;
  genSort(ord,antNames);

  logSink() << "The instrumental polarization solutions are: " << LogIO::POST;

  logSink().output().precision(4);

  for (Int ispw=0;ispw<nSpw();++ispw) {
    
    currSpw()=ispw;
    
    if (cs().nTime(ispw)>0) {

      if (cs().nTime(ispw)>1)
	logSink() << " Spw " << ispw << " has a time-dep D solution." << endl;
      else {
    
	logSink() << " Spw " << ispw << ":" << endl;
	
	// References to ease access to solutions
	Array<Complex> sol(cs().par(ispw));
	Array<Bool> sok(cs().parOK(ispw));
	
	IPosition ip(4,0,0,0,0);

	logSink().output().setf(ios::left, ios::adjustfield);

	for (Int iant=0;iant<sol.shape()(2);++iant) {
	  ip(2)=ord(iant);
	  logSink() << "  Ant=" << antNames(ord(iant)) << ": ";
	  for (Int ipar=0;ipar<2;++ipar) {
	    logSink() << rl(ipar);
	    ip(0)=ipar;
	    if (sok(ip)) {
	      logSink() << "A="; 
	      logSink().output().width(10);
	      logSink() << abs(sol(ip));
	      logSink() << " P=";
	      logSink().output().width(8);
	      logSink() << arg(sol(ip))*180.0/C::pi;
	      if (ipar==0) logSink() << " ; ";
	    }
	    else {
	      logSink().output().width(26);
	      logSink() << "(flagged)" << " ";
	    }
	  } // ipol
	  logSink() << endl;
	} // iant
	logSink() << LogIO::POST;
      } // nTime==1
    } // nTime>0
  } // ispw

}



// Fill the trivial DJ matrix elements
void DJones::initTrivDJ() {

  if (prtlev()>4) cout << "   D::initTrivDJ" << endl;

  // Must be trivial
  AlwaysAssert((trivialDJ()),AipsError);

  //  0 1     0 0
  //  0 0  &  1 0

  if (jonesType()==Jones::General) {
    diffJElem().resize(IPosition(4,4,2,1,1));
    diffJElem()=0.0;
    diffJElem()(IPosition(4,1,0,0,0))=Complex(1.0);
    diffJElem()(IPosition(4,2,1,0,0))=Complex(1.0);
  }
  else {
    diffJElem().resize(IPosition(4,2,2,1,1));
    diffJElem()=0.0;
    diffJElem()(IPosition(4,0,0,0,0))=Complex(1.0);
    diffJElem()(IPosition(4,1,1,0,0))=Complex(1.0);
  }

}



Int DJones::setupSim(VisSet& vs, const Record& simpar, Vector<Int>& nChunkPerSol, Vector<Double>& solTimes)
{

  LogIO os(LogOrigin("D", "setupSim()", WHERE));

  if (prtlev()>2) cout << "   D::setupSim()" << endl;
  AlwaysAssert((isSimulated()),AipsError);

  // this is trivial since we don't need slots, just a constant D with time.

  Int nSim = 1;
  // still need a corruptor to have a curr_time in it
  corruptor_p = new CalCorruptor(nSim);
  corruptor_p->prtlev()=prtlev();
    Vector<Double> timevec;
  corruptor_p->curr_time()=vs.iter().time(timevec)[0];
  corruptor_p->amp()=simpar.asFloat("amplitude");

  return nSim;
}






Bool DJones::simPar(VisIter& vi, const Int nChunks){
  LogIO os(LogOrigin("D", "simPar()", WHERE));
  if (prtlev()>4) cout << "   D::simPar()" << endl;
  
  AlwaysAssert((isSimulated()),AipsError);
#ifndef RI_DEBUG
  try {
#endif
    Vector<Int> a1;
    Vector<Int> a2;
    Matrix<Bool> flags;
    Float& amp(corruptor_p->amp());

    solveCPar()=Complex(0.0);
    // n good VI elements averaged in each CPar() entry:
    solveParOK()=False;
    
    Vector<Double> timevec;
    Double starttime,stoptime;
    starttime=vi.time(timevec)[0];
    
    for (Int ichunk=0;ichunk<nChunks;++ichunk) {
      Int spw(vi.spectralWindow());
      corruptor_p->currSpw()=spw; 

      for (vi.origin(); vi.more(); vi++) {
        // these need to be updated whenever the VI scrolls forward
        vi.antenna1(a1);
        vi.antenna2(a2);
        vi.flag(flags);
	vi.time(timevec);

	for (Int irow=0;irow<vi.nRow();++irow) {
      
	  if (nfalse(flags.column(irow))> 0 ) { 	    
	    corruptor_p->currAnt()=a1(irow);
	    for (Int icorr=0;icorr<corruptor_p->nCorr();icorr++) {
	      if ( a1(irow)==a2(irow) ) {
		solveCPar().xyPlane(a1(irow)).row(icorr) = Complex(1.0); 
	      } else {	       	      
		solveCPar().xyPlane(a1(irow)).row(icorr) = amp; 
	      }		
	      solveParOK().xyPlane(a1(irow)) = True;
	    }
	  } // if not flagged 
	} // irow loop
      } // vi loop
      if (vi.moreChunks()) vi.nextChunk();
    } // chunk loop

    
    vi.time(timevec);
    Int tvsize;
    timevec.shape(tvsize);
    stoptime=timevec[tvsize-1];
    refTime() = 0.5*(starttime+stoptime);
    interval() = (stoptime-starttime);
    currField() = vi.fieldId();

#ifndef RI_DEBUG
  } catch (AipsError x) {
    if (gcorruptor_p) delete gcorruptor_p;
    os << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  } 
#endif
  return True;
}







// **********************************************************
//  DfJones Implementations
//

DfJones::DfJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  DJones(vs)              // immediate parent
{
  if (prtlev()>2) cout << "Df::Df(vs)" << endl;
}

DfJones::DfJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  DJones(nAnt)
{
  if (prtlev()>2) cout << "Df::Df(nAnt)" << endl;
}

DfJones::~DfJones() {
  if (prtlev()>2) cout << "Df::~Df()" << endl;
}








// **********************************************************
//  JJones Implementations
//

JJones::JJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  SolvableVisJones(vs)    // immediate parent
{
  if (prtlev()>2) cout << "J::J(vs)" << endl;
}

JJones::JJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  SolvableVisJones(nAnt)
{
  if (prtlev()>2) cout << "J::J(nAnt)" << endl;
}

JJones::~JJones() {
  if (prtlev()>2) cout << "J::~J()" << endl;
}

void JJones::setSolve(const Record& solvepar) {

  // Call parent
  SolvableVisJones::setSolve(solvepar);

  // For J insist preavg is meaningful (5 minutes or user-supplied)
  if (preavg()<0.0)
    preavg()=300.0;

}


void JJones::guessPar(VisBuffer& vb) {

  if (prtlev()>4) cout << "   ::guessPar(vb)" << endl;

  // Make a guess at antenna-based J
  //  Correlation membership-dependence
  //  assumes corrs in canonical order
  //  nCorr = 1: use icorr=0
  //  nCorr = 2: use icorr=[0,1]
  //  nCorr = 4: use icorr=[0,3]

  // This method sets the off-diag = 0.0,
  //  and the on-diag as if this were G

  Int nCorr(2);
  Int nDataCorr(vb.visCube().shape()(0));
  Vector<Int> corridx(nCorr,0);
  if (nDataCorr==2) {
    corridx(0)=0;
    corridx(1)=1;
  } 
  else if (nDataCorr==4) {
    corridx(0)=0;
    corridx(1)=3;
  }

  Cube<Complex>& V(vb.visCube());
  Float amp(0.0),ampave(0.0);
  Int namp(0);
  solveCPar()=Complex(0.0);
  for (Int irow=1;irow<nAnt();++irow) {

    for (Int icorr=0;icorr<nCorr;icorr++) {
      Complex& Vi(V(corridx(icorr),0,irow));
      amp=abs(Vi);
      if (amp>0.0f) {
	solveCPar()(3*icorr,0,irow)=(conj(Vi)/amp);
	ampave+=amp;
	namp++;
	//	cout << "          " << abs(Vi) << " " << arg(Vi)*180.0/C::pi << endl;
      }
    }

  }

  // Scale them by the mean amplitude
  ampave/=Float(namp);
  ampave=sqrt(ampave);
  solveCPar()*=Complex(ampave);
  solveParOK()=True;

  //  cout << "post-guess:" << endl;
  //  cout << "solveCPar()   = " << solveCPar() << endl;
  //  cout << "phases       = " << phase(solveCPar())*180.0/C::pi << endl;
  //  cout << "solveParOK() = " << solveParOK() << endl;

}

// Fill the trivial DJ matrix elements
void JJones::initTrivDJ() {

  if (prtlev()>4) cout << "   J::initTrivDJ" << endl;

  // Must be trivial
  AlwaysAssert((trivialDJ()),AipsError);

  //  1 0     0 1     0 0     0 0
  //  0 0     0 0     1 0     0 1

  diffJElem().resize(IPosition(4,4,4,1,1));
  diffJElem()=0.0;
  diffJElem()(IPosition(4,0,0,0,0))=Complex(1.0);
  diffJElem()(IPosition(4,1,1,0,0))=Complex(1.0);
  diffJElem()(IPosition(4,2,2,0,0))=Complex(1.0);
  diffJElem()(IPosition(4,3,3,0,0))=Complex(1.0);

}

// **********************************************************
//  MMueller: baseline-based (closure) solution
//

MMueller::MMueller(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  SolvableVisMueller(vs)    // immediate parent
{
  if (prtlev()>2) cout << "M::M(vs)" << endl;
}

MMueller::MMueller(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  SolvableVisMueller(nAnt)
{
  if (prtlev()>2) cout << "M::M(nAnt)" << endl;
}

MMueller::~MMueller() {
  if (prtlev()>2) cout << "M::~M()" << endl;
}

void MMueller::setApply(const Record& apply) {

  SolvableVisCal::setApply(apply);

  // Force calwt to False for now
  calWt()=False;

}

void MMueller::newselfSolve(VisSet& vs, VisEquation& ve) {

  if (prtlev()>4) cout << "   M::selfSolve(ve)" << endl;

  // Inform logger/history
  logSink() << "Solving for " << typeName()
            << LogIO::POST;

  // Initialize the svc according to current VisSet
  //  (this counts intervals, sizes CalSet)
  Vector<Int> nChunkPerSol;
  Int nSol = sizeUpSolve(vs,nChunkPerSol);
  
  // The iterator, VisBuffer
  VisIter& vi(vs.iter());
  VisBuffer vb(vi);

  //  cout << "nSol = " << nSol << endl;
  //  cout << "nChunkPerSol = " << nChunkPerSol << endl;

  Vector<Int> slotidx(vs.numberSpw(),-1);


  ve.state();

  Int nGood(0);
  vi.originChunks();
  for (Int isol=0;isol<nSol && vi.moreChunks();++isol) {

    // Arrange to accumulate
    VisBuffAccumulator vba(nAnt(),preavg(),False);
    
    for (Int ichunk=0;ichunk<nChunkPerSol(isol);++ichunk) {

      // Current _chunk_'s spw
      Int spw(vi.spectralWindow());

      // Abort if we encounter a spw for which a priori cal not available
      if (!ve.spwOK(spw))
        throw(AipsError("Pre-applied calibration not available for at least 1 spw. Check spw selection carefully."));


      // Collapse each timestamp in this chunk according to VisEq
      //  with calibration and averaging
      for (vi.origin(); vi.more(); vi++) {

        // Force read of the field Id
        vb.fieldId();

	// Apply the channel mask
	this->applyChanMask(vb);

        // This forces the data/model/wt I/O, and applies
        //   any prior calibrations
        ve.collapse(vb);

        // If permitted/required by solvable component, normalize
        if (normalizable())
          vb.normalize();

	// If this solve not freqdep, and channels not averaged yet, do so
	if (!freqDepMat() && vb.nChannel()>1) 
	  vb.freqAveCubes();

        // Accumulate collapsed vb in a time average
        vba.accumulate(vb);
      }
      // Advance the VisIter, if possible
      if (vi.moreChunks()) vi.nextChunk();

    }

    // Finalize the averged VisBuffer
    vba.finalizeAverage();

    // The VisBuffer to solve with
    VisBuffer& svb(vba.aveVisBuff());

    // Make data amp- or phase-only
    enforceAPonData(svb);

    // Establish meta-data for this interval
    //  (some of this may be used _during_ solve)
    //  (this sets currSpw() in the SVC)
    Bool vbOk=syncSolveMeta(svb,-1);

    Int thisSpw=spwMap()(svb.spectralWindow());
    slotidx(thisSpw)++;

    // Fill solveCPar() with 1, nominally, and flagged
    solveCPar()=Complex(1.0);
    solveParOK()=False;
    
    if (vbOk && svb.nRow()>0) {

      // Insist that channel,row shapes match
      IPosition visshape(svb.visCube().shape());
      AlwaysAssert(solveCPar().shape().getLast(2)==visshape.getLast(2),AipsError);
      
      // Zero flagged data
      IPosition vblc(3,0,0,0);
      IPosition vtrc(visshape);  vtrc-=1;      
      Int nCorr(visshape(0));
      for (Int i=0;i<nCorr;++i) {
	vblc(0)=vtrc(0)=i;
	svb.visCube()(vblc,vtrc).reform(visshape.getLast(2))(svb.flag())=Complex(1.0);
      }
      
      // Form correct slice of visCube to copy to solveCPar
      IPosition vcblc(3,0,0,0);
      IPosition vctrc(svb.visCube().shape()); vctrc-=1;
      IPosition vcstr(3,1,1,1);

      IPosition spblc(3,0,0,0);
      IPosition sptrc(solveCPar().shape()); sptrc-=1;

      IPosition flshape(svb.flag().shape());
      
      switch (nCorr) {
      case 1: {
	// fill 1st par only
	spblc(0)=sptrc(0)=0; 
	solveCPar()(spblc,sptrc)=svb.visCube();
	// first pol flags
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();
	break;
      }
      case 2: {
	// shapes match
	solveCPar()=svb.visCube();
	spblc(0)=sptrc(0)=0; 
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();
	spblc(0)=sptrc(0)=1; 
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();

	break;
      }
      case 4: {
	// Slice visCube with stride
	vcstr(0)=3;
	solveCPar()=svb.visCube()(vcblc,vctrc,vcstr);
	spblc(0)=sptrc(0)=0; 
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();
	spblc(0)=sptrc(0)=1; 
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();

	break;
      }
      default:
	throw(AipsError("Problem in MMueller::selfSolve."));
	break;
      }

      nGood++;
    }

    keep(slotidx(thisSpw));
    
  }
  
  logSink() << "  Found good "
            << typeName() << " solutions in "
            << nGood << " intervals."
            << LogIO::POST;

  // Store whole of result in a caltable
  if (nGood==0)
    logSink() << "No output calibration table written."
              << LogIO::POST;
  else {

    // Do global post-solve tinkering (e.g., normalization)
    globalPostSolveTinker();

    // write the table
    store();
  }

}

void MMueller::oldselfSolve(VisSet& vs, VisEquation& ve) {

  if (prtlev()>4) cout << "   M::selfSolve(ve)" << endl;

  // Arrange for iteration over data
  Block<Int> columns;
  if (interval()==0.0) {
    // include scan iteration
    // avoid scan iteration
    columns.resize(5);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::SCAN_NUMBER;
    columns[2]=MS::FIELD_ID;
    columns[3]=MS::DATA_DESC_ID;
    columns[4]=MS::TIME;
  } else {
    // avoid scan iteration
    columns.resize(4);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::FIELD_ID;
    columns[2]=MS::DATA_DESC_ID;
    columns[3]=MS::TIME;
  }
  vs.resetVisIter(columns,interval());

  // Initial the solve (sets shapes)
  initSolve(vs);

  // Solve each solution interval (chunk)
  Vector<Int> islot(nSpw(),0);
  VisIter& vi(vs.iter());
  VisBuffer vb(vi);
  for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {

    Int spw(vi.spectralWindow());

    // Abort if we encounter a spw for which a priori cal not available
    if (!ve.spwOK(spw))
      throw(AipsError("Pre-applied calibration not available for at least 1 spw. Check spw selection carefully."));

    // Arrange to accumulate
    VisBuffAccumulator vba(nAnt(),preavg(),False);

    // Collapse each timestamp in this chunk according to VisEq
    //  with calibration and averaging

    for (vi.origin(); vi.more(); vi++) {

      ve.collapse(vb);

      vb.normalize();

      // If this solve not freqdep, and channels not averaged yet, do so
      if (!freqDepMat() && vb.nChannel()>1)
	vb.freqAveCubes();

      // Accumulate collapsed vb in a time average
      vba.accumulate(vb);
     
    }
    vba.finalizeAverage();

    // The VisBuffer to solve with
    VisBuffer& svb(vba.aveVisBuff());

    // Extract meta data from visBuffer
    syncSolveMeta(svb,vi.fieldId());

    // Fill solveCPar() with 1, nominally, and flagged
    solveCPar()=Complex(1.0);
    solveParOK()=False;

    if (svb.nRow()>0) {

      // Insist that channel,row shapes match
      IPosition visshape(svb.visCube().shape());
      AlwaysAssert(solveCPar().shape().getLast(2)==visshape.getLast(2),AipsError);
      
      // Zero flagged data
      IPosition vblc(3,0,0,0);
      IPosition vtrc(visshape);  vtrc-=1;      
      Int nCorr(visshape(0));
      for (Int i=0;i<nCorr;++i) {
	vblc(0)=vtrc(0)=i;
	svb.visCube()(vblc,vtrc).reform(visshape.getLast(2))(svb.flag())=Complex(1.0);
      }
      
      // Form correct slice of visCube to copy to solveCPar
      IPosition vcblc(3,0,0,0);
      IPosition vctrc(svb.visCube().shape()); vctrc-=1;
      IPosition vcstr(3,1,1,1);

      IPosition spblc(3,0,0,0);
      IPosition sptrc(solveCPar().shape()); sptrc-=1;

      IPosition flshape(svb.flag().shape());
      
      switch (nCorr) {
      case 1: {
	// fill 1st par only
	spblc(0)=sptrc(0)=0; 
	solveCPar()(spblc,sptrc)=svb.visCube();
	// first pol flags
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();
	break;
      }
      case 2: {
	// shapes match
	solveCPar()=svb.visCube();
	spblc(0)=sptrc(0)=0; 
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();
	spblc(0)=sptrc(0)=1; 
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();

	break;
      }
      case 4: {
	// Slice visCube with stride
	vcstr(0)=3;
	solveCPar()=svb.visCube()(vcblc,vctrc,vcstr);
	spblc(0)=sptrc(0)=0; 
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();
	spblc(0)=sptrc(0)=1; 
	solveParOK()(spblc,sptrc).reform(flshape)=!svb.flag();

	break;
      }
      default:
	throw(AipsError("Problem in MMueller::selfSolve."));
	break;
      }
      
    }

    keep(islot(spw));

    islot(spw)++;

  }
  
  // Store it.
  store();

}

void MMueller::globalPostSolveTinker() {

  // normalize, if requested
  if (solnorm()) normalize();

}


// File a solved solution (and meta-data) into a slot in the CalSet
void MMueller::keep(const Int& slot) {

  if (prtlev()>4) cout << " M::keep(i)" << endl;

  if (slot<cs().nTime(currSpw())) {
    // An available valid slot

   
    //    cout << "Result: solveCPar() = " << solveCPar() << endl;

    //    cout << "   Amp: " << amplitude(solveCPar()) << endl;
    //    cout << " Phase: " << phase(solveCPar()/solveCPar()(0,0,0))*180.0/C::pi << endl;

    //    cout << "Result: solveParOK() = " << solveParOK() << endl;

    cs().fieldId(currSpw())(slot)=currField();
    cs().time(currSpw())(slot)=refTime();

    // Only stop-start diff matters
    //  TBD: change CalSet to use only the interval
    //  TBD: change VisBuffAcc to calculate exposure properly
    cs().startTime(currSpw())(slot)=0.0;
    cs().stopTime(currSpw())(slot)=interval();

    // For now, just make these non-zero:
    cs().iFit(currSpw()).column(slot)=1.0;
    cs().iFitwt(currSpw()).column(slot)=1.0;
    cs().fit(currSpw())(slot)=1.0;
    cs().fitwt(currSpw())(slot)=1.0;

    IPosition blc4(4,0,       0,           0,        slot);
    IPosition trc4(4,nPar()-1,nChanPar()-1,nElem()-1,slot);
    cs().par(currSpw())(blc4,trc4).nonDegenerate(3) = solveCPar();
    cs().parOK(currSpw())(blc4,trc4).nonDegenerate(3)= solveParOK();

    cs().solutionOK(currSpw())(slot) = anyEQ(solveParOK(),True);

  }
  else
    throw(AipsError("MMueller::keep: Attempt to store solution in non-existent CalSet slot"));

}




Int MMueller::setupSim(VisSet& vs, const Record& simpar, Vector<Int>& nChunkPerSol, Vector<Double>& solTimes)
{
  // prtlev()=4; // debug
  if (prtlev()>2) cout << "   MM::setupSim()" << endl;
  AlwaysAssert((isSimulated()),AipsError);

  Int nSim = sizeUpSim(vs,nChunkPerSol,solTimes);
  if (prtlev()>3) cout << " sized for Sim." << endl;

  atmcorruptor_p = new AtmosCorruptor();
  corruptor_p = atmcorruptor_p;
  if (prtlev()>3) cout << " AtmosCorruptor created." << endl;

  atmcorruptor_p->freqDep()=freqDepPar();

  // RI TODO mm:setupSim move spw info to generic setupSim?
  VisIter& vi(vs.iter());
  const ROMSSpWindowColumns& spwcols = vi.msColumns().spectralWindow();

  if (prtlev()>3) cout << " SpwCols accessed:" << endl;
  if (prtlev()>3) cout << "   nSpw()= " << nSpw() << endl;
  if (prtlev()>3) cout << "   spwcols.nrow()= " << nSpw() << endl;

  AlwaysAssert(nSpw()==spwcols.nrow(),AipsError);

  // things will break if spw mapping, ie not in same order as in vs
  corruptor_p->nSpw()=nSpw();
  corruptor_p->nAnt()=nAnt();
  corruptor_p->currAnt()=0;
  corruptor_p->currSpw()=0;
  corruptor_p->fRefFreq().resize(nSpw());
  corruptor_p->fnChan().resize(nSpw());
  corruptor_p->fWidth().resize(nSpw());

  for (Int irow=0;irow<nSpw();++irow) { 
    corruptor_p->fRefFreq()[irow]=spwcols.refFrequency()(irow);
    corruptor_p->fnChan()[irow]=spwcols.numChan()(irow);
    corruptor_p->fWidth()[irow]=spwcols.totalBandwidth()(irow); 
    // totalBandwidthQuant ?  in other places its assumed to be in Hz
  }
  // see MSsummary.cc for more info/examples


  atmcorruptor_p->initialize(simpar);
  
  return nSim;
}




Bool MMueller::simPar(VisIter& vi,const Int nChunks) {
  if (prtlev()>4) cout << "  MM::simPar()" << endl;

  AlwaysAssert((isSimulated()),AipsError);
  try {
    
    Vector<Int> a1;
    Vector<Int> a2;
    Matrix<Bool> flags;  // matrix foreach row and chan - if want pol send cube.
    Vector<Double> antDiams = vi.msColumns().antenna().dishDiameter().getColumn();
    Vector<Double> timevec;
    Double starttime,stoptime;
    Int ibln;
    
    // initSolvePar: solveCPar().resize(nPar(),nChanPar(),nBln());
    solveCPar()=Complex(0.0);
    IPosition cparshape=solveCPar().shape();

    // n good VI elements to average in each CPar entry
    Cube<Complex> nGood(cparshape);
    solveParOK()=False;
    
    // RI TODO MM::simPar CPar and VI shape checks?
    
    starttime=vi.time(timevec)[0];

    Complex factor(1.0);
    Float tau0(0.),airmass1(1.0),airmass2(1.0),A(1.0),tsys(0.),tau(0.),factor0(1.0);
    factor0 = atmcorruptor_p->amp();
    
    if (atmcorruptor_p->mode() == "tsys") {
      // live dangerously: assume all vis have the same tint
      Double tint = vi.msColumns().exposure()(0);  
      Int iSpW = vi.spectralWindow();
      Float deltaNu = 
	vi.msColumns().spectralWindow().totalBandwidth()(iSpW) / 
	Float(vi.msColumns().spectralWindow().numChan()(iSpW));	    
      factor0 /= sqrt( deltaNu * tint ) ;  
    }
    Float el1(1.5708);    
    Float el2(1.5708);
      
    for (Int ichunk=0;ichunk<nChunks;++ichunk) {
      Int spw(vi.spectralWindow());	
      corruptor_p->currSpw()=spw;
      currSpw()=spw;
      
      // assume one time slot per chunk
      Vector<MDirection> antazel(vi.azel(refTime()));
            
      for (vi.origin(); vi.more(); vi++) {
	
	vi.antenna1(a1);
	vi.antenna2(a2);
	vi.flag(flags);
	
	for (Int irow=0;irow<vi.nRow();++irow)
	  if ( nfalse(flags.column(irow)) > 0 ) {   
	    if ( a1(irow)==a2(irow) ) {
	    if (freqDepPar()) {
	      for (Int icorr=0;icorr<corruptor_p->nCorr();icorr++) {		
		solveCPar()(icorr,focusChan(),ibln) += Complex(1.);
		nGood(icorr,focusChan(),ibln) += + Complex(1.);	    
		solveParOK()(icorr,focusChan(),ibln) = True;
	      }
	    } else {
	      solveCPar().xyPlane(ibln) = solveCPar().xyPlane(ibln) + Complex(1.);
	      nGood.xyPlane(ibln) = nGood.xyPlane(ibln) + Complex(1.);	    
	      solveParOK().xyPlane(ibln) = True;
	    }
	    } else {	       
	    // don't need loop to find the corruptor time slot here,
	    // since just drawing from random distribution.
	    
	    factor=factor0;

	    if (atmcorruptor_p->mode() == "tsys") {	      
	      el1 = antazel(a1(irow)).getAngle("rad").getValue()(1);
	      el2 = antazel(a2(irow)).getAngle("rad").getValue()(1);
	      
	      if (el1 > 0.0 && el2 > 0.0) {
		airmass1 = 1.0/ sin(el1);
		airmass2 = 1.0/ sin(el2);
	      } else {
		airmass1 = 1.0;
		airmass2 = 1.0;
	      }

	      if (freqDepPar()) {
		tau = atmcorruptor_p->opac(focusChan());
	      }	else {
		tau = 1.;
	      }
	      // user could be overriding the tau scale, but keep the
	      // ATM freq dependence - see atmcorruptor::initialize
	      tau *= atmcorruptor_p->tauscale();

	      A = exp(-tau *0.5*(airmass1+airmass2));	      
	      // this is tsys above atmosphere
	      tsys = atmcorruptor_p->tsys0() + A*atmcorruptor_p->tsys1();
	      	      
	      factor *= tsys / antDiams(a1(irow)) / antDiams(a2(irow));
	    }
	    
	    // corr,chan,bln = xyz
	    ibln=blnidx(a1(irow),a2(irow));
	    if (freqDepPar()) {
	      for (Int icorr=0;icorr<corruptor_p->nCorr();icorr++) {		
		solveCPar()(icorr,focusChan(),ibln) += factor;
		nGood(icorr,focusChan(),ibln) += + Complex(1.);	    
		solveParOK()(icorr,focusChan(),ibln) = True;
	      }
	    } else {
	      solveCPar().xyPlane(ibln) = solveCPar().xyPlane(ibln) + factor;   
	      nGood.xyPlane(ibln) = nGood.xyPlane(ibln) + Complex(1.);	    
	      solveParOK().xyPlane(ibln) = True;
	    }
	    }// if not autocorr	    
	  } // if good row
      } // advancing VI
      if (vi.moreChunks()) vi.nextChunk();
    } // nchunks loop
    
    vi.time(timevec);
    Int tvsize;
    timevec.shape(tvsize);
    stoptime=timevec[tvsize-1];
    
    // RI TODO MM::simPar  don't divide by zero **
    for (Int i=0;i<cparshape(2);i++)
      solveCPar().xyPlane(i) =  solveCPar().xyPlane(i) / nGood.xyPlane(i);
    
    // since we're not going to syncMeta to a VBA in Simulator, we need to set
    // various things here by hand so that keep() has them:
    // refTime(), currSpw(), currField(), and interval()
    refTime() = 0.5*(starttime+stoptime);
    interval() = (stoptime-starttime);
    currField() = vi.fieldId();
    
  } catch (AipsError x) {
    if (corruptor_p) delete corruptor_p;
    cout << LogIO::SEVERE << "Caught exception: " << x.getMesg() << LogIO::POST;
    return False;
  }
  return True;
}









// **********************************************************
//  MfMueller: freq-dep MMueller
//

MfMueller::MfMueller(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  MMueller(vs)            // immediate parent
{
  if (prtlev()>2) cout << "Mf::Mf(vs)" << endl;
}

MfMueller::MfMueller(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  MMueller(nAnt)
{
  if (prtlev()>2) cout << "Mf::Mf(nAnt)" << endl;
}

MfMueller::~MfMueller() {
  if (prtlev()>2) cout << "Mf::~Mf()" << endl;
}

void MfMueller::normalize() {

  // This is just like BJones

  logSink() << "Normalizing solutions per spw, pol, baseline, time"
            << LogIO::POST;

  // Iteration axes (norm per spw, pol, ant, timestamp)
  //  (this normalizes each baseline spectrum)
  IPosition itax(3,0,2,3);

  for (Int ispw=0;ispw<nSpw();++ispw)
    if (cs().nTime(ispw)>0) {
      ArrayIterator<Complex> soliter(cs().par(ispw),itax,False);
      ArrayIterator<Bool> okiter(cs().parOK(ispw),itax,False);
      while (!soliter.pastEnd()) {
        normSolnArray(soliter.array(),okiter.array(),True);
        soliter.next();
        okiter.next();
      }

    }

}



// **********************************************************
//  TOpac
//

TOpac::TOpac(VisSet& vs) :
  VisCal(vs), 
  VisMueller(vs),
  TJones(vs),
  za_()
{
  if (prtlev()>2) cout << "TOpac::TOpac(vs)" << endl;
}

TOpac::~TOpac() {
  if (prtlev()>2) cout << "TOpac::~TOpac()" << endl;
}

void TOpac::setApply(const Record& applypar) {
  
  // TBD: Handle opacity table case properly
  // TBD: Handle spwmap properly  (opacity is not spw-dep)

  // TBD: Call parent?
  //  T::setApply(applypar);

  // This version uses user-supplied opacity value for all ants

  //  Double opacity(0.0);
  if (applypar.isDefined("opacity"))
    opacity_=applypar.asFloat("opacity");  
 
  Int oldspw; oldspw=currSpw();
  for (Int ispw=0;ispw<nSpw();++ispw) {
    currSpw()=ispw;
    currRPar().resize(1,1,nAnt());
    currRPar()=opacity_;
    currParOK().resize(1,1,nAnt());
    currParOK()=True;
  }
  currSpw()=oldspw;

  // Resize za()
  za().resize(nAnt());

}

String TOpac::applyinfo() {

  ostringstream o;
  o << typeName()
    << ": opacity=" << opacity_
    << boolalpha
    << " calWt=" << calWt();
    //    << " t="      << interval();

  return String(o);

}


void TOpac::calcPar() {

  if (prtlev()>6) cout << "      TOpac::calcPar()" << endl;

  // NB: z.a. calc here because it is needed only 
  //   if we have a new timestamp...
  //  (opacity parameter is already ok)

  za().resize(nAnt());
  Vector<MDirection> antazel(vb().azel(currTime()));
  Double* a=za().data();
  for (Int iant=0;iant<nAnt();++iant,++a) 
    (*a)=C::pi_2 - antazel(iant).getAngle().getValue()(1);

  // Pars now valid, matrices not yet
  validateP();
  invalidateJ();

}


void TOpac::calcAllJones() {

  if (prtlev()>6) cout << "       TOpac::calcAllJones()" << endl;

  // Nominally no opacity
  currJElem()=Complex(1.0);
  currJElemOK()=currParOK();

  Complex* J=currJElem().data();
  Float*  op=currRPar().data();
  Bool*   opok=currParOK().data();
  Double* a=za().data();
  for (Int iant=0; iant<nAnt(); ++iant,++J,++op,++opok,++a) {
    if ((*opok) && (*a)<C::pi_2) 
      (*J) = Complex(sqrt(exp(-1.0 * Double(*op)/cos(*a))));
  }

}



// **********************************************************
//  TfOpac
//

TfOpac::TfOpac(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  TOpac(vs)              // immediate parent
{
  if (prtlev()>2) cout << "TfOpac::TfOpac(vs)" << endl;
}

//TfOpac::TfOpac(const Int& nAnt) :
//  VisCal(nAnt), 
//  VisMueller(nAnt),
//  TOpac(nAnt)
//{
//  if (prtlev()>2) cout << "TfOpac::TfOpac(nAnt)" << endl;
//}

TfOpac::~TfOpac() {
  if (prtlev()>2) cout << "TfOpac::~TfOpac()" << endl;
}







// **********************************************************
//  XMueller: positiona angle for circulars
//

XMueller::XMueller(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  SolvableVisMueller(vs)    // immediate parent
{
  if (prtlev()>2) cout << "X::X(vs)" << endl;
}

XMueller::XMueller(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  SolvableVisMueller(nAnt)
{
  if (prtlev()>2) cout << "X::X(nAnt)" << endl;
}

XMueller::~XMueller() {
  if (prtlev()>2) cout << "X::~X()" << endl;
}

void XMueller::setApply(const Record& apply) {

  SolvableVisCal::setApply(apply);

  // Force calwt to False 
  calWt()=False;

}


void XMueller::setSolve(const Record& solvepar) {

  SolvableVisCal::setSolve(solvepar);

  // Force calwt to False 
  calWt()=False;

  // For X insist preavg is meaningful (5 minutes or user-supplied)
  if (preavg()<0.0)
    preavg()=300.0;

}

void XMueller::newselfSolve(VisSet& vs, VisEquation& ve) {

  if (prtlev()>4) cout << "   M::selfSolve(ve)" << endl;

  MeasurementSet ms(msName());
  MSFieldColumns msfldcol(ms.field());

  // Inform logger/history
  logSink() << "Solving for " << typeName()
            << LogIO::POST;

  // Initialize the svc according to current VisSet
  //  (this counts intervals, sizes CalSet)
  Vector<Int> nChunkPerSol;
  Int nSol = sizeUpSolve(vs,nChunkPerSol);
  
  // The iterator, VisBuffer
  VisIter& vi(vs.iter());
  VisBuffer vb(vi);

  //  cout << "nSol = " << nSol << endl;
  //  cout << "nChunkPerSol = " << nChunkPerSol << endl;

  Vector<Int> slotidx(vs.numberSpw(),-1);

  Int nGood(0);
  vi.originChunks();
  for (Int isol=0;isol<nSol && vi.moreChunks();++isol) {

    // Arrange to accumulate
    VisBuffAccumulator vba(nAnt(),preavg(),False);
    
    for (Int ichunk=0;ichunk<nChunkPerSol(isol);++ichunk) {

      // Current _chunk_'s spw
      Int spw(vi.spectralWindow());

      // Abort if we encounter a spw for which a priori cal not available
      if (!ve.spwOK(spw))
        throw(AipsError("Pre-applied calibration not available for at least 1 spw. Check spw selection carefully."));


      // Collapse each timestamp in this chunk according to VisEq
      //  with calibration and averaging
      for (vi.origin(); vi.more(); vi++) {

        // Force read of the field Id
        vb.fieldId();

        // This forces the data/model/wt I/O, and applies
        //   any prior calibrations
        ve.collapse(vb);

        // If permitted/required by solvable component, normalize
        //if (normalizable())
	//          vb.normalize();

	// If this solve not freqdep, and channels not averaged yet, do so
	if (!freqDepMat() && vb.nChannel()>1)
	  vb.freqAveCubes();

        // Accumulate collapsed vb in a time average
        vba.accumulate(vb);
      }
      // Advance the VisIter, if possible
      if (vi.moreChunks()) vi.nextChunk();

    }

    // Finalize the averged VisBuffer
    vba.finalizeAverage();

    // The VisBuffer to solve with
    VisBuffer& svb(vba.aveVisBuff());

    // Establish meta-data for this interval
    //  (some of this may be used _during_ solve)
    //  (this sets currSpw() in the SVC)
    Bool vbOk=syncSolveMeta(svb,-1);

    Int thisSpw=spwMap()(svb.spectralWindow());
    slotidx(thisSpw)++;

    // Fill solveCPar() with 1, nominally, and flagged
    // TBD: drop unneeded basline-dependence    
    solveCPar()=Complex(1.0);
    solveParOK()=False;
    
    if (vbOk && svb.nRow()>0) {

      // solve for the R-L phase term in the current VB
      solveOneVB(svb);

      if (solveParOK()(0,0,0))
	logSink() << "Position angle offset solution for " 
		  << msfldcol.name()(currField())
		  << " (spw = " << currSpw() << ") = "
		  << arg(solveCPar()(0,0,0))*180.0/C::pi/2.0
		  << " deg."
		  << LogIO::POST;
      else
	logSink() << "Position angle offset solution for " 
		  << msfldcol.name()(currField())
		  << " (spw = " << currSpw() << ") "
		  << " was not determined (insufficient data)."
		  << LogIO::POST;
	
      nGood++;
    }

    keep(slotidx(thisSpw));
    
  }
  
  logSink() << "  Found good "
            << typeName() << " solutions in "
            << nGood << " intervals."
            << LogIO::POST;

  // Store whole of result in a caltable
  if (nGood==0)
    logSink() << "No output calibration table written."
              << LogIO::POST;
  else {

    // Do global post-solve tinkering (e.g., phase-only, normalization, etc.)
    //  TBD
    // globalPostSolveTinker();

    // write the table
    store();
  }

}

void XMueller::oldselfSolve(VisSet& vs, VisEquation& ve) {

  if (prtlev()>4) cout << "   M::selfSolve(ve)" << endl;

  MeasurementSet ms(msName());
  MSFieldColumns msfldcol(ms.field());

  // Arrange for iteration over data
  Block<Int> columns;
  if (interval()==0.0) {
    // include scan iteration
    // avoid scan iteration
    columns.resize(5);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::SCAN_NUMBER;
    columns[2]=MS::FIELD_ID;
    columns[3]=MS::DATA_DESC_ID;
    columns[4]=MS::TIME;
  } else {
    // avoid scan iteration
    columns.resize(4);
    columns[0]=MS::ARRAY_ID;
    columns[1]=MS::FIELD_ID;
    columns[2]=MS::DATA_DESC_ID;
    columns[3]=MS::TIME;
  }
  vs.resetVisIter(columns,interval());

  // Initial the solve (sets shapes)
  initSolve(vs);

  // Solve each solution interval (chunk)
  Vector<Int> islot(nSpw(),0);
  VisIter& vi(vs.iter());
  VisBuffer vb(vi);
  for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {

    Int spw(vi.spectralWindow());
    //      cout << "Spw=" << spw << " slot=" << islot(spw) << " field="
    //           << vi.fieldId() << " " << MVTime(vb.time()(0)/86400.0) << " -------------------" << endl;

    // Arrange to accumulate
    VisBuffAccumulator vba(nAnt(),preavg(),False);

    // Collapse each timestamp in this chunk according to VisEq
    //  with calibration and averaging

    for (vi.origin(); vi.more(); vi++) {

      ve.collapse(vb);

      //      vb.normalize();

      // If this solve not freqdep, and channels not averaged yet, do so
      if (!freqDepMat() && vb.nChannel()>1)
	vb.freqAveCubes();

      // Accumulate collapsed vb in a time average
      vba.accumulate(vb);
     
    }
    vba.finalizeAverage();

    // The VisBuffer to solve with
    VisBuffer& svb(vba.aveVisBuff());

    // Extract meta data from visBuffer
    syncSolveMeta(svb,vi.fieldId());

    // Fill solveCPar() with 1, nominally, and flagged
    // TBD: drop unneeded basline-dependence    
    solveCPar()=Complex(1.0);
    solveParOK()=False;

    if (svb.nRow()>0) {

      // solve for the R-L phase term in the current VB
      solveOneVB(svb);

      if (solveParOK()(0,0,0))
	logSink() << "Position angle offset solution for " 
		  << msfldcol.name()(currField())
		  << " (spw = " << currSpw() << ") = "
		  << arg(solveCPar()(0,0,0))*180.0/C::pi/2.0
		  << " deg."
		  << LogIO::POST;
      else
	logSink() << "Position angle offset solution for " 
		  << msfldcol.name()(currField())
		  << " (spw = " << currSpw() << ") "
		  << " was not determined (insufficient data)."
		  << LogIO::POST;
	
    }

    keep(islot(spw));

    islot(spw)++;

  }
  
  // Store it.
  store();

}

// File a solved solution (and meta-data) into a slot in the CalSet
void XMueller::keep(const Int& slot) {

  if (prtlev()>4) cout << " M::keep(i)" << endl;

  if (slot<cs().nTime(currSpw())) {
    // An available valid slot

   
    //    cout << "Result: solveCPar() = " << solveCPar() << endl;

    //    cout << "   Amp: " << amplitude(solveCPar()) << endl;
    //    cout << " Phase: " << phase(solveCPar()/solveCPar()(0,0,0))*180.0/C::pi << endl;

    //    cout << "Result: solveParOK() = " << solveParOK() << endl;

    cs().fieldId(currSpw())(slot)=currField();
    cs().time(currSpw())(slot)=refTime();

    // Only stop-start diff matters
    //  TBD: change CalSet to use only the interval
    //  TBD: change VisBuffAcc to calculate exposure properly
    cs().startTime(currSpw())(slot)=0.0;
    cs().stopTime(currSpw())(slot)=interval();

    // For now, just make these non-zero:
    cs().iFit(currSpw()).column(slot)=1.0;
    cs().iFitwt(currSpw()).column(slot)=1.0;
    cs().fit(currSpw())(slot)=1.0;
    cs().fitwt(currSpw())(slot)=1.0;

    IPosition blc4(4,0,       0,           0,        slot);
    IPosition trc4(4,nPar()-1,nChanPar()-1,nElem()-1,slot);
    cs().par(currSpw())(blc4,trc4).nonDegenerate(3) = solveCPar();
    cs().parOK(currSpw())(blc4,trc4).nonDegenerate(3)= solveParOK();

    cs().solutionOK(currSpw())(slot) = anyEQ(solveParOK(),True);

  }
  else
    throw(AipsError("XMueller::keep: Attempt to store solution in non-existent CalSet slot"));

}

void XMueller::calcAllMueller() {

  //  cout << "currMElem().shape() = " << currMElem().shape() << endl;

  // Put the phase factor into the cross-hand diagonals
  //  (1,0) for the para-hands  
  IPosition blc(3,0,0,0);
  IPosition trc(3,0,nChanMat()-1,nElem()-1);
  currMElem()(blc,trc)=Complex(1.0);

  blc(0)=trc(0)=1;
  currMElem()(blc,trc)=currCPar()(0,0,0);
  blc(0)=trc(0)=2;
  currMElem()(blc,trc)=conj(currCPar()(0,0,0));

  blc(0)=trc(0)=3;
  currMElem()(blc,trc)=Complex(1.0);

  currMElemOK()=True;

}


void XMueller::solveOneVB(const VisBuffer& vb) {

  // This just a simple average of the cross-hand
  //  visbilities...

  Complex d,md;
  Float wt,a;
  DComplex rl(0.0),lr(0.0);
  Double sumwt(0.0);
  for (Int irow=0;irow<vb.nRow();++irow) {
    if (!vb.flagRow()(irow) &&
	vb.antenna1()(irow)!=vb.antenna2()(irow)) {

      for (Int ich=0;ich<vb.nChannel();++ich) {
	if (!vb.flag()(ich,irow)) {
	  
	  // A common weight for both crosshands
	  // TBD: we should probably consider this carefully...
	  //  (also in D::guessPar...)
	  wt=Double(vb.weightMat()(1,irow)+
		    vb.weightMat()(2,irow))/2.0;

	  // correct weight for model normalization
	  a=abs(vb.modelVisCube()(1,ich,irow));
	  wt*=(a*a);
	  
	  if (wt>0.0) {
	    // Cross-hands only
	    for (Int icorr=1;icorr<3;++icorr) {
	      md=vb.modelVisCube()(icorr,ich,irow);
	      d=vb.visCube()(icorr,ich,irow);
	      
	      if (abs(d)>0.0) {
		
		if (icorr==1) 
		  rl+=DComplex(Complex(wt)*d/md);
		else
		  lr+=DComplex(Complex(wt)*d/md);
		
		sumwt+=Double(wt);
		
	      } // abs(d)>0
	    } // icorr
	  } // wt>0
	} // !flag
      } // ich
    } // !flagRow
  } // row
  
/*
  cout << "spw = " << currSpw() << endl;
  cout << " rl = " << rl << " " << arg(rl)*180.0/C::pi << endl;
  cout << " lr = " << lr << " " << arg(lr)*180.0/C::pi << endl;
*/

    // combine lr with rl
  rl+=conj(lr);
  
  // Normalize to unit amplitude
  //  (note that the phase result is insensitive to sumwt)
  Double amp=abs(rl);
  if (sumwt>0 && amp>0.0) {
    rl/=DComplex(amp);
    
    solveCPar()=Complex(rl);
    solveParOK()=True;
  }
  
}



// **********************************************************
//  KJones Implementations
//

KJones::KJones(VisSet& vs) :
  VisCal(vs),             // virtual base
  VisMueller(vs),         // virtual base
  GJones(vs)             // immediate parent
{
  if (prtlev()>2) cout << "K::K(vs)" << endl;

  // Extract per-spw ref Freq for phase(delay) calculation
  MSSpectralWindow msSpw(vs.msName()+"/SPECTRAL_WINDOW");
  MSSpWindowColumns msCol(msSpw);
  msCol.refFrequency().getColumn(KrefFreqs_,True);
  KrefFreqs_/=1.0e9;  // in GHz

}

KJones::KJones(const Int& nAnt) :
  VisCal(nAnt), 
  VisMueller(nAnt),
  GJones(nAnt)
{
  if (prtlev()>2) cout << "K::K(nAnt)" << endl;
}

KJones::~KJones() {
  if (prtlev()>2) cout << "K::~K()" << endl;
}


void KJones::specify(const Record& specify) {


  LogMessage message(LogOrigin("SolvableVisCal","specify"));

  Vector<Int> spws;
  Vector<Int> antennas;
  Vector<Int> pols;
  Vector<Double> parameters;

  Int Nspw(1);
  Int Ntime(1);
  Int Nant(0);
  Int Npol(1);
  
  Bool repspw(False);
  
  IPosition ip0(4,0,0,0,0);
  IPosition ip1(4,0,0,0,0);

/*   Not yet supporting time....
  if (specify.isDefined("time")) {
    // TBD: the time label
    cout << "time = " << specify.asString("time") << endl;

    cout << "refTime() = " << refTime() << endl;


  }
*/

  if (specify.isDefined("spw")) {
    // TBD: the spws (in order) identifying the solutions
    spws=specify.asArrayInt("spw");
    cout << "spws = " << spws << endl;
    Nspw=spws.nelements();
    if (Nspw<1) {
      // None specified, so loop over all, repetitively
      //  (We need to optimize this...)
      cout << "Specified parameters repeated on all spws." << endl;
      repspw=True;
      Nspw=nSpw();
      spws.resize(Nspw);
      indgen(spws);
    }
  }


  if (specify.isDefined("antenna")) {
    // TBD: the antennas (in order) identifying the solutions
    antennas=specify.asArrayInt("antenna");
    cout << "antenna indices = " << antennas << endl;
    Nant=antennas.nelements();
    if (Nant<1) {
      // Use specified values for _all_ antennas implicitly
      Nant=1;   // For the antenna loop below
      ip0(2)=0;
      ip1(2)=nAnt()-1;
    }
    else {
      // Point to first antenna
      ip0(2)=antennas(0);
      ip1(2)=ip0(2);
    }
  }
  if (specify.isDefined("pol")) {
    // TBD: the pols (in order) identifying the solutions
    String polstr=specify.asString("pol");
    cout << "pol = " << polstr << endl;
    if (polstr=="R" || polstr=="X") 
      // Fill in only first pol
      pols=Vector<Int>(1,0);
    else if (polstr=="L" || polstr=="Y") 
      // Fill in only second pol
      pols=Vector<Int>(1,1);
    else if (polstr=="R,L" || polstr=="X,Y") {
      // Fill in both pols explicity
      pols=Vector<Int>(2,0);
      pols(1)=1;
    }
    else if (polstr=="L,R" || polstr=="Y,X") {
      // Fill in both pols explicity
      pols=Vector<Int>(2,0);
      pols(0)=1;
    }
    else if (polstr=="")
      // Fill in both pols implicitly
      pols=Vector<Int>();
    else
      throw(AipsError("Invalid pol specification"));
    
    Npol=pols.nelements();
    if (Npol<1) {
      // No pol axis specified
      Npol=1;
      ip0(0)=0;
      ip1(0)=nPar()-1;
    }
    else {
      // Point to the first polarization
      ip0(0)=pols(0);
      ip1(0)=ip0(0);
    }
  }
  if (specify.isDefined("parameter")) {
    // TBD: the actual cal values
    cout << "parameter = " << specify.asArrayDouble("parameter") << endl;

    parameters=specify.asArrayDouble("parameter");

  }
  
  cout << "Shapes = " << parameters.nelements() << " " 
       << (repspw ? (Ntime*Nant*Npol) : (Nspw*Ntime*Nant*Npol)) << endl;

  Int ipar(0);
  for (Int ispw=0;ispw<Nspw;++ispw) {
    // reset par index if we are repeating for all spws
    if (repspw) ipar=0;
    
    // Loop over specified timestamps
    for (Int itime=0;itime<Ntime;++itime) {
      ip1(3)=ip0(3)=itime;
      
      // Loop over specified antennas
      for (Int iant=0;iant<Nant;++iant) {
	if (Nant>1)
	  ip1(2)=ip0(2)=antennas(iant);
	
	// Loop over specified polarizations
	for (Int ipol=0;ipol<Npol;++ipol) {
	  if (Npol>1)
	    ip1(0)=ip0(0)=pols(ipol);
	  
	  Array<Complex> slice(cs().par(spws(ispw))(ip0,ip1));

	  // Acccumulation is addition for delays
	  slice+=Complex(parameters(ipar));
	  ++ipar;
	}
      }
    }
  }
}

void KJones::calcAllJones() {

  if (prtlev()>6) cout << "       VJ::calcAllJones()" << endl;

  if (False) {
    Vector<Complex> x(16,Complex(1.0));
    cout << "x = " << x << endl;
  
    ArrayLattice<Complex> arx(x);
  
    LatticeFFT::cfft(arx);
    cout << "x = " << x << endl;
  }

  // Should handle OK flags in this method, and only
  //  do Jones calc if OK

  Vector<Complex> oneJones;
  Vector<Bool> oneJOK;
  Vector<Complex> onePar;
  Vector<Bool> onePOK;

  ArrayIterator<Complex> Jiter(currJElem(),1);
  ArrayIterator<Bool>    JOKiter(currJElemOK(),1);
  ArrayIterator<Complex> Piter(currCPar(),1);
  ArrayIterator<Bool>    POKiter(currParOK(),1);

  Double phase(0.0);
  for (Int iant=0; iant<nAnt(); iant++) {

    for (Int ich=0; ich<nChanMat(); ich++) {
      
      oneJones.reference(Jiter.array());
      oneJOK.reference(JOKiter.array());
      onePar.reference(Piter.array());
      onePOK.reference(POKiter.array());

      for (Int ipar=0;ipar<nPar();++ipar) {
	if (onePOK(ipar)) { 
	  phase=2.0*C::pi*real(onePar(ipar))*(currFreq()(ich)-KrefFreqs_(currSpw()));
	  oneJones(ipar)=Complex(cos(phase),sin(phase));
	  oneJOK(ipar)=True;
	}
      }
      
      // Advance iterators
      Jiter.next();
      JOKiter.next();
      if (freqDepPar()) {
        Piter.next();
        POKiter.next();
      }

    }
    // Step to next antenns's pars if we didn't in channel loop
    if (!freqDepPar()) {
      Piter.next();
      POKiter.next();
    }
  }
}




} //# NAMESPACE CASA - END
