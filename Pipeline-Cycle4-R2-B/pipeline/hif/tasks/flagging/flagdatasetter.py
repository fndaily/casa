"""
The flagdatasetter module interfaces hid heuristic flaggers to CASA flagdata.
"""
from __future__ import absolute_import
import os
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
from pipeline.hif.tasks.common.arrayflaggerbase import FlagCmd

# the logger for this module
LOG = infrastructure.get_logger(__name__)


class FlagdataSetterInputs(basetask.StandardInputs):
    """
    FlagdataSetter manages the inputs for the FlagdataSetter task.
    """    

    def __init__(self, context, table, vis=None, output_dir=None, inpfile=None): 
        """
        Initialise the Inputs, initialising any property values to those given
        here.
        
        :param context: the pipeline Context state object
        :type context: :class:`~pipeline.infrastructure.launcher.Context`
        :param table: the measurement set or caltable to flag
        :type table: a string
        :param output_dir: the output directory for pipeline data
        :type output_dir: string
        :param inpfile: file with flagcmds
        :type inpfile: string
        """        
        self._init_properties(vars())

    @property
    def inpfile(self):
        if type(self.table) is types.ListType:
            return self._handle_multiple_vis('inpfile')
        return self._inpfile

    @inpfile.setter
    def inpfile(self, value):
        self._inpfile = value
    

class FlagdataSetterResults(basetask.Results):
    def __init__(self, jobs=[], results=[]):
        """
        Initialise the results object with the given list of JobRequests.
        """
        super(FlagdataSetterResults, self).__init__()
        self.jobs = jobs
        self.results = results

    def __repr__(self):
        s = 'flagdata results:\n'
        for job in self.jobs:
            s += '%s performed.' % str(job)
        return s 


class FlagdataSetter(basetask.StandardTaskTemplate):
    """
    """
    Inputs = FlagdataSetterInputs

    def prepare(self):
        """
        Prepare and execute a flagdata flagging job appropriate to the
        task inputs.
        """
        inputs = self.inputs
        
        # to save inspecting the file, also log the flag commands
        LOG.debug('Flag commands for %s:' % inputs.table)
        if type(inputs.inpfile) is types.ListType:
            if inputs.inpfile:
                # create a flagdata job
                job = casa_tasks.flagdata(vis=inputs.table, mode='list', action='apply',
                  inpfile=inputs.inpfile, savepars=False, flagbackup=False,
                  reason='any')
                jobs = [job]
            else:
                LOG.debug('no flagcmds generated by this stage')
                jobs = []

        else:
            raise Exception, 'flag commands not in list'

        # execute any jobs
        results = []
        for job in jobs:
            results.append(self._executor.execute(job))
        
        return FlagdataSetterResults(jobs, results)

    def analyse(self, results):
        """
        """
        return results

    def flags_to_set(self, flags):
        """
        Set the list of flags.
        """
        inputs = self.inputs
        
        if type(inputs.inpfile) is types.ListType:
            # Always start with an empty list of flagcmds,
            # i.e. do not retain what flagcmds were added 
            # previously (e.g. for previous calls to 
            # FlagdataSetter).
            inputs.inpfile = []
            
            # Add to list of flagcmds.
            for flag in flags:
                if isinstance(flag, FlagCmd):
                    inputs.inpfile.append(flag.flagcmd)
                else:
                    inputs.inpfile.append(flag)
        else:
            raise Exception, 'flagcmds not as list'
