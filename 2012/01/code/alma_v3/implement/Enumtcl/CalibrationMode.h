/*
 *
 * /////////////////////////////////////////////////////////////////
 * // WARNING!  DO NOT MODIFY THIS FILE!                          //
 * //  ---------------------------------------------------------  //
 * // | This is generated code using a C++ template function!   | //
 * // ! Do not modify this file.                                | //
 * // | Any changes will be lost when the file is re-generated. | //
 * //  ---------------------------------------------------------  //
 * /////////////////////////////////////////////////////////////////
 *
 */


#if     !defined(_CALIBRATIONMODE_H)

#include <CCalibrationMode.h>
#define _CALIBRATIONMODE_H
#endif 

#if     !defined(_CALIBRATIONMODE_HH)

#include "Enum.hpp"

using namespace CalibrationModeMod;

template<>
 struct enum_set_traits<CalibrationModeMod::CalibrationMode> : public enum_set_traiter<CalibrationModeMod::CalibrationMode,5,CalibrationModeMod::WVR> {};

template<>
class enum_map_traits<CalibrationModeMod::CalibrationMode,void> : public enum_map_traiter<CalibrationModeMod::CalibrationMode,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<CalibrationModeMod::CalibrationMode,EnumPar<void> >
     (CalibrationModeMod::HOLOGRAPHY,ep((int)CalibrationModeMod::HOLOGRAPHY,"HOLOGRAPHY","un-documented")));
    m_.insert(pair<CalibrationModeMod::CalibrationMode,EnumPar<void> >
     (CalibrationModeMod::INTERFEROMETRY,ep((int)CalibrationModeMod::INTERFEROMETRY,"INTERFEROMETRY","un-documented")));
    m_.insert(pair<CalibrationModeMod::CalibrationMode,EnumPar<void> >
     (CalibrationModeMod::OPTICAL,ep((int)CalibrationModeMod::OPTICAL,"OPTICAL","un-documented")));
    m_.insert(pair<CalibrationModeMod::CalibrationMode,EnumPar<void> >
     (CalibrationModeMod::RADIOMETRY,ep((int)CalibrationModeMod::RADIOMETRY,"RADIOMETRY","un-documented")));
    m_.insert(pair<CalibrationModeMod::CalibrationMode,EnumPar<void> >
     (CalibrationModeMod::WVR,ep((int)CalibrationModeMod::WVR,"WVR","un-documented")));
    return true;
  }
  static map<CalibrationModeMod::CalibrationMode,EnumPar<void> > m_;
};
#define _CALIBRATIONMODE_HH
#endif
