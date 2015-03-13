from __future__ import absolute_import
import types
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
from pipeline.infrastructure import casa_tasks
from pipeline.hif.heuristics import caltable as caltable_heuristic
from pipeline.hif.heuristics import wvrgcal as wvrgcal_heuristic

from .. import bandpass
from .. import gaincal
from pipeline.hif.tasks.common import arrayflaggerbase
from . import resultobjects 
from . import wvrg_qa2

LOG = infrastructure.get_logger(__name__)


class WvrgcalInputs(basetask.StandardInputs):
    
    def __init__(self, context, output_dir=None, vis=None,
      caltable=None, hm_toffset=None, toffset=None, segsource=None, 
      hm_tie=None, tie=None, sourceflag=None, nsol=None,
      disperse=None, wvrflag=None, hm_smooth=None, smooth=None,
      scale=None, maxdistm=None, minnumants=None, qa2_intent=None,
      qa2_bandpass_intent=None, accept_threshold=None, 
      bandpass_result=None, nowvr_result=None):
        self._init_properties(vars())

    @property
    def hm_toffset(self):
        if self._hm_toffset is None:
            return 'automatic'
        return self._hm_toffset

    @hm_toffset.setter
    def hm_toffset(self, value):
        self._hm_toffset = value

    @property
    def toffset(self):
        if self._toffset is None:
            return 0
        return self._toffset

    @toffset.setter
    def toffset(self, value):
        self._toffset = value

    @property
    def segsource(self):
        if self._segsource is None:
            return True
        return self._segsource

    @segsource.setter
    def segsource(self, value):
        self._segsource = value

    @property
    def hm_tie(self):
        if self._hm_tie is None:
            return 'automatic'
        return self._hm_tie

    @hm_tie.setter
    def hm_tie(self, value):
        self._hm_tie = value

    @property
    def tie(self):
        if self._tie is None:
            return []
        return self._tie

    @tie.setter
    def tie(self, value):
        self._tie = value

    @property
    def sourceflag(self):
        if self._sourceflag is None:
            return []
        return self._sourceflag

    @sourceflag.setter
    def sourceflag(self, value):
        self._sourceflag = value

    @property
    def nsol(self):
        if self._nsol is None:
            return 1
        return self._nsol

    @nsol.setter
    def nsol(self, value):
        self._nsol = value

    @property
    def disperse(self):
        if self._disperse is None:
            return False
        return self._disperse

    @disperse.setter
    def disperse(self, value):
        self._disperse = value

    @property
    def wvrflag(self):
        return self._wvrflag

    @wvrflag.setter
    def wvrflag(self, value):
        if value is None:
            self._wvrflag = []
        elif type(value) is types.StringType:
            if value == '':
                self._wvrflag = []
            else:
                if value[0] == '[':
                    strvalue=value.replace('[','').replace(']','').replace("'","")
                else:
                    strvalue = value
                self._wvrflag = list(strvalue.split(','))
        else:
            self._wvrflag = value

    @property
    def hm_smooth(self):
        if self._hm_smooth is None:
            return 'automatic'
        return self._hm_smooth

    @hm_smooth.setter
    def hm_smooth(self, value):
        self._hm_smooth = value

    @property
    def smooth(self):
        if self._smooth is None:
            return '1s'
        return self._smooth

    @smooth.setter
    def smooth(self, value):
        self._smooth = value

    @property
    def scale(self):
        if self._scale is None:
            return 1.0
        return self._scale

    @scale.setter
    def scale(self, value):
        self._scale = value

    @property
    def maxdistm(self):
        if self._maxdistm is None:
            return 500.0
        return self._maxdistm

    @maxdistm.setter
    def maxdistm(self, value):
        self._maxdistm = value

    @property
    def minnumants(self):
        if self._minnumants is None:
            return 2
        return self._minnumants

    @minnumants.setter
    def minnumants(self, value):
        self._minnumants = value

    @property
    def qa2_intent(self):
        if self._qa2_intent is None:
            return ''
        return self._qa2_intent

    @qa2_intent.setter
    def qa2_intent(self, value):
        self._qa2_intent = value

    @property
    def qa2_bandpass_intent(self):
        if self._qa2_bandpass_intent is None:
            return 'BANDPASS'
        return self._qa2_bandpass_intent

    @qa2_bandpass_intent.setter
    def qa2_bandpass_intent(self, value):
        self._qa2_bandpass_intent = value

    @property
    def accept_threshold(self):
        if self._accept_threshold is None:
            return 1.0
        return self._accept_threshold

    @accept_threshold.setter
    def accept_threshold(self, value):
        self._accept_threshold = value


class Wvrgcal(basetask.StandardTaskTemplate):
    Inputs = WvrgcalInputs

    def __init__(self, inputs):
        super(Wvrgcal, self).__init__(inputs)
    
    def prepare(self):
        inputs = self.inputs
        jobs = []

        # get parameters that can be set from outside or which will be derived
        # from heuristics otherwise
        wvrheuristics = wvrgcal_heuristic.WvrgcalHeuristics(
          context=inputs.context, vis=inputs.vis, hm_tie=inputs.hm_tie, 
          tie=inputs.tie, hm_smooth = inputs.hm_smooth, smooth=inputs.smooth,
          sourceflag=inputs.sourceflag, nsol=inputs.nsol,
          segsource=inputs.segsource)

        # return an empty results object if no WVR data available
        if not wvrheuristics.wvr_available():
            LOG.warning('WVR data not available')
            return resultobjects.WvrgcalResult(vis=inputs.vis)

        if inputs.hm_toffset == 'automatic':
            toffset = wvrheuristics.toffset()
        else:
            toffset = inputs.toffset

        if inputs.segsource is None:
            segsource = wvrheuristics.segsource()
        else:
            segsource = inputs.segsource

        if inputs.hm_tie == 'automatic':
            tie = wvrheuristics.tie()
        else:
            tie = inputs.tie

        if inputs.sourceflag is None:
            sourceflag = wvrheuristics.sourceflag()
        else:
            sourceflag = inputs.sourceflag

        if inputs.nsol is None:
            nsol = wvrheuristics.nsol()
        else:
            nsol = inputs.nsol

        # get parameters that must be set from outside
        disperse = inputs.disperse
        wvrflag = inputs.wvrflag
        scale = inputs.scale
	maxdistm = inputs.maxdistm
	minnumants = inputs.minnumants

        # smooth may vary with spectral window so need to ensure we calculate
        # results that can cover them all
        ms = inputs.context.observing_run.get_ms(name=inputs.vis)
        spws = ms.spectral_windows
        science_spwids = [spw.id for spw in spws
          if spw.num_channels not in (1,4) and not spw.intents.isdisjoint(
          ['BANDPASS', 'AMPLITUDE', 'PHASE', 'TARGET'])]

        smooths_done = set()
        callist = []
        caltables = []
        for spw in science_spwids:
            if inputs.hm_smooth == 'automatic':
                smooth = wvrheuristics.smooth(spw)
            else:
                smooth = inputs.smooth

            # prepare to run the wvrgcal task if necessary
            caltable = caltable_heuristic.WvrgCaltable()
            caltable = caltable(output_dir=inputs.output_dir,
              stage=inputs.context.stage,
              vis=inputs.vis, smooth=smooth)
            if smooth not in smooths_done:
                # different caltable for each smoothing, remove old versions
                os.system('rm -fr %s' % caltable)

                jobs.append(casa_tasks.wvrgcal(vis=inputs.vis,
                  caltable=caltable, toffset=toffset, segsource=segsource,
                  tie=tie, sourceflag=sourceflag, nsol=nsol,
                  disperse=disperse, wvrflag=wvrflag,
                  smooth=smooth, scale=scale, maxdistm=maxdistm,
		  minnumants=minnumants))

                smooths_done.add(smooth)

            # add this wvrg table to the callibrary for this spw
            calto = callibrary.CalTo(vis=inputs.vis, spw=spw)
            calfrom = callibrary.CalFrom(caltable, caltype='wvr', spwmap=[],
              interp='nearest', calwt=False)
            calapp = callibrary.CalApplication(calto, calfrom)
            callist.append(calapp)
            caltables.append(caltable)

        # execute the jobs
        for job in jobs:
            self._executor.execute(job)
        LOG.info('wvrgcal complete')

        # removed any 'unsmoothed' wvrcal tables generated by the wvrgcal jobs
        for caltable in caltables:
            os.system('rm -fr %s_unsmoothed' % caltable)       

        return resultobjects.WvrgcalResult(vis=inputs.vis, pool=callist,
          wvrflag=inputs.wvrflag)

    def analyse(self, result):
        inputs = self.inputs

        # double-check that the caltable was actually generated
        on_disk = [ca for ca in result.pool
                   if ca.exists() or self._executor._dry_run]
        missing = [ca for ca in result.pool
                   if ca not in on_disk and not self._executor._dry_run]
        result.error.clear()
        result.error.update(missing)

        # wvrcal files to be applied
        result.final[:] = on_disk

        # calculate the qa2 results if required
        if inputs.qa2_intent.strip() != '':

            if not len(result.final):
                LOG.warning(
                  'qa2: no wvr available, cannot calculate qa2 results')
            else:
                LOG.info('qa2: calculate B and accept into copy of context')
                bandpassinputs = bandpass.PhcorBandpass.Inputs(
                  context=inputs.context, output_dir=inputs.output_dir,
                  vis=inputs.vis, mode='channel',
                  intent=inputs.qa2_bandpass_intent,
                  solint='inf,7.8125MHz', maxchannels=0,
                  qa2_intent='', run_qa2=False)
                
                if inputs.bandpass_result is not None:
                    # table already exists use it
                    LOG.info('qa2: reusing B calibration result: \n%s' %
                      inputs.bandpass_result)
                    bp_result = inputs.bandpass_result
                    bp_result.accept(inputs.context)
                    result.bandpass_result = bp_result
                else:
                    # calculate the B result and store it in the context
                    # scratch area
                    bandpasstask = bandpass.PhcorBandpass(bandpassinputs)
                    bp_result = self._executor.execute(bandpasstask,
                      merge=True)
                    LOG.info('bandpass complete')
                    result.bandpass_result = bp_result

                # do a phase calibration on the bandpass and phase
                # calibrators with B preapplied
                LOG.info('qa2: calculating phase calibration with B applied')
                gaincalinputs = gaincal.GTypeGaincal.Inputs(
                  context=inputs.context,
                  output_dir=inputs.output_dir, vis=inputs.vis,
                  intent=inputs.qa2_intent, solint='int', calmode='p',
                  minsnr=0.0)
                caltable = gaincalinputs.caltable
                # give the qa2 gain table a 'nowvr' component
                nowvr_caltable = caltable.replace('.tbl', '.nowvr.tbl')

                if inputs.nowvr_result is not None:
                    # table already exists use it
                    LOG.info('qa2: reusing wvr-not-corrected gain result for rms:\n %s' %
                      inputs.nowvr_result)
                    nowvr_result = inputs.nowvr_result
                    result.nowvr_result = nowvr_result
                else:
                    gaincalinputs.caltable = nowvr_caltable
                    LOG.info('qa2: wvr-not-corrected phase rms gain table is %s' %
                      os.path.basename(nowvr_caltable))
                    gaincaltask = gaincal.GTypeGaincal(gaincalinputs)
                    self.gresults_nowvr = self._executor.execute(gaincaltask,
                      merge=False)
                    LOG.info('gaincal complete')
                    result.nowvr_result = self.gresults_nowvr

                LOG.info('qa2: accept wvr results into copy of context')
                result.accept(inputs.context)

                # do a phase calibration on the bandpass and phase calibrators 
                # with B and wvr preapplied.
                LOG.info('qa2: calculating phase calibration with B and wvr applied')
                gaincalinputs = gaincal.GTypeGaincal.Inputs(
                  context=inputs.context, output_dir=inputs.output_dir,
                  vis=inputs.vis, intent=inputs.qa2_intent, solint='int',
                  calmode='p', minsnr=0.0)
                caltable = gaincalinputs.caltable
                # give the qa2 gain table name a flag and 'wvr' component
                flagname = '_'.join(inputs.wvrflag)
                if flagname:
                    flagname = '.flag%s' % flagname
                wvr_caltable = caltable.replace('.tbl', '%s.wvr.tbl' % flagname)
                gaincalinputs.caltable = wvr_caltable
                LOG.info('qa2: wvr-corrected phase rms gain table is %s' %
                  os.path.basename(wvr_caltable))

                gaincaltask = gaincal.GTypeGaincal(gaincalinputs)
                self.gresults_wvr = self._executor.execute(gaincaltask,
                  merge=False)
                LOG.info('gaincal complete')

                LOG.info('qa2: calculate ratio no-wvr phase rms / with-wvr phase rms')
                result.qa2.gaintable_wvr = wvr_caltable
                wvrg_qa2.calculate_view(inputs.context, nowvr_caltable,
                  wvr_caltable, result.qa2)

                wvrg_qa2.calculate_qa2_numbers(result.qa2)

                # if the qa2 score indicates that applying the wvrg file will
                # make things worse then remove it from the results so that
                # it cannot be accepted into the context.
                if result.qa2.overall_score < inputs.accept_threshold:
                    LOG.warning(
                      'wvrgcal has qa2 score (%s) below accept_threshold (%s) and will not be applied' %
                      (result.qa2.overall_score, inputs.accept_threshold))
                    result.final = []

        return result
