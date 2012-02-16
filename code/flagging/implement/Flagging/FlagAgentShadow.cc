//# FlagAgentShadow.cc: This file contains the implementation of the FlagAgentShadow class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#include <flagging/Flagging/FlagAgentShadow.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Definition of static members for common pre-processing
vector<Int> FlagAgentShadow::shadowedAntennas_p;
casa::async::Mutex FlagAgentShadow::staticMembersMutex_p;
vector<bool> FlagAgentShadow::startedProcessing_p;
bool FlagAgentShadow::preProcessingDone_p = false;
uShort FlagAgentShadow::nAgents_p = 0;

FlagAgentShadow::FlagAgentShadow(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube, Bool flag):
		FlagAgentBase(dh,config,ROWS_PREPROCESS_BUFFER,writePrivateFlagCube,flag)
{
	setAgentParameters(config);

	// Set preProcessingDone_p static member to false
	preProcessingDone_p = false;

	// Request loading antenna1,antenna2 and uvw
	flagDataHandler_p->preLoadColumn(VisBufferComponents::Ant1);
	flagDataHandler_p->preLoadColumn(VisBufferComponents::Ant2);
	flagDataHandler_p->preLoadColumn(VisBufferComponents::Uvw);
	/////flagDataHandler_p->preLoadColumn(VisBufferComponents::Time);
	flagDataHandler_p->preLoadColumn(VisBufferComponents::TimeCentroid);
	flagDataHandler_p->preLoadColumn(VisBufferComponents::PhaseCenter);
	/////flagDataHandler_p->preLoadColumn(VisBufferComponents::Direction1);

	// FlagAgentShadow counters and ids to handle static variables
	staticMembersMutex_p.acquirelock();
	agentNumber_p = nAgents_p;
	nAgents_p += 1;
	staticMembersMutex_p.unlock();

	// Set timekeeper to zero - this will later detect when the timestep changes.
        currTime_p=0.0;

        // Append the supplied additional antennas to COPIES of existing base-class lists.

	// Append to existing lists of antenna info.
	Int nAntsInMS = flagDataHandler_p->antennaNames_p->nelements();
	Int nNewAnts=0;


        //  antennaNames_p
        // antennaDiameters_p
	// antennaPositions_p
        if( additionalAntennas_p.nfields() )
	  {

            // For debugging...
	    //ostringstream recprint;
	    //additionalAntennas_p.print(recprint);
	    //cout << " Additional Antennas : " << recprint.str() << endl;

	    // TODO : Verify input Record. If invalid, print warning and proceed with no extra antennas.
            Bool validants=True;
	    String errorReason;
	    for(Int anew=0; anew<additionalAntennas_p.nfields(); anew++)
	      {
		// Extract the record.
		Record arec = additionalAntennas_p.subRecord(RecordFieldId(String::toString(anew)));

                if( ! arec.isDefined("diameter") || 
                    ( arec.type(arec.fieldNumber("diameter")) != casa::TpFloat && 
		      arec.type(arec.fieldNumber("diameter")) != casa::TpDouble  ) )
		  {
		    validants=False;
		    errorReason += String("Input Record [") + String::toString(anew) + ("] needs a field 'diameter' of type <double> \n");
		  }

                if( ! arec.isDefined("position") || 
                    ( arec.type(arec.fieldNumber("position")) != casa::TpArrayFloat && 
		      arec.type(arec.fieldNumber("position")) != casa::TpArrayDouble  ) )
		  {
		    validants=False;
		    errorReason += String("Input Record [") + String::toString(anew) + ("] needs a field 'position' of type Array<double>\n");
		  }
		else
		{		  
		  Array<Double> tpos;
		  arec.get( RecordFieldId(String("position")) , tpos );
                  if(tpos.shape() != IPosition(1,3))
		    {
		      validants=False;
		      errorReason += String("'position' for Record [") + String::toString(anew)+ ("] must be a vector of 3 floats or doubles\n");
		    }
		}
		
	      }// end of valid-ants loop
	    
	    // If antenna list is valid, set the number of new antennas to add.
            if(validants)
	      {
		nNewAnts = additionalAntennas_p.nfields();
	      }
	    else // warn and continue.
	      {
		*logger_p << LogIO::WARN << "NOT using additional antennas for shadow calculations, for the following reason(s) : " << errorReason << LogIO::POST;
	      }
	  }// if additionalAnts exist.
	

	// Make holders for cumulative information
	shadowAntennaPositions_p.resize(nAntsInMS+nNewAnts);
	///        shadowAntennaNames_p.resize(nAntsInMS+nNewAnts);
        shadowAntennaDiameters_p.resize(nAntsInMS+nNewAnts);	

	// Copy existing antennas into these arrays
	for(Int antid=0;antid<nAntsInMS;antid++)
	  {
	    shadowAntennaPositions_p[antid] = flagDataHandler_p->antennaPositions_p->operator()(antid);
	    ///shadowAntennaNames_p[antid] = flagDataHandler_p->antennaNames_p->operator()(antid);
	    shadowAntennaDiameters_p[antid] = flagDataHandler_p->antennaDiameters_p->operator()(antid);
	  } 
	
	// If any additional antennas are given, and are valid, add them to the lists
	for(Int antid=0;antid<nNewAnts;antid++)
	  {
            // Extract the record.
	    Record arec = additionalAntennas_p.subRecord(RecordFieldId(String::toString(antid)));

            // Extract and add new positions
	    Array<Double> aposarr;
	    arec.get( RecordFieldId(String("position")) , aposarr );
	    Vector<Double> aposvec(aposarr);
	    MVPosition apos(aposvec(0),aposvec(1),aposvec(2));
	    shadowAntennaPositions_p[nAntsInMS+antid] = MPosition(apos,MPosition::Types(MPosition::ITRF));

            // Extract and add new diameters
            Double adia;
            arec.get( RecordFieldId(String("diameter")) , adia );            
	    shadowAntennaDiameters_p[nAntsInMS+antid] = adia;

            // Extract and add new names
            ///String aname aname;
            ///arec.get( RecordFieldId(String("name")) , aname );            
	    ///shadowAntennaNames_p[nAntsInMS+antid] = aname;

	  }
	
	
}// end of constructor
  
  FlagAgentShadow::~FlagAgentShadow()
  {
    // Compiler automagically calls FlagAgentBase::~FlagAgentBase()
    
    // NOTE: The following is necessary because the static variables
    // persist even if all the instances of the class were deleted!
    staticMembersMutex_p.acquirelock();
    agentNumber_p = nAgents_p;
    nAgents_p -= 1;
    staticMembersMutex_p.unlock();
  }
  
  void
  FlagAgentShadow::setAgentParameters(Record config)
  {
    logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
    int exists;
    
    // Amount of shadowing to allow. Float or Double, in units of Meters.
    exists = config.fieldNumber ("tolerance");
    if (exists >= 0)
      {
	shadowTolerance_p = config.asDouble("tolerance");
      }
    else
      {
	shadowTolerance_p = 0.0;
      }
    
    *logger_p << logLevel_p << " tolerance is " << shadowTolerance_p << " meters "<< LogIO::POST;
    
    // A list of antenna parameters, to add to those in the antenna subtable, to calculate shadows.
    exists = config.fieldNumber ("addantenna");
    if (exists >= 0)
      {
	additionalAntennas_p = config.subRecord( RecordFieldId("addantenna") );
      }
    else
      {
	additionalAntennas_p = Record();
      }
    
    ostringstream recprint;
    additionalAntennas_p.print(recprint);
    *logger_p << logLevel_p << " addantenna is " << recprint.str() << LogIO::POST;
    
    // Choose which mode of shadowing to use.
    // recalculate UVW = True : If there are extra antennas, or missing baselines.
    // recalculate UVW = False : If baselines represent *all* antennas at *all* times.
    exists = config.fieldNumber ("recalcuvw");
    if (exists >= 0)
      {
	recalculateUVW_p = config.asBool("recalcuvw");

      }
    else
      {
	recalculateUVW_p = False;
      }
    
    *logger_p << logLevel_p << " recalcuvw is " << recalculateUVW_p << LogIO::POST;

    // catch the inconsistent case, and force consistency.
    if( recalculateUVW_p == False && additionalAntennas_p.nfields()>0 )
      {
	*logger_p << LogIO::WARN << "Additional antennas have been specified. Changing 'recalcUVW' to True" << LogIO::POST;
	recalculateUVW_p = True;
      }
    
    return;
  }
  
  void
  FlagAgentShadow::preProcessBuffer(const VisBuffer &visBuffer)
  {
    if (nAgents_p > 1)
      {
	staticMembersMutex_p.acquirelock();
	
	if (!preProcessingDone_p)
	  {
	    // Reset processing state variables
	    if (startedProcessing_p.size() != nAgents_p) startedProcessing_p.resize(nAgents_p,false);
	    for (vector<bool>::iterator iter = startedProcessing_p.begin();iter != startedProcessing_p.end();iter++)
	      {
		*iter = false;
	      }
	    
	    // Do actual pre-processing
	    preProcessBufferCore(visBuffer);
	    
	    // Mark pre-processing as done so that other agents don't redo it
	    preProcessingDone_p = true;
	  }
	
	staticMembersMutex_p.unlock();
      }
    else
      {
	preProcessBufferCore(visBuffer);
      }
    
    return;
  }
  
  void
  FlagAgentShadow::preProcessBufferCore(const VisBuffer &visBuffer)
  {
    // This function is empty, because shadowedAntennas_p needs to be re-calculated for
    // every new timestep, and it is done inside computeRowFlags(), whenever the
    // timestep changes. 
  }
  
  void FlagAgentShadow::calculateShadowedAntennas(const VisBuffer &visBuffer, Int rownr)
  {
    shadowedAntennas_p.clear();
    Double u,v,w, uvDistance;
    Int nAnt = shadowAntennaDiameters_p.nelements(); 

    Double reftime = 4.794e+09;

    if(recalculateUVW_p)
      {
	//  (1) For the current timestep, compute UVWs for all antennas.
	//    uvwAnt_p will be filled these values.
	computeAntUVW(visBuffer, rownr);

	// debug code.
	// Int   tant1 = visBuffer.antenna1()(rownr);
	// Int   tant2 = visBuffer.antenna2()(rownr);
	
	//  (2) For all antenna pairs, calculate UVW of the baselines, and check for shadowing.
	for (Int antenna1=0; antenna1<nAnt; antenna1++)
	  {	    
	    Double u1=uvwAnt_p(0,antenna1), v1=uvwAnt_p(1,antenna1), w1=uvwAnt_p(2,antenna1);
	    for (Int antenna2=antenna1; antenna2<nAnt; antenna2++)
	      {
		// Check if this row corresponds to autocorrelation (Antennas don't shadow themselves)
		if (antenna1 == antenna2) continue;
		
		Double u2=uvwAnt_p(0,antenna2), v2=uvwAnt_p(1,antenna2), w2=uvwAnt_p(2,antenna2);
		
		u = u2-u1;
		v = v2-v1;
		w = w2-w1;
		uvDistance = sqrt(u*u + v*v);

                //if(rownr==0 && antenna1==tant1 && antenna2==tant2) cout << " (r)Row : " << rownr << "   uvdist : " << uvDistance << "  w : " << w << " time-x : " << visBuffer.timeCentroid()(rownr)-reftime << endl;
		
		decideBaselineShadow(uvDistance, w, antenna1, antenna2);
		
	      }// end for antenna2
	  }// end for antenna1
	
      }// end of recalculateUVW_p==True
    else // recalculateUVW_p = False
      {

        // We know the starting row for this timestep. Find the ending row.
        // This assumes that all baselines are grouped together. 
	// This is guaranteed by the sort-order defined for the visIterator.
        Int endrownr = rownr;
	Double timeval = visBuffer.timeCentroid()(rownr) ;
	for (Int row_i=rownr;row_i<visBuffer.nRow();row_i++)
	  {
	    if(timeval < visBuffer.timeCentroid()(row_i)) // we have touched the next timestep
	      {
		endrownr = row_i-1;
		break;
	      }
	    else
	      {
		endrownr = row_i;
	      }
	  }

	//cout << "For time : " << timeval-4.73423e+09 << " start : " << rownr << " end : " << endrownr << endl;

	// Now, for all rows between 'rownr' and 'endrownr', calculate shadowed Ants.
        // This row range represents all baselines in the "current" timestep.
	Int antenna1, antenna2;
	for (Int row_i=rownr;row_i<endrownr;row_i++)
	  {
	    // Retrieve antenna ids
	    antenna1 = visBuffer.antenna1()(row_i);
	    antenna2 = visBuffer.antenna2()(row_i);
	    
	    // Check if this row corresponds to autocorrelation (Antennas don't shadow themselves)
	    if (antenna1 == antenna2) continue;

	    // Compute uv distance
	    u = visBuffer.uvw()(row_i)(0);
	    v = visBuffer.uvw()(row_i)(1);
	    w = visBuffer.uvw()(row_i)(2);
	    uvDistance = sqrt(u*u + v*v);

            //if(row_i==0 && rownr==0) cout << " Row : " << row_i << "   uvdist : " << uvDistance << " w : " << w << " time-x : " << visBuffer.timeCentroid()(row_i)-reftime << endl;
	
	    decideBaselineShadow(uvDistance, w, antenna1, antenna2);

	  }// end of for 'row'
      }
    
  }// end of calculateShadowedAntennas
  
  void FlagAgentShadow::decideBaselineShadow(Double uvDistance, Double w, Int antenna1, Int antenna2)
  {
    Double antennaDiameter1,antennaDiameter2, antennaDistance;

    // Get antenna diameter
    antennaDiameter1 = shadowAntennaDiameters_p[antenna1];
    antennaDiameter2 = shadowAntennaDiameters_p[antenna2];
    
    //  Compute effective distance for shadowing
    antennaDistance = (antennaDiameter1+antennaDiameter2)/2.0;
    
    // Check if one of the antennas can be shadowed
    if (uvDistance < antennaDistance - shadowTolerance_p)
      {
	if (w>0)
	  {
	    if (std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna1) == shadowedAntennas_p.end())
	      {
		shadowedAntennas_p.push_back(antenna1);
	      }
	  }
	else
	  {
	    if (std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna2) == shadowedAntennas_p.end())
	      {
		shadowedAntennas_p.push_back(antenna2);
	      }
	  }
      }
  }
  
  /// NOTE : This function is almost a copy of
  ///  ms/MeasurementSets/NewMSSimulator::calcAntUVW
  ///  -- TODO : try to re-use that code by moving out all private-member accesses in the simulator.
  ///  -- TOCHECK : Should we use vb.timeCentroid() ??  This gives closest results so far, for real and simulated data.
  Bool FlagAgentShadow::computeAntUVW(const VisBuffer &vb, Int rownr)
  {
    // Get time and timeinterval from the visbuffer.
    Double Time;
    //////// Code from the simulator. Gives wrong uvws
    /////Time = vb.time()(rownr);// + vb.timeInterval()(rownr)/2.0; 
    
    // Centroid gives the closest values to uvws in the MS. For simulated data, gives exact values.
    Time = vb.timeCentroid()(rownr);

    // Make the Time epoch.
    MEpoch epoch(Quantity((Time), "s"), MEpoch::UT1);

    // Get the MDirection of the feed of antenna 1. Assume all ants point in the same direction.
    //MDirection refdir(vb.direction1()(rownr));  
    MDirection refdir(vb.phaseCenter());    // Each visbuf sees only one fieldId

    // read position of first antenna as reference. Does not matter, since uvws are only differences.
    MPosition obsPos( shadowAntennaPositions_p[0] ); 
    
    MVPosition basePos=obsPos.getValue();
    MeasFrame measFrame(obsPos);
    measFrame.set(epoch);
    measFrame.set(refdir);
    MVBaseline mvbl;
    MBaseline basMeas;
    MBaseline::Ref basref(MBaseline::ITRF, measFrame);
    basMeas.set(mvbl, basref);
    basMeas.getRefPtr()->set(measFrame);
    // going to convert from ITRF vector to J2000 baseline vector I guess !
    if(refdir.getRef().getType() != MDirection::J2000)
      throw(AipsError("Internal FlagAgentShadow restriction : Ref direction must be in  J2000 "));
    
    Int nAnt = shadowAntennaDiameters_p.nelements();
    if(uvwAnt_p.shape() != IPosition(2,3,nAnt)) 
      {
	uvwAnt_p.resize(3,nAnt);
      }
    
    MBaseline::Convert elconv(basMeas, MBaseline::Ref(MBaseline::J2000));
    Muvw::Convert uvwconv(Muvw(), Muvw::Ref(Muvw::J2000, measFrame));
    for(Int k=0; k< nAnt; ++k)
      {
	MPosition antpos=shadowAntennaPositions_p(k);   // msc.antenna().positionMeas()(k);
	
	MVBaseline mvblA(obsPos.getValue(), antpos.getValue());
	basMeas.set(mvblA, basref);
	MBaseline bas2000 =  elconv(basMeas);
	MVuvw uvw2000 (bas2000.getValue(), refdir.getValue());
	const Vector<double>& xyz = uvw2000.getValue();
	uvwAnt_p.column(k)=xyz;
      }
    
    return True;
  }
  
  
  bool
  FlagAgentShadow::computeRowFlags(const VisBuffer &visBuffer, FlagMapper &flags, uInt row)
  {
    // If we have advanced to a new timestep, calculate new antenna UVW values and shadowed antennas
    // This function resets and fills 'shadowedAntennas_p'.
    if( currTime_p != visBuffer.timeCentroid()(row) )
      {  
	currTime_p = visBuffer.timeCentroid()(row) ;
	calculateShadowedAntennas(visBuffer, row);
      }
    
    bool flagRow = false;
    // Flag row if either antenna1 or antenna2 are in the list of shadowed antennas
    Int antenna1 = visBuffer.antenna1()[row];
    Int antenna2 = visBuffer.antenna2()[row];
    if (	(std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna1) != shadowedAntennas_p.end()) or
		(std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna2) != shadowedAntennas_p.end()) )
      {
	flagRow = true;
      }
    
    if ((nAgents_p > 1) and preProcessingDone_p)
      {
	startedProcessing_p[agentNumber_p] = true;
	if (std::find (startedProcessing_p.begin(), startedProcessing_p.end(), false) == startedProcessing_p.end())
	  {
	    preProcessingDone_p = false;
	  }
      }
    
    return flagRow;
  }
  
  
#if 0
  void
  FlagAgentShadow::preProcessBufferCore(const VisBuffer &visBuffer)
  {
    Vector<Int> antenna1list =  visBuffer.antenna1();
    Vector<Int> antenna2list =  visBuffer.antenna2();
    shadowedAntennas_p.clear();
    Double u,v,w, uvDistance;
    Int antenna1, antenna2;
    Double antennaDiameter1,antennaDiameter2, antennaDistance;
    for (Int row_i=0;row_i<antenna1list.size();row_i++)
      {
	// Retrieve antenna ids
	antenna1 = antenna1list[row_i];
	antenna2 = antenna2list[row_i];
	
	// Check if this row corresponds to autocorrelation (Antennas don't shadow themselves)
	if (antenna1 == antenna2) continue;
	
	// Get antenna diameter
	if (antennaDiameter_p>0)
	  {
	    antennaDiameter1 = antennaDiameter_p;
	    antennaDiameter2 = antennaDiameter_p;
	  }
	else
	  {
	    Vector<Double> *antennaDiameters = flagDataHandler_p->antennaDiameters_p;
	    antennaDiameter1 = (*antennaDiameters)[antenna1];
	    antennaDiameter2 = (*antennaDiameters)[antenna2];
	  }
	
	// Compute effective distance for shadowing
	antennaDistance = (antennaDiameter1+antennaDiameter2)*(antennaDiameter1+antennaDiameter2)/4.0;
	
	// Compute uv distance
	u = visBuffer.uvw()(row_i)(0);
	v = visBuffer.uvw()(row_i)(1);
	w = visBuffer.uvw()(row_i)(2);
	uvDistance = u*u + v*v;
	
	// Check if one of the antennas can be shadowed
	if (uvDistance < antennaDistance)
	  {
	    if (w>0)
	      {
		if (std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna1) == shadowedAntennas_p.end())
		  {
		    shadowedAntennas_p.push_back(antenna1);
		  }
	      }
	    else
	      {
		if (std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna2) == shadowedAntennas_p.end())
		  {
		    shadowedAntennas_p.push_back(antenna2);
		  }
	      }
	  }
      }
  }
  
  bool
  FlagAgentShadow::computeRowFlags(const VisBuffer &visBuffer, FlagMapper &flags, uInt row)
  {
    bool flagRow = false;
    // Flag row if either antenna1 or antenna2 are in the list of shadowed antennas
    Int antenna1 = visBuffer.antenna1()[row];
    Int antenna2 = visBuffer.antenna2()[row];
    if (	(std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna1) != shadowedAntennas_p.end()) or
		(std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna2) != shadowedAntennas_p.end()) )
      {
	flagRow = true;
      }
    
    if ((nAgents_p > 1) and preProcessingDone_p)
      {
	startedProcessing_p[agentNumber_p] = true;
	if (std::find (startedProcessing_p.begin(), startedProcessing_p.end(), false) == startedProcessing_p.end())
	  {
	    preProcessingDone_p = false;
	  }
      }
    
    return flagRow;
  }
  
  
  
#endif
  
  
} //# NAMESPACE CASA - END


