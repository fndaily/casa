
/***
 * Framework independent implementation file for utils...
 *
 * Implement the utils component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <fstream>
#include <xmlcasa/utils/utils_cmpt.h>
#include <xmlcasa/utils/stdBaseInterface.h>
#include <xmlcasa/xerces/stdcasaXMLUtil.h>
#include <casa/Logging/LogIO.h>
#include <casa/BasicSL/String.h>
#include <casa/System/Aipsrc.h>



using namespace std;
using namespace casa;

namespace casac {

utils::utils()
{
  myConstraints = 0;
  itsLog = new casa::LogIO;
}

utils::~utils()
{
  if(myConstraints)
     delete myConstraints;
  delete itsLog;
}

bool
utils::verify(const ::casac::record& input, const ::casac::variant& xmldescriptor)
{

   bool rstat(true);
   record *constraints(0);
   *itsLog << LogOrigin("utils", "verify") << LogIO::NORMAL3 << "Verifying arguments....";
   switch(xmldescriptor.type()){
      case variant::STRING :
	 constraints = torecord(xmldescriptor.getString());
	 // std::cerr << "constraints record: ";
	 // dumpRecord(std::cerr, *constraints);
         break;
      case variant::RECORD :
         constraints = new record(xmldescriptor.getRecord());
         break;
      default :
         rstat = false;
         break;
   }
   if(rstat){
	   rstat = stdBaseInterface::verify(const_cast<record &>(input), *constraints, *itsLog);
	   if(rstat){
		   *itsLog << LogOrigin("utils", "verify") << LogIO::NORMAL3 << "verified." << LogIO::POST;
	   }else{
		   *itsLog <<  LogIO::POST;
		   *itsLog << LogOrigin("utils", "verify") << LogIO::WARN << "Some arguments failed to verify!" << LogIO::POST;
	   }
   }
   if(constraints)
	   delete constraints;
   // std::cerr << "return from verify is " << rstat << std::endl;
   return rstat;
}

bool
utils::setconstraints(const ::casac::variant& xmldescriptor)
{
   bool rstat(true);
   if(myConstraints)
	   delete myConstraints;
   *itsLog << LogOrigin("utils", "setconstraints") << LogIO::NORMAL3 << "Setting constraints ...";
   switch(xmldescriptor.type()){
      case variant::STRING :
	 myConstraints = torecord(xmldescriptor.getString());
	 // std::cerr << "constraints record: ";
	 // dumpRecord(std::cerr, *constraints);
         break;
      case variant::RECORD :
         myConstraints = new record(xmldescriptor.getRecord());
         break;
      default :
	 rstat = false;
   }
   *itsLog << LogIO::NORMAL3 << "Constraints set." << LogIO::POST;
	 // std::cerr << "constraints record: ";
	 // dumpRecord(std::cerr, *myConstraints);
   return rstat;
}

bool
utils::verifyparam(const ::casac::record& param)
{
   bool rstat(true);
   if(myConstraints){
      rec_map::iterator iter = myConstraints->begin(); // We need the underlying record...
      rstat = stdBaseInterface::verifyOne(const_cast<record &>(param), (*iter).second.asRecord(), *itsLog);
      /*
      if(rstat){
         *itsLog << LogOrigin("utils", "verifyparam") <<LogIO::INFO<< "verified." << LogIO::POST;
      }else{
         *itsLog << LogIO::POST;
         *itsLog << LogOrigin("utils", "verifyparam") << LogIO::WARN 
		 << "Some arguments failed to verify!" << LogIO::POST;
      }
      */
   } else {
         *itsLog << LogOrigin("utils", "verifyparam") << LogIO::WARN
		 << "Contraints record not set, unable to verify parameter" << LogIO::POST;
   }
   return rstat;
}


::casac::record*
utils::torecord(const std::string& input)
{
   stdcasaXMLUtil xmlUtils;
   casac::record *rstat = new casac::record;
   if(!input.find("<?xml version")){
      xmlUtils.toCasaRecord(*rstat, input);
   }else{
      if(!input.find("file:///")){
         Bool ok = xmlUtils.readXMLFile(*rstat, input.substr(7));
	 if(!ok){
            *itsLog << LogIO::SEVERE << "Unable to read XML file " << input << ", unable to verify input" << LogIO::POST;
	 }
      } else {
         *itsLog << LogIO::SEVERE << "Defaults specified are not an XML string, unable to verify input" << LogIO::POST;
      }
   }
   return rstat;
}

std::string
utils::toxml(const ::casac::record& input, const bool asfile, const std::string& filename)
{   string rstat;

   stdcasaXMLUtil xmlUtils;
   if(asfile){
      std::ofstream xmlout(filename.c_str(), ios::out);
      xmlUtils.fromCasaRecord(xmlout, input);
      rstat = filename;
   } else {
      ostringstream xmlout;
      xmlUtils.fromCasaRecord(xmlout, input);
      rstat = xmlout.str();
   }
   return rstat;
}

std::string
utils::getrc(const std::string& rcvar)
{
  String rstat1;
  if(!rcvar.length()){
	  rstat1 = Aipsrc::aipsRoot();
  } else {
	  if(!Aipsrc::find(rstat1, rcvar))
		  rstat1 = "Unknown value";
  }
  string rstat(rstat1.c_str());
  return rstat;
}

} // casac namespace

