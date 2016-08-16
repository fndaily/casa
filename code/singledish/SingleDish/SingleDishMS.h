//# SingleDishMS.h: this defines a class that handles single dish MSes
//#
//# Copyright (C) 2014,2015
//# National Astronomical Observatory of Japan
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
#ifndef _CASA_SINGLEDISH_MS_H_
#define _CASA_SINGLEDISH_MS_H_

#include <iostream>
#include <string>

#include <casa/aipstype.h>
#include <casa/Containers/Record.h>
#include <libsakura/sakura.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <scimath/Mathematics/FFTServer.h>
#include <singledish/SingleDish/SDMSManager.h>

#define SinusoidWaveNumber_kUpperLimit    -999

namespace casa { //# NAMESPACE CASA - BEGIN

class SingleDishMS {
public:
  // Default constructor
  SingleDishMS();
  // Construct from MS name string
  SingleDishMS(string const& ms_name);

  // Destructor
  ~SingleDishMS();

  /* 
   * Return the name of the MeasurementSet
   */
  string name() const {
    return msname_;
  }

  bool close();

  /*
   * Formats selection parameters for single dish processing.
   * @param [in] selection A Record consists of selection key and values
   * @param [in] verbose If true, print summary of selection logger
   */
  void setSelection(Record const& selection, bool const verbose = true);

  /*
   * Set time averaging parameters.
   */
  void setAverage(Record const& average, bool const verbose = true);

  // Multiply a scale factor to selected spectra
  void scale(float const factor, string const& in_column_name,
      string const& out_ms_name);

  // Invoke baseline subtraction
  // (polynomial, write results in new MS)
  void subtractBaseline(string const& in_column_name,
			string const& out_ms_name,
			string const& out_bloutput_name,
			bool const& do_subtract,
			string const& in_spw,
			string const& blfunc,
			int const order,
			float const clip_threshold_sigma,
			int const num_fitting_max,
			bool const linefinding,
			float const threshold,
			int const avg_limit,
			int const minwidth,
			vector<int> const& edge);

  //Cubicspline  
  void subtractBaselineCspline(string const& in_column_name,
			       string const& out_ms_name, 
			       string const& out_bloutput_name,
			       bool const& do_subtract,
			       string const& in_spw,
			       int const npiece,
			       float const clip_threshold_sigma,
			       int const num_fitting_max,
			       bool const linefinding,
			       float const threshold,
			       int const avg_limit,
			       int const minwidth,
			       vector<int> const& edge);

  //Sinusoid  
   void subtractBaselineSinusoid(string const& in_column_name,
				 string const& out_ms_name,
				 string const& out_bloutput_name,
				 bool const& do_subtract,
				 string const& in_spw,
				 string const& addwn0,
				 string const& rejwn0,
				 bool const applyfft,
				 string const fftmethod,
				 string const fftthresh,
				 float const clip_threshold_sigma,
				 int const num_fitting_max,
				 bool const linefinding,
				 float const threshold,
				 int const avg_limit,
				 int const minwidth,
				 vector<int> const& edge);

  // variable fitting parameters stored in a text file
  void subtractBaselineVariable(string const& in_column_name,
				string const& out_ms_name,
				string const& out_bloutput_name,
				bool const& do_subtract,
				string const& in_spw,
				string const& param_file);

  // apply baseline table
  void applyBaselineTable(string const& in_column_name,
      string const& in_bltable_name, string const& in_spw,
      string const& out_ms_name);

  // fit line profile
  void fitLine(string const& in_column_name, string const& in_spw,
      string const& in_pol, string const& fitfunc, string const& in_nfit,
      bool const linefinding, float const threshold, int const avg_limit,
      int const minwidth, vector<int> const& edge,
      string const& tempfile_name, string const& temp_out_ms_name);

  // smooth data with arbitrary smoothing kernel
  // smoothing kernels currently supported include gaussian and boxcar
  void smooth(string const &kernelType, float const kernelWidth,
      string const &columnName, string const&outMsName);

private:
  /////////////////////////
  /// Utility functions ///
  /////////////////////////
  /*
   *  Initializes member variables: in_column_
   */
  void initialize();
  /*
   * Formats selection parameters for single dish processing.
   * @param [in] selection A Record consists of selection key and values
   */
  void format_selection(Record &selection);

  // retrieve a field by name from Record as casa::String.
  String get_field_as_casa_string(Record const &in_data,
      string const &field_name);

  bool prepare_for_process(string const &in_column_name,
      string const&out_ms_name);

  bool prepare_for_process(string const &in_column_name,
      string const&out_ms_name, Block<Int> const &sortColumns,
      bool const addDefaultSortCols = false);
  void finalize_process();

  // check column 'in' is in input MS and set to 'out' if it exists.
  // if not, out is set to MS::UNDEFINED_COLUMN
  bool set_column(MSMainEnums::PredefinedColumns const &in,
      MSMainEnums::PredefinedColumns &out);
  // Convert a Complex Array to Float Array
  void convertArrayC2F(Array<Float> &from, Array<Complex> const &to);
  // Split a string with given delimiter
  std::vector<string> split_string(string const &s, char delim);
  // examine if a file with specified name exists
  bool file_exists(string const &filename);
  /* Convert spw selection string to vectors of spwid and channel
     ranges by parsing msseltoindex output. Also creates some
     placeholder vectors to store mask and the muber of channels
     in each selected SPW.
     [in] in_spw: input SPW selection string
     [out] spw : a vector of selected SPW IDs. the number of
                 elements is the number of selected SPWs
     [out] chan : a vector of selected SPW IDs and channel ranges
                  returned by msseltoindex. [[SPWID, start, end, stride], ...]
     [out] nchan : a vector of length spw.size() initialized by zero
     [out] mask : an uninitialized vector of length spw.size() 
     [out] nchan_set: a vector of length spw.size() initilazed by false
   */
  void parse_spw(string const &in_spw, Vector<Int> &spw, Matrix<Int> &chan,
      Vector<size_t> &nchan, Vector<Vector<Bool> > &mask,
      Vector<bool> &nchan_set);
  /* Go through chunk and set valudes to nchan and mask selection
     vectors of the SPW if not already done.
    [in] rec_spw: a vector of selected SPW IDs. the number of
                 elements is the number of selected SPWs
    [in] data_spw: a vector of SPW IDs in current chunk. the
                   number of elements is equals to the number
                   of rows in the chunk.
    [in] rec_chan: a vector of selected SPW IDs and channel ranges
                  returned by msseltoindex. [[SPWID, start, end, stride], ...]
    [in] num_chan: a vector of the number of channels in current chunk.
                   the number of elements is equals to the number
                   of rows in the chunk.
    [out] nchan: a vector of length spw.size(). the number of channels 
                 in the corresponding SPW is set when the loop traverses
		 the SPW for the first time.
    [out] mask: a vector of length spw.size().
    [in,out] nchan_set: a boolean vector of length spw.size().
                        the value indicates if nchan, and mask of
                        the corresponding SPW is already set.
    [out] new_nchan: returns true if this is the first time finding
                     SPWs with the same number of channels.
   */
  void get_nchan_and_mask(Vector<Int> const &rec_spw,
      Vector<Int> const &data_spw, Matrix<Int> const &rec_chan,
      size_t const num_chan, Vector<size_t> &nchan, Vector<Vector<Bool> > &mask,
      Vector<bool> &nchan_set, bool &new_nchan);
  void get_mask_from_rec(Int spwid, Matrix<Int> const &rec_chan,
      Vector<Bool> &mask, bool initialize = true);
  void get_masklist_from_mask(size_t const num_chan, bool const *mask,
      Vector<uInt> &masklist);
  // Create a set of baseline contexts (if necessary)
  void get_baseline_context(size_t const bltype,
      uint16_t order,
      size_t num_chan,
      Vector<size_t> const &nchan,
      Vector<bool> const &nchan_set,
      Vector<size_t> &ctx_indices,
      std::vector<LIBSAKURA_SYMBOL(LSQFitContextFloat) *> &bl_contexts);
  void get_baseline_context(size_t const bltype,
      uint16_t order,
      size_t num_chan,
      size_t ispw,
      Vector<size_t> &ctx_indices,
      std::vector<size_t> &ctx_nchans,
      std::vector<LIBSAKURA_SYMBOL(LSQFitContextFloat) *> &bl_contexts);
  // Destroy a set of baseline contexts
  void destroy_baseline_contexts(std::vector<LIBSAKURA_SYMBOL(LSQFitContextFloat) *> &bl_contexts);
  void check_sakura_status(string const &name, LIBSAKURA_SYMBOL(Status) const status);
  void check_baseline_status(LIBSAKURA_SYMBOL(LSQFitStatus) const bl_status);
  template<typename T, typename U>
  void set_matrix_for_bltable(size_t const num_pol, size_t const num_data_max,
      std::vector<std::vector<T> > const &in_data, Array<U> &out_data) {
    for (size_t ipol = 0; ipol < num_pol; ++ipol) {
      for (size_t i = 0; i < num_data_max; ++i) {
        out_data[i][ipol] = static_cast<U>(0);
      }
      size_t num_data = in_data[ipol].size();
      for (size_t i = 0; i < num_data; ++i) {
        out_data[i][ipol] = static_cast<U>(in_data[ipol][i]);
      }
    }
  }
  template<typename T, typename U>
  void set_array_for_bltable(size_t const ipol, size_t const num_data,
      T const *in_data, Array<U> &out_data) {
    for (size_t i = 0; i < num_data; ++i) {
      out_data[i][ipol] = static_cast<U>(in_data[i]);
    }
  }
  size_t get_num_coeff_bloutput(size_t const bltype,
				size_t order,
                                size_t &num_coeff_max);
  vector<int> string_to_list(string const &wn_str, char const delim);
  void get_effective_nwave(std::vector<int> const &addwn,
			   std::vector<int> const &rejwn,
			   int const wn_ulimit,
			   std::vector<int> &effwn);
  void finalise_effective_nwave(std::vector<int> const &blparam_eff_base,
				std::vector<int> const &blparam_exclude,
				int const &blparam_upperlimit,
				size_t const &num_chan,
				float const *spec, bool const *mask,
				bool const &applyfft,
				string const &fftmethod, string const &fftthresh,
				std::vector<size_t> &blparam_eff);
  void parse_fftthresh(string const& fftthresh_str,
		       string& fftthresh_attr,
		       float& fftthresh_sigma,
		       int& fftthresh_top);
  void select_wavenumbers_via_fft(size_t const num_chan,
				  float const *spec,
				  bool const *mask,
				  string const &fftmethod,
				  string const &fftthresh_attr,
				  float const fftthresh_sigma,
				  int const fftthresh_top,
				  int const blparam_upperlimit,
				  std::vector<int> &blparam_fft);
  void exec_fft(size_t const num_chan,
		float const *in_spec,
		bool const *in_mask,
		bool const get_real_imag,
		bool const get_ampl_only,
		std::vector<float> &fourier_spec);
  void interpolate_constant(int const num_chan,
			    float const *in_spec,
			    bool const *in_mask,
			    Vector<Float> &spec);
  void merge_wavenumbers(std::vector<int> const &blparam_eff_base,
			 std::vector<int> const &blparam_fft,
			 std::vector<int> const &blparam_exclude,
			 std::vector<size_t> &blparam_eff);
  
  list<pair<size_t, size_t>> findLineAndGetRanges(size_t const num_data,
      float const data[/*num_data*/],
      bool mask[/*num_data*/], float const threshold,
      int const avg_limit, int const minwidth, vector<int> const& edge,
      bool const invert);

  void findLineAndGetMask(size_t const num_data, float const data[/*num_data*/],
      bool const in_mask[/*num_data*/], float const threshold,
      int const avg_limit, int const minwidth, vector<int> const& edge,
      bool const invert, bool out_mask[/*num_data*/]);

  template<typename Func0, typename Func1, typename Func2, typename Func3>
  void doSubtractBaseline(string const& in_column_name,
			  string const& out_ms_name, 
                          string const& out_bloutput_name,
			  bool const& do_subtract,
			  string const& in_spw,
			  LIBSAKURA_SYMBOL(Status)& status,
			  std::vector<LIBSAKURA_SYMBOL(LSQFitContextFloat) *> &bl_contexts,
			  size_t const bltype,
			  vector<int> const& blparam,
			  vector<int> const& blparam_exclude,
			  bool const& applyfft,
			  string const& fftmethod,
			  string const& fftthresh,
			  float const clip_threshold_sigma,
			  int const num_fitting_max,
			  bool const linefinding,
			  float const threshold,
			  int const avg_limit,
			  int const minwidth,
			  vector<int> const& edge,
			  Func0 func0,
			  Func1 func1,
			  Func2 func2,
			  Func3 func3,
			  LogIO os);

  /////////////////////////////
  /// MS handling functions ///
  /////////////////////////////
  // retrieve a spectrum at the row and plane (polarization) from data cube
  void get_spectrum_from_cube(Cube<Float> &data_cube, size_t const row,
      size_t const plane, size_t const num_data,
      float out_data[/*num_data*/]);
  // set a spectrum at the row and plane (polarization) to data cube
  void set_spectrum_to_cube(Cube<Float> &data_cube, size_t const row,
      size_t const plane, size_t const num_data, float in_data[/*num_data*/]);
  // get data cube (npol*nchan*nvirow) in in_column_ from visbuffer
  // and convert it to float cube
  void get_data_cube_float(vi::VisBuffer2 const &vb, Cube<Float> &data_cube);
  // get flag cube (npol*nchan*nvirow) from visbuffer
  void get_flag_cube(vi::VisBuffer2 const &vb, Cube<Bool> &flag_cube);
  // retrieve a flag at the row and plane (polarization) from flag cube
  void get_flag_from_cube(Cube<Bool> &flag_cube, size_t const row,
      size_t const plane, size_t const num_flag,
      bool out_flag[/*num_flag*/]);
  // set a flag at the row and plane (polarization) to flag cube
  void set_flag_to_cube(Cube<Bool> &flag_cube, size_t const row,
      size_t const plane, size_t const num_flag, bool in_flag[/*num_data*/]);

  // flag all channels in a supectrum in cube at the row and plane (polarization)
  void flag_spectrum_in_cube(Cube<Bool> &flag_cube, size_t const row,
      size_t const plane);

  // return true if all channels are flagged
  bool allchannels_flagged(size_t const num_flag, bool const* flag);
  // returns the number of channels with true in input mask
  size_t NValidMask(size_t const num_mask, bool const* mask);

  /////////////////////////////////
  /// Array execution functions ///
  /////////////////////////////////
  // multiply a scaling factor to a float array
  void do_scale(float const factor, size_t const num_data,
      float data[/*num_data*/]);

  ////////////////////////
  /// Member vairables ///
  ////////////////////////
  // the name of input MS
  string msname_;
  // columns to read and save data
  MSMainEnums::PredefinedColumns in_column_;  //, out_column_;
  // Record of selection
  Record selection_;
  // Record of average
  Record average_;
  // SDMSManager
  SDMSManager *sdh_;
  // pointer to accessor function
  void (*visCubeAccessor_)(vi::VisBuffer2 const &vb, Cube<Float> &cube);
  // smoothing flag
  Bool doSmoothing_;

  string bloutputname_csv;
  string bloutputname_text;
  string bloutputname_table;

  //split the name  
  void split_bloutputname(string str);

  //max number of rows to get in each iteration
  constexpr static Int kNRowBlocking = 1000;

public:
  static bool importAsap(string const &infile, string const &outfile, bool const parallel=false);
  static bool importNRO(string const &infile, string const &outfile, bool const parallel=false);
};
// class SingleDishMS -END

} //# NAMESPACE CASA - END

#endif /* _CASA_SINGLEDISH_MS_H_ */
