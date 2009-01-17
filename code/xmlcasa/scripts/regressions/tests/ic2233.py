import casac
imager = casac.homefinder.find_home_by_name('imagerHome')
image  = casac.homefinder.find_home_by_name('imageHome')

quantity=casac.Quantity
im = imager.create()
ia = image.create()
def run() :
    im.open('ic2233_1.ms')
    npix=1024
    im.setdata(mode='channel', spwid=[0], nchan=[6], start=[0], step=[1])
    im.setimage(nx=npix, ny=npix, cellx=quantity(3.0,"arcsec"), celly=quantity(3.0,"arcsec"), stokes="IV", spwid=[0])
    im.weight(type='briggs', robust=0.7)
    im.setvp(dovp=bool(1), usedefaultvp=bool(1), dosquint=bool(1),
             parangleinc='5.0deg')
    im.make('squint_corr')
    im.clean(algorithm='mfhogbom', niter=1000, model=['squint_corr'], residual=['squint_corr.residual'], image=['squint_corr.restored'], threshold='0Jy')
    im.done()
    return ['squint_corr.restored']



def data():
    return ['ic2233_1.ms']
