
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
 * File FlagCmdTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <FlagCmdTable.h>
#include <FlagCmdRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::FlagCmdTable;
using asdm::FlagCmdRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "boost/filesystem/operations.hpp"


namespace asdm {

	string FlagCmdTable::tableName = "FlagCmd";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> FlagCmdTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> FlagCmdTable::getKeyName() {
		return key;
	}


	FlagCmdTable::FlagCmdTable(ASDM &c) : container(c) {

	
		key.push_back("timeInterval");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("FlagCmdTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for FlagCmdTable.
 */
 
	FlagCmdTable::~FlagCmdTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &FlagCmdTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	unsigned int FlagCmdTable::size() {
		return privateRows.size();
	}
	

	/**
	 * Return the name of this table.
	 */
	string FlagCmdTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity FlagCmdTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void FlagCmdTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	FlagCmdRow *FlagCmdTable::newRow() {
		return new FlagCmdRow (*this);
	}
	

	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param timeInterval 
	
 	 * @param type 
	
 	 * @param reason 
	
 	 * @param level 
	
 	 * @param severity 
	
 	 * @param applied 
	
 	 * @param command 
	
     */
	FlagCmdRow* FlagCmdTable::newRow(ArrayTimeInterval timeInterval, string type, string reason, int level, int severity, bool applied, string command){
		FlagCmdRow *row = new FlagCmdRow(*this);
			
		row->setTimeInterval(timeInterval);
			
		row->setType(type);
			
		row->setReason(reason);
			
		row->setLevel(level);
			
		row->setSeverity(severity);
			
		row->setApplied(applied);
			
		row->setCommand(command);
	
		return row;		
	}	
	


FlagCmdRow* FlagCmdTable::newRow(FlagCmdRow* row) {
	return new FlagCmdRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	
		
		  
	FlagCmdRow* FlagCmdTable::add(FlagCmdRow* x) {
		FlagCmdRow* aRow = getRowByKey(
		
			x->getTimeInterval()
		
		);
		// There is a row with x's key section return it.
		if (aRow) throw DuplicateKey("Duplicate key exception in ", "FlagCmdTable");
		
		// Insert the row x in the table in such a way that the vector row is sorted
		// by ascending values on timeInterval.getStart().
		return insertByStartTime(x, row);
	}
		
	




	// 
	// A private method to append a row to its table, used by input conversion
	// methods.
	//

	
	
		
		
			
	FlagCmdRow*  FlagCmdTable::checkAndAdd(FlagCmdRow* x) {
		if (getRowByKey(
		
			x->getTimeInterval()
				
		)) throw DuplicateKey("Duplicate key exception in ", "FlagCmdTable");

		return insertByStartTime(x, row);
	}
					
		







	

	
	
		
	/**
	 * Get all rows.
	 * @return Alls rows as an array of FlagCmdRow
	 */
	vector<FlagCmdRow *> FlagCmdTable::get() {
		return privateRows;
		// return row;
	}
		
	


	
		
		
			
			
/*
 ** Returns a FlagCmdRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	FlagCmdRow* FlagCmdTable::getRowByKey(ArrayTimeInterval timeInterval)  {
	FlagCmdRow* aRow = 0;
		for (unsigned int i = 0; i < row.size(); i++) {
			aRow = row.at(i);
			if (aRow->timeInterval.contains(timeInterval.getStart())) return aRow;
		}
		return 0;		
	}
			
		
		
		
	




#ifndef WITHOUT_ACS
	// Conversion Methods

	FlagCmdTableIDL *FlagCmdTable::toIDL() {
		FlagCmdTableIDL *x = new FlagCmdTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<FlagCmdRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void FlagCmdTable::fromIDL(FlagCmdTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			FlagCmdRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	
	string FlagCmdTable::toXML()  {
		string buf;

		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<FlagCmdTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:flgcmd=\"http://Alma/XASDM/FlagCmdTable\" xsi:schemaLocation=\"http://Alma/XASDM/FlagCmdTable http://almaobservatory.org/XML/XASDM/2/FlagCmdTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.53\">\n");
	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<FlagCmdRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</FlagCmdTable> ");
		return buf;
	}

	
	void FlagCmdTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<FlagCmdTable")) 
			error();
		// cout << "Parsing a FlagCmdTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "FlagCmdTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		FlagCmdRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a FlagCmdRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"FlagCmdTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"FlagCmdTable");	
			}
			catch (...) {
				// cout << "Unexpected error in FlagCmdTable::checkAndAdd called from FlagCmdTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</FlagCmdTable>")) 
			error();
			
		archiveAsBin = false;
		fileAsBin = false;
		
	}

	
	void FlagCmdTable::error()  {
		throw ConversionException("Invalid xml document","FlagCmd");
	}
	
	
	string FlagCmdTable::MIMEXMLPart(const asdm::ByteOrder* byteOrder) {
		string UID = getEntity().getEntityId().toString();
		string withoutUID = UID.substr(6);
		string containerUID = getContainer().getEntity().getEntityId().toString();
		ostringstream oss;
		oss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		oss << "\n";
		oss << "<FlagCmdTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:flgcmd=\"http://Alma/XASDM/FlagCmdTable\" xsi:schemaLocation=\"http://Alma/XASDM/FlagCmdTable http://almaobservatory.org/XML/XASDM/2/FlagCmdTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.53\">\n";
		oss<< "<Entity entityId='"<<UID<<"' entityIdEncrypted='na' entityTypeName='FlagCmdTable' schemaVersion='1' documentVersion='1'/>\n";
		oss<< "<ContainerEntity entityId='"<<containerUID<<"' entityIdEncrypted='na' entityTypeName='ASDM' schemaVersion='1' documentVersion='1'/>\n";
		oss << "<BulkStoreRef file_id='"<<withoutUID<<"' byteOrder='"<<byteOrder->toString()<<"' />\n";
		oss << "<Attributes>\n";

		oss << "<timeInterval/>\n"; 
		oss << "<type/>\n"; 
		oss << "<reason/>\n"; 
		oss << "<level/>\n"; 
		oss << "<severity/>\n"; 
		oss << "<applied/>\n"; 
		oss << "<command/>\n"; 

		oss << "</Attributes>\n";		
		oss << "</FlagCmdTable>\n";

		return oss.str();				
	}
	
	string FlagCmdTable::toMIME(const asdm::ByteOrder* byteOrder) {
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

	
	void FlagCmdTable::setFromMIME(const string & mimeMsg) {
    string xmlPartMIMEHeader = "Content-ID: <header.xml>\n\n";
    
    string binPartMIMEHeader = "--MIME_boundary\nContent-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
    
    // Detect the XML header.
    string::size_type loc0 = mimeMsg.find(xmlPartMIMEHeader, 0);
    if ( loc0 == string::npos) {
      throw ConversionException("Failed to detect the beginning of the XML header", "FlagCmd");
    }
    loc0 += xmlPartMIMEHeader.size();
    
    // Look for the string announcing the binary part.
    string::size_type loc1 = mimeMsg.find( binPartMIMEHeader, loc0 );
    
    if ( loc1 == string::npos ) {
      throw ConversionException("Failed to detect the beginning of the binary part", "FlagCmd");
    }
    
    //
    // Extract the xmlHeader and analyze it to find out what is the byte order and the sequence
    // of attribute names.
    //
    string xmlHeader = mimeMsg.substr(loc0, loc1-loc0);
    xmlDoc *doc;
    doc = xmlReadMemory(xmlHeader.data(), xmlHeader.size(), "BinaryTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
    if ( doc == NULL ) 
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "FlagCmd");
    
   // This vector will be filled by the names of  all the attributes of the table
   // in the order in which they are expected to be found in the binary representation.
   //
    vector<string> attributesSeq;
      
    xmlNode* root_element = xmlDocGetRootElement(doc);
    if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "FlagCmd");
    
    const ByteOrder* byteOrder;
    if ( string("ASDMBinaryTable").compare((const char*) root_element->name) == 0) {
      // Then it's an "old fashioned" MIME file for tables.
      // Just try to deserialize it with Big_Endian for the bytes ordering.
      byteOrder = asdm::ByteOrder::Big_Endian;
      
 	 //
    // Let's consider a  default order for the sequence of attributes.
    //
     
    attributesSeq.push_back("timeInterval") ; 
     
    attributesSeq.push_back("type") ; 
     
    attributesSeq.push_back("reason") ; 
     
    attributesSeq.push_back("level") ; 
     
    attributesSeq.push_back("severity") ; 
     
    attributesSeq.push_back("applied") ; 
     
    attributesSeq.push_back("command") ; 
    
              
     }
    else if (string("FlagCmdTable").compare((const char*) root_element->name) == 0) {
      // It's a new (and correct) MIME file for tables.
      //
      // 1st )  Look for a BulkStoreRef element with an attribute byteOrder.
      //
      xmlNode* bulkStoreRef = 0;
      xmlNode* child = root_element->children;
      
      // Skip the two first children (Entity and ContainerEntity).
      bulkStoreRef = (child ==  0) ? 0 : ( (child->next) == 0 ? 0 : child->next->next );
      
      if ( bulkStoreRef == 0 || (bulkStoreRef->type != XML_ELEMENT_NODE)  || (string("BulkStoreRef").compare((const char*) bulkStoreRef->name) != 0))
      	throw ConversionException ("Could not find the element '/FlagCmdTable/BulkStoreRef'. Invalid XML header '"+ xmlHeader + "'.", "FlagCmd");
      	
      // We found BulkStoreRef, now look for its attribute byteOrder.
      _xmlAttr* byteOrderAttr = 0;
      for (struct _xmlAttr* attr = bulkStoreRef->properties; attr; attr = attr->next) 
	  if (string("byteOrder").compare((const char*) attr->name) == 0) {
	   byteOrderAttr = attr;
	   break;
	 }
      
      if (byteOrderAttr == 0) 
	     throw ConversionException("Could not find the element '/FlagCmdTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader +"'.", "FlagCmd");
      
      string byteOrderValue = string((const char*) byteOrderAttr->children->content);
      if (!(byteOrder = asdm::ByteOrder::fromString(byteOrderValue)))
		throw ConversionException("No valid value retrieved for the element '/FlagCmdTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader + "'.", "FlagCmd");
		
	 //
	 // 2nd) Look for the Attributes element and grab the names of the elements it contains.
	 //
	 xmlNode* attributes = bulkStoreRef->next;
     if ( attributes == 0 || (attributes->type != XML_ELEMENT_NODE)  || (string("Attributes").compare((const char*) attributes->name) != 0))	 
       	throw ConversionException ("Could not find the element '/FlagCmdTable/Attributes'. Invalid XML header '"+ xmlHeader + "'.", "FlagCmd");
 
 	xmlNode* childOfAttributes = attributes->children;
 	
 	while ( childOfAttributes != 0 && (childOfAttributes->type == XML_ELEMENT_NODE) ) {
 		attributesSeq.push_back(string((const char*) childOfAttributes->name));
 		childOfAttributes = childOfAttributes->next;
    }
    }
    // Create an EndianISStream from the substring containing the binary part.
    EndianISStream eiss(mimeMsg.substr(loc1+binPartMIMEHeader.size()), byteOrder);
    
    entity = Entity::fromBin(eiss);
    
    // We do nothing with that but we have to read it.
    Entity containerEntity = Entity::fromBin(eiss);
    
    int numRows = eiss.readInt();
    try {
      for (int i = 0; i < numRows; i++) {
	FlagCmdRow* aRow = FlagCmdRow::fromBin(eiss, *this, attributesSeq);
	checkAndAdd(aRow);
      }
    }
    catch (DuplicateKey e) {
      throw ConversionException("Error while writing binary data , the message was "
				+ e.getMessage(), "FlagCmd");
    }
    catch (TagFormatException e) {
      throw ConversionException("Error while reading binary data , the message was "
				+ e.getMessage(), "FlagCmd");
    }
    archiveAsBin = true;
    fileAsBin = true;
	}

	
	void FlagCmdTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}

		string fileName = directory + "/FlagCmd.xml";
		ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not open file " + fileName + " to write ", "FlagCmd");
		if (fileAsBin) 
			tableout << MIMEXMLPart();
		else
			tableout << toXML() << endl;
		tableout.close();
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not close file " + fileName, "FlagCmd");

		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/FlagCmd.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "FlagCmd");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "FlagCmd");
		}
	}

	
	void FlagCmdTable::setFromFile(const string& directory) {
    if (boost::filesystem::exists(boost::filesystem::path(directory + "/FlagCmd.xml")))
      setFromXMLFile(directory);
    else if (boost::filesystem::exists(boost::filesystem::path(directory + "/FlagCmd.bin")))
      setFromMIMEFile(directory);
    else
      throw ConversionException("No file found for the FlagCmd table", "FlagCmd");
	}			

	
  void FlagCmdTable::setFromMIMEFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/FlagCmd.bin";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "FlagCmd");
    }
    // Read in a stringstream.
    stringstream ss; ss << tablefile.rdbuf();
    
    if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file " + tablePath,"FlagCmd");
    }
    
    // And close.
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file " + tablePath,"FlagCmd");
    
    setFromMIME(ss.str());
  }	

	
void FlagCmdTable::setFromXMLFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/FlagCmd.xml";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "FlagCmd");
    }
      // Read in a stringstream.
    stringstream ss;
    ss << tablefile.rdbuf();
    
    if  (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file '" + tablePath + "'", "FlagCmd");
    }
    
    // And close
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file '" + tablePath + "'", "FlagCmd");

    // Let's make a string out of the stringstream content and empty the stringstream.
    string xmlDocument = ss.str(); ss.str("");

    // Let's make a very primitive check to decide
    // whether the XML content represents the table
    // or refers to it via a <BulkStoreRef element.
    if (xmlDocument.find("<BulkStoreRef") != string::npos)
      setFromMIMEFile(directory);
    else
      fromXML(xmlDocument);
  }

	

	

			
	
		
		
	/**
	 * Insert a FlagCmdRow* in a vector of FlagCmdRow* so that it's ordered by ascending start time.
	 *
	 * @param FlagCmdRow* x . The pointer to be inserted.
	 * @param vector <FlagCmdRow*>& row. A reference to the vector where to insert x.
	 *
	 */
	 FlagCmdRow* FlagCmdTable::insertByStartTime(FlagCmdRow* x, vector<FlagCmdRow*>& row) {
				
		vector <FlagCmdRow*>::iterator theIterator;
		
		ArrayTime start = x->timeInterval.getStart();

    	// Is the row vector empty ?
    	if (row.size() == 0) {
    		row.push_back(x);
    		privateRows.push_back(x);
    		x->isAdded();
    		return x;
    	}
    	
    	// Optimization for the case of insertion by ascending time.
    	FlagCmdRow* last = *(row.end()-1);
        
    	if ( start > last->timeInterval.getStart() ) {
 	    	//
	    	// Modify the duration of last if and only if the start time of x
	    	// is located strictly before the end time of last.
	    	//
	  		if ( start < (last->timeInterval.getStart() + last->timeInterval.getDuration()))   		
    			last->timeInterval.setDuration(start - last->timeInterval.getStart());
    		row.push_back(x);
    		privateRows.push_back(x);
    		x->isAdded();
    		return x;
    	}
    	
    	// Optimization for the case of insertion by descending time.
    	FlagCmdRow* first = *(row.begin());
        
    	if ( start < first->timeInterval.getStart() ) {
			//
	  		// Modify the duration of x if and only if the start time of first
	  		// is located strictly before the end time of x.
	  		//
	  		if ( first->timeInterval.getStart() < (start + x->timeInterval.getDuration()) )	  		
    			x->timeInterval.setDuration(first->timeInterval.getStart() - start);
    		row.insert(row.begin(), x);
    		privateRows.push_back(x);
    		x->isAdded();
    		return x;
    	}
    	
    	// Case where x has to be inserted inside row; let's use a dichotomy
    	// method to find the insertion index.
		unsigned int k0 = 0;
		unsigned int k1 = row.size() - 1;
	
		while (k0 != (k1 - 1)) {
			if (start == row[k0]->timeInterval.getStart()) {
				if (row[k0]->equalByRequiredValue(x))
					return row[k0];
				else
					throw DuplicateKey("DuplicateKey exception in ", "FlagCmdTable");	
			}
			else if (start == row[k1]->timeInterval.getStart()) {
				if (row[k1]->equalByRequiredValue(x))
					return row[k1];
				else
					throw DuplicateKey("DuplicateKey exception in ", "FlagCmdTable");	
			}
			else {
				if (start <= row[(k0+k1)/2]->timeInterval.getStart())
					k1 = (k0 + k1) / 2;
				else
					k0 = (k0 + k1) / 2;				
			} 	
		}
	
		row[k0]->timeInterval.setDuration(start-row[k0]->timeInterval.getStart());
		x->timeInterval.setDuration(row[k0+1]->timeInterval.getStart() - start);
		row.insert(row.begin()+(k0+1), x);
		privateRows.push_back(x);
   		x->isAdded();
		return x;   
    } 
    	
	
	

	
} // End namespace asdm
 
