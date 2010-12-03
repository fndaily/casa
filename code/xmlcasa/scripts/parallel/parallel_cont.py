import os
from taskinit import *
import time
import numpy as np
from cleanhelper import *
class imagecont():
    def __init__(self, ftmachine='ft', wprojplanes=10, facets=1, pixels=[3600, 3600], cell=['3arcsec', '3arcsec'], spw='', field='', phasecenter='', weight='natural'):
        self.im=imtool.create()
        self.dc=dctool.create()
        self.ft=ftmachine
        self.wprojplanes=wprojplanes
        self.facets=facets
        print 'cell', cell
        self.pixels=pixels
        self.cell=cell
        if(spw==''):
            spw='*'
        self.spw=spw
        self.field=field
        self.phCen=phasecenter
        self.weight=weight
        self.imageparamset=False
        self.robust=0.0
        self.weightnpix=0
        self.imagetilevol=1000000
        self.visInMem=False
        self.painc=360.0
        self.pblimit=0.1
        self.dopbcorr=True
        self.novaliddata=False
        self.applyoffsets=False
        self.cfcache='cfcache.dir'
        self.epjtablename=''
    def imagecont(self, msname='spw00_4chan351rowTile.ms', start=0, numchan=1, spw=0, field=0, freq='1.20GHz', band='200MHz', imname='newmodel'):
        im=self.im
        origname=msname
        ###either psf 0 or no channel selected
        if(self.novaliddata):
            return
        #j=start
        #end=start+numchan-1
        #spwstring=str(spw)+':'+str(start)+'~'+str(end)
        #print 'spwstring', spwstring
        msname=origname
        if(not self.imageparamset):
            im.selectvis(vis=msname, field=field, spw=spw, nchan=numchan, start=start, step=1, datainmemory=self.visInMem)
            #im.selectvis(vis=msname, field=field, spw=spwstring, datainmemory=True)
####
        #imname=imname+'_%02d'%(j)
            im.defineimage(nx=self.pixels[0], ny=self.pixels[1], cellx=self.cell[0], celly=self.cell[1], phasecenter=self.phCen, mode='frequency', nchan=1, start=freq, step=band, facets=self.facets)
            if(np.sum(numchan)==0):
                self.novaliddata=True
                ###make blanks
                im.make(imname+'.image')
                im.make(imname+'.residual')
                im.make(imname+'.model')
                im.make(imname+'.psf')
                return
            im.weight(type=self.weight, rmode='norm', npixels=self.weightnpix, 
                  robust=self.robust)
            im.setoptions(ftmachine=self.ft, wprojplanes=self.wprojplanes, pastep=self.painc, pblimit=self.pblimit, cfcachedirname=self.cfcache, dopbgriddingcorrections=self.dopbcorr, applypointingoffsets=self.applyoffsets, imagetilevol=self.imagetilevol)
        #im.regionmask(mask='lala.mask', boxes=[[0, 0, 3599, 3599]])
        #im.setmfcontrol(cyclefactor=0.0)
        if(not self.imageparamset):
            try:
                im.clean(algorithm='mfclark', niter=0, threshold='0.05mJy', model=imname+'.model', image=imname+'.image', residual=imname+'.residual', psfimage=imname+'.psf')
            except Exception, instance:
                if(string.count(instance.message, 'PSFZero') >0):
                    self.novaliddata=True
                    ###make a blank image
                    im.make(imname+'.image')
                else:
                    raise instance
        else:
            if(not self.novaliddata):
                im.restore(model=imname+'.model',  image=imname+'.image', residual=imname+'.residual')
            
        #im.done()
        self.imageparamset=True
    def cleancont(self, niter=100, alg='clark', thr='0.0mJy', psf='newmodel.psf', dirty='newmodel.dirty', model='newmodel.model', mask='', scales=[0]):
        dc=self.dc
        dc.open(dirty=dirty, psf=psf)
        if((alg=='hogbom') or (alg == 'msclean')):
            sca=scales if (alg=='msclean') else [0]
            dc.setscales(scalemethod='uservector', uservector=sca)
            alg='fullmsclean'
        if(alg=='clark'):
            dc.clarkclean(niter=niter, threshold=thr, model=model, mask=mask)
        else:
            dc.clean(algorithm=alg, niter=niter, threshold=thr, model=model, mask=mask)
        dc.done()

    def imagechan(self, msname='spw00_4chan351rowTile.ms', start=0, numchan=1, spw=0, field=0, imroot='newmodel', imchan=0, niter=100, alg='clark', thr='0.0mJy', mask='', majcycle=1, scales=[0]):
        origname=msname
#        a=cleanhelper()
        imname=imroot+str(imchan)
 #       a.getchanimage(cubeimage=imroot+'.model', outim=imname+'.model', chan=imchan)
        im.selectvis(vis=msname, field=field, spw=spw, nchan=numchan, start=start, step=1, datainmemory=self.visInMem)
        im.weight(type=self.weight, rmode='norm', npixels=self.weightnpix, 
                  robust=self.robust)
        im.defineimage(nx=self.pixels[0], ny=self.pixels[1], cellx=self.cell[0], celly=self.cell[1], phasecenter=self.phCen, facets=self.facets)
        im.setoptions(ftmachine=self.ft, wprojplanes=self.wprojplanes, imagetilevol=self.imagetilevol, singleprecisiononly=True)
        im.setscales(scalemethod='uservector', uservector=scales)
        im.setmfcontrol(cyclefactor=0.0)  
        majcycle = majcycle if (niter/majcycle) >0 else 1
        for kk in range(majcycle):
            im.clean(algorithm=alg, niter= (niter/majcycle), threshold=thr, model=imname+'.model', image=imname+'.image', residual=imname+'.residual', psfimage='')
        im.done()
  #      a.putchanimage(cubimage=imroot+'.model', inim=imname+'.model', 
  #                     chan=imchan)
  #     a.putchanimage(cubimage=imroot+'.residual', inim=imname+'.residual', 
  #                    chan=imchan) 
  #     a.putchanimage(cubimage=imroot+'.image', inim=imname+'.image', 
  #                  chan=imchan)
        ###need to clean up here the channel image...will do after testing phase


    def imagechan_new(self, msname='spw00_4chan351rowTile.ms', start=0, numchan=1, spw=0, field=0, cubeim='imagecube', imroot='newmodel', imchan=0, chanchunk=1, niter=100, alg='clark', thr='0.0mJy', mask='', majcycle=1, scales=[0]):
        origname=msname
#        a=cleanhelper()
        imname=imroot+str(imchan)
        self.getchanimage(inimage=cubeim+'.model', outimage=imname+'.model', chan=imchan*chanchunk, nchan=chanchunk)
        im.selectvis(vis=msname, field=field, spw=spw, nchan=numchan, start=start, step=1, datainmemory=self.visInMem)
        im.weight(type=self.weight, rmode='norm', npixels=self.weightnpix, 
                  robust=self.robust)
        im.defineimage(nx=self.pixels[0], ny=self.pixels[1], cellx=self.cell[0], celly=self.cell[1], phasecenter=self.phCen, facets=self.facets)
        im.setoptions(ftmachine=self.ft, wprojplanes=self.wprojplanes, imagetilevol=self.imagetilevol, singleprecisiononly=True)
        im.setscales(scalemethod='uservector', uservector=scales)
        im.setmfcontrol(cyclefactor=0.0)  
        majcycle = majcycle if (niter/majcycle) >0 else 1
        for kk in range(majcycle):
            im.clean(algorithm=alg, niter= (niter/majcycle), threshold=thr, model=imname+'.model', image=imname+'.image', residual=imname+'.residual', psfimage='')
        im.done()
        #self.putchanimage(cubeim+'.model', imname+'.model', imchan*chanchunk)
        #self.putchanimage(cubeim+'.residual', imname+'.residual', imchan*chanchunk)
        #self.putchanimage(cubeim+'.image', imname+'.image', imchan*chanchunk)

    @staticmethod
    def getchanimage(inimage, outimage, chan, nchan=1):
        """
        create a slice of channels image from cubeimage
        """
    #pdb.set_trace()
        ia.open(inimage)
        modshape=ia.shape()
        if (modshape[3]==1) or (chan > (modshape[3]-1)) :
            return False
        if((nchan+chan) < modshape[3]):
            endchan= chan+nchan-1
        else:
            endchan=modshape[3]-1
        blc=[0,0,modshape[2]-1,chan]
        trc=[modshape[0]-1,modshape[1]-1,modshape[2]-1,endchan]
        sbim=ia.subimage(outfile=outimage, region=rg.box(blc,trc), overwrite=True)
        sbim.close()
        ia.close()
        return True
    #getchanimage = staticmethod(getchanimage)
    def cleanupcubeimages(self, readyputchan, doneputchan, imagename, nchanchunk, chanchunk):
        """
        This function will put the True values of readyputchan into the final cubes and set the doneputchan to True
        Confused ..read the code 
        """
        for k in range(nchanchunk):
            if(readyputchan[k] and (not doneputchan[k])):
                self.putchanimage(imagename+'.model', imagename+str(k)+'.model', k*chanchunk, False)
                self.putchanimage(imagename+'.residual', imagename+str(k)+'.residual', k*chanchunk, False)
                self.putchanimage(imagename+'.image', imagename+str(k)+'.image', k*chanchunk, False)
                doneputchan[k]=True
    def cleanupmodelimages(self, readyputchan,  imagename, nchanchunk, chanchunk):
        """
        This function will put model images only 
        """
        for k in range(nchanchunk):
            if(readyputchan[k]):
                self.putchanimage(imagename+'.model', imagename+str(k)+'.model', k*chanchunk, True)
    
    def cleanupresidualimages(self, readyputchan,  imagename, nchanchunk, chanchunk):
        """
        This function will put residual images only 
        """
        for k in range(nchanchunk):
            if(readyputchan[k]):
                self.putchanimage(imagename+'.residual', imagename+str(k)+'.residual', k*chanchunk, True)
    
    def cleanuprestoredimages(self, readyputchan,  imagename, nchanchunk, chanchunk):
        """
        This function will put residual images only 
        """
        for k in range(nchanchunk):
            if(readyputchan[k]):
                self.putchanimage(imagename+'.image', imagename+str(k)+'.image', k*chanchunk, True) 
                
    @staticmethod
    def putchanimage(cubimage,inim,chan, removeinfile=True):
        """
        put channel image back to a pre-exisiting cubeimage
        """
        if( not os.path.exists(inim)):
            return False
        ia.open(inim)
        inimshape=ia.shape()
        ############
        #imdata=ia.getchunk()
        #immask=ia.getchunk(getmask=True)
        ##############
        ia.close()
        ia.open(cubimage)
        cubeshape=ia.shape()
        blc=[0,0,0,chan]
        trc=[inimshape[0]-1,inimshape[1]-1,inimshape[2]-1,chan+inimshape[3]-1]
        if( not (cubeshape[3] > (chan+inimshape[3]-1))):
            return False

        ############
        #rg0=ia.setboxregion(blc=blc,trc=trc)
        ###########
        if inimshape[0:3]!=cubeshape[0:3]: 
            return False
        ########
        #ia.putregion(pixels=imdata,pixelmask=immask, region=rg0)
        ###########
        ia.insert(infile=inim, locate=blc)
        ia.close()
        if(removeinfile):
            ia.removefile(inim)
        return True
    #putchanimage=staticmethod(putchanimage)
