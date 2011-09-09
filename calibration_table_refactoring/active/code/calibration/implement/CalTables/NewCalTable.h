//# NewCalTable.h: Calibration table access and creation
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef CALIBRATION_NEWCALTABLE_H
#define CALIBRATION_NEWCALTABLE_H

#include <casa/aips.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ExprNode.h>
#include <casa/Containers/Record.h>
#include <casa/OS/Path.h>
#include <casa/Utilities/Sort.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSHistory.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSHistoryColumns.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSHistoryColumns.h>
#include <calibration/CalTables/NewCalTableDesc.h>
#include <calibration/CalTables/NewCalMainRecord.h>
#include <calibration/CalTables/NewCalMainColumns.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// NewCalTable: New Calibration table access and creation
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalTableDesc">CalTableDesc</linkto> module
// </prerequisite>
//
// <etymology>
// From "calibration" and "table".
// </etymology>
//
// <synopsis>
// The NewCalTable classes provide basic access to calibration tables. This 
// includes the creation of calibration tables, sorting and selection
// capabilities, and basic data access. Specializations for baseline-based,
// time-variable and solvable VisJones types, and sub-types, are provided 
// through inheritance.
// </etymology>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// This class is used by other calibration table accessors and iterators.
// </motivation>
//
// <todo asof="11/06/10">
// </todo>

class NewCalTable : public Table
{
 public:
   // Default null constructor, and destructor
   NewCalTable();
   virtual ~NewCalTable();

   // Construct from a specified table name, calibration table descriptor 
   // and table access option. Used for creating new tables.
   NewCalTable (const String& tableName, NewCalTableDesc& ctableDesc,
	     Table::TableOption access = Table::New, Table::TableType ttype = Table::Plain);

   // Construct from setupNewTable 
   NewCalTable (SetupNewTable& newTab, uInt nrow = 0, Bool initialize = False);
   
   // Construct from a specified table name, and access option. Used
   // for accessing existing tables.
   NewCalTable (const String& tableName, Table::TableOption access = Table::Old, 
             Table::TableType ttype = Table::Memory);

   // Construct from an existing table object
   NewCalTable (const Table& table);

   // Copy constructor
   NewCalTable (const NewCalTable& other);

   // Assignment operator
   NewCalTable& operator= (const NewCalTable& other);

   // initialize reference to subtables
   void initSubtables();
   void clearSubtables();

   void copyMemCalSubtables(const NewCalTable& other);
   void copyMemCalSubtable(const Table& otherSubtable, Table& subTable);
   // get number of rows in cal main table, and subtables  
   //Int nRowMain() const;
   //Int nRowHistory() const;

   // Add rows to cal_main, or cal_history
   //void addRowMain (uInt nrrow = 1, Bool initialize = False)
   //         {this->addRow(nrrow, initialize);};
   //void addRowHistory (uInt nrrow = 1, Bool initialize = False)
   //                         {itsHistoryTable->addRow(nrrow, initialize);};

   // Get a row from cal_main
   Record getRowMain (const Int& jrow);
    
   // Put a row to cal_main, cal_desc or cal_history
   void putRowMain (const Int& jrow, NewCalMainRecord& tableRec);

   // Get rows of the data from the main table
   void fillRowsMain ();

   // Set Meta data info from a parent MS 
   void setMetaInfo (const String& msName);

   //validate input table desc. over required caltable desc
   static Bool conformant(const TableDesc& tabDesc);

   // save to disk
   void writeToDisk(const String& tableName); 

   typedef MSField CalField;
   typedef MSAntenna CalAntenna;
   typedef MSSpectralWindow CalSpectralWindow;
   typedef MSHistory CalHistory;
   typedef MSFieldColumns CalFieldColumns;
   typedef MSAntennaColumns CalAntennaColumns;
   typedef MSSpWindowColumns CalSpWindowColumns;
   typedef MSHistoryColumns CalHistoryColumns;
   typedef ROMSFieldColumns ROCalFieldColumns;
   typedef ROMSAntennaColumns ROCalAntennaColumns;
   typedef ROMSSpWindowColumns ROCalSpWindowColumns;
   typedef ROMSHistoryColumns ROCalHistoryColumns;
   
   CalAntenna& antenna() {return antenna_p;}
   CalField& field() {return field_p;}
   CalSpectralWindow& spectralWindow() {return spectralWindow_p;}
   CalHistory& history() {return history_p;}
   const CalAntenna& antenna() const {return antenna_p;}
   const CalField& field() const {return field_p;}
   const CalSpectralWindow& spectralWindow() const {return spectralWindow_p;}
   const CalHistory& history() const {return history_p;}

/***
 protected:
   friend class NewCalColumns;
   friend class RONewCalColumns;
   
***/
 
 private:
   //cal_main Table object
   //Table caltable_;
   Table::TableType ttype_p;
   CalAntenna antenna_p;
   CalField field_p;
   CalSpectralWindow spectralWindow_p;
   CalHistory history_p;

 };


} //# NAMESPACE CASA - END

#endif
