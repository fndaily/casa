//# ImagerControl.h: connect to synthesis imager task for control
//# Copyright (C) 2013
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
//# $Id$

#ifndef CASADBUS_IMAGERCONTROL_H_
#define CASADBUS_IMAGERCONTROL_H_

/*******************************************************************************************
********************************************************************************************
****  Note: the DBus-C++ header files should be include *BEFORE* the                    ****
****        Qt header files. Otherwise, the Qt header files (which use                  ****
****        the c-preprocessor to define "signals" to be "protected"                    ****
****        cause the following errors for DBus-C++ (and anything else                  ****
****        that has a variable etc. named "signals":                                   ****
****        ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----  ****
**** .../dbus-c++/introspection.h:62: error: expected unqualified-id before ?protected? ****
**** .../dbus-c++/introspection.h:62: error: abstract declarator ?const DBus::Intros... ****
**** .../dbus-c++/introspection.h:62: error: expected ?;? before ?protected?            ****
********************************************************************************************
*******************************************************************************************/
#include <casadbus/types/variant.h>
#ifdef INTERACTIVE_ITERATION
#include <casadbus/interfaces/SynthImager.proxy.h>
#endif
#include <casadbus/utilities/Conversion.h>

namespace casa {

	class ImagerControl
#ifdef INTERACTIVE_ITERATION
							: private edu::nrao::casa::SynthImager_proxy,
							public DBus::IntrospectableProxy,
							public DBus::ObjectProxy
#endif
							{
		public:
			static std::string dbusName( ) { return ""; }

			ImagerControl( const std::string& serviceName=dbusName( ) );
			~ImagerControl( );

		    bool incrementController( ) {
#ifdef INTERACTIVE_ITERATION
					return edu::nrao::casa::SynthImager_proxy::incrementController( );
#else
					return false;
#endif
			}
		    bool decrementController( ) {
#ifdef INTERACTIVE_ITERATION
					return edu::nrao::casa::SynthImager_proxy::decrementController( );
#else
					return false;
#endif
			}

		    void changePauseFlag( const bool &state ) {
#ifdef INTERACTIVE_ITERATION
					edu::nrao::casa::SynthImager_proxy::changePauseFlag( state );
#endif
			}

		    void changeStopFlag(const bool& state) {
#ifdef INTERACTIVE_ITERATION
					edu::nrao::casa::SynthImager_proxy::changeStopFlag( state );
#endif
			}

			// slots that are required for signals generated by SynthImager_adaptor...
			void interactionRequired( const bool &/*required*/ ) { }
			void detailUpdate( const std::map<std::string, DBus::Variant>& /*updatedParams*/ ) { }
			void summaryUpdate( const DBus::Variant& /*summary*/ ) { }
			void disconnect( ) { }

			std::string getDescription( ) {
#ifdef INTERACTIVE_ITERATION
					return edu::nrao::casa::SynthImager_proxy::getDescription( );
#else
					return std::string( );
#endif
			}
			std::map<std::string,dbus::variant> getDetails( )
				{
#ifdef INTERACTIVE_ITERATION
					return dbus::toStdMap( edu::nrao::casa::SynthImager_proxy::getDetails( ) );
#else
					return std::map<std::string,dbus::variant>( );
#endif
				}
			/******************************************************************************
			**********methods*provided*by*SynthImager_proxy********************************
			*******************************************************************************
		    bool incrementController()
		    bool decrementController()
		    void controlUpdate(const std::map< std::string, ::DBus::Variant >& newParams)
		    void interactionComplete()
		    void changeInteractiveMode(const bool& interactiveMode)
		    std::map< std::string, ::DBus::Variant > getDetails()
		    ::DBus::Variant getSummary()
			std::string getDescription( )
			*******************************************************************************
			*******************************************************************************
			******************************************************************************/

#if 0
			/***** boilerplate from tSIIterBot.cc *****/
			void sendInteractionComplete() {
				checkDetails = true;

				casa::Record record;
				record.define( casa::RecordFieldId("niter"), 123);
				record.define( casa::RecordFieldId("cycleniter"), 456);
				record.define( casa::RecordFieldId("interactiveniter"), 789);
				
				record.define( casa::RecordFieldId("threshold"), 5.67);
				record.define( casa::RecordFieldId("cyclethreshold"), 7.89);
				record.define( casa::RecordFieldId("interactivethreshold"), 8.91);

				record.define( casa::RecordFieldId("cyclefactor"), 4.56);
				record.define( casa::RecordFieldId("loopgain"), 6.78);

				std::map<std::string, DBus::Variant> map= fromRecord(record);
				controlUpdate(map);
				interactionComplete();
			}


			int serviceLoop() {
				incrementController();
				while (!doneFlag) {
					if (interactiveIRQ) {
						interactiveIRQ = false;
						if (serviceInteractiveFlag) {
							sendInteractionComplete();
						}
						if (exitOnInteractiveFlag) {
							doneFlag = true;
						}
					}
					usleep(10000);
				}
				return exitCondition;
			}

		protected:
			/* Control Flags */
			const bool serviceInteractiveFlag;
			const bool exitOnInteractiveFlag;
			const bool checkDetailsFlag;

			/* State Flags */
			bool interactiveIRQ;

			bool checkDetails;
			bool doneFlag;
			int  exitCondition;
#endif

	};
}

#endif
