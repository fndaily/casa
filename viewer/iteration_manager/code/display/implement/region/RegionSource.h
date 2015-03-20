//# regionsource.h: regionsource producing persistent regions used within the casa viewer
//# Copyright (C) 2011
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


#ifndef REGION_REGIONSOURCE_H_
#define REGION_REGIONSOURCE_H_
#include <tr1/memory>
#include <display/region/RegionCreator.h>

namespace casa {

    class WorldCanvas;

    namespace viewer {

	class Rectangle;
	class Polygon;
	class Ellipse;

	class RegionSource {
	    public:
		virtual std::tr1::shared_ptr<Rectangle> rectangle( WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) = 0;
		virtual std::tr1::shared_ptr<Polygon> polygon( WorldCanvas *wc, double x1, double y1 ) = 0;
		virtual std::tr1::shared_ptr<Polygon> polygon( WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts ) = 0;
		virtual std::tr1::shared_ptr<Rectangle> ellipse( WorldCanvas *wc, double blc_x, double blc_y, double trc_x, double trc_y ) = 0;
		virtual std::tr1::shared_ptr<Rectangle> point( WorldCanvas *wc, double x, double y ) = 0;

		virtual void revokeRegion( Region *r ) { region_creator->revokeRegion(r); }

		RegionSource( RegionCreator *rc ) : region_creator(rc) { }
		virtual ~RegionSource( );

	    private:
		RegionCreator *region_creator;
	};
    }
}

#endif