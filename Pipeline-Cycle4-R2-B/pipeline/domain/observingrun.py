from __future__ import absolute_import
import datetime
import itertools
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.utils as utils

LOG = infrastructure.get_logger(__name__)


class ObservingRun(object):
    def __init__(self):        
        self.measurement_sets = []

    def add_measurement_set(self, ms):
        if ms.basename in [m.basename for m in self.measurement_sets]:
            msg = '{0} is already in the pipeline context'.format(ms.name)
            LOG.error(msg)
            raise Exception, msg
        
        self.measurement_sets.append(ms)
            
    def get_ms(self, name=None, intent=None):
        """Returns the first measurement set matching the given identifier. 
        Identifier precedence is name then intent.
        """
        if name:
            for ms in self.measurement_sets:
                if name in (ms.name, ms.basename):
                    return ms
            
            for ms in self.measurement_sets:
                # single dish data are registered without the MS suffix
                with_suffix = '%s.ms' % name
                if with_suffix in (ms.name, ms.basename):
                    return ms
            
            raise KeyError, ('No measurement set found with '
                             'name {0}'.format(name))

        if intent:
            # Remove any extraneous characters, as intent could be specified
            # as *BANDPASS* for example
            intent = intent.replace('*','')
            for ms in self.measurement_sets:
                for field in ms.fields:
                    if intent in field.intents:
                        return ms
            raise KeyError, ('No measurement set found with '
                             'intent {0}'.format(intent))

    def get_measurement_sets(self, names=None, intents=None, fields=None, imaging_preferred=False):
        """
        Returns measurement sets matching the given arguments.
        """
        candidates = self.measurement_sets

        # filter out MeasurementSets with no vis hits
        if names is not None:
            candidates = [ms for ms in candidates
                          if ms.name in names]

        # filter out MeasurementSets with no intent hits
        if intents is not None:
            if type(intents) is types.StringType:
                intents = utils.safe_split(intents)
            intents = set(intents)

            candidates = [ms for ms in candidates
                          if intents.issubset(ms.intents)]

        # filter out MeasurementSets with no field name hits
        if fields is not None:
            if type(fields) is types.StringType:
                fields = utils.safe_split(fields)
            fields_to_match = set(fields)

            candidates = [ms for ms in candidates
                          if fields_to_match.isdisjoint({field.name for field in ms.fields})]

        # When requested, and if any imaging MeasurementSets have been
        # registered with the context, filter out the non-imaging objects
        if imaging_preferred:
            imaging_flags = [getattr(ms, 'is_imaging_ms', False) for ms in candidates]
            if any(imaging_flags):
                candidates = [ms for ms, is_imaging_ms in zip(candidates, imaging_flags)
                              if is_imaging_ms]

        return candidates

    def get_fields(self, names=None):
        """
        Returns fields matching the given arguments from all measurement sets.
        """        
        match = [ms.fields for ms in self.measurement_sets]
        # flatten the fields lists to one sequence
        match = itertools.chain(*match)
        
        if names is not None:
            if type(names) is types.StringType:
                names = utils.safe_split(names)
            names = set(names)
            match = [f for f in match if f.name in names] 

        return match
    
    @property
    def start_time(self):
        if not self.measurement_sets:
            return None
        qt = casatools.quanta
        s = sorted(self.measurement_sets, 
                   key=lambda ms: ms.start_time['m0'],
                   cmp=lambda x,y: 1 if qt.gt(x,y) else 0 if qt.eq(x,y) else -1)
        return s[0].start_time

    @property
    def start_datetime(self):
        if not self.start_time:
            return None
        qt = casatools.quanta
        mt = casatools.measures
        s = qt.time(mt.getvalue(self.start_time)['m0'], form=['fits'])
        #return datetime.datetime.strptime(s, '%Y-%m-%dT%H:%M:%S')
        return datetime.datetime.strptime(s[0], '%Y-%m-%dT%H:%M:%S')

    @property
    def end_time(self):
        if not self.measurement_sets:
            return None
        qt = casatools.quanta
        s = sorted(self.measurement_sets, 
                   key=lambda ms: ms.end_time['m0'],
                   cmp=lambda x,y: 1 if qt.gt(x,y) else 0 if qt.eq(x,y) else -1)
        return s[-1].end_time

    @property
    def end_datetime(self):
        if not self.end_time:
            return None
        qt = casatools.quanta
        mt = casatools.measures
        s = qt.time(mt.getvalue(self.end_time)['m0'], form=['fits'])
        #return datetime.datetime.strptime(s, '%Y-%m-%dT%H:%M:%S')
        return datetime.datetime.strptime(s[0], '%Y-%m-%dT%H:%M:%S')

    @property
    def project_ids(self):
        return set([ms.project_id for ms in self.measurement_sets])
    
    @property
    def schedblock_ids(self):
        return set([ms.schedblock_id for ms in self.measurement_sets])
    
    @property
    def execblock_ids(self):
        return set([ms.execblock_id for ms in self.measurement_sets])
        
    @property
    def observers(self):
        return set([ms.observer for ms in self.measurement_sets])
