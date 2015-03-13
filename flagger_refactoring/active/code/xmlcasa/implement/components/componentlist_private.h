// Private members and functions
//
casa::LogIO         *itsLog;
casa::ComponentList *itsList;
casa::ComponentList *itsBin;

casa::Vector<casa::Int> checkIndices(int which,
                                     const casa::String& function,
                                     const casa::String& message) const;

casa::Vector<casa::Int> checkIndices(const vector<int>& which,
                                     const casa::String& function,
                                     const casa::String& message) const;

int checkIndex(int which, const casa::String& function) const;

int checkFluxPol(const casa::String &polString);