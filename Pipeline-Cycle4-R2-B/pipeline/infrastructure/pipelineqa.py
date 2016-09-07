from __future__ import absolute_import
import abc
import operator

from . import logging

LOG = logging.get_logger(__name__)


class QAScore(object):
    def __init__(self, score, longmsg='', shortmsg='', vis=None):
        self.score = score
        self.longmsg = longmsg
        self.shortmsg = shortmsg
        
        # set target to be a dict, as we expect some targets to be sessions,
        # some to be MSes, some to be whole runs, etc.
        self.target = {}
        # ... but for now we just handle MSes
        if vis is not None:
            self.target['vis'] = vis

    def __repr__(self):
        return 'QAScore(%s, "%s", "%s", %s)' % (self.score, self.longmsg, 
                                                self.shortmsg, self.target)


class QAScorePool(object):
    all_unity_longmsg = 'All QA completed successfully'
    all_unity_shortmsg = 'QA pass'
    
    def __init__(self):
        self.pool = []
        self._representative = None

    @property
    def representative(self):
        if self._representative is not None:
            return self._representative
        
        if not self.pool:
            return QAScore(None, 'No QA scores registered for this task', 'No QA')
        
        # if all([s.score >= 0.9 for s in self.pool]):
        #     return QAScore(min([s.score for s in self.pool]), self.all_unity_longmsg, self.all_unity_shortmsg)

        # maybe have different algorithms here. for now just return the
        # QAScore with minimum score
        return min(self.pool, key=operator.attrgetter('score'))

    @representative.setter
    def representative(self, value):
        self._representative = value


class QAResultHandler(object):
    __metaclass__ = abc.ABCMeta

    # the Results class this handler is expected to handle
    result_cls = None
    # if result_cls is a list, the type of classes it is expected to contain
    child_cls = None
    # the task class that generated the results, or None if it should handle
    # all results of this type regardless of which task generated it
    generating_task = None

    def is_handler_for(self, result):
        # if the result is not a list or the expected results class,
        # return False
        if not isinstance(result, self.result_cls):
            return False
        
        # this is the expected class and we weren't expecting any
        # children, so we should be able to handle the result
        if self.child_cls is None and (self.generating_task is None 
                                       or result.task is self.generating_task):
            return True

        try:
            if all([isinstance(r, self.child_cls) and 
                    (self.generating_task is None or r.task is self.generating_task)
                    for r in result]):
                return True
            return False
        except:
            # catch case when result does not have a task attribute
            return False

    @abc.abstractmethod
    def handle(self, context, result):
        pass


class QAHandler(object):
    def __init__(self):
        self.__handlers = []

    def add_handler(self, handler):
        task = handler.generating_task.__name__ if handler.generating_task else 'all'
        #container = 's of %s' % handler.child_cls.__name__ if handler.child_cls else ''
        child_name = ''
        if hasattr(handler.child_cls, '__name__'):
            child_name = handler.child_cls.__name__
        elif type(handler.child_cls) in (tuple, list):
            child_name = str(map(lambda x: x.__name__, handler.child_cls))
        container = 's of %s' % child_name
        s = '%s%s results generated by %s tasks' % (handler.result_cls.__name__,
                                                    container, task)
        LOG.debug('Registering %s as new pipeline QA handler for %s', 
                  handler.__class__.__name__, s)
        self.__handlers.append(handler)

    def do_qa(self, context, result):
        # if this result is a list, process the lower-level scalar results
        # first
        if isinstance(result, list):
            for r in result:
                self.do_qa(context, r)

        # so that the upper-level handler can collate the lower-level scores
        # or process as a group
        for handler in self.__handlers:
            if handler.is_handler_for(result):
                LOG.debug('%s handling QA analysis for %s' % (handler.__class__.__name__,
                                                              result.__class__.__name__))
                handler.handle(context, result)


registry = QAHandler()
