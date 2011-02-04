//# SubMS.tcc 
//# Copyright (C) 2009
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This library is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//# 
//# You should have received a copy of the GNU General Public License
//# along with this library; if not, write to the Free Software
//# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
#include <msvis/MSVis/SubMS.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <casa/Arrays/Array.h>
#include <casa/BasicSL/String.h>
#include <casa/System/ProgressMeter.h>
#include <cmath>

namespace casa { //# NAMESPACE CASA - BEGIN

  // Add optional columns to outTab if present in inTab and possColNames.
  // Returns the number of added columns.
  template<class M>
  uInt SubMS::addOptionalColumns(const M& inTab, M& outTab, const Bool beLazy)
  {
    uInt nAdded = 0;    
    const TableDesc& inTD = inTab.actualTableDesc();
    
    // Only rely on the # of columns if you are sure that inTab and outTab
    // can't have the same # of columns without having _different_ columns,
    // i.e. use beLazy if outTab.actualTableDesc() is in its default state.
    uInt nInCol = inTD.ncolumn();
    if(!beLazy || nInCol > outTab.actualTableDesc().ncolumn()){
      LogIO os(LogOrigin("SubMS", "addOptionalColumns()"));

      Vector<String> oldColNames = inTD.columnNames();
      
      for(uInt k = 0; k < nInCol; ++k){
        if(!outTab.actualTableDesc().isColumn(oldColNames[k])){
          //TableDesc tabDesc;
          try{
            //M::addColumnToDesc(tabDesc, M::columnType(oldColNames[k]));
            //if(tabDesc.ncolumn())                 // The tabDesc[0] is too 
            //  outTab.addColumn(tabDesc[0]);       // dangerous otherwise - it 
            //else                                  // can dump core without
            //  throw(AipsError("Unknown column")); // throwing an exception.
	    outTab.addColumn(inTD.columnDesc(k), false);
            ++nAdded;
          }
          catch(...){   // NOT AipsError x
            os << LogIO::WARN 
               << "Could not add column " << oldColNames[k] << " to "
               << outTab.tableName()
               << LogIO::POST;
          }
	}
      }
    }
    return nAdded;
  }

template<class M>
void SubMS::filterChans(const ROArrayColumn<M>& data, ArrayColumn<M>& outDataCol,
			const Bool doSpWeight, ROArrayColumn<Float>& wgtSpec,
			const Int nrow,
			const Bool calcWtSig, const ROArrayColumn<Float>& rowWt,
			const ROArrayColumn<Float>& sigma)
{
  Bool deleteIptr;
  Matrix<M> indatatmp;

  LogIO os(LogOrigin("SubMS", "filterChans()"));

  // Sigh, iflg itself is const, but it points at the start of inflagtmp,
  // which is continually refreshed by a row of flag.
  ROArrayColumn<Bool> flag(mscIn_p->flag());
  Matrix<Bool> inflagtmp;
  Bool deleteIFptr;
  Matrix<Bool> outflag;
  
  Vector<Float> inrowwttmp;
  Vector<Float> outrowwt;
  
  Vector<Float> inrowsigtmp;
  Vector<Float> outrowsig;
  
  Matrix<Float> inwgtspectmp;
  Bool deleteIWptr;
   
  Matrix<M> outdata;
  Vector<M> outdatatmp;
  //    const Complex *optr = outdatatmp.getStorage(deleteOptr);

  Matrix<Float> outspweight;
  Vector<Float> outwgtspectmp;
  //   const Float *owptr = outwgtspectmp.getStorage(deleteOWptr);

  Vector<Int> avcounter;

  const ROScalarColumn<Int> dataDescIn(mscIn_p->dataDescId());

  // Guarantee oldOutDDID != ddID on 1st iteration.
  Int oldOutDDID = spwRelabel_p[oldDDSpwMatch_p[dataDescIn(0)]] - 1;

  // chanStart_p is Int, therefore inChanInd is too.
  Int inChanInc;
  Int nperbin;

  Double rowwtfac, sigfac;
  
  ProgressMeter meter(0.0, nrow * 1.0, "split", "rows filtered", "", "",
		      True, nrow / 100);

  for(Int row = 0; row < nrow; ++row){
    Int outDDID = spwRelabel_p[oldDDSpwMatch_p[dataDescIn(row)]];
    Bool newDDID = (outDDID != oldOutDDID);

    if(newDDID){
      if(outDDID < 0){                      // Paranoia
        os << LogIO::WARN
           << "Treating DATA_DESCRIPTION_ID " << outDDID << " as 0."
           << LogIO::POST;
	outDDID = 0;
      }
      
      inChanInc = averageChannel_p ? 1 : chanStep_p[outDDID];
      nperbin = averageChannel_p ? chanStep_p[outDDID] : 1;
      // .tcc files are hard to debug without print statements,
      //  but it is too easy to  make the logger thrash
      // the disk if these are left in.
      // os << LogIO::DEBUG1
      // 	 << outDDID << ": inChanInc = " << inChanInc
      // 	 << " nperbin = " << nperbin
      // 	 << "\nrow " << row << ": inNumCorr_p[outDDID] = "
      //         << inNumCorr_p[outDDID]
      // 	 << ", ncorr_p[outDDID] = " << ncorr_p[outDDID]
      // 	 << "\ninNumChan_p[outDDID] = " << inNumChan_p[outDDID]
      // 	 << ", nchan_p[outDDID] = " << nchan_p[outDDID]
      // 	 << LogIO::POST;
      
      // resize() will return right away if the size does not change, so
      // it is not essential to check ncorr_p[outDDID] != ncorr_p[oldOutDDID], etc.
      indatatmp.resize(inNumCorr_p[outDDID], inNumChan_p[outDDID]);
      inflagtmp.resize(inNumCorr_p[outDDID], inNumChan_p[outDDID]);
      outflag.resize(ncorr_p[outDDID], nchan_p[outDDID]);
      outdata.resize(ncorr_p[outDDID], nchan_p[outDDID]);
      outdatatmp.resize(ncorr_p[outDDID]);
      if(doSpWeight){
        inwgtspectmp.resize(inNumCorr_p[outDDID], inNumChan_p[outDDID]);
	outspweight.resize(ncorr_p[outDDID], nchan_p[outDDID]);
	outwgtspectmp.resize(ncorr_p[outDDID]);
      }

      if(calcWtSig){
	rowwtfac = static_cast<Float>(nchan_p[outDDID]) / inNumChan_p[outDDID];
	if(averageChannel_p)
	  rowwtfac *= chanStep_p[outDDID];
	sigfac = 1.0 / sqrt(rowwtfac);
	os << LogIO::DEBUG1
	   << outDDID << ": inNumChan_p[outDDID] = " << inNumChan_p[outDDID]
	   << ", nchan_p[outDDID] = " << nchan_p[outDDID]
	   << "\nrowwtfac = " << rowwtfac
	   << ", sigfac = " << sigfac
	   << LogIO::POST;
	inrowwttmp.resize(inNumCorr_p[outDDID]);
	outrowwt.resize(ncorr_p[outDDID]);
	inrowsigtmp.resize(inNumCorr_p[outDDID]);
	outrowsig.resize(ncorr_p[outDDID]);
	os << LogIO::DEBUG1
	   << "inNumCorr_p[outDDID] = " << inNumCorr_p[outDDID]
	   << ", ncorr_p[outDDID] = " << ncorr_p[outDDID]
	   << LogIO::POST;
      }

      avcounter.resize(ncorr_p[outDDID]);

      oldOutDDID = outDDID;
    }

    // Should come after any resize()s.
    outflag.set(false);
    data.get(row, indatatmp);
    flag.get(row, inflagtmp);
    // These were more to say "I made it here!" than anything.
    //os << LogIO::DEBUG1 << "doSpWeight: " << doSpWeight << LogIO::POST;
    //os << LogIO::DEBUG1 << "calcWtSig: " << calcWtSig << LogIO::POST;
    if(doSpWeight){
      outrowwt.set(0.0);
      if(calcWtSig)
	wgtSpec.get(row, inwgtspectmp);
    }
    else if(calcWtSig)
      rowWt.get(row, inrowwttmp);
    if(calcWtSig)
      sigma.get(row, inrowsigtmp);
    
    uInt outChanInd = 0;
    Int chancounter = 0;
    outdatatmp.set(0); outwgtspectmp.set(0);
    avcounter.set(0);
    
    const M *iptr = indatatmp.getStorage(deleteIptr);
    const Float *inwptr = inwgtspectmp.getStorage(deleteIWptr);
    const Bool *iflg = inflagtmp.getStorage(deleteIFptr);
    for(Int inChanInd = chanStart_p[outDDID];
	inChanInd < (nchan_p[outDDID] * chanStep_p[outDDID] +
		     chanStart_p[outDDID]); inChanInd += inChanInc){
      if(chancounter == nperbin){
        outdatatmp.set(0); outwgtspectmp.set(0);
        chancounter = 0;
        avcounter.set(0);
      }
      ++chancounter;

      for(Int outCorrInd = 0; outCorrInd < ncorr_p[outDDID]; ++outCorrInd){
        Int offset = inPolOutCorrToInCorrMap_p[polID_p[spw_uniq_p[outDDID]]][outCorrInd]
	             + inChanInd * inNumCorr_p[outDDID];
	// //if(ncorr_p[outDDID] != inNumCorr_p[outDDID])
	//   os << LogIO::DEBUG2		       // 
	//      << "outCorrInd = " << outCorrInd  //
	//      << "\ninChanInd = " << inChanInd	//
	//      << "\noffset = " << offset		// 
	//      << LogIO::POST;			// 
	// os << LogIO::DEBUG2
	//    << "iflg[offset] = " << iflg[offset]
	//    << "\niptr[offset] = " << iptr[offset]
	//   //<< "\ninwptr[offset] = " << inwptr[offset]
	//    << LogIO::POST;
        if(!iflg[offset]){
          if(doSpWeight){
            outdatatmp[outCorrInd] += iptr[offset] * inwptr[offset];
            outwgtspectmp[outCorrInd] += inwptr[offset];
          }
          else
            outdatatmp[outCorrInd] += iptr[offset];	   
          ++avcounter[outCorrInd];
        }

        if(chancounter == nperbin){
	  // //if(ncorr_p[outDDID] != inNumCorr_p[outDDID])
	  //   os << LogIO::DEBUG2
	  //      << "row " << row
	  //      << ": avcounter[outCorrInd] = " << avcounter[outCorrInd]
	  //     << LogIO::POST;
          if(avcounter[outCorrInd] != 0){
            if(doSpWeight){
              if(outwgtspectmp[outCorrInd] != 0.0){
                outdata(outCorrInd,
			outChanInd) = outdatatmp[outCorrInd] / 
		                      outwgtspectmp[outCorrInd];
		outrowwt[outCorrInd] += outwgtspectmp[outCorrInd];
	      }
              else{
                outdata(outCorrInd, outChanInd) = 0.0;
                outflag(outCorrInd, outChanInd) = True;
              }
              outspweight(outCorrInd, outChanInd) = outwgtspectmp[outCorrInd];
            }
            else{
              outdata(outCorrInd,
		      outChanInd) = outdatatmp[outCorrInd] / avcounter[outCorrInd];
            }
          }
          else{
            outdata(outCorrInd, outChanInd) = 0;
            outflag(outCorrInd, outChanInd) = True;
            if(doSpWeight)
              outspweight(outCorrInd, outChanInd) = 0;
          }	
        }
      }
      if(chancounter == chanStep_p[outDDID])
        ++outChanInd;
    }
    outDataCol.put(row, outdata);
    msc_p->flag().put(row, outflag);
    if(calcWtSig){
      for(Int outCorrInd = 0; outCorrInd < ncorr_p[outDDID]; ++outCorrInd){
        Int inCorr = inPolOutCorrToInCorrMap_p[polID_p[spw_uniq_p[outDDID]]][outCorrInd];
	if(!doSpWeight)
	  outrowwt[outCorrInd] = rowwtfac * inrowwttmp[inCorr];
	outrowsig[outCorrInd] = sigfac * inrowsigtmp[inCorr];
      }
      msc_p->weight().put(row, outrowwt);
      msc_p->sigma().put(row, outrowsig);
    }
    if(doSpWeight)
      msc_p->weightSpectrum().put(row, outspweight);

    meter.update(row);
  }
}

} //# NAMESPACE CASA - END


