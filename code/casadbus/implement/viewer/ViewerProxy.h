//# ViewerProxy.h: allows control of the viewer from C++ via DBus
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
//# $Id$

#ifndef DBUS_VIEWERPROXY_H_
#define DBUS_VIEWERPROXY_H_

#include <vector>
#include <string>
#include <casadbus/viewer/ViewerProxy.dbusproxy.h>
#include <casadbus/utilities/Conversion.h>
#include <casa/Containers/Record.h>
#include <xmlcasa/variant.h>

namespace casa {
    class ViewerProxy :
	private edu::nrao::casa::viewer_proxy,
	public DBus::IntrospectableProxy,
	public DBus::ObjectProxy {

    public:

	static const char **execArgs( );

	ViewerProxy( const std::string &name );
	ViewerProxy( const char *path, const char *name );
	ViewerProxy( const std::string &path, const std::string &name );

	dbus::variant start_interact( const dbus::variant &input, int panel )
			{ return dbus::toVariant( edu::nrao::casa::viewer_proxy::start_interact(dbus::fromVariant(input), panel) ); }
	dbus::variant load( const std::string &path, const std::string &displaytype = "raster", int panel=0 )
			{ return dbus::toVariant( edu::nrao::casa::viewer_proxy::load( path, displaytype, panel ) ); }
	dbus::variant reload( int panel_or_data )
			{ return dbus::toVariant( edu::nrao::casa::viewer_proxy::reload(panel_or_data) ); }
	dbus::variant unload( int data )
			{ return dbus::toVariant( edu::nrao::casa::viewer_proxy::unload(data) ); }

	dbus::variant restore( const std::string &path, bool new_window = true )
			{ return dbus::toVariant( edu::nrao::casa::viewer_proxy::restore(path,new_window) ); }

	dbus::variant hide( int panel )
			{ return dbus::toVariant( edu::nrao::casa::viewer_proxy::hide(panel) ); }
	dbus::variant show( int panel )
			{ return dbus::toVariant( edu::nrao::casa::viewer_proxy::show(panel) ); }
	dbus::variant close( int panel )
			{ return dbus::toVariant( edu::nrao::casa::viewer_proxy::close(panel) ); }

	std::string cwd( const std::string &new_path = "" )
			{ return edu::nrao::casa::viewer_proxy::cwd( new_path ); }
	dbus::variant panel( const std::string &type="viewer" )
			{ return dbus::toVariant( edu::nrao::casa::viewer_proxy::panel(type) ); }

	// device:	file name or printer name
	// devicetype:	"file", "printer", or "ghostscript"
	// format:	"jpg", "pdf", "eps", "ps", "png", "xbm", "xpm", "ppm"
	//		[only used with devicetype == "file"] [extension on device, e.g. "outfile.pdf" overrides "format"]
	// scale:	size scale factor for raster output (e.g. jpg etc.)
	// dpi:		resolution of PS or EPS images
	// orientation: "portrait", "landscape"
	// media:	"letter" or "a4"
	bool output( const std::string &device, const std::string &devicetype = "file", int panel = 0,
		     double scale = 1.0, int dpi = 300, const std::string &format = "jpg",
		     const std::string &orientation = "portrait", const std::string &media = "letter" )
			{ return edu::nrao::casa::viewer_proxy::output( device, devicetype, panel, scale,
									dpi, format, orientation, media ); }

	std::vector<std::string> keyinfo( int key )
			{ return edu::nrao::casa::viewer_proxy::keyinfo(key); }

	bool done( ) 	{ return edu::nrao::casa::viewer_proxy::done( ); }
    };
}
#endif
