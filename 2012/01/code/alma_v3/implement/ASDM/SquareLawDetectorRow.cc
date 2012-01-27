
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
 * File SquareLawDetectorRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <SquareLawDetectorRow.h>
#include <SquareLawDetectorTable.h>
	

using asdm::ASDM;
using asdm::SquareLawDetectorRow;
using asdm::SquareLawDetectorTable;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	SquareLawDetectorRow::~SquareLawDetectorRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	SquareLawDetectorTable &SquareLawDetectorRow::getTable() const {
		return table;
	}

	bool SquareLawDetectorRow::isAdded() const {
		return hasBeenAdded;
	}	

	void SquareLawDetectorRow::isAdded(bool added) {
		hasBeenAdded = added;
	}
	
#ifndef WITHOUT_ACS
	using asdmIDL::SquareLawDetectorRowIDL;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a SquareLawDetectorRowIDL struct.
	 */
	SquareLawDetectorRowIDL *SquareLawDetectorRow::toIDL() const {
		SquareLawDetectorRowIDL *x = new SquareLawDetectorRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->squareLawDetectorId = squareLawDetectorId.toIDLTag();
			
		
	

	
  		
		
		
			
				
		x->numBand = numBand;
 				
 			
		
	

	
  		
		
		
			
				
		x->bandType = bandType;
 				
 			
		
	

	
	
		
		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct SquareLawDetectorRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void SquareLawDetectorRow::setFromIDL (SquareLawDetectorRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setSquareLawDetectorId(Tag (x.squareLawDetectorId));
			
 		
		
	

	
		
		
			
		setNumBand(x.numBand);
  			
 		
		
	

	
		
		
			
		setBandType(x.bandType);
  			
 		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"SquareLawDetector");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string SquareLawDetectorRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(squareLawDetectorId, "squareLawDetectorId", buf);
		
		
	

  	
 		
		
		Parser::toXML(numBand, "numBand", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("bandType", bandType));
		
		
	

	
	
		
		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void SquareLawDetectorRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setSquareLawDetectorId(Parser::getTag("squareLawDetectorId","SquareLawDetector",rowDoc));
			
		
	

	
  		
			
	  	setNumBand(Parser::getInteger("numBand","SquareLawDetector",rowDoc));
			
		
	

	
		
		
		
		bandType = EnumerationParser::getDetectorBandType("bandType","SquareLawDetector",rowDoc);
		
		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"SquareLawDetector");
		}
	}
	
	void SquareLawDetectorRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	squareLawDetectorId.toBin(eoss);
		
	

	
	
		
						
			eoss.writeInt(numBand);
				
		
	

	
	
		
					
			eoss.writeString(CDetectorBandType::name(bandType));
			/* eoss.writeInt(bandType); */
				
		
	


	
	
	}
	
void SquareLawDetectorRow::squareLawDetectorIdFromBin(EndianISStream& eiss) {
		
	
		
		
		squareLawDetectorId =  Tag::fromBin(eiss);
		
	
	
}
void SquareLawDetectorRow::numBandFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numBand =  eiss.readInt();
			
		
	
	
}
void SquareLawDetectorRow::bandTypeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		bandType = CDetectorBandType::literal(eiss.readString());
			
		
	
	
}

		
	
	SquareLawDetectorRow* SquareLawDetectorRow::fromBin(EndianISStream& eiss, SquareLawDetectorTable& table, const vector<string>& attributesSeq) {
		SquareLawDetectorRow* row = new  SquareLawDetectorRow(table);
		
		map<string, SquareLawDetectorAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter != row->fromBinMethods.end()) {
				(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);			
			}
			else {
				BinaryAttributeReaderFunctor* functorP = table.getUnknownAttributeBinaryReader(attributesSeq.at(i));
				if (functorP)
					(*functorP)(eiss);
				else
					throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "SquareLawDetectorTable");
			}
				
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get squareLawDetectorId.
 	 * @return squareLawDetectorId as Tag
 	 */
 	Tag SquareLawDetectorRow::getSquareLawDetectorId() const {
	
  		return squareLawDetectorId;
 	}

 	/**
 	 * Set squareLawDetectorId with the specified Tag.
 	 * @param squareLawDetectorId The Tag value to which squareLawDetectorId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SquareLawDetectorRow::setSquareLawDetectorId (Tag squareLawDetectorId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("squareLawDetectorId", "SquareLawDetector");
		
  		}
  	
 		this->squareLawDetectorId = squareLawDetectorId;
	
 	}
	
	

	

	
 	/**
 	 * Get numBand.
 	 * @return numBand as int
 	 */
 	int SquareLawDetectorRow::getNumBand() const {
	
  		return numBand;
 	}

 	/**
 	 * Set numBand with the specified int.
 	 * @param numBand The int value to which numBand is to be set.
 	 
 	
 		
 	 */
 	void SquareLawDetectorRow::setNumBand (int numBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numBand = numBand;
	
 	}
	
	

	

	
 	/**
 	 * Get bandType.
 	 * @return bandType as DetectorBandTypeMod::DetectorBandType
 	 */
 	DetectorBandTypeMod::DetectorBandType SquareLawDetectorRow::getBandType() const {
	
  		return bandType;
 	}

 	/**
 	 * Set bandType with the specified DetectorBandTypeMod::DetectorBandType.
 	 * @param bandType The DetectorBandTypeMod::DetectorBandType value to which bandType is to be set.
 	 
 	
 		
 	 */
 	void SquareLawDetectorRow::setBandType (DetectorBandTypeMod::DetectorBandType bandType)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->bandType = bandType;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	/**
	 * Create a SquareLawDetectorRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	SquareLawDetectorRow::SquareLawDetectorRow (SquareLawDetectorTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	
	
	
	
	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
bandType = CDetectorBandType::from_int(0);
	

	
	
	 fromBinMethods["squareLawDetectorId"] = &SquareLawDetectorRow::squareLawDetectorIdFromBin; 
	 fromBinMethods["numBand"] = &SquareLawDetectorRow::numBandFromBin; 
	 fromBinMethods["bandType"] = &SquareLawDetectorRow::bandTypeFromBin; 
		
	
	
	}
	
	SquareLawDetectorRow::SquareLawDetectorRow (SquareLawDetectorTable &t, SquareLawDetectorRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

			
		}
		else {
	
		
			squareLawDetectorId = row.squareLawDetectorId;
		
		
		
		
			numBand = row.numBand;
		
			bandType = row.bandType;
		
		
		
		
		}
		
		 fromBinMethods["squareLawDetectorId"] = &SquareLawDetectorRow::squareLawDetectorIdFromBin; 
		 fromBinMethods["numBand"] = &SquareLawDetectorRow::numBandFromBin; 
		 fromBinMethods["bandType"] = &SquareLawDetectorRow::bandTypeFromBin; 
			
	
			
	}

	
	bool SquareLawDetectorRow::compareNoAutoInc(int numBand, DetectorBandTypeMod::DetectorBandType bandType) {
		bool result;
		result = true;
		
	
		
		result = result && (this->numBand == numBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->bandType == bandType);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool SquareLawDetectorRow::compareRequiredValue(int numBand, DetectorBandTypeMod::DetectorBandType bandType) {
		bool result;
		result = true;
		
	
		if (!(this->numBand == numBand)) return false;
	

	
		if (!(this->bandType == bandType)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the SquareLawDetectorRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool SquareLawDetectorRow::equalByRequiredValue(SquareLawDetectorRow* x) {
		
			
		if (this->numBand != x->numBand) return false;
			
		if (this->bandType != x->bandType) return false;
			
		
		return true;
	}	
	
/*
	 map<string, SquareLawDetectorAttributeFromBin> SquareLawDetectorRow::initFromBinMethods() {
		map<string, SquareLawDetectorAttributeFromBin> result;
		
		result["squareLawDetectorId"] = &SquareLawDetectorRow::squareLawDetectorIdFromBin;
		result["numBand"] = &SquareLawDetectorRow::numBandFromBin;
		result["bandType"] = &SquareLawDetectorRow::bandTypeFromBin;
		
		
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
