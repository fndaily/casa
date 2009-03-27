//# PlotMSCache.h: Data cache for plotms.
//# Copyright (C) 2009
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
//# $Id: $
#ifndef PLOTMSCACHE_H_
#define PLOTMSCACHE_H_

#include <plotms/Actions/PlotMSThread.qo.h>
#include <plotms/PlotMS/PlotMSConstants.h>
#include <plotms/PlotMS/PlotMSLogger.h>

#include <casa/aips.h>
#include <casa/Arrays.h>
#include <casa/Containers/Block.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisSet.h>

namespace casa {

class PlotMSCache {
  
public:
    
    static const PMS::Axis METADATA[];
    static const unsigned int N_METADATA;
    
    static bool axisIsMetaData(PMS::Axis axis);
    
    static const unsigned int THREAD_SEGMENT;

  // Constructor
  PlotMSCache();
  
  // Destructor
  ~PlotMSCache();

  // Report the number of chunks
  Int nChunk() const { return nChunk_; };

  // Report the total number of points currently arranged for plotting
  //  (TBD: this is incorrect unless ALL cache spaces are full!!)
  Int nPoints() const { return nPoints_(nChunk_-1); };

  // Clears the cache of all stored values.  This should be called when the
  // underlying MS or MS selection is changed, thus invalidating stored data.
  void clear();
  
  // Loads the cache using the given VisSet, for the given axes and data
  // columns.  IMPORTANT: this method assumes that any currently loaded data is
  // valid for the given VisIter; i.e., if the meta-information or either of
  // the axes are already loaded, then they don't need to be reloaded.  If this
  // is not the case, then clear() should be called BEFORE append().  If a
  // PlotMSCacheThreadHelper object is given, it will be used to report
  // progress information.
  void load(VisSet& visSet, const vector<PMS::Axis>& axes,
            const vector<PMS::DataColumn>& data,
            const PlotMSAveraging& averaging,
            PlotMSCacheThread* thread = NULL);

  // Convenience method for loading x and y axes.
  void load(VisSet& visSet, PMS::Axis xAxis, PMS::Axis yAxis,
            PMS::DataColumn xData, PMS::DataColumn yData,
            const PlotMSAveraging& averaging,
            PlotMSCacheThread* thread = NULL) {
      vector<PMS::Axis> axes(2);
      axes[0] = xAxis; axes[1] = yAxis;
      vector<PMS::DataColumn> data(2);
      data[0] = xData; data[1] = yData;
      load(visSet, axes, data, averaging, thread);
  }
  
  // Releases the given axes from the cache.
  void release(const vector<PMS::Axis>& axes);
  
  // Set up indexing for the plot (amp vs freq hardwired version)
  void setUpPlot(PMS::Axis xAxis, PMS::Axis yAxis);
  void getAxesMask(PMS::Axis axis,Vector<Bool>& axismask);
  
  // Returns whether the cache is ready to return plotting data via the get
  // methods or not.
  bool readyForPlotting() const;

  // Get X and Y limits (amp vs freq hardwired version)
  void getRanges(Double& minX, Double& maxX, Double& minY, Double& maxY);

  // Get single values for x-y plotting 
  Double getX(Int i);
  Double getY(Int i);
  void getXY(Int i, Double& x, Double& y);
  Double get(PMS::Axis axis);

  // Axis-specific gets
  inline Double getScan()      { return scan_(currChunk_); };
  inline Double getField()     { return field_(currChunk_); };
  inline Double getTime()      { return time_(currChunk_); };
  inline Double getTimeIntr()  { return timeIntr_(currChunk_); };
  inline Double getSpw()       { return spw_(currChunk_); };
  inline Double getFreq() { return *(freq_[currChunk_]->data()+(irel_/nperchan_(currChunk_))%ichanmax_(currChunk_)); };
  inline Double getChan() { return *(chan_[currChunk_]->data()+(irel_/nperchan_(currChunk_))%ichanmax_(currChunk_)); };
  inline Double getCorr() { return *(corr_[currChunk_]->data()+(irel_%icorrmax_(currChunk_))); };
  inline Double getAnt1() { return *(antenna1_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };
  inline Double getAnt2() { return *(antenna2_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };
  inline Double getBsln() { return *(baseline_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };
  inline Double getUVDist() { return *(uvdist_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };
  inline Double getUVDistL() { return *(uvdistL_[currChunk_]->data()+(irel_/nperchan_(currChunk_))%ichanbslnmax_(currChunk_)); };
  inline Double getU() { return *(u_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };
  inline Double getV() { return *(v_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };
  inline Double getW() { return *(w_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };

  inline Double getAmp()  { return *(amp_[currChunk_]->data()+(irel_%idatamax_(currChunk_))); }; 
  // flag_[currChunk_]->data()+(irel_%idatamax_(currChunk_)) ? -1.0 : 
  inline Double getPha()  { return  *(pha_[currChunk_]->data()+(irel_%idatamax_(currChunk_))); };
  // flag_[currChunk_]->data()+(irel_%idatamax_(currChunk_)) ? -999.0 : 
  inline Double getReal() { return *(real_[currChunk_]->data()+(irel_%idatamax_(currChunk_))); };
  inline Double getImag() { return *(imag_[currChunk_]->data()+(irel_%idatamax_(currChunk_))); };
  inline Double getFlag() { return *(flag_[currChunk_]->data()+(irel_%idatamax_(currChunk_))); };
  inline Double getAz() { return *(az_[currChunk_]->data()+(irel_%iantmax_(currChunk_))); };
  inline Double getEl() { return *(el_[currChunk_]->data()+(irel_%iantmax_(currChunk_))); };
  inline Double getParAng() { return *(parang_[currChunk_]->data()+(irel_%iantmax_(currChunk_))); };
  inline Double getRow() { return *(row_[currChunk_]->data()+(irel_/nperbsln_(currChunk_))%ibslnmax_(currChunk_)); };


  // Locate datum nearest to specified x,y (amp vs freq hardwired versions)
  PlotLogMessage* locateNearest(Double x, Double y);
  PlotLogMessage* locateRange(Double xmin,Double xmax,Double ymin,Double ymax);
  
  // Returns which axes have been loaded into the cache, including metadata.
  // Also includes the size (number of points) for each axis (which will
  // eventually be used for a cache manager to let the user know the
  // relative memory use of each axis).
  vector<pair<PMS::Axis, unsigned int> > loadedAxes() const;


private:
    
  // Forbid copy for now
  PlotMSCache(const PlotMSCache& mc);

  // Increase the number of chunks
  void increaseChunks(Int nc=0);

  // Fill a chunk with a VisBuffer.  
  void append(const VisBuffer& vb, Int vbnum, PMS::Axis xAxis, PMS::Axis yAxis,
              PMS::DataColumn xData, PMS::DataColumn yData);
  
  // Issue mete info report to the given stringstream.
  void reportMeta(Double x, Double y, stringstream& ss);

  // Set currChunk_ according to a supplied index
  void setChunk(Int i);

  // Clean up the PtrBlocks
  void deleteCache();
  
  // Loads the specific axis/metadata into the cache using the given VisBuffer.
  void loadAxis(const VisBuffer& vb, Int vbnum, PMS::Axis axis,
          PMS::DataColumn data = PMS::DEFAULT_DATACOLUMN);
  
  // Returns the number of points loaded for the given axis or 0 if not loaded.
  unsigned int nPointsForAxis(PMS::Axis axis) const;
  
  // Computes the X and Y limits for the currently set axes.  In the future we
  // may want to cache ALL ranges for all loaded values to avoid recomputation.
  void computeRanges();

  // Private data

  // The number of chunks
  Int nChunk_;

  // The in-focus chunk and relative index offset
  Int currChunk_, irel_;

  // The cumulative running total of points
  Vector<Int> nPoints_;

  Double minX_,maxX_,minY_,maxY_;

  // The fundamental meta-data cache
  Matrix<Int> chshapes_;
  Vector<Double> time_, timeIntr_;
  Vector<Int> field_, spw_, scan_;
  PtrBlock<Vector<uInt>*> row_;
  PtrBlock<Vector<Int>*> antenna1_, antenna2_, baseline_;
  PtrBlock<Vector<Double>*> uvdist_, u_, v_, w_;
  PtrBlock<Matrix<Double>*> uvdistL_;
  PtrBlock<Vector<Double>*> freq_;
  PtrBlock<Vector<Int>*> chan_;
  PtrBlock<Vector<Int>*> corr_;

  // Optional parts of the cache
  PtrBlock<Vector<Float>*> pa_;

  // Data (the heavy part)
  PtrBlock<Array<Float>*> amp_, pha_, real_, imag_;
  PtrBlock<Array<Bool>*> flag_;
  PtrBlock<Vector<Bool>*> flagrow_;

  PtrBlock<Vector<Float>*> parang_;
  PtrBlock<Vector<Double>*> az_,el_;

  // Indexing help
  Vector<Int> icorrmax_, ichanmax_, ibslnmax_, idatamax_;
  Vector<Int> nperchan_, nperbsln_;
  Vector<Int> ichanbslnmax_;
  Vector<Int> iantmax_;

  // Current setup/state.
  bool dataLoaded_;
  bool currentSet_;
  PMS::Axis currentX_, currentY_;
  map<PMS::Axis, bool> loadedAxes_;
  map<PMS::Axis, PMS::DataColumn> loadedAxesData_;
};
typedef CountedPtr<PlotMSCache> PlotMSCachePtr;

}

#endif /* PLOTMSCACHE_H_ */
