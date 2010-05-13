import os
from taskinit import *
from simutil import *
import pylab as pl
import pdb

def simdata2(
    project=None, 
    modifymodel=None,
    skymodel=None, inbright=None, indirection=None, incell=None, 
    incenter=None, inwidth=None, # innchan=None,
    setpointings=None,
    ptgfile=None, integration=None, direction=None, mapsize=None, 
    maptype=None, pointingspacing=None, caldirection=None, calflux=None, 
    predict=None, 
    refdate=None, complist=None, totaltime=None, antennalist=None, 
    sdantlist=None, sdant=None,
    thermalnoise=None,
    user_pwv=None, t_ground=None, t_sky=None, tau0=None, leakage=None,
    image=None,
    vis=None, modelimage=None, cell=None, imsize=None, niter=None, threshold=None,
    weighting=None, outertaper=None, stokes=None,     
    analyze=None, 
    showarray=None, showuv=None, showpsf=None, showmodel=None, 
    showconvolved=None, showclean=None, showresidual=None, showdifference=None, 
    showfidelity=None,
    graphics=None,
    verbose=None, 
    overwrite=None,
    async=False):


    # RI TODO for inbright=unchanged, need to scale input image to jy/pix

    casalog.origin('simdata')
    if verbose: casalog.filter(level="DEBUG2")

    # some hardcoded variables that may be reintroduced in future development
    relmargin=.5  # number of PB between edge of model and pointing centers
    scanlength=1  # number of integrations per scan

    # as of 20100507, newfig will delete the previous one

    # create the utility object:
    util=simutil(direction)  # this is the dir of the observation - could be ""
    if verbose: util.verbose=True
    msg=util.msg
    
    if type(skymodel)==type([]):
        skymodel=skymodel[0]
    skymodel=skymodel.replace('$project',project)
            
    if((not os.path.exists(skymodel)) and (not os.path.exists(complist))):
        msg("No sky input found.  At least one of skymodel or complist must be set.",priority="error")
        return False

    grscreen=False
    grfile=False
    if graphics=="both":
        grscreen=True
        grfile=True
    if graphics=="screen":
        grscreen=True
    if graphics=="file":
        grfile=True
    
#    try:
    if True:

        if type(complist)==type([]):
            complist=complist[0]

        ##################################################################
        # set up skymodelimage
        if os.path.exists(skymodel):
            components_only=False

            # if the skymodel is okay, work from it directly
            if util.is4d(skymodel) and not modifymodel:
                newmodel=skymodel
            else:
                # otherwise create $newmodel
                default_model=project+".skymodel"
                if skymodel==default_model:
                    newmodel=project+".newmodel"
                else:
                    newmodel=default_model
                if os.path.exists(newmodel):
                    if overwrite:
                        shutil.rmtree(newmodel)
                    else:
                        msg(newmodel+" exists -- please delete it, change skymodel, or set overwrite=T",priority="error")
                        return False

            # modifymodel just collects info if skymodel==newmodel
            innchan=-1
            (model_refdir,model_cell,model_size,
             model_nchan,model_center,model_width,
             model_stokes) = util.modifymodel(skymodel,
             newmodel,modifymodel,inbright,indirection,incell,
             incenter,inwidth,innchan,
             flatimage=False) 

            modelflat=newmodel+".flat"
            if os.path.exists(modelflat) and (not predict) and analyze:
                # if we're not predicting, then we want to use the previously
                # created modelflat, because it may have components added 
                msg("flat sky model "+modelflat+" exists, predict not requested",priority="warn")
                msg(" working from existing image - please delete it if you wish to overwrite.",priority="warn")
            else:
                # create and add components into modelflat with util.flatimage()
                util.flatimage(newmodel,complist=complist,verbose=verbose)

            casalog.origin('simdata')

            # set startfeq and bandwidth in util object after modifymodel
            bandwidth=qa.mul(qa.quantity(model_nchan),qa.quantity(model_width))
            util.bandwidth=bandwidth

            if len(mapsize)==0:
                mapsize=model_size
                if verbose: msg("setting map size to "+str(model_size))
            else:
                 if type(mapsize)==type([]):
                     if len(mapsize[0])==0:
                         mapsize=model_size
                         if verbose: msg("setting map size to "+str(model_size))

        else:
            # if there are only components, modifymodel=T doesn't 
            # make sense
            if modifymodel:
                msg("can't find model image "+skymodel+" to modify",priority="error")
                return False
            components_only=True
            # if only components, the pointings 
            # can be displayed on blank sky, with symbols at the locations 
            # of components, but if analysis is going to be peformed, 
            # TODO create a sky model image here!!!

            # TODO we need model_center below for calibrator, so get that from 
            # components





        ##################################################################
        # set up pointings
        dir=model_refdir
        dir0=dir
        if type(direction)==type([]):
            if len(direction)>0:
                if util.isdirection(direction[0],halt=False):
                    dir=direction
                    dir0=direction[0]
        else:
            if util.isdirection(direction,halt=False):
                dir=direction
                dir0=dir
        util.direction=dir0

        if setpointings:
            if verbose:
                util.msg("calculating map pointings centered at "+str(dir0))
            pointings = util.calc_pointings2(pointingspacing,mapsize,maptype=maptype, direction=dir)
            nfld=len(pointings)
            etime = qa.convert(qa.quantity(integration),"s")['value']
            ptgfile = project+".ptg.txt"
        else:
            if type(ptgfile)==type([]):
                ptgfile=ptgfile[0]
            ptgfile=ptgfile.replace('$project',project)
            nfld, pointings, etime = util.read_pointings(ptgfile)
            if max(etime) <=0:
                etime = qa.convert(qa.quantity(integration),"s")['value']
            # expects that the cal is separate, and this is just one round of the mosaic

        # find imcenter - phase center
        imcenter , offsets = util.average_direction(pointings)        
        epoch, ra, dec = util.direction_splitter(imcenter)

        # model is centered at model_refdir, and has model_size; this is the offset in 
        # angular arcsec from the model center to the imcenter:        
        mepoch, mra, mdec = util.direction_splitter(model_refdir)
        shift = [ (qa.convert(ra,'deg')['value'] - 
                   qa.convert(mra,'deg')['value'])/pl.cos(qa.convert(mdec,'rad')['value'] ), 
                  (qa.convert(dec,'deg')['value']-qa.convert(mdec,'deg')['value']) ]
        if verbose: 
            msg("pointings are shifted relative to the model by %g,%g arcsec" % (shift[0]*3600,shift[1]*3600))
        xmax=qa.convert(model_size[0],'deg')['value']
        ymax=qa.convert(model_size[1],'deg')['value']
        overlap=False        
        for i in range(offsets.shape[1]):
            xc= pl.absolute(offsets[0,i]+shift[0])  # offsets and shift are in degrees
            yc= pl.absolute(offsets[1,i]+shift[1])
            if xc<xmax and yc<ymax:
                overlap=True
                break

        if setpointings:
            if os.path.exists(ptgfile):
                if overwrite:
                    os.remove(ptgfile)
                else:
                    util.msg("pointing file "+ptgfile+" already exists and user does not want to overwrite",priority="error")
            util.write_pointings(ptgfile,pointings,etime)

        msg("phase center = " + imcenter)
        if nfld>1 and verbose:
            for dir in pointings:
                msg("   "+dir)
 
        if not overlap:
            msg("No overlap between model and pointings",priority="error")
            return False



        ##################################################################
        # calibrator is not explicitly contained in the pointing file
        # but interleaved with etime=intergration
        util.isquantity(calflux)
        calfluxjy=qa.convert(calflux,'Jy')['value']
        # XML returns a list even for a string:
        if type(caldirection)==type([]): caldirection=caldirection[0]
        if len(caldirection)<4: caldirection=""
        if calfluxjy > 0 and caldirection != "":            
            docalibrator=True
            util.isdirection(caldirection)
            cl.done()
            cl.addcomponent(flux=calfluxjy,dir=caldirection,label="phase calibrator")
            # set reference freq to center freq of model
            cl.rename(project+'.cal.cclist')
            cl.done()
        else:
            docalibrator=False





        ##################################################################
        # read antenna file here to get Primary Beam
        predict_uv=False
        predict_sd=False
        tp_only=False
        aveant=-1
        stnx=[]  # for later, to know if we read an array in or not

        # experimental: alma;0.4arcsec  allowed string
        if str.upper(antennalist[0:4])=="ALMA":
            tail=antennalist[5:]
            if util.isquantity(tail,halt=False):
                resl=qa.convert(tail,"arcsec")['value']
                repodir=os.getenv("CASAPATH").split(' ')[0]+"/data/alma/simmos/"
                if os.path.exists(repodir):
                    confnum=(2.867-pl.log10(resl*1000*qa.convert(model_center,"GHz")['value']/672.))/0.0721
                    confnum=max(1,min(28,confnum))
                    conf=str(int(round(confnum)))
                    if len(conf)<2: conf='0'+conf
                    antennalist=repodir+"alma.out"+conf+".cfg"
                    msg("converted resolution to antennalist "+antennalist)

        if os.path.exists(antennalist):
            stnx, stny, stnz, stnd, padnames, nant, telescopename = util.readantenna(antennalist)
            antnames=[]
            for k in range(0,nant): antnames.append('A%02d'%k)
            aveant=stnd.mean()
            # TODO use max ant = min PB instead?  
            # (set back to simdata - there must be an automatic way to do this)
            casalog.origin('simdata')
            predict_uv=True
            
        if os.path.exists(sdantlist):
            tpx, tpy, tpz, tpd, tp_padnames, tp_nant, tp_telescopename = util.readantenna(sdantlist)
            tp_antnames=[]
            #for k in range(0,tp_nant): tp_antnames.append('TP%02d'%k)
            #select an antenna from thelist
            if sdant > tp_nant-1:
                msg("antenna index %d is out of range. setting sdant=0"%sdant,priority="warn")
                sdant=0
            tp_antnames.append('TP%02d'%sdant)
            tpx=[tpx[sdant]]
            tpy=[tpy[sdant]]
            tpz=[tpz[sdant]]
            tpd=pl.array(tpd[sdant])
            tp_padnames=[tp_padnames[sdant]]
            tp_nant=1
            tp_aveant=tpd.mean()
            casalog.origin('simdata')
            predict_sd=True
            if not predict_uv:
                aveant=tp_aveant
                tp_only=True
        




        ##################################################################
        # create one figure for model and pointings - need antenna diam 
        # to determine primary beam
        if modifymodel or setpointings:
            if grfile:
                file=project+".skymodel.png"
            else:
                file=""                            
    
            if grscreen or grfile:
                util.newfig(show=grscreen)

                if components_only:
                    pl.plot()
                    # TODO add symbols at locations of components
                else:
                    discard = util.statim(modelflat,plot=True,incell=model_cell)
                lims=pl.xlim(),pl.ylim()
                pb=1.2*0.3/qa.convert(qa.quantity(model_center),'GHz')['value']/aveant*3600.*180/pl.pi                
                if pb<=0 and verbose:
                    msg("unknown primary beam size for plot",priority="warn")
                if max(max(lims)) > pb/2:
                    plotcolor='w'
                else:
                    plotcolor='k'

                if offsets.shape[1]>18 or pb<=0:
                    pl.plot((offsets[0]+shift[0])*3600.,(offsets[1]+shift[1])*3600.,
                            plotcolor+'+',markeredgewidth=1)
                    if pb>0:
                        plotpb(pb,pl.gca(),lims=lims)
                else:
                    from matplotlib.patches import Circle
                    for i in range(offsets.shape[1]):
                        pl.gca().add_artist(Circle(
                            ((offsets[0,i]+shift[0])*3600,
                             (offsets[1,i]+shift[1])*3600),
                            radius=pb/2.,edgecolor=plotcolor,fill=False,
                            label='beam',transform=pl.gca().transData))

                xlim=max(abs(pl.array(lims[0])))
                ylim=max(abs(pl.array(lims[1])))
                # show entire pb: (statim doesn't by default)
                pl.xlim([max([xlim,pb/2]),min([-xlim,-pb/2])])
                pl.ylim([min([-ylim,-pb/2]),max([ylim,pb/2])])            
                pl.xlabel("resized model sky",fontsize="x-small")
                util.endfig(show=grscreen,filename=file)
    







        ##################################################################
        # set up observatory, feeds, etc        
        quickpsf_current=False

        msfile=project+'.ms'
        sdmsfile=project+'.sd.ms'
        if predict:
            if not(predict_uv or predict_sd):
                util.msg("must specify at least one of antennalist, sdantlist",priority="error")
            # TODO check for frequency overlap here - if zero stop
            # position overlap already checked above in pointing section

            if verbose:
                msg("preparing empty measurement set",origin="simdata",priority="warn")
            else:
                msg("preparing empty measurement set",origin="simdata")

            nbands = 1;    
            fband  = 'band'+qa.tos(model_center,prec=1)

            ############################################
            # predict interferometry observation
            if predict_uv: 
                if os.path.exists(msfile):
                    if not overwrite:
                        util.msg("measurement set "+msfile+" already exists and user does not wish to overwrite",priority="error")
                sm.open(msfile)
                posobs=me.observatory(telescopename)
                diam=stnd;
                # WARNING: sm.setspwindow is not consistent with clean::center
                model_start=qa.sub(model_center,qa.mul(model_width,0.5*model_nchan))

                sm.setconfig(telescopename=telescopename, x=stnx, y=stny, z=stnz, 
                             dishdiameter=diam.tolist(), 
                             mount=['alt-az'], antname=antnames, padname=padnames, 
                             coordsystem='global', referencelocation=posobs)
                if str.upper(telescopename).find('VLA')>0:
                    sm.setspwindow(spwname=fband, freq=qa.tos(model_start), 
                                   deltafreq=qa.tos(model_width), 
                                   freqresolution=qa.tos(model_width), 
                                   nchannels=model_nchan, 
                                   stokes='RR LL')
                    sm.setfeed(mode='perfect R L',pol=[''])
                else:            
                    sm.setspwindow(spwname=fband, freq=qa.tos(model_start), 
                                   deltafreq=qa.tos(model_width), 
                                   freqresolution=qa.tos(model_width), 
                                   nchannels=model_nchan, 
                                   stokes='XX YY')
                    sm.setfeed(mode='perfect X Y',pol=[''])

                if verbose: msg(" spectral window set at %s" % qa.tos(model_center))
                sm.setlimits(shadowlimit=0.01, elevationlimit='10deg')
                sm.setauto(0.0)
                for k in range(0,nfld):
                    src=project+'_%d'%k
                    sm.setfield(sourcename=src, sourcedirection=pointings[k],
                                calcode="OBJ", distance='0m')
                    if k==0:
                        sourcefieldlist=src
                    else:
                        sourcefieldlist=sourcefieldlist+','+src
                if docalibrator:
                    sm.setfield(sourcename="phase calibrator", 
                                sourcedirection=caldirection,calcode='C',
                                distance='0m')
                reftime = me.epoch('TAI', refdate)
                sm.settimes(integrationtime=integration, usehourangle=True, 
                            referencetime=reftime)
                totalsec=qa.convert(qa.quantity(totaltime),'s')['value']
                scantime=qa.mul(qa.quantity(integration),str(scanlength))
                scansec=qa.convert(qa.quantity(scantime),'s')['value']
                nscan=int(totalsec/scansec)
                kfld=0
                # RI todo progress meter for simdata Sim::observe

                if nscan<nfld:
                    msg("Only %i pointings of %i in the mosaic will be observed - check mosaic setup and exposure time parameters!" % (nscan,nfld),priority="error")
                    return
        
                # sm.observemany
                observemany=True
                if observemany:
                    srces=[]
                    starttimes=[]
                    stoptimes=[]
                    dirs=[]

                for k in range(0,nscan) :
                    sttime=-totalsec/2.0+scansec*k
                    endtime=sttime+scansec
                    src=project+'_%d'%kfld
                    if observemany:
                        srces.append(src)
                        starttimes.append(str(sttime)+"s")
                        stoptimes.append(str(endtime)+"s")
                        dirs.append(pointings[kfld])
                    else:
                    # this only creates blank uv entries
                        sm.observe(sourcename=src, spwname=fband,
                                   starttime=qa.quantity(sttime, "s"),
                                   stoptime=qa.quantity(endtime, "s"));
                    kfld=kfld+1
                    if kfld==nfld: 
                        if docalibrator:
                            sttime=-totalsec/2.0+scansec*k
                            endtime=sttime+scansec
                            if observemany:
                                srces.append(src)
                                starttimes.append(str(sttime)+"s")
                                stoptimes.append(str(endtime)+"s")
                                dirs.append(caldirection)
                            else:
                                sm.observe(sourcename="phase calibrator", spwname=fband,
                                           starttime=qa.quantity(sttime, "s"),
                                           stoptime=qa.quantity(endtime, "s"));
                        kfld=kfld+1                
                    if kfld > nfld: kfld=0
                # if directions is unset, NewMSSimulator::observemany 
                # looks up the direction in the field table.
                if observemany:
                    sm.observemany(sourcenames=srces,spwname=fband,starttimes=starttimes,stoptimes=stoptimes)

                sm.setdata(fieldid=range(0,nfld))
                sm.setvp()

                msg("done setting up observations (blank visibilities)")
                if verbose:
                    sm.summary()

                # do actual calculation of visibilities:

                if not components_only:                
                    if len(complist)>1:
                        msg("predicting from "+newmodel+" and "+complist,priority="warn")
                    else:
                        msg("predicting from "+newmodel,priority="warn")
                    sm.predict(imagename=newmodel,complist=complist)
                else:   # if we're doing only components
                    msg("predicting from "+complist,priority="warn")
                    sm.predict(complist=complist)
            
                sm.done()        
                msg('generation of measurement set ' + msfile + ' complete')

            ############################################
            # create figure 
            if grfile:            
                file=project+".predict.png"
            else:
                file=""
            if predict_uv:
                multi=[2,2,1]
            else:
                multi=0

            if (grscreen or grfile):
                util.newfig(multi=multi)
                if tp_only: telescopename=tp_telescopename
                util.ephemeris(refdate,direction=util.direction,telescope=telescopename)
                if predict_uv:
                    util.nextfig()
                    util.plotants(stnx, stny, stnz, stnd, padnames)                
                    
                    # uv coverage
                    util.nextfig()
                    tb.open(msfile)  
                    rawdata=tb.getcol("UVW")
                    tb.done()
                    pl.box()
                    maxbase=max([max(rawdata[0,]),max(rawdata[1,])])  # in m
                    klam_m=300/qa.convert(model_center,'GHz')['value']
                    pl.plot(rawdata[0,]/klam_m,rawdata[1,]/klam_m,'b,')
                    pl.plot(-rawdata[0,]/klam_m,-rawdata[1,]/klam_m,'b,')
                    ax=pl.gca()
                    ax.yaxis.LABELPAD=-4
                    pl.xlabel('u[klambda]',fontsize='x-small')
                    pl.ylabel('v[klambda]',fontsize='x-small')
                    pl.axis('equal')

                    # show dirty beam from observed uv coverage
                    util.nextfig()
                    im.open(msfile)  
                    # TODO spectral parms
                    im.defineimage(cellx=qa.tos(model_cell[0]))  
                    #im.makeimage(type='psf',image=project+".quick.psf")
                    if os.path.exists(project+".quick.psf"):
                        shutil.rmtree(project+".quick.psf")
                    im.approximatepsf(psf=project+".quick.psf")
                    quickpsf_current=True
                    beam=im.fitpsf(psf=project+".quick.psf")
                    im.done()
                    ia.open(project+".quick.psf")            
                    beamcs=ia.coordsys()
                    beam_array=ia.getchunk(axes=[beamcs.findcoordinate("spectral")['pixel'],beamcs.findcoordinate("stokes")['pixel']],dropdeg=True)
                    pixsize=(qa.convert(qa.quantity(model_cell[0]),'arcsec')['value'])
                    xextent=128*pixsize*0.5
                    xextent=[xextent,-xextent]
                    yextent=128*pixsize*0.5
                    yextent=[-yextent,yextent]
                    flipped_array=beam_array.transpose()
                    ttrans_array=flipped_array.tolist()
                    ttrans_array.reverse()
                    pl.imshow(ttrans_array,interpolation='bilinear',cmap=pl.cm.jet,extent=xextent+yextent,origin="bottom")
                    pl.title(project+".quick.psf",fontsize="x-small")
                    b=qa.convert(beam['bmaj'],'arcsec')['value']
                    pl.xlim([-3*b,3*b])
                    pl.ylim([-3*b,3*b])
                    ax=pl.gca()
                    pl.text(0.05,0.95,"bmaj=%7.1e\nbmin=%7.1e" % (beam['bmaj']['value'],beam['bmin']['value']),transform = ax.transAxes,bbox=dict(facecolor='white', alpha=0.7),size="x-small",verticalalignment="top")
                    ia.done()
                util.endfig()



            ##################################################################
            # predict single dish observation
            if predict_sd:
                if os.path.exists(sdmsfile):
                    if not overwrite:
                        util.msg("measurement set "+sdmsfile+" already exists and user does not wish to overwrite",priority="error")
                sm.open(sdmsfile)
                posobs=me.observatory(tp_telescopename)
                diam=tpd
                # WARNING: sm.setspwindow is not consistent with clean::center
                model_start=qa.sub(model_center,qa.mul(model_width,0.5*model_nchan))

                sm.setconfig(telescopename=tp_telescopename, x=tpx, y=tpy, z=tpz, 
                             dishdiameter=diam.tolist(),
                             mount=['alt-az'], antname=tp_antnames, padname=tp_padnames, 
                             coordsystem='global', referencelocation=posobs)
                sm.setspwindow(spwname=fband, freq=qa.tos(model_start), 
                               deltafreq=qa.tos(model_width), 
                               freqresolution=qa.tos(model_width), 
                               nchannels=model_nchan, 
                               stokes='XX YY')
                sm.setfeed(mode='perfect X Y',pol=[''])

                if verbose: msg(" spectral window set at %s" % qa.tos(model_center))
                sm.setlimits(shadowlimit=0.01, elevationlimit='10deg')
                # auto-correlation should be unity for single dish obs.
                sm.setauto(1.0)
                for k in range(0,nfld):
                    src=project+'_%d'%k
                    sm.setfield(sourcename=src, sourcedirection=pointings[k],
                                calcode="OBJ", distance='0m')
                    if k==0:
                        sourcefieldlist=src
                    else:
                        sourcefieldlist=sourcefieldlist+','+src
                if docalibrator:
                    msg("calibration is not supported for SD observation...skipped")
                #    sm.setfield(sourcename="phase calibrator", 
                #                sourcedirection=caldirection,calcode='C',
                #                distance='0m')
                reftime = me.epoch('TAI', refdate)
                sm.settimes(integrationtime=integration, usehourangle=True, 
                            referencetime=reftime)
                totalsec=qa.convert(qa.quantity(totaltime),'s')['value']
                scantime=qa.mul(qa.quantity(integration),str(scanlength))
                scansec=qa.convert(qa.quantity(scantime),'s')['value']
                nscan=int(totalsec/scansec)
                kfld=0
                # RI todo progress meter for simdata Sim::observe

                if nscan<nfld:
                    msg("Only %i pointings of %i in the mosaic will be observed - check mosaic setup and exposure time parameters!" % (nscan,nfld),priority="error")
                    return
        
                # sm.observemany
                observemany=True
                #if observemany:
                srces=[]
                starttimes=[]
                stoptimes=[]
                dirs=[]

                for k in range(0,nscan) :
                    sttime=-totalsec/2.0+scansec*k
                    endtime=sttime+scansec
                    src=project+'_%d'%kfld
                    #if observemany:
                    srces.append(src)
                    starttimes.append(str(sttime)+"s")
                    stoptimes.append(str(endtime)+"s")
                    dirs.append(pointings[kfld])
                    #else:
                    ## this only creates blank uv entries
                    #    sm.observe(sourcename=src, spwname=fband,
                    #               starttime=qa.quantity(sttime, "s"),
                    #               stoptime=qa.quantity(endtime, "s"));
                    kfld=kfld+1
                    if predict_uv and kfld==nfld:
                        # calibration obs is disabled for SD but add a gap to synchronize with interferometer
                        #if docalibrator:
                        #    sttime=-totalsec/2.0+scansec*k
                        #    endtime=sttime+scansec
                        #    if observemany:
                        #        srces.append(src)
                        #        starttimes.append(str(sttime)+"s")
                        #        stoptimes.append(str(endtime)+"s")
                        #        dirs.append(caldirection)
                        #    else:
                        #        sm.observe(sourcename="phase calibrator", spwname=fband,
                        #                   starttime=qa.quantity(sttime, "s"),
                        #                   stoptime=qa.quantity(endtime, "s"));
                        kfld=kfld+1                
                    if kfld > nfld: kfld=0
                # if directions is unset, NewMSSimulator::observemany 
                # looks up the direction in the field table.
                #if observemany:
                sm.observemany(sourcenames=srces,spwname=fband,starttimes=starttimes,stoptimes=stoptimes)

                sm.setdata(fieldid=range(0,nfld))
                sm.setvp()

                msg("done setting up observations (blank visibilities)")
                if verbose:
                    sm.summary()

                #######################################################
                # do actual calculation of visibilities:

                sm.setoptions(gridfunction='pb', ftmachine="sd", location=posobs, cache=100000)
                if not components_only:                
                    if len(complist)>1:
                        msg("predicting from "+newmodel+" and "+complist,priority="warn")
                    else:
                        msg("predicting from "+newmodel,priority="warn")
                    sm.predict(imagename=newmodel,complist=complist)
                else:   # if we're doing only components
                    msg("predicting from "+complist,priority="warn")
                    sm.predict(complist=complist)
            
                sm.done()

                # modify STATE table information for ASAP
                # Ugly part!! need improvement. 
                nstate=1
                tb.open(tablename=sdmsfile+'/STATE',nomodify=False)
                tb.addrows(nrow=nstate)
                tb.putcol(columnname='CAL',value=[0.]*nstate,startrow=0,nrow=nstate,rowincr=1)
                tb.putcol(columnname='FLAG_ROW',value=[False]*nstate,startrow=0,nrow=nstate,rowincr=1)
                tb.putcol(columnname='LOAD',value=[0.]*nstate,startrow=0,nrow=nstate,rowincr=1)
                tb.putcol(columnname='REF',value=[False]*nstate,startrow=0,nrow=nstate,rowincr=1)
                tb.putcol(columnname='SIG',value=[True]*nstate,startrow=0,nrow=nstate,rowincr=1)
                tb.putcol(columnname='SUB_SCAN',value=[0]*nstate,startrow=0,nrow=nstate,rowincr=1)
                tb.flush()
                tb.close()
            
                tb.open(tablename=sdmsfile,nomodify=False)
                tb.putcol(columnname='STATE_ID',value=[0]*nfld,startrow=0,nrow=nfld,rowincr=1)
                tb.flush()
                tb.close()
                
                msg('generation of measurement set ' + sdmsfile + ' complete')

        else:
            # if not predicting this time, but are imageing or analyzing, 
            # get telescopename from ms
            if image or analyze:
                tb.open(project+".ms/OBSERVATION")
                n=tb.getcol("TELESCOPE_NAME")
                telescopename=n[0]
                util.telescopename=telescopename
                # todo add check that entire column is the same
                tb.done()

        ######################################################################
        # noisify

        noise_any=False
        msroot=project  # if leakage, can just copy from this project
    
        if thermalnoise!="":
            if not (telescopename == 'ALMA' or telescopename == 'ACA'):
                msg("thermal noise only works properly for ALMA/ACA",origin="noise",priority="warn")
                
            noise_any=True

            eta_p, eta_s, eta_b, eta_t, eta_q, t_rx = util.noisetemp()

            # antenna efficiency
            eta_a = eta_p * eta_s * eta_b * eta_t
            if verbose: 
                msg('antenna efficiency    = ' + str(eta_a),origin="noise")
                msg('spillover efficiency  = ' + str(eta_s),origin="noise")
                msg('correlator efficiency = ' + str(eta_q),origin="noise")
 
            # Cosmic background radiation temperature in K. 
            t_cmb = 2.275

            noisymsroot = msroot+".noisy"

            # check for interferometric ms:
            if os.path.exists(msroot+".ms"):
                msg('copying '+msroot+'.ms to ' + 
                    noisymsroot+'.ms and adding thermal noise',
                    origin="noise",priority="warn")
                
                if os.path.exists(noisymsroot+".ms"):
                    shutil.rmtree(noisymsroot+".ms")                
                shutil.copytree(msfile,noisymsroot+".ms")
                if sm.name()!='':
                    msg("table persistence error on %s" % sm.name(),priority="error")
                    return

                sm.openfromms(noisymsroot+".ms")    # an existing MS
                sm.setdata(fieldid=[]) # force to get all fields
                if thermalnoise=="tsys-manual":
                    if verbose:
                        msg("sm.setnoise(spillefficiency="+str(eta_s)+
                            ",correfficiency="+str(eta_q)+",antefficiency="+str(eta_a)+
                            ",trx="+str(t_rx)+",tau="+str(tau0)+
                            ",tatmos="+str(t_sky)+",tground="+str(t_ground)+
                            ",tcmb="+str(t_cmb)+",mode='tsys-manual')");
                        msg("** this may take a few minutes, but will be faster in future releases",priority="warn")
                    sm.setnoise(spillefficiency=eta_s,correfficiency=eta_q,
                                antefficiency=eta_a,trx=t_rx,
                                tau=tau0,tatmos=t_sky,tground=t_ground,tcmb=t_cmb,
                                mode="tsys-manual")
                else:
                    if verbose:
                        msg("sm.setnoise(spillefficiency="+str(eta_s)+
                            ",correfficiency="+str(eta_q)+",antefficiency="+str(eta_a)+
                            ",trx="+str(t_rx)+",tground="+str(t_ground)+
                            ",tcmb="+str(t_cmb)+",mode='tsys-atm'"+
                            ",pground='650mbar',altitude='5000m',waterheight='2km',relhum=20,pwv="+str(user_pwv)+"mm)");
                        msg("** this may take a few minutes, but will be faster in the future",priority="warn")
                    sm.setnoise(spillefficiency=eta_s,correfficiency=eta_q,
                                antefficiency=eta_a,trx=t_rx,
                                tground=t_ground,tcmb=t_cmb,pwv=str(user_pwv)+"mm",
                                mode="tsys-atm")
                    # don't set table, that way it won't save to disk
                    #                        mode="calculate",table=noisymsroot)
                sm.corrupt();
                sm.done();

            # now TP ms:
            if os.path.exists(msroot+".sd.ms"):
                msg('copying '+msroot+'.sd.ms to ' + 
                    noisymsroot+'.sd.ms and adding thermal noise',
                    origin="noise",priority="warn")
                
                if os.path.exists(noisymsroot+".sd.ms"):
                    shutil.rmtree(noisymsroot+".sd.ms")                
                shutil.copytree(msfile,noisymsroot+".sd.ms")
                if sm.name()!='':
                    msg("table persistence error on %s" % sm.name(),priority="error")
                    return

                sm.openfromms(noisymsroot+".sd.ms")    # an existing MS
                sm.setdata(fieldid=[]) # force to get all fields
                if thermalnoise=="tsys-manual":
                    if verbose:
                        msg("sm.[old]setnoise(spillefficiency="+str(eta_s)+
                            ",correfficiency="+str(eta_q)+",antefficiency="+str(eta_a)+
                            ",trx="+str(t_rx)+",tau="+str(tau0)+
                            ",tatmos="+str(t_sky)+
                            ",tcmb="+str(t_cmb));
                    sm.oldsetnoise(spillefficiency=eta_s,correfficiency=eta_q,
                                   antefficiency=eta_a,trx=t_rx,
                                   tau=tau0,tatmos=t_sky,tcmb=t_cmb,
                                   mode="calculate")
                else:
                    msg("Can't corrupt SD data using ATM library - please use tsys-manual",priority="error")
                sm.corrupt();
                sm.done();

            msroot=noisymsroot
            if verbose: msg("done corrupting with thermal noise",origin="noise")


        if leakage>0:
            noise_any=True
            if msroot==project:
                noisymsroot = project+".noisy"
            else:
                noisymsroot = project+".noisier"
            if os.path.exists(msroot+".sd.ms"):
                msg("Can't corrupt SD data with polarization leakage",priority="warn")
            if os.path.exists(msroot+".ms"):
                msg('copying '+project+'.ms to ' + 
                    noisymsroot+'.ms and adding polarization leakage',
                    origin="noise",priority="warn")
                if os.path.exists(noisymsroot+".ms"):
                    shutil.rmtree(noisymsroot+".ms")                
                shutil.copytree(msfile,noisymsroot+".ms")
                if sm.name()!='':
                    msg("table persistence error on %s" % sm.name(),priority="error")
                    return

                sm.openfromms(noisymsroot+".ms")    # an existing MS
                sm.setdata(fieldid=[]) # force to get all fields
                sm.setleakage(amplitude=leakage,table=noisymsroot+".cal")
                sm.corrupt();
                sm.done();

                

            



        #####################################################################
        # clean if desired, use noisy image for further calculation if present
        # todo suggest a cell size from psf?

        # make sure cell is defined
        if type(cell)==type([]):
            if len(cell)>0:
                cell0=cell[0]
            else:
                cell0=""
        else:
            cell0=cell
        if len(cell0)<=0:
            cell=model_cell
        if type(cell)==type([]):
            if len(cell)==1:
                cell=[cell[0],cell[0]]
        else:
            cell=[cell,cell]


        #pdb.set_trace()
        # and imsize
        if type(imsize)==type([]):
            if len(imsize)>0:
                imsize0=imsize[0]
            else:
                imsize0=-1
        else:
            imsize0=imsize
        if imsize0<=0:
            imsize = [int(pl.ceil(qa.convert(qa.div(model_size[0],cell[0]),"")['value'])),
                      int(pl.ceil(qa.convert(qa.div(model_size[1],cell[1]),"")['value']))]



        #####################################################################
        if image:
            tpms=None
            tpset=False
            if predict_sd:
                tpms=project+'.sd.ms'
                tpset=True

            if not tpset and os.path.exists(modelimage):
                # should be CASA image so far. 
                tpimage=modelimage
                tpset=True

            # parse ms parameter and check for existance
            # TODO if noisy ms was created, switch automagically?  probably best do 
            # do that in the xml defaults instead so the user can change it if they want
            # if noise_any:
            #     mstoimage = noisymsfile
            # else:
            #     mstoimage = msfile            
                        
            mslist=vis.split(',')
            mstoimage=[]
            for ms0 in mslist:
                ms1=ms0.replace('$project',project)
                if os.path.exists(ms1):
                    # check if the ms is tp data or not.
                    tb.open(ms1+'/ANTENNA')
                    antname=tb.getcol('NAME')
                    tb.close()
                    if antname[0].find('TP') > -1:
                        if not tpset: 
                            tpms=ms1
                            tpset=True
                    else: mstoimage.append(ms1)
                else:
                    if verbose:
                        msg("measurement set "+ms1+" not found -- removing from clean list",priority="warn")
                    else:
                        msg("measurement set "+ms1+" not found -- removing from clean list")
            if len(mstoimage)<=0 and not tpms:
                msg("no measurement sets found to image",priority="warn")
                image=False

            # Do single dish imaging first if tpms exists.
            if tpms and os.path.exists(tpms):
                msg('creating image from generated ms: '+tpms)
                if len(mstoimage):
                    tpimage = project+'.sd.image'
                else:
                    tpimage = project+'.image'
                #im.open(msfile)
                im.open(tpms)
                im.selectvis(nchan=model_nchan,start=0,step=1,spw=0)
                im.defineimage(mode='channel',nx=imsize[0],ny=imsize[1],cellx=cell,celly=cell,phasecenter=model_refdir,nchan=model_nchan,start=0,step=1,spw=0)
                #im.setoptions(ftmachine='sd',gridfunction='pb')
                im.setoptions(ftmachine='sd',gridfunction='pb')
                im.makeimage(type='singledish',image=tpimage)
                im.close()

                # For single dish: manually set the primary beam
                ia.open(tpimage)
                beam=ia.restoringbeam()
                if len(beam) == 0:
                    msg('setting primary beam information to image.')
                    # !! aveant will only be set if modifymodel or setpointingsm and in 
                    # any case it will the the aveant of the INTERFM array - we want the SD
                    tb.open(tpms+"/ANTENNA")
                    diams=tb.getcol("DISH_DIAMETER")
                    tb.done()
                    aveant=pl.mean(diams)
                    # model_center should be set even if we didn't predict this execution
                    pb=1.2*0.3/qa.convert(qa.quantity(model_center),'GHz')['value']/aveant*3600.*180/pl.pi
                    beam['major'] = beam['minor'] = qa.quantity(pb,'arcsec')
                    beam['positionangle'] = qa.quantity(0.0,'deg')
                    msg('Primary beam: '+str(beam['major']))
                    ia.setrestoringbeam(beam=beam)
                ia.done()
                del beam

                msg('generation of total power image ' + tpimage + ' complete.')
                # End of single dish imaging part

            if not predict:
                # get nfld, sourcefieldlist, from (interfm) ms if it was not just created
                tb.open(mstoimage[0]+"/SOURCE")
                code=tb.getcol("CODE")
                sourcefieldlist=pl.where(code=='OBJ')[0]
                nfld=len(sourcefieldlist)
                tb.done()
                msfile=mstoimage[0]

            # set cleanmode automatically (for interfm)
            if len(mstoimage):
                if nfld==1:
                    cleanmode="csclean"
                else:
                    cleanmode="mosaic"

        outflat_current=False
        convsky_current=False
        beam_current=False
        imagename=project

        #if image:
        if image and len(mstoimage):
            if not docalibrator:
                sourcefieldlist=""  # sourcefieldlist should be ok, but this is safer
            
            # clean insists on using an existing model if its present
            if os.path.exists(imagename+".image"): shutil.rmtree(imagename+".image")
            if os.path.exists(imagename+".model"): shutil.rmtree(imagename+".model")

            # todo add imcenter param instead of just using the model_refdir?
            util.image(mstoimage,imagename,
                       cleanmode,cell,imsize,model_refdir,
                       niter,threshold,weighting,
                       outertaper,stokes,sourcefieldlist=sourcefieldlist,
                       modelimage=modelimage)

            # create imagename.flat and imagename.residual.flat:
            util.flatimage(imagename+".image",verbose=verbose)
            util.flatimage(imagename+".residual",verbose=verbose)
            outflat_current=True

            msg("done inverting and cleaning")
            if not type(cell)==type([]):
                cell=[cell,cell]
            if len(cell)<=1:
                cell=[qa.quantity(cell[0]),qa.quantity(cell[0])]
            else:
                cell=[qa.quantity(cell[0]),qa.quantity(cell[1])]

        #if image:
            # get beam from output clean image
            if verbose: msg("getting beam from "+imagename+".image",origin="analysis")
            ia.open(imagename+".image")
            beam=ia.restoringbeam()
            beam_current=True
            ia.done()
            # model has units of Jy/pix - calculate beam area from clean image
            # (even if we are not plotting graphics)
            bmarea=beam['major']['value']*beam['minor']['value']*1.1331 #arcsec2
            bmarea=bmarea/(cell[0]['value']*cell[1]['value']) # bm area in pix
            msg("synthesized beam area in output pixels = %f" % bmarea)


            # show model, convolved model, clean image, and residual 
            if grfile:            
                file=project+".image.png"
            else:
                file=""

            if grscreen or grfile:
                util.newfig(multi=[2,2,1],show=grscreen)

                # create regridded and convolved sky model image
                util.convimage(modelflat,imagename+".image.flat")
                convsky_current=True # don't remake this for analysis in this run

                disprange=[]  # passing empty list causes return of disprange

                # original sky regridded to output pixels but not convolved with beam
                discard = util.statim(modelflat+".regrid",disprange=disprange)
                util.nextfig()

                # disprange from skymodel.regrid is in Jy/pix, but convolved im is in Jy/bm
                # bmarea is in units of output image pixels
                disprange=[disprange[0]*bmarea,disprange[1]*bmarea]

                # convolved sky model - units of Jy/bm
                discard = util.statim(modelflat+".regrid.conv",disprange=disprange)                
                util.nextfig()
                
                # clean image - also in Jy/beam
                # although because of DC offset, better to reset disprange
                disprange=[]
                discard = util.statim(imagename+".image.flat",disprange=disprange)
                util.nextfig()

                # clean residual image - Jy/bm
                discard = util.statim(imagename+".residual.flat",disprange=disprange)
                util.endfig(show=grscreen,filename=file)
        



        #####################################################################
        # analysis

        if analyze:
            # will need skymodel, so skymodel has to be set:
            if not os.path.exists(skymodel):
                msg("skymodel "+str(skymodel)+" not found",priority="warn")
                msg("If you are simulating from componentlist only, analysis is not fully implemented.  If you have a sky model image, please set the skymodel parameter",priority="error")
            tmpname=skymodel.replace(project,'$project')

            #if tmpname=="$project.skymodel":
                # TODO clarify the logic for newmodel vs skymodel
                # i.e. here, if util.is4d(skymodel) then use skymodel
                # unchanged, otherwise newmodel will have been created 
                # above, and use that
                # for now, we're *always* creating newmodel, so use newmodel here
                # whether tmpname is skymodel or not.
            modelim=newmodel

            if not os.path.exists(modelim):
                msg("sky model image "+str(modelim)+" not found",priority="error")

            # so we should have modelim and modelim.flat created above, 
            # whether modifymodel is true or not.

            if not image:
                # get beam from output clean image
                if verbose: msg("getting beam from "+imagename+".image",origin="analysis")
                ia.open(imagename+".image")
                beam=ia.restoringbeam()
                beam_current=True
                ia.done()
                # model has units of Jy/pix - calculate beam area from clean image
                # (even if we are not plotting graphics)
                bmarea=beam['major']['value']*beam['minor']['value']*1.1331 #arcsec2
                bmarea=bmarea/(cell[0]['value']*cell[1]['value']) # bm area in pix
                msg("synthesized beam area in output pixels = %f" % bmarea)


            # what about the output image?
            outim=project+".image"
            if not os.path.exists(outim):
                msg("output image"+str(outim)+" not found",priority="warn")
                msg("you may need to run simdata.image, or if you deconvolved manually, rename your output to "+outim,priority="error")

            # flat output:?  if the user manually cleaned, this may not exist
            outflat=imagename+".image.flat"
            if (not outflat_current) or (not os.path.exists(outflat)):
                # create imagename.flat and imagename.residual.flat
                if not image:
                    # get cell from outim
                    cell=util.cellsize(outim)
                util.flatimage(imagename+".image",verbose=verbose)
                util.flatimage(imagename+".residual",verbose=verbose)
                outflat_current=True
                
            # regridded and convolved input:?
            if not convsky_current:                
                util.convimage(modelim+".flat",imagename+".image.flat")
                convsky_current=True
            
            # now should have all the flat, convolved etc even if didn't run "image" 

            # make difference image.
            # immath does Jy/bm if image but only if ia.setbrightnessunit("Jy/beam") in convimage()
            convolved = modelim+".flat.regrid.conv"
            difference = imagename + '.diff'
            ia.imagecalc(difference, "'%s' - '%s'" % (convolved, outflat), overwrite = True)
            
            # get rms of difference image for fidelity calculation
            ia.open(difference)
            diffstats = ia.statistics(robust=True, verbose=False,list=False)
            maxdiff=diffstats['medabsdevmed']            
            if maxdiff!=maxdiff: maxdiff=0.
            if type(maxdiff)!=type(0.):
                if maxdiff.__len__()>0: 
                    maxdiff=maxdiff[0]
                else:
                    maxdiff=0.
            # Make fidelity image.
            absdiff = imagename + '.absdiff'
            ia.imagecalc(absdiff, "max(abs('%s'), %f)" % (difference,
                                                          maxdiff/pl.sqrt(2.0)), overwrite = True)
            fidelityim = imagename + '.fidelity'
            ia.imagecalc(fidelityim, "abs('%s') / '%s'" % (convolved, absdiff), overwrite = True)
            ia.done()

            msg("fidelity image calculated",origin="analysis")



            # now, what does the user want to actually display?
            if len(stnx)<=0:
                showarray=False
            if not (predict or image):
                msfile=project+".ms"

            # if the order in the task input changes, change it here too
            figs=[showarray,showuv,showpsf,showmodel,showconvolved,showclean,showresidual,showdifference,showfidelity]
            nfig=figs.count(True)
            if nfig>6:
                msg("only displaying first 6 selected panels in graphic output",priority="warn")
            if nfig<=0:
                return True
            if nfig<4:
                multi=[1,nfig,1]
            else:
                if nfig==4:
                    multi=[2,2,1]
                else:
                    multi=[2,3,1]
                    
            if grfile:            
                file=project+".analysis.png"
            else:
                file=""

            if grscreen or grfile:
                util.newfig(multi=multi,show=grscreen)

                # if order in task parameters changes, change here too
                if showarray:
                    util.plotants(stnx, stny, stnz, stnd, padnames)
                    util.nextfig()

                if showuv:
                    tb.open(msfile)  
                    rawdata=tb.getcol("UVW")
                    tb.done()
                    pl.box()
                    maxbase=max([max(rawdata[0,]),max(rawdata[1,])])  # in m
                    klam_m=300/qa.convert(model_center,'GHz')['value']
                    pl.plot(rawdata[0,]/klam_m,rawdata[1,]/klam_m,'b,')
                    pl.plot(-rawdata[0,]/klam_m,-rawdata[1,]/klam_m,'b,')
                    ax=pl.gca()
                    ax.yaxis.LABELPAD=-4
                    pl.xlabel('u[klambda]',fontsize='x-small')
                    pl.ylabel('v[klambda]',fontsize='x-small')
                    pl.axis('equal')
                    util.nextfig()

                if showpsf:
                    if image:
                        psfim=imagename+".psf"
                    else:
                        psfim=project+".quick.psf"
                        if not quickpsf_current:
                            im.open(msfile)  
                            # TODO spectral parms
                            im.defineimage(cellx=qa.tos(model_cell[0]))  
                            if os.path.exists(psfim):
                                shutil.rmtree(psfim)
                            im.approximatepsf(psf=psfim)
                            # beam is set above (even in "analyze" only)
                            # note that if image, beam has fields 'major' whereas if not, it 
                            # has fields like 'bmaj'.  
                            # beam=im.fitpsf(psf=psfim)  
                            im.done()
                    ia.open(psfim)            
                    beamcs=ia.coordsys()
                    beam_array=ia.getchunk(axes=[beamcs.findcoordinate("spectral")['pixel'],beamcs.findcoordinate("stokes")['pixel']],dropdeg=True)
                    pixsize=(qa.convert(qa.quantity(model_cell[0]),'arcsec')['value'])
                    nn=beam_array.shape
                    xextent=nn[0]*pixsize*0.5
                    xextent=[xextent,-xextent]
                    yextent=nn[1]*pixsize*0.5
                    yextent=[-yextent,yextent]
                    flipped_array=beam_array.transpose()
                    ttrans_array=flipped_array.tolist()
                    ttrans_array.reverse()
                    pl.imshow(ttrans_array,interpolation='bilinear',cmap=pl.cm.jet,extent=xextent+yextent,origin="bottom")
                    pl.title(psfim,fontsize="x-small")
                    b=qa.convert(beam['major'],'arcsec')['value']
                    pl.xlim([-3*b,3*b])
                    pl.ylim([-3*b,3*b])
                    ax=pl.gca()
                    pl.text(0.05,0.95,"bmaj=%7.1e\nbmin=%7.1e" % (beam['major']['value'],beam['minor']['value']),transform = ax.transAxes,bbox=dict(facecolor='white', alpha=0.7),size="x-small",verticalalignment="top")
                    ia.done()
                    util.nextfig()

                disprange=[]  # first plot will define range
                if showmodel:
                    discard = util.statim(modelflat+".regrid",incell=cell,disprange=disprange)
                    util.nextfig()

                if showconvolved:
                    discard = util.statim(modelflat+".regrid.conv")
                    # if disprange gets set here, it'll be Jy/bm
                    util.nextfig()
                
                if showclean:
                    # own scaling because of DC/zero spacing offset
                    discard = util.statim(imagename+".image.flat")
                    util.nextfig()

                if showresidual:
                    # it gets its own scaling
                    discard = util.statim(imagename+".residual.flat")
                    util.nextfig()

                if showdifference:
                    # it gets its own scaling.
                    discard = util.statim(imagename+".diff")
                    util.nextfig()

                if showfidelity:
                    # it gets its own scaling.
                    discard = util.statim(imagename+".fidelity")
                    util.nextfig()

                util.endfig(show=grscreen,filename=file)
            else:
                sim_min,sim_max,sim_rms = util.statim(imagename+".image.flat",plot=False)
                # if not displaying still print stats:
                # 20100505 ia.stats changed to return Jy/bm:
                msg('Simulation rms: '+str(sim_rms/bmarea)+" Jy/pix = "+
                    str(sim_rms)+" Jy/bm",origin="analysis")
                msg('Simulation max: '+str(sim_max/bmarea)+" Jy/pix = "+
                    str(sim_max)+" Jy/bm",origin="analysis")
                #msg('Simulation rms: '+str(sim_rms)+" Jy/pix = "+
                #    str(sim_rms*bmarea)+" Jy/bm",origin="analysis")
                #msg('Simulation max: '+str(sim_max)+" Jy/pix = "+
                #    str(sim_max*bmarea)+" Jy/bm",origin="analysis")
                msg('Beam bmaj: '+str(beam['major']['value'])+' bmin: '+str(beam['minor']['value'])+' bpa: '+str(beam['positionangle']['value']),origin="analysis")




        # TODO cleanup - delete newmodel, newmodel.flat etc
        # shutil.rmtree(modelflat)  


#    except TypeError, e:
#        msg("task_simdata -- TypeError: %s" % e,priority="error")
#        return
#    except ValueError, e:
#        print "task_simdata -- OptionError: ", e
#        return
#    except Exception, instance:
#        print '***Error***',instance
#        return


##### Helper functions to plot primary beam
def plotpb(pb,axes,lims=None):
    # This beam is automatically scaled when you zoom in/out but
    # not anchored in plot area. We'll wait for Matplotlib 0.99
    # for that function. 
    #major=major
    #minor=minor
    #rangle=rangle
    #bwidth=max(major*pl.cos(rangle),minor*pl.sin(rangle))*1.1
    #bheight=max(major*pl.sin(rangle),minor*pl.cos(rangle))*1.1
    boxsize=pb*1.1
    if not lims: lims=axes.get_xlim(),axes.get_ylim()
    incx=1
    incy=1
    if axes.xaxis_inverted(): incx=-1
    if axes.yaxis_inverted(): incy=-1
    #ecx=lims[0][0]+bwidth/2.*incx
    #ecy=lims[1][0]+bheight/2.*incy
    ccx=lims[0][0]+boxsize/2.*incx
    ccy=lims[1][0]+boxsize/2.*incy
    from matplotlib.patches import Rectangle, Circle #,Ellipse
    #box=Rectangle((lims[0][0],lims[1][0]),incx*bwidth,incy*bheight,
    box=Rectangle((lims[0][0],lims[1][0]),incx*boxsize,incy*boxsize,
                  alpha=0.7,facecolor='w',
                  transform=axes.transData) #Axes
    #beam=Ellipse((ecx,ecy),major,minor,angle=rangle,
    beam=Circle((ccx,ccy), radius=pb/2.,
                 edgecolor='k',fill=False,
                 label='beam',transform=axes.transData)
    #props={'pad': 3, 'edgecolor': 'k', 'linewidth':2, 'facecolor': 'w', 'alpha': 0.5}
    #pl.matplotlib.patches.bbox_artist(beam,axes.figure.canvas.get_renderer(),props=props)
    axes.add_artist(box)
    axes.add_artist(beam)

 
