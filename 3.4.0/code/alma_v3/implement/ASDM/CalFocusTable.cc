
/*
 * ALMA - Atacama Large Millimeter Array
 * (c) European Southern Observatory, 2002
 * (c) Associated Universities Inc., 2002
 * Copyright by ESO (in the framework of the ALMA collaboration),
 * Copyright by AUI (in the framework of the ALMA collaboration),
 * All rights reserved.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307  USA
 *
 * Warning!
 *  -------------------------------------------------------------------- 
 * | This is generated code!  Do not modify this file.                  |
 * | If you do, all changes will be lost when the file is re-generated. |
 *  --------------------------------------------------------------------
 *
 * File CalFocusTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalFocusTable.h>
#include <CalFocusRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalFocusTable;
using asdm::CalFocusRow;
using asdm::Parser;

#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>
#include <set>
#include <algorithm>
using namespace std;

#include <Misc.h>
using namespace asdm;

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "boost/filesystem/operations.hpp"
#include <boost/algorithm/string.hpp>
using namespace boost;

namespace asdm {

	string CalFocusTable::itsName = "CalFocus";
	vector<string> CalFocusTable::attributesNames; 
	vector<string> CalFocusTable::attributesNamesInBin; 
	bool CalFocusTable::initAttributesNamesDone = CalFocusTable::initAttributesNames();
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalFocusTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalFocusTable::getKeyName() {
		return key;
	}


	CalFocusTable::CalFocusTable(ASDM &c) : container(c) {

	
		key.push_back("antennaName");
	
		key.push_back("receiverBand");
	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalFocusTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
		
		// By default the table is considered as present in memory
		presentInMemory = true;
		
		// By default there is no load in progress
		loadInProgress = false;
	}
	
/**
 * A destructor for CalFocusTable.
 */
	CalFocusTable::~CalFocusTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalFocusTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	unsigned int CalFocusTable::size() const {
		if (presentInMemory) 
			return privateRows.size();
		else
			return declaredSize;
	}
	
	/**
	 * Return the name of this table.
	 */
	string CalFocusTable::getName() const {
		return itsName;
	}
	
	/**
	 * Return the name of this table.
	 */
	string CalFocusTable::name() {
		return itsName;
	}
	
	/**
	 * Build the vector of attributes names.
	 */
	bool CalFocusTable::initAttributesNames() {

		attributesNames.push_back("antennaName");

		attributesNames.push_back("receiverBand");

		attributesNames.push_back("calDataId");

		attributesNames.push_back("calReductionId");


		attributesNames.push_back("startValidTime");

		attributesNames.push_back("endValidTime");

		attributesNames.push_back("ambientTemperature");

		attributesNames.push_back("atmPhaseCorrection");

		attributesNames.push_back("focusMethod");

		attributesNames.push_back("frequencyRange");

		attributesNames.push_back("pointingDirection");

		attributesNames.push_back("numReceptor");

		attributesNames.push_back("polarizationTypes");

		attributesNames.push_back("wereFixed");

		attributesNames.push_back("offset");

		attributesNames.push_back("offsetError");

		attributesNames.push_back("offsetWasTied");

		attributesNames.push_back("reducedChiSquared");

		attributesNames.push_back("position");


		attributesNames.push_back("polarizationsAveraged");

		attributesNames.push_back("focusCurveWidth");

		attributesNames.push_back("focusCurveWidthError");

		attributesNames.push_back("focusCurveWasFixed");

		attributesNames.push_back("offIntensity");

		attributesNames.push_back("offIntensityError");

		attributesNames.push_back("offIntensityWasFixed");

		attributesNames.push_back("peakIntensity");

		attributesNames.push_back("peakIntensityError");

		attributesNames.push_back("peakIntensityWasFixed");


    
    	 
    	attributesNamesInBin.push_back("antennaName") ; 
    	 
    	attributesNamesInBin.push_back("receiverBand") ; 
    	 
    	attributesNamesInBin.push_back("calDataId") ; 
    	 
    	attributesNamesInBin.push_back("calReductionId") ; 
    	 
    	attributesNamesInBin.push_back("startValidTime") ; 
    	 
    	attributesNamesInBin.push_back("endValidTime") ; 
    	 
    	attributesNamesInBin.push_back("ambientTemperature") ; 
    	 
    	attributesNamesInBin.push_back("atmPhaseCorrection") ; 
    	 
    	attributesNamesInBin.push_back("focusMethod") ; 
    	 
    	attributesNamesInBin.push_back("frequencyRange") ; 
    	 
    	attributesNamesInBin.push_back("pointingDirection") ; 
    	 
    	attributesNamesInBin.push_back("numReceptor") ; 
    	 
    	attributesNamesInBin.push_back("polarizationTypes") ; 
    	 
    	attributesNamesInBin.push_back("wereFixed") ; 
    	 
    	attributesNamesInBin.push_back("offset") ; 
    	 
    	attributesNamesInBin.push_back("offsetError") ; 
    	 
    	attributesNamesInBin.push_back("offsetWasTied") ; 
    	 
    	attributesNamesInBin.push_back("reducedChiSquared") ; 
    	 
    	attributesNamesInBin.push_back("position") ; 
    	
    	 
    	attributesNamesInBin.push_back("polarizationsAveraged") ; 
    	 
    	attributesNamesInBin.push_back("focusCurveWidth") ; 
    	 
    	attributesNamesInBin.push_back("focusCurveWidthError") ; 
    	 
    	attributesNamesInBin.push_back("focusCurveWasFixed") ; 
    	 
    	attributesNamesInBin.push_back("offIntensity") ; 
    	 
    	attributesNamesInBin.push_back("offIntensityError") ; 
    	 
    	attributesNamesInBin.push_back("offIntensityWasFixed") ; 
    	 
    	attributesNamesInBin.push_back("peakIntensity") ; 
    	 
    	attributesNamesInBin.push_back("peakIntensityError") ; 
    	 
    	attributesNamesInBin.push_back("peakIntensityWasFixed") ; 
    	
    
    	return true; 
	}
	

	const vector<string>& CalFocusTable::getAttributesNames() { return attributesNames; }
	
	const vector<string>& CalFocusTable::defaultAttributesNamesInBin() { return attributesNamesInBin; }

	/**
	 * Return this table's Entity.
	 */
	Entity CalFocusTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalFocusTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalFocusRow *CalFocusTable::newRow() {
		return new CalFocusRow (*this);
	}
	

	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param antennaName 
	
 	 * @param receiverBand 
	
 	 * @param calDataId 
	
 	 * @param calReductionId 
	
 	 * @param startValidTime 
	
 	 * @param endValidTime 
	
 	 * @param ambientTemperature 
	
 	 * @param atmPhaseCorrection 
	
 	 * @param focusMethod 
	
 	 * @param frequencyRange 
	
 	 * @param pointingDirection 
	
 	 * @param numReceptor 
	
 	 * @param polarizationTypes 
	
 	 * @param wereFixed 
	
 	 * @param offset 
	
 	 * @param offsetError 
	
 	 * @param offsetWasTied 
	
 	 * @param reducedChiSquared 
	
 	 * @param position 
	
     */
	CalFocusRow* CalFocusTable::newRow(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, Temperature ambientTemperature, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, FocusMethodMod::FocusMethod focusMethod, vector<Frequency > frequencyRange, vector<Angle > pointingDirection, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<bool > wereFixed, vector<vector<Length > > offset, vector<vector<Length > > offsetError, vector<vector<bool > > offsetWasTied, vector<vector<double > > reducedChiSquared, vector<vector<Length > > position){
		CalFocusRow *row = new CalFocusRow(*this);
			
		row->setAntennaName(antennaName);
			
		row->setReceiverBand(receiverBand);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setAmbientTemperature(ambientTemperature);
			
		row->setAtmPhaseCorrection(atmPhaseCorrection);
			
		row->setFocusMethod(focusMethod);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setPointingDirection(pointingDirection);
			
		row->setNumReceptor(numReceptor);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setWereFixed(wereFixed);
			
		row->setOffset(offset);
			
		row->setOffsetError(offsetError);
			
		row->setOffsetWasTied(offsetWasTied);
			
		row->setReducedChiSquared(reducedChiSquared);
			
		row->setPosition(position);
	
		return row;		
	}	
	


CalFocusRow* CalFocusTable::newRow(CalFocusRow* row) {
	return new CalFocusRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	 
	/**
	 * Add a row.
	 * @throws DuplicateKey Thrown if the new row has a key that is already in the table.
	 * @param x A pointer to the row to be added.
	 * @return x
	 */
	CalFocusRow* CalFocusTable::add(CalFocusRow* x) {
		
		if (getRowByKey(
						x->getAntennaName()
						,
						x->getReceiverBand()
						,
						x->getCalDataId()
						,
						x->getCalReductionId()
						))
			//throw DuplicateKey(x.getAntennaName() + "|" + x.getReceiverBand() + "|" + x.getCalDataId() + "|" + x.getCalReductionId(),"CalFocus");
			throw DuplicateKey("Duplicate key exception in ","CalFocusTable");
		
		row.push_back(x);
		privateRows.push_back(x);
		x->isAdded(true);
		return x;
	}

	
		
	void CalFocusTable::addWithoutCheckingUnique(CalFocusRow * x) {
		if (getRowByKey(
						x->getAntennaName()
						,
						x->getReceiverBand()
						,
						x->getCalDataId()
						,
						x->getCalReductionId()
						) != (CalFocusRow *) 0) 
			throw DuplicateKey("Dupicate key exception in ", "CalFocusTable");
		row.push_back(x);
		privateRows.push_back(x);
		x->isAdded(true);
	}




	// 
	// A private method to append a row to its table, used by input conversion
	// methods, with row uniqueness.
	//

	
	/**
	 * If this table has an autoincrementable attribute then check if *x verifies the rule of uniqueness and throw exception if not.
	 * Check if *x verifies the key uniqueness rule and throw an exception if not.
	 * Append x to its table.
	 * @param x a pointer on the row to be appended.
	 * @returns a pointer on x.
	 * @throws DuplicateKey
	 
	 */
	CalFocusRow*  CalFocusTable::checkAndAdd(CalFocusRow* x)  {
		
		
		if (getRowByKey(
	
			x->getAntennaName()
	,
			x->getReceiverBand()
	,
			x->getCalDataId()
	,
			x->getCalReductionId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalFocusTable");
		
		row.push_back(x);
		privateRows.push_back(x);
		x->isAdded(true);
		return x;	
	}	



	//
	// A private method to brutally append a row to its table, without checking for row uniqueness.
	//

	void CalFocusTable::append(CalFocusRow *x) {
		privateRows.push_back(x);
		x->isAdded(true);
	}





	 vector<CalFocusRow *> CalFocusTable::get() {
	 	checkPresenceInMemory();
	    return privateRows;
	 }
	 
	 const vector<CalFocusRow *>& CalFocusTable::get() const {
	 	const_cast<CalFocusTable&>(*this).checkPresenceInMemory();	
	    return privateRows;
	 }	 
	 	




	

	
/*
 ** Returns a CalFocusRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalFocusRow* CalFocusTable::getRowByKey(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId)  {
 	checkPresenceInMemory();
	CalFocusRow* aRow = 0;
	for (unsigned int i = 0; i < privateRows.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->antennaName != antennaName) continue;
			
		
			
				if (aRow->receiverBand != receiverBand) continue;
			
		
			
				if (aRow->calDataId != calDataId) continue;
			
		
			
				if (aRow->calReductionId != calReductionId) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
/**
 * Look up the table for a row whose all attributes 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param antennaName.
 	 		
 * @param receiverBand.
 	 		
 * @param calDataId.
 	 		
 * @param calReductionId.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param ambientTemperature.
 	 		
 * @param atmPhaseCorrection.
 	 		
 * @param focusMethod.
 	 		
 * @param frequencyRange.
 	 		
 * @param pointingDirection.
 	 		
 * @param numReceptor.
 	 		
 * @param polarizationTypes.
 	 		
 * @param wereFixed.
 	 		
 * @param offset.
 	 		
 * @param offsetError.
 	 		
 * @param offsetWasTied.
 	 		
 * @param reducedChiSquared.
 	 		
 * @param position.
 	 		 
 */
CalFocusRow* CalFocusTable::lookup(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, Temperature ambientTemperature, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, FocusMethodMod::FocusMethod focusMethod, vector<Frequency > frequencyRange, vector<Angle > pointingDirection, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<bool > wereFixed, vector<vector<Length > > offset, vector<vector<Length > > offsetError, vector<vector<bool > > offsetWasTied, vector<vector<double > > reducedChiSquared, vector<vector<Length > > position) {
		CalFocusRow* aRow;
		for (unsigned int i = 0; i < privateRows.size(); i++) {
			aRow = privateRows.at(i); 
			if (aRow->compareNoAutoInc(antennaName, receiverBand, calDataId, calReductionId, startValidTime, endValidTime, ambientTemperature, atmPhaseCorrection, focusMethod, frequencyRange, pointingDirection, numReceptor, polarizationTypes, wereFixed, offset, offsetError, offsetWasTied, reducedChiSquared, position)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	



#ifndef WITHOUT_ACS
	using asdmIDL::CalFocusTableIDL;
#endif

#ifndef WITHOUT_ACS
	// Conversion Methods

	CalFocusTableIDL *CalFocusTable::toIDL() {
		CalFocusTableIDL *x = new CalFocusTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalFocusRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalFocusTable::fromIDL(CalFocusTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalFocusRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	
	string CalFocusTable::toXML()  {
		string buf;

		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<CalFocusTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:clfcs=\"http://Alma/XASDM/CalFocusTable\" xsi:schemaLocation=\"http://Alma/XASDM/CalFocusTable http://almaobservatory.org/XML/XASDM/3/CalFocusTable.xsd\" schemaVersion=\"3\" schemaRevision=\"1.61\">\n");
	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalFocusRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalFocusTable> ");
		return buf;
	}

	
	string CalFocusTable::getVersion() const {
		return version;
	}
	

	void CalFocusTable::fromXML(string& tableInXML)  {
		//
		// Look for a version information in the schemaVersion of the XML
		//
		xmlDoc *doc;
		doc = xmlReadMemory(tableInXML.data(), tableInXML.size(), "XMLTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
		if ( doc == NULL )
			throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "CalFocus");
		
		xmlNode* root_element = xmlDocGetRootElement(doc);
   		if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
      		throw ConversionException("Failed to retrieve the root element in the DOM structure.", "CalFocus");
      		
      	xmlChar * propValue = xmlGetProp(root_element, (const xmlChar *) "schemaVersion");
      	if ( propValue != 0 ) {
      		version = string( (const char*) propValue);
      		xmlFree(propValue);   		
      	}
      		     							
		Parser xml(tableInXML);
		if (!xml.isStr("<CalFocusTable")) 
			error();
		// cout << "Parsing a CalFocusTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalFocusTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalFocusRow *row;
		if (getContainer().checkRowUniqueness()) {
			try {
				while (s.length() != 0) {
					row = newRow();
					row->setFromXML(s);
					checkAndAdd(row);
					s = xml.getElementContent("<row>","</row>");
				}
				
			}
			catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalFocusTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalFocusTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalFocusTable::checkAndAdd called from CalFocusTable::fromXML " << endl;
			}
		}
		else {
			try {
				while (s.length() != 0) {
					row = newRow();
					row->setFromXML(s);
					addWithoutCheckingUnique(row);
					s = xml.getElementContent("<row>","</row>");
				}
			}
			catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalFocusTable");
			} 
			catch (...) {
				// cout << "Unexpected error in CalFocusTable::addWithoutCheckingUnique called from CalFocusTable::fromXML " << endl;
			}
		}				
				
				
		if (!xml.isStr("</CalFocusTable>")) 
		error();
			
		archiveAsBin = false;
		fileAsBin = false;
		
	}

	
	void CalFocusTable::error()  {
		throw ConversionException("Invalid xml document","CalFocus");
	}
	
	
	string CalFocusTable::MIMEXMLPart(const asdm::ByteOrder* byteOrder) {
		string UID = getEntity().getEntityId().toString();
		string withoutUID = UID.substr(6);
		string containerUID = getContainer().getEntity().getEntityId().toString();
		ostringstream oss;
		oss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		oss << "\n";
		oss << "<CalFocusTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:clfcs=\"http://Alma/XASDM/CalFocusTable\" xsi:schemaLocation=\"http://Alma/XASDM/CalFocusTable http://almaobservatory.org/XML/XASDM/3/CalFocusTable.xsd\" schemaVersion=\"3\" schemaRevision=\"1.61\">\n";
		oss<< "<Entity entityId='"<<UID<<"' entityIdEncrypted='na' entityTypeName='CalFocusTable' schemaVersion='1' documentVersion='1'/>\n";
		oss<< "<ContainerEntity entityId='"<<containerUID<<"' entityIdEncrypted='na' entityTypeName='ASDM' schemaVersion='1' documentVersion='1'/>\n";
		oss << "<BulkStoreRef file_id='"<<withoutUID<<"' byteOrder='"<<byteOrder->toString()<<"' />\n";
		oss << "<Attributes>\n";

		oss << "<antennaName/>\n"; 
		oss << "<receiverBand/>\n"; 
		oss << "<calDataId/>\n"; 
		oss << "<calReductionId/>\n"; 
		oss << "<startValidTime/>\n"; 
		oss << "<endValidTime/>\n"; 
		oss << "<ambientTemperature/>\n"; 
		oss << "<atmPhaseCorrection/>\n"; 
		oss << "<focusMethod/>\n"; 
		oss << "<frequencyRange/>\n"; 
		oss << "<pointingDirection/>\n"; 
		oss << "<numReceptor/>\n"; 
		oss << "<polarizationTypes/>\n"; 
		oss << "<wereFixed/>\n"; 
		oss << "<offset/>\n"; 
		oss << "<offsetError/>\n"; 
		oss << "<offsetWasTied/>\n"; 
		oss << "<reducedChiSquared/>\n"; 
		oss << "<position/>\n"; 

		oss << "<polarizationsAveraged/>\n"; 
		oss << "<focusCurveWidth/>\n"; 
		oss << "<focusCurveWidthError/>\n"; 
		oss << "<focusCurveWasFixed/>\n"; 
		oss << "<offIntensity/>\n"; 
		oss << "<offIntensityError/>\n"; 
		oss << "<offIntensityWasFixed/>\n"; 
		oss << "<peakIntensity/>\n"; 
		oss << "<peakIntensityError/>\n"; 
		oss << "<peakIntensityWasFixed/>\n"; 
		oss << "</Attributes>\n";		
		oss << "</CalFocusTable>\n";

		return oss.str();				
	}
	
	string CalFocusTable::toMIME(const asdm::ByteOrder* byteOrder) {
		EndianOSStream eoss(byteOrder);
		
		string UID = getEntity().getEntityId().toString();
		
		// The MIME Header
		eoss <<"MIME-Version: 1.0";
		eoss << "\n";
		eoss << "Content-Type: Multipart/Related; boundary='MIME_boundary'; type='text/xml'; start= '<header.xml>'";
		eoss <<"\n";
		eoss <<"Content-Description: Correlator";
		eoss <<"\n";
		eoss <<"alma-uid:" << UID;
		eoss <<"\n";
		eoss <<"\n";		
		
		// The MIME XML part header.
		eoss <<"--MIME_boundary";
		eoss <<"\n";
		eoss <<"Content-Type: text/xml; charset='ISO-8859-1'";
		eoss <<"\n";
		eoss <<"Content-Transfer-Encoding: 8bit";
		eoss <<"\n";
		eoss <<"Content-ID: <header.xml>";
		eoss <<"\n";
		eoss <<"\n";
		
		// The MIME XML part content.
		eoss << MIMEXMLPart(byteOrder);

		// The MIME binary part header
		eoss <<"--MIME_boundary";
		eoss <<"\n";
		eoss <<"Content-Type: binary/octet-stream";
		eoss <<"\n";
		eoss <<"Content-ID: <content.bin>";
		eoss <<"\n";
		eoss <<"\n";	
		
		// The MIME binary content
		entity.toBin(eoss);
		container.getEntity().toBin(eoss);
		eoss.writeInt((int) privateRows.size());
		for (unsigned int i = 0; i < privateRows.size(); i++) {
			privateRows.at(i)->toBin(eoss);	
		}
		
		// The closing MIME boundary
		eoss << "\n--MIME_boundary--";
		eoss << "\n";
		
		return eoss.str();	
	}

	
	void CalFocusTable::setFromMIME(const string & mimeMsg) {
    string xmlPartMIMEHeader = "Content-ID: <header.xml>\n\n";
    
    string binPartMIMEHeader = "--MIME_boundary\nContent-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
    
    // Detect the XML header.
    string::size_type loc0 = mimeMsg.find(xmlPartMIMEHeader, 0);
    if ( loc0 == string::npos) {
      // let's try with CRLFs
      xmlPartMIMEHeader = "Content-ID: <header.xml>\r\n\r\n";
      loc0 = mimeMsg.find(xmlPartMIMEHeader, 0);
      if  ( loc0 == string::npos ) 
	      throw ConversionException("Failed to detect the beginning of the XML header", "CalFocus");
    }

    loc0 += xmlPartMIMEHeader.size();
    
    // Look for the string announcing the binary part.
    string::size_type loc1 = mimeMsg.find( binPartMIMEHeader, loc0 );
    
    if ( loc1 == string::npos ) {
      throw ConversionException("Failed to detect the beginning of the binary part", "CalFocus");
    }
    
    //
    // Extract the xmlHeader and analyze it to find out what is the byte order and the sequence
    // of attribute names.
    //
    string xmlHeader = mimeMsg.substr(loc0, loc1-loc0);
    xmlDoc *doc;
    doc = xmlReadMemory(xmlHeader.data(), xmlHeader.size(), "BinaryTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
    if ( doc == NULL ) 
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "CalFocus");
    
   // This vector will be filled by the names of  all the attributes of the table
   // in the order in which they are expected to be found in the binary representation.
   //
    vector<string> attributesSeq;
      
    xmlNode* root_element = xmlDocGetRootElement(doc);
    if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "CalFocus");
    
    const ByteOrder* byteOrder;
    if ( string("ASDMBinaryTable").compare((const char*) root_element->name) == 0) {
      // Then it's an "old fashioned" MIME file for tables.
      // Just try to deserialize it with Big_Endian for the bytes ordering.
      byteOrder = asdm::ByteOrder::Big_Endian;
      
 	 //
    // Let's consider a  default order for the sequence of attributes.
    //
    
    	 
    attributesSeq.push_back("antennaName") ; 
    	 
    attributesSeq.push_back("receiverBand") ; 
    	 
    attributesSeq.push_back("calDataId") ; 
    	 
    attributesSeq.push_back("calReductionId") ; 
    	 
    attributesSeq.push_back("startValidTime") ; 
    	 
    attributesSeq.push_back("endValidTime") ; 
    	 
    attributesSeq.push_back("ambientTemperature") ; 
    	 
    attributesSeq.push_back("atmPhaseCorrection") ; 
    	 
    attributesSeq.push_back("focusMethod") ; 
    	 
    attributesSeq.push_back("frequencyRange") ; 
    	 
    attributesSeq.push_back("pointingDirection") ; 
    	 
    attributesSeq.push_back("numReceptor") ; 
    	 
    attributesSeq.push_back("polarizationTypes") ; 
    	 
    attributesSeq.push_back("wereFixed") ; 
    	 
    attributesSeq.push_back("offset") ; 
    	 
    attributesSeq.push_back("offsetError") ; 
    	 
    attributesSeq.push_back("offsetWasTied") ; 
    	 
    attributesSeq.push_back("reducedChiSquared") ; 
    	 
    attributesSeq.push_back("position") ; 
    	
    	 
    attributesSeq.push_back("polarizationsAveraged") ; 
    	 
    attributesSeq.push_back("focusCurveWidth") ; 
    	 
    attributesSeq.push_back("focusCurveWidthError") ; 
    	 
    attributesSeq.push_back("focusCurveWasFixed") ; 
    	 
    attributesSeq.push_back("offIntensity") ; 
    	 
    attributesSeq.push_back("offIntensityError") ; 
    	 
    attributesSeq.push_back("offIntensityWasFixed") ; 
    	 
    attributesSeq.push_back("peakIntensity") ; 
    	 
    attributesSeq.push_back("peakIntensityError") ; 
    	 
    attributesSeq.push_back("peakIntensityWasFixed") ; 
    	
     
    
    
    // And decide that it has version == "2"
    version = "2";         
     }
    else if (string("CalFocusTable").compare((const char*) root_element->name) == 0) {
      // It's a new (and correct) MIME file for tables.
      //
      // 1st )  Look for a BulkStoreRef element with an attribute byteOrder.
      //
      xmlNode* bulkStoreRef = 0;
      xmlNode* child = root_element->children;
      
      if (xmlHasProp(root_element, (const xmlChar*) "schemaVersion")) {
      	xmlChar * value = xmlGetProp(root_element, (const xmlChar *) "schemaVersion");
      	version = string ((const char *) value);
      	xmlFree(value);	
      }
      
      // Skip the two first children (Entity and ContainerEntity).
      bulkStoreRef = (child ==  0) ? 0 : ( (child->next) == 0 ? 0 : child->next->next );
      
      if ( bulkStoreRef == 0 || (bulkStoreRef->type != XML_ELEMENT_NODE)  || (string("BulkStoreRef").compare((const char*) bulkStoreRef->name) != 0))
      	throw ConversionException ("Could not find the element '/CalFocusTable/BulkStoreRef'. Invalid XML header '"+ xmlHeader + "'.", "CalFocus");
      	
      // We found BulkStoreRef, now look for its attribute byteOrder.
      _xmlAttr* byteOrderAttr = 0;
      for (struct _xmlAttr* attr = bulkStoreRef->properties; attr; attr = attr->next) 
	  if (string("byteOrder").compare((const char*) attr->name) == 0) {
	   byteOrderAttr = attr;
	   break;
	 }
      
      if (byteOrderAttr == 0) 
	     throw ConversionException("Could not find the element '/CalFocusTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader +"'.", "CalFocus");
      
      string byteOrderValue = string((const char*) byteOrderAttr->children->content);
      if (!(byteOrder = asdm::ByteOrder::fromString(byteOrderValue)))
		throw ConversionException("No valid value retrieved for the element '/CalFocusTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader + "'.", "CalFocus");
		
	 //
	 // 2nd) Look for the Attributes element and grab the names of the elements it contains.
	 //
	 xmlNode* attributes = bulkStoreRef->next;
     if ( attributes == 0 || (attributes->type != XML_ELEMENT_NODE)  || (string("Attributes").compare((const char*) attributes->name) != 0))	 
       	throw ConversionException ("Could not find the element '/CalFocusTable/Attributes'. Invalid XML header '"+ xmlHeader + "'.", "CalFocus");
 
 	xmlNode* childOfAttributes = attributes->children;
 	
 	while ( childOfAttributes != 0 && (childOfAttributes->type == XML_ELEMENT_NODE) ) {
 		attributesSeq.push_back(string((const char*) childOfAttributes->name));
 		childOfAttributes = childOfAttributes->next;
    }
    }
    // Create an EndianISStream from the substring containing the binary part.
    EndianISStream eiss(mimeMsg.substr(loc1+binPartMIMEHeader.size()), byteOrder);
    
    entity = Entity::fromBin((EndianIStream&) eiss);
    
    // We do nothing with that but we have to read it.
    Entity containerEntity = Entity::fromBin((EndianIStream&) eiss);

	// Let's read numRows but ignore it and rely on the value specified in the ASDM.xml file.    
    int numRows = ((EndianIStream&) eiss).readInt();
    if ((numRows != -1)                        // Then these are *not* data produced at the EVLA.
    	&& ((unsigned int) numRows != this->declaredSize )) { // Then the declared size (in ASDM.xml) is not equal to the one 
    	                                       // written into the binary representation of the table.
		cout << "The a number of rows ('" 
			 << numRows
			 << "') declared in the binary representation of the table is different from the one declared in ASDM.xml ('"
			 << this->declaredSize
			 << "'). I'll proceed with the value declared in ASDM.xml"
			 << endl;
    }                                           

	if (getContainer().checkRowUniqueness()) {
    	try {
      		for (uint32_t i = 0; i < this->declaredSize; i++) {
				CalFocusRow* aRow = CalFocusRow::fromBin((EndianIStream&) eiss, *this, attributesSeq);
				checkAndAdd(aRow);
      		}
    	}
    	catch (DuplicateKey e) {
      		throw ConversionException("Error while writing binary data , the message was "
				+ e.getMessage(), "CalFocus");
    	}
    	catch (TagFormatException e) {
     		 throw ConversionException("Error while reading binary data , the message was "
				+ e.getMessage(), "CalFocus");
    	}
    }
    else {
 		for (uint32_t i = 0; i < this->declaredSize; i++) {
			CalFocusRow* aRow = CalFocusRow::fromBin((EndianIStream&) eiss, *this, attributesSeq);
			append(aRow);
      	}   	
    }
    archiveAsBin = true;
    fileAsBin = true;
	}
	
	void CalFocusTable::setUnknownAttributeBinaryReader(const string& attributeName, BinaryAttributeReaderFunctor* barFctr) {
		//
		// Is this attribute really unknown ?
		//
		for (vector<string>::const_iterator iter = attributesNames.begin(); iter != attributesNames.end(); iter++) {
			if ((*iter).compare(attributeName) == 0) 
				throw ConversionException("the attribute '"+attributeName+"' is known you can't override the way it's read in the MIME binary file containing the table.", "CalFocus"); 
		}
		
		// Ok then register the functor to activate when an unknown attribute is met during the reading of a binary table?
		unknownAttributes2Functors[attributeName] = barFctr;
	}
	
	BinaryAttributeReaderFunctor* CalFocusTable::getUnknownAttributeBinaryReader(const string& attributeName) const {
		map<string, BinaryAttributeReaderFunctor*>::const_iterator iter = unknownAttributes2Functors.find(attributeName);
		return (iter == unknownAttributes2Functors.end()) ? 0 : iter->second;
	}

	
	void CalFocusTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}

		string fileName = directory + "/CalFocus.xml";
		ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not open file " + fileName + " to write ", "CalFocus");
		if (fileAsBin) 
			tableout << MIMEXMLPart();
		else
			tableout << toXML() << endl;
		tableout.close();
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not close file " + fileName, "CalFocus");

		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalFocus.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalFocus");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalFocus");
		}
	}

	
	void CalFocusTable::setFromFile(const string& directory) {		
    if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/CalFocus.xml"))))
      setFromXMLFile(directory);
    else if (boost::filesystem::exists(boost::filesystem::path(uniqSlashes(directory + "/CalFocus.bin"))))
      setFromMIMEFile(directory);
    else
      throw ConversionException("No file found for the CalFocus table", "CalFocus");
	}			

	
  void CalFocusTable::setFromMIMEFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/CalFocus.bin";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "CalFocus");
    }
    // Read in a stringstream.
    stringstream ss; ss << tablefile.rdbuf();
    
    if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file " + tablePath,"CalFocus");
    }
    
    // And close.
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file " + tablePath,"CalFocus");
    
    setFromMIME(ss.str());
  }	
/* 
  void CalFocusTable::openMIMEFile (const string& directory) {
  		
  	// Open the file.
  	string tablePath ;
    tablePath = directory + "/CalFocus.bin";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open())
      throw ConversionException("Could not open file " + tablePath, "CalFocus");
      
	// Locate the xmlPartMIMEHeader.
    string xmlPartMIMEHeader = "CONTENT-ID: <HEADER.XML>\n\n";
    CharComparator comparator;
    istreambuf_iterator<char> BEGIN(tablefile.rdbuf());
    istreambuf_iterator<char> END;
    istreambuf_iterator<char> it = search(BEGIN, END, xmlPartMIMEHeader.begin(), xmlPartMIMEHeader.end(), comparator);
    if (it == END) 
    	throw ConversionException("failed to detect the beginning of the XML header", "CalFocus");
    
    // Locate the binaryPartMIMEHeader while accumulating the characters of the xml header.	
    string binPartMIMEHeader = "--MIME_BOUNDARY\nCONTENT-TYPE: BINARY/OCTET-STREAM\nCONTENT-ID: <CONTENT.BIN>\n\n";
    string xmlHeader;
   	CharCompAccumulator compaccumulator(&xmlHeader, 100000);
   	++it;
   	it = search(it, END, binPartMIMEHeader.begin(), binPartMIMEHeader.end(), compaccumulator);
   	if (it == END) 
   		throw ConversionException("failed to detect the beginning of the binary part", "CalFocus");
   	
	cout << xmlHeader << endl;
	//
	// We have the xmlHeader , let's parse it.
	//
	xmlDoc *doc;
    doc = xmlReadMemory(xmlHeader.data(), xmlHeader.size(), "BinaryTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
    if ( doc == NULL ) 
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "CalFocus");
    
   // This vector will be filled by the names of  all the attributes of the table
   // in the order in which they are expected to be found in the binary representation.
   //
    vector<string> attributesSeq(attributesNamesInBin);
      
    xmlNode* root_element = xmlDocGetRootElement(doc);
    if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "CalFocus");
    
    const ByteOrder* byteOrder;
    if ( string("ASDMBinaryTable").compare((const char*) root_element->name) == 0) {
      // Then it's an "old fashioned" MIME file for tables.
      // Just try to deserialize it with Big_Endian for the bytes ordering.
      byteOrder = asdm::ByteOrder::Big_Endian;
        
      // And decide that it has version == "2"
    version = "2";         
     }
    else if (string("CalFocusTable").compare((const char*) root_element->name) == 0) {
      // It's a new (and correct) MIME file for tables.
      //
      // 1st )  Look for a BulkStoreRef element with an attribute byteOrder.
      //
      xmlNode* bulkStoreRef = 0;
      xmlNode* child = root_element->children;
      
      if (xmlHasProp(root_element, (const xmlChar*) "schemaVersion")) {
      	xmlChar * value = xmlGetProp(root_element, (const xmlChar *) "schemaVersion");
      	version = string ((const char *) value);
      	xmlFree(value);	
      }
      
      // Skip the two first children (Entity and ContainerEntity).
      bulkStoreRef = (child ==  0) ? 0 : ( (child->next) == 0 ? 0 : child->next->next );
      
      if ( bulkStoreRef == 0 || (bulkStoreRef->type != XML_ELEMENT_NODE)  || (string("BulkStoreRef").compare((const char*) bulkStoreRef->name) != 0))
      	throw ConversionException ("Could not find the element '/CalFocusTable/BulkStoreRef'. Invalid XML header '"+ xmlHeader + "'.", "CalFocus");
      	
      // We found BulkStoreRef, now look for its attribute byteOrder.
      _xmlAttr* byteOrderAttr = 0;
      for (struct _xmlAttr* attr = bulkStoreRef->properties; attr; attr = attr->next) 
	  if (string("byteOrder").compare((const char*) attr->name) == 0) {
	   byteOrderAttr = attr;
	   break;
	 }
      
      if (byteOrderAttr == 0) 
	     throw ConversionException("Could not find the element '/CalFocusTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader +"'.", "CalFocus");
      
      string byteOrderValue = string((const char*) byteOrderAttr->children->content);
      if (!(byteOrder = asdm::ByteOrder::fromString(byteOrderValue)))
		throw ConversionException("No valid value retrieved for the element '/CalFocusTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader + "'.", "CalFocus");
		
	 //
	 // 2nd) Look for the Attributes element and grab the names of the elements it contains.
	 //
	 xmlNode* attributes = bulkStoreRef->next;
     if ( attributes == 0 || (attributes->type != XML_ELEMENT_NODE)  || (string("Attributes").compare((const char*) attributes->name) != 0))	 
       	throw ConversionException ("Could not find the element '/CalFocusTable/Attributes'. Invalid XML header '"+ xmlHeader + "'.", "CalFocus");
 
 	xmlNode* childOfAttributes = attributes->children;
 	
 	while ( childOfAttributes != 0 && (childOfAttributes->type == XML_ELEMENT_NODE) ) {
 		attributesSeq.push_back(string((const char*) childOfAttributes->name));
 		childOfAttributes = childOfAttributes->next;
    }
    }
    // Create an EndianISStream from the substring containing the binary part.
    EndianIFStream eifs(&tablefile, byteOrder);
    
    entity = Entity::fromBin((EndianIStream &) eifs);
    
    // We do nothing with that but we have to read it.
    Entity containerEntity = Entity::fromBin((EndianIStream &) eifs);

	// Let's read numRows but ignore it and rely on the value specified in the ASDM.xml file.    
    int numRows = eifs.readInt();
    if ((numRows != -1)                        // Then these are *not* data produced at the EVLA.
    	&& ((unsigned int) numRows != this->declaredSize )) { // Then the declared size (in ASDM.xml) is not equal to the one 
    	                                       // written into the binary representation of the table.
		cout << "The a number of rows ('" 
			 << numRows
			 << "') declared in the binary representation of the table is different from the one declared in ASDM.xml ('"
			 << this->declaredSize
			 << "'). I'll proceed with the value declared in ASDM.xml"
			 << endl;
    }    
  } 
 */

	
void CalFocusTable::setFromXMLFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/CalFocus.xml";
    
    /*
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "CalFocus");
    }
      // Read in a stringstream.
    stringstream ss;
    ss << tablefile.rdbuf();
    
    if  (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file '" + tablePath + "'", "CalFocus");
    }
    
    // And close
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file '" + tablePath + "'", "CalFocus");

    // Let's make a string out of the stringstream content and empty the stringstream.
    string xmlDocument = ss.str(); ss.str("");
	
    // Let's make a very primitive check to decide
    // whether the XML content represents the table
    // or refers to it via a <BulkStoreRef element.
    */
    
    string xmlDocument;
    try {
    	xmlDocument = getContainer().getXSLTransformer()(tablePath);
    	if (getenv("ASDM_DEBUG")) cout << "About to read " << tablePath << endl;
    }
    catch (XSLTransformerException e) {
    	throw ConversionException("Caugth an exception whose message is '" + e.getMessage() + "'.", "CalFocus");
    }
    
    if (xmlDocument.find("<BulkStoreRef") != string::npos)
      setFromMIMEFile(directory);
    else
      fromXML(xmlDocument);
  }

	

	

			
	
	

	
} // End namespace asdm
 
