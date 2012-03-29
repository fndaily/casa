import sys
import os
from taskinit import *
(cb,) = gentools(['cb'])
def gencal(vis=None,caltable=None,caltype=None,
           spw=None,antenna=None,pol=None,
           parameter=None):

       """ Externally specify calibration soltuions af various types
       """

       #Python script
       try:

              if ((type(vis)==str) & (os.path.exists(vis))):
                     # don't need scr col for this
                     cb.open(filename=vis,compress=False,addcorr=False,addmodel=False)  
              else:
                     raise Exception, 'Visibility data set not found - please verify the name'

              if (caltable==''):
                     raise Exception, 'A caltable name must be specified'

              # call a Python function to retreive ant position offsets automatically (currently EVLA only)
              if (caltype=='antpos' and antenna==''):
                casalog.post(" Determine antenna position offests from the baseline correction database")
                import correct_ant_posns as getantposns 
                # correct_ant_posns returns a list , [return_code, antennas, offsets]
                antenna_offsets=getantposns.correct_ant_posns(vis,False)
                if len(antenna_offsets)==0:
                   # no offsets were found
                   raise Exception, 'No offsets found. The caltable is not created'
                if antenna_offsets[0] == 0:
                   antenna = antenna_offsets[1]
                   parameter = antenna_offsets[2] 
                else:
                   raise Exception, 'No lookup information'
              
              cb.specifycal(caltable=caltable,time="",spw=spw,antenna=antenna,pol=pol,
                            caltype=caltype,parameter=parameter)

              cb.close()

       except Exception, instance:
              #print '*** Error ***',instance
              cb.close()
              raise Exception, instance

