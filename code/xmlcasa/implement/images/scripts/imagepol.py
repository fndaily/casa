"""
<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" ?>
<casaxml xmlns="http://casa.nrao.edu/schema/psetTypes.html"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xsi:schemaLocation="http://casa.nrao.edu/schema/casa.xsd
file:///opt/casa/code/xmlcasa/xml/casa.xsd">


 
        <tool name="imagepol" module="images">
        <needs>image</needs>
        <shortdescription>Polarimetric analysis of images</shortdescription>
       



   %
   %ahkeyword registers module keywords
   %
<keyword>image</keyword>
<keyword>polarimetry</keyword>
<code>
	<include>xmlcasa/images/imagepol_forward.h</include>
	<private>
		#include <xmlcasa/images/imagepol_private.h>	</private>
</code>

<description>

\medskip
\noindent{\bf Summary}

An Imagepol \tool\ provides specialized polarimetric analysis of images.  
Some of these things could be done with the Lattice Expression Language
(LEL; see \htmladdnormallink{note223}{../../notes/223/223.html})
but are more conveniently offered separately.

\bigskip
{\bf General}

Before it can be used, the Imagepol tool must be attached to an image
(\casa, \fits, and Miriad formats are supported) with a Stokes
coordinate axis.  What you can then do with your Imagepol \tool\
depends on exactly which Stokes parameters you have in the image.  You
must have some combination of Stokes I, Q, U and V on the Stokes axis.
These refer to total intensity, two components of linear polarization,
and circular polarization, respectively.  Therefore, if you ask for
linear polarization and the image only has Stokes I and V, you will
get an error.

The Imagepol \tool\ functions generally return, by default, an <link
anchor="images:image">on-the-fly Image</link> \tool\ as their output.
In most cases, this is a ``virtual'' image.  There are a range of
different sorts of ``virtual'' images in \casa\ (see <link
anchor="images:image">Image</link>).  But the Imagepol \toolfunctions\
generally return reference Image \tools.  That is, these reference
different pieces of the original image attached to the
Imagepol \tool, either directly, or as mathematical expressions (e.g.
the polarized intensity).  If you delete the attached image, you
render your Imagepol \tool\ and its outputs useless.  If you wish,
rather than return a virtual image \tool, you can fill in the {\stfaf
outfile} argument of most Imagepol \tool\ functions and write the
image out to disk, associating the Image \tool\ with the disk file.

In some of the functions, the standard deviation of the thermal noise
is needed.  This is for debiasing polarized intensity images or
working out statistical error images.  By default it is worked out for
you from the attached image with outliers from the mean discarded.
Since it is the thermal noise it is trying to find, it is worked out
from the V, Q \& U, and finally I data in that order of
precedence.  This is because Stokes V is much less likely to contain
source signal than Stokes I.  You can supply the noise level if you
know it better.  For example, for small images or images with few
signal-free pixels, the theoretical estimate may be better.

\bigskip
{\bf Analysis and Display}

Traditionally, when generating secondary and tertiary images (e.g. 
position angle, fractional polarization, rotation measure etc), one
masks the output image according to some statistical test.  For example,
if the error in the output image is greater than some value, or the
errors in the input images are greater than some value.  Imagepol
\tools\ do not offer this kind of masking.  It does provide you with the
error images for the derived images.  By using LEL when you analyze your
images, you can mask the images however you want when you use them. 
That is, we defer the error interpretation as long as possible.  Here is
an example. 

<example>
\begin{verbatim}
"""
#
print "\t----\t Tool level Ex 1 \t----"
potool = casac.homefinder.find_home_by_name('imagepolHome')
po = potool.create()
po.imagepoltestimage(outfile='stokes.image')  # Create test image
po.close()                      # Close so we can illustrate opening an image
po.open('stokes.image')         # Open image with Imagepol tool
lpa = po.linpolposang()         # Linearly polarized position angle image
lpaerr = po.sigmalinpolposang() # Error in linearly polarized position angle image
lpa.statistics();               # Get statistics on position angle image
#viewer(mask=lpaerr.name()+'<5')  # Display when p.a. error < 5 degrees
#
"""
\end{verbatim}
</example>

\bigskip

Display is handled via the  <link anchor="display:viewer">Viewer</link> \tool. It
can display and overlay combinations of raster, contour and vector
representations of your data.  <!-- Please see the <link anchor="images:image.view.function">view</link>
function of the Image \tool\ and the document 
\htmlref{Getting Results}{GRimageanalysis}) for high-level
information on how to display images in general.-->

It is common to display linear polarization data via vectors where the
position angle of the vector is the position angle of the linear
polarized radiation, and the amplitude of the vector is proportional to
either the total polarized intensity or fractional polarized intensity. 

The data source of a vector display is either a Complex or a Float
image.  If it is a Complex image (e.g.  the complex linear polarization
$Q + iU$) then both the amplitude and the phase (position angle) are
available.  If it is just a Float image, then it is assumed to be the
position angle and an amplitude of unity will be provided.  The angular
units are given by image brightness units which you can set with
function <link anchor="images:image.setbrightnessunits.function">setbrightnessunits</link>. 
If the units are not recognized as angular, degrees are assumed. 

The position angle is measured positive North through East when you display a
plane holding a celestial coordinate (the usual astronomical
convention).  For other  axis/coordinate combinations, a positive
position angle is measured  from +x to +y in the
absolute world coordinate frame.

The Imagepol \tool\ can create Complex disk images for you via
functions  
<link anchor="images:imagepol.complexlinpol.function">complexlinpol</link> (complex linear polarization),
<link anchor="images:imagepol.complexfraclinpol.function">complexfraclinpol</link> (complex fractional linear polarization)
and <link anchor="images:imagepol.makecomplex.function">makecomplex</link> (takes amp/phase or real/imag).
As well as these Complex images,  you can also make Float images of the
linearly polarized intensity, linearly polarized position angle, and the
fractional linearly polarized intensity (see below).

Now, the Image \tool\ cannot yet deal with Complex images (it will
in the future).    This means that you cannot currently do

\begin{verbatim}
"""
#
print "\t----\t Tool level Ex 2 \t----"
po.open('stokes.image')  # Open image with Imagepol tool
po.complexlinpol('clp')  # Make complex image of linear polarization disk file
try:
  print "Expect SEVERE error and Exception here"
  ia.open('clp')         # Error
except Exception, e:
  print "Expected exception occurred!"
po.close()
#
"""
\end{verbatim}

\medskip 
\noindent
which is a bit annoying presently.  However, the 
<link anchor="display:viewer">Viewer</link> \tool\ is able to read Complex images
so that you are able to display them ok.

\begin{verbatim}
"""
#
print "\t----\t Tool level Ex 3 \t----"
po.open('stokes.image')   # Open image with Imagepol tool
po.complexlinpol('clp2')  # Make complex image of linear polarization disk file 
#viewer()                 # Start viewer to give access to Complex image
#
"""
\end{verbatim}

If you wanted to make a vector map display you would select
the appropriate image in the Viewer's data manager GUI,
click 'Vector Map' on the right hand side and
it would appear in the display.   Note that the Viewer's
Vector map display capability also offers you amplitude
noise debiasing and the On-The-Fly mask.

\bigskip
{\bf Overview of Imagepol \tool\ functions}

\begin{itemize}

\item Access to the different Stokes subimages is via functions
<link anchor="images:imagepol.stokes.function">stokes</link>, 
<link anchor="images:imagepol.stokesi.function">stokesi</link>, 
<link anchor="images:imagepol.stokesq.function">stokesq</link>, 
<link anchor="images:imagepol.stokesu.function">stokesu</link>,  and
<link anchor="images:imagepol.stokesv.function">stokesv</link>.

\item Create the standard secondary and tertiary polarization images with

\begin{itemize}

\item <link anchor="images:imagepol.complexfraclinpol.function">complexfraclinpol</link> - complex fractional linear polarization 

\item <link anchor="images:imagepol.complexlinpol.function">complexlinpol</link> - complex linear polarization 

\item <link anchor="images:imagepol.makecomplex.function">makecomplex</link> - make complex image from amp/phase or real/imag

\item <link anchor="images:imagepol.pol.function">pol</link> - polarized quantities as specified

\item <link anchor="images:imagepol.linpolint.function">linpolint</link> - linearly polarized intensity

\item <link anchor="images:imagepol.linpolposang.function">linpolposang</link> - linearly polarized position angle

\item <link anchor="images:imagepol.totpolint.function">totpolint</link> - total polarized intensity

\item <link anchor="images:imagepol.fraclinpol.function">fraclinpol</link> - fractional linear polarization

\item <link anchor="images:imagepol.fractotpol.function">fractotpol</link> - fractional total polarization

\item <link anchor="images:imagepol.depolratio.function">depolratio</link> - linear depolarization ratio

\end{itemize}

\item Create the standard secondary and tertiary polarization error
images (simple propagation of Gaussian errors) with

\begin{itemize}

\item <link anchor="images:imagepol.sigma.function">sigma</link> - best guess at thermal noise

\item <link anchor="images:imagepol.sigmastokes.function">sigmastokes</link> - specified Stokes parameter

\item <link anchor="images:imagepol.sigmastokesi.function">sigmastokesi</link> - Stokes I

\item <link anchor="images:imagepol.sigmastokesi.function">sigmastokesq</link> - Stokes Q

\item <link anchor="images:imagepol.sigmastokesi.function">sigmastokesu</link> - Stokes U

\item <link anchor="images:imagepol.sigmastokesi.function">sigmastokesv</link> - Stokes V

\item <link anchor="images:imagepol.sigmalinpolint.function">sigmalinpolint</link> - linearly polarized intensity

\item <link anchor="images:imagepol.sigmalinposang.function">sigmalinpolposang</link> - linearly polarized position angle

\item <link anchor="images:imagepol.sigmatotpolint.function">sigmatotpolint</link> - total polarized intensity

\item <link anchor="images:imagepol.sigmafraclinpol.function">sigmafraclinpol</link> - fractional linear polarization

\item <link anchor="images:imagepol.sigmafractotpol.function">sigmafractotpol</link> - fractional total polarization

\item <link anchor="images:imagepol.sigmadepolratio.function">sigmadepolratio</link> - linear depolarization ratio

\end{itemize}

\item You can compute Rotation Measure images via either the
traditional techniques involving a number of different frequencies
(regularly or irregularly spaced) with function <link
anchor="images:imagepol.rotationmeasure.function">rotationmeasure</link>
or via a new Fourier Technique for many regularly-spaced frequencies
with function <link
anchor="images:imagepol.fourierrotationmeasure.function">fourierrotationmeasure</link>.

\end{itemize}

</description>

<!--
   <method type="constructor" name="imagepol">
   <shortdescription>Create an Imagepol tool from an \casa\ image file</shortdescription>
   
<input>
  
     <param xsi:type="string" direction="in" name="infile">
     <description>Input image file name</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimagepol"/>
<description>

The constructor takes an image as its input.  This can be specified as
either the name of a disk file, or you can give a pre-existing Image
\tool.   If you give a disk file name, the disk image file may
be in \casa, \fits\ or Miriad format.  

The input image must have a Stokes axis.  The exact collection of Stokes
that the image has, determines what the Imagepol \tool\ can compute. 
Stokes I, Q, U, and V refer to total intensity, two components of linear
polarization, and circular polatization, respectively.  Therefore, if
you ask for linear polarization and the image only has Stokes I and V,
you will get an error. 

The input image may contain data at many frequencies.  For example, the
image may be a 4D image with axes RA, DEC, Stokes and Frequency (order
not important) where the Frequency axis is regularly sampled.  However,
the image may also contain many frequencies at irregular intervals. 
Such an image may be created with the Image \tool\ constructor called
<link anchor="images:image.imageconcat.function">imageconcat</link>.  It enables you to concatenate
images along an axis, and it allows irregular coordinate values along that
axis. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t-\t constructor Ex 1 \t-"
#- im = image('$AD/myimage')
#- p1 = imagepol(im)
#- p2 = imagepol('$AD/myimage')
po.imagepoltestimage('stokes.cube')
po.done()
p1 = ia.newimagefromfile('stokes.cube')
po.open(p1.torecord())
po.close()
p1.done()
po.open('stokes.cube')
#
"""
\end{verbatim}

We create two Imagepol \tools, one from an Image
\tool\ and one directly from the disk file.

</example>

<example>
\begin{verbatim}
"""
#
print "\t-\t constructor Ex 2 \t-"
#- im = imageconcat(infiles="im.f1 im.f2 im.f3 im.f4 im.f5", axis=4)
#- p = imagepol(im)
po.open('stokes.cube')
po.rotationmeasure(rm='rm', rmerr='rmerr', rmmax=500, maxpaerr=10)
po.close()
#
"""
\end{verbatim}

Say we have 5 images, each with axes RA, DEC, Stokes, and Frequency in
that order.  We make an Image \tool\ which concatenates these images
along the frequency axis - you have ordered them in increasing or
decreasing frequency order.  Note that the Image \tool\ is virtual - it
is not written to an output file.  This just means that the data are
read from the input images as needed.  The Imagepol \tool\ is then made
from this virtual Image \tool.   We then compute the Rotation Measure
and Rotation Measure error images with the traditional method and
write them out to disk.

</example>
</method>

--> 

 
   <method type="function" name="imagepoltestimage">
   <shortdescription>Attach the Imagepol tool to a test image file</shortdescription>
   
<input>
  
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name</description>
     <value>imagepol.iquv</value>
     </param>
  
     <param xsi:type="doubleArray" direction="in" name="rm">
     <description>Rotation Measure (rad/m/m).  Default is
     auto no-ambiguity determine.</description>
     <value>0.0</value>
     </param>
  
     <param xsi:type="double" direction="in" name="pa0">
     <description>Position angle (degrees) at zero wavelength</description>
     <value>0.0</value>
     </param>
  
     <param xsi:type="double" direction="in" name="sigma">
     <description>Fractional noise level</description>
     <value>0.01</value>
     </param>
  
     <param xsi:type="int" direction="in" name="nx">
     <description>Shape of image in x direction</description>
     <value>32</value>
     </param>
  
     <param xsi:type="int" direction="in" name="ny">
     <description>Shape of image in y direction</description>
     <value>32</value>
     </param>
  
     <param xsi:type="int" direction="in" name="nf">
     <description>Shape of image in frequency direction</description>
     <value>32</value>
     </param>
  
     <param xsi:type="double" direction="in" name="f0">
     <description>Reference frequency (Hz)</description>
     <value>1.4e9</value>
     </param>
  
     <param xsi:type="double" direction="in" name="bw">
     <description>Bandwidth (Hz)</description>
     <value>128.0e6</value>
     </param>
</input>
<returns xsi:type="bool"/>

<description>

This function can be used to generate a test image and then
attach the Imagepol \tool\ to it.

The test image is 4-dimensional (RA, DEC, Stokes and Frequency).  The
Stokes axis holds I,Q,U and V.  The source is just a constant I (if you
don't add noise all spatial pixels will be identical) and V.  Q and U
vary with frequency according to the specified Rotation Measure
components (no attempt to handle bandwidth smearing within channels is
made).  The actual values of I,Q,U, and V are chosen arbitrarily
otherwise (could be added as arguments if desired). 

You can use this image, in particular, to explore the Rotation Measure
algorithms in functions <link anchor="images:imagepol.rotationmeasure.function">rotationmeasure</link> and
<link anchor="images:imagepol.fourierrotationmeasure.function">fourierrotationmeasure</link>. 

If you don't specify the Rotation Measure, then it is chosen for you so
that there is no position angle ambiguity between adjacent channels
(the value will be sent to the Logger).

The noise added to the image is specified as a fraction of the total
intensity (constant).  Gaussian noise with a standard deviation of
{\stfaf sigma * $I_{max}$} is then added to the image. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t imagepoltestimage Ex 1 \t----"
po.imagepoltestimage(outfile='imagepoltestimage', rm=200)
po.rotationmeasure(rm='rm.out',rmmax=250)
ia.open('rm.out')
ia.statistics()
#viewer()
#
"""
\end{verbatim}
</example>
</method>
 

 
   <method type="function" name="complexlinpol">
   <shortdescription>Complex linear polarization</shortdescription>
   
<input>
  
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name</description>
     </param>
</input>
<returns xsi:type="bool">T or fail</returns>

<description> 

This function <!-- (short-hand name {\stff clp}) --> produces
the complex linear polarization; $Q+iU$ and writes
it to a disk image file.

The <link anchor="images:image">Image</link> \tool\ cannot yet handle Complex
images.  You must therefore write the Complex image to disk.  The 
<link anchor="display:viewer">Viewer</link> can display Complex images. Also the
<link anchor="tables:table">Table</link> tool can access Complex images.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t complexlinpol Ex 1 \t----"
po.open('stokes.image')
po.complexlinpol('cplx')
tb.open('cplx')
#tb.browse()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="complexfraclinpol">
   <shortdescription>Complex fractional linear polarization</shortdescription>
   
<input>
  
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name</description>
     </param>
</input>
<returns xsi:type="bool">T or fail</returns>

<description> 

This function <!-- (short-hand name {\stff clp}) --> produces
the complex fractional linear polarization; $(Q+iU)/I$ and writes
it to a disk image file.

The <link anchor="images:image">Image</link> \tool\ cannot yet handle Complex
images.  You must therefore write the Complex image to disk.  The 
<link anchor="display:viewer">Viewer</link> can display Complex images. Also the
<link anchor="tables:table">Table</link> tool can access Complex images.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t complexfraclinpol Ex 1 \t----"
po.open('stokes.image')
po.complexfraclinpol('cplx2')
tb.open('cplx2')
#tb.browse()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="depolratio">
   <shortdescription>Linear depolarization ratio</shortdescription>
   
<input>
  
     <param xsi:type="string" direction="in" name="infile">
     <description>Other image</description>
     </param>
  
     <param xsi:type="bool" direction="in" name="debias">
     <description>Debias the linearly polarized intensity ?</description>
     <value>false</value>
     </param>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
  
     <param xsi:type="double" direction="in" name="sigma">
     <description>Standard deviation of thermal noise.  Default is auto determined.</description>
     <value>-1</value>
     </param>
  
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimage"/>

<description> 

This function <!-- (short-hand name {\stff dr}) --> returns the linear
depolarization ratio computed from two frequencies; this is the ratio of
the fractional linear polarization at the two frequencies.  Generally
this is done when you have generated two images, each at a different frequency
(continuum work). Thus if the fractional linear polarization images are
$m_{\nu 1}$ and $m_{\nu 2}$ then the depolarization ratio is 
$m_{\nu 1}/ m_{\nu 2}$.

This function operates with two images; the first (at frequency $\nu
1$) is the one attached to your Imagepol \tool.  The second (at
frequency $\nu 2$) is supplied via the argument {\stfaf infile}, which
is <!-- can be an Image \tool, or--> a String holding the name of the
\imagefile.

In generating the depolarization ratio, you may optionally debias the 
linearly polarized intensity.  This requires the standard deviation of
the thermal noise.  You can either supply it if you know it, or it will
be worked out for you with outliers from the mean clipped at the
specified level. 

You can get the depolarization ratio error image with function
<link anchor="images:imagepol.sigmadepolratio.function">sigmadepolratio</link>.

</description>

<example>
\begin{verbatim}
"""
#
#print "\t----\t depolratio Ex 1 \t----"
#po.open('stokes.4800')
#dpr = po.depolratio(infile='stokes.8300')        # m_4800 / m_8300
#edpr = po.sigmadepolratio(infile='stokes.8300');
#dpr.done()
#edpr.done()
#
"""
\end{verbatim}
</example>
</method>

   <method type="function" name="close">
   <shortdescription>Close the image tool</shortdescription>
   
<returns xsi:type="bool"/>
<description>

This function closes the imagepol tool.  This means that it detaches
the tool from its \imagefile\ (flushing all the changes first).  The
imagepol tool is ``null'' after this change (it is not destroyed) and
calling any \toolfunction\ other than <link
anchor="images:image.open.function">open</link> will result in an
error.

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t close Ex 1 \t----"
# First create image and attach it to imagepol tool
po.imagepoltestimage('myimagepol')
po.close()              # Detaches image from Imagepol tool
print "!!!EXPECT ERROR HERE!!!"
po.summary()            # No image so this results in an error.
po.open('myimagepol')   # Image is reattached
po.summary()            # No error
po.close()
#
"""
\end{verbatim}
</example>
</method>
 
   <method type="function" name="done">
   <shortdescription>Close this Imagepol tool</shortdescription>
   
<returns xsi:type="bool">T or fail</returns>
<description>

<!--
If you no longer need to use an Imagepol \tool, calling this function
will free up its resources.  That is, it destroys the \tool.  You can no
longer call any functions on the \tool\ after it has been {\stff done}. 
Note that the underlying \imagefile\ with which the \tool\ was
constructed is not deleted.  In addition, any images that you created
with the Imagepol \toolfunctions\ will also still be viable. 
-->

This function is the same as close().

</description>
<example>
\begin{verbatim}
"""
#
print "\t----\t done Ex 1 \t----"
po.open('myimagepol')
po.done()             # Detaches image from Imagepol tool
print "!!!EXPECT ERROR HERE!!!"
po.summary()          # No image so this results in an error.
po.open('myimagepol') # Image is reattached
po.summary()          # No error
po.done()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="fourierrotationmeasure">
   <shortdescription>Find Rotation Measure (Fourier approach)</shortdescription>
   
<input>
  
     <param xsi:type="string" direction="in" name="complex">
     <description>Output complex linear polarization image file name.  Default is unset.</description>
     <value></value>
     </param>
  
     <param xsi:type="string" direction="in" name="amp">
     <description>Output linear polarization amplitude image file name  Default is unset.</description>
     <value></value>
     </param>
  
     <param xsi:type="string" direction="in" name="pa">
     <description>Output linear polarization position angle (degrees) image file name  Default is unset.</description>
     <value></value>
     </param>
  
     <param xsi:type="string" direction="in" name="real">
     <description>Output linear polarization real image file name  Default is unset.</description>
     <value></value>
     </param>
  
     <param xsi:type="string" direction="in" name="imag">
     <description>Output linear polarization imaginary angle image file name  Default is unset.</description>
     <value></value>
     </param>
  
     <param xsi:type="bool" direction="in" name="zerolag0">
     <description>Force zero lag to 0 ?</description>
     <value>false</value>
     </param>
</input>
<returns xsi:type="bool">T or fail</returns>

<description>

This function <!-- (short-hand name {\stff frm}) --> will only work if
you attach the Imagepol \tool\ (using open) to an image containing
Stokes Q and U, and a regular frequency axis.  It Fourier transforms
the complex linear polarization (Q+iU) over the spectral axis to the
rotation measure axis.  Thus, if your input image contained
RA/DEC/Stokes/Frequency, the output image would be
RA/DEC/RotationMeasure.  The Rotation Measure axis has as many pixels
as the spectral axis.

This method enables you to see the polarization as a function of
Rotation Meausure.  Its main use is when searching for large RMs.  See
Killeen, Fluke, Zhao and Ekers (1999, preprint) for a description of
this method (or http://www.atnf.csiro.au/\verb+~+nkilleen/rm.ps) and its
advantages over the traditional method
(<link anchor="images:imagepol.rotationmeasure.function">rotationmeasure</link>) of
extracting the Rotation Measure. 

Although you can write out the complex polarization image with the
argument {\stfaf complex}, you can't do much with it because Image
\tools\ cannot handle complex images.  Hence you can
also write out the complex linear polarization image in any or all of
the other forms. 

The argument {\stfaf zerolag0} allows you to force the zero lag (or
central bin) of the Rotation Measure spectrum to zero (effectively by
subtracting the mean of Q and U from the Q and U images).  This may
avoid Gibbs phenomena from any strong low Rotation Measure signal which
would normally fall into the central bin. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t fourierrotationmeasure Ex 1 \t----"
po.imagepoltestimage(outfile='iquv.im', rm=[5.0e5,1e6], nx=8, ny=8, nf=512, 
                     f0=1.4e9, bw=8e6)
po.fourierrotationmeasure(amp='amp')
ia.open('amp')
ia.statistics()
#viewer()                     # And reorder to put RM along X-axis 
#
"""
\end{verbatim}
</example>
In this example we give two RM components and recover them.
</method>

 
   <method type="function" name="fraclinpol">
   <shortdescription>Fractional linear polarization</shortdescription>
   
<input>
  
     <param xsi:type="bool" direction="in" name="debias">
     <description>Debias the linearly polarized intensity ?</description>
     <value>false</value>
     </param>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
  
     <param xsi:type="double" direction="in" name="sigma">
     <description>Standard deviation of thermal noise.  Default is auto determined.</description>
     <value>-1</value>
     </param>
  
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimage"/>

<description> This function <!-- (short-hand name {\stff flp}) -->
returns the fractional linear polarization; $\sqrt{(Q^2+U^2)}/I$.

You may optionally debias the polarized intensity.  This requires the
standard deviation of the thermal noise.  You can either supply it if
you know it, or it will be worked out for you with outliers from the
mean clipped at the specified level. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t fraclinpol Ex 1 \t----"
po.open('stokes.image')
flp = po.fraclinpol()
flp.summary()
flp.done()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="fractotpol">
   <shortdescription>Fractional total polarization</shortdescription>
   
<input>
  
     <param xsi:type="bool" direction="in" name="debias">
     <description>Debias the total polarized intensity ?</description>
     <value>false</value>
     </param>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
  
     <param xsi:type="double" direction="in" name="sigma">
     <description>Standard deviation of thermal noise.  Default is auto determined.</description>
     <value>-1</value>
     </param>
  
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimage"/>

<description> This function <!-- (short-hand name {\stff ftp}) -->
returns the fractional linear polarization; $\sqrt{(Q^2+U^2+V^2)}/I$.

You may optionally debias the polarized intensity.  This requires the
standard deviation of the thermal noise.  You can either supply it if
you know it, or it will be worked out for you with outliers from the
mean clipped at the specified level. 

If your image contains only Q and U, or only V, then just
those components contribute to the total polarized intensity.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t fractotpol Ex 1 \t----"
po.open('stokes.image')
ftp = po.fractotpol()
ftp.statistics()
ftp.done()
#
"""
\end{verbatim}
</example>
</method>


<!-- 
   <method type="function" name="id">
   <shortdescription>Return the fundamental identifier of this tool</shortdescription>
   
<description>

This function should be of little interest to users. It returns the
fundamental identifier of the tool.

</description>

<returns xsi:type="string">ToolID - a record</returns>

<example>
\begin{verbatim}
- p = imagepol('stokes.cube')
- p.id()
[sequence=15, pid=20665, time=941768782, host=elara, agentid=3] 
\end{verbatim}
</example>
</method>
-->
 
   <method type="function" name="linpolint">
   <shortdescription>Linearly polarized intensity</shortdescription>
   
<input>
  
     <param xsi:type="bool" direction="in" name="debias">
     <description>Debias the linearly polarized intensity ?</description>
     <value>false</value>
     </param>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
  
     <param xsi:type="double" direction="in" name="sigma">
     <description>Standard deviation of thermal noise.  Default is auto determined.</description>
     <value>-1</value>
     </param>
  
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimage"/>

<description> This function <!-- (short-hand name {\stff lpi}) -->
returns the linearly polarized intensity; $\sqrt{(Q^2+U^2)}$. 

You may optionally debias the polarized intensity.  This requires the
standard deviation of the thermal noise.  You can either supply it if
you know it, or it will be worked out for you with outliers from the
mean clipped at the specified level. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t linpolint Ex 1 \t----"
po.open('stokes.image')
lpi = po.linpolint()
lpi.statistics()
lpi.done()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="linpolposang">
   <shortdescription>Linearly polarized position angle</shortdescription>

<input>   
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimage"/>

<description> 

This function <!-- (short-hand name {\stff lppa}) --> returns the linearly
polarized position angle image ($0.5 \tan^{-1}(U/Q)$) in degrees. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t linpolposang Ex 1 \t----"
po.open('stokes.image')
lppa = po.linpolposang()
lppa.statistics()
lppa.done()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="makecomplex">
   <shortdescription>Make a Complex image</shortdescription>
   
<input>
  
     <param xsi:type="string" direction="in" name="complex">
     <description>Output complex image file name.  Must be specified.</description>
     </param>
  
     <param xsi:type="string" direction="in" name="real">
     <description>Input real image file name. Default is unset.</description>
     <value></value>
     </param>
  
     <param xsi:type="string" direction="in" name="imag">
     <description>Input imaginary image file name. Default is unset.</description>
     <value></value>
     </param>
  
     <param xsi:type="string" direction="in" name="amp">
     <description>Input amplitude image file name. Default is unset.</description>
     <value></value>
     </param>
  
     <param xsi:type="string" direction="in" name="phase">
     <description>Input phase image file name. Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="bool">T or fail</returns>

<description> 

This function generates a Complex \imagefile\ from either
a real and imaginary, or an amplitude and phase pair of images.
If you give a linear position angle image for the phase, 
it will be multipled by two before the real and imaginary
parts are formed.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t makecomplex Ex 1 \t----"
po.open('stokes.image')
po.complexlinpol('qu.cplx1')        
q = po.stokesq()
u = po.stokesu()
q2 = q.subimage('q',overwrite=true)
u2 = u.subimage('u',overwrite=true)
po.makecomplex('qu.cplx2', real='q', imag='u')
po.close()
#
"""
\end{verbatim}
In this example we make two complex linear polarization
images which should be identical.   
</example>
</method>


   <method type="function" name="open">
   <shortdescription>Open a new image with this imagepol tool</shortdescription>
   

<input>
  
     <param xsi:type="any" direction="in" name="image">
     <description>image file name or image record (generated by
     ia.torecord())
     </description>
     <any type="variant"/>
     <value></value>
     </param>
</input>
<returns xsi:type="bool">T or fail</returns>
<description>

Before polarimetric analysis can commence, an \imagefile\ must be
attached to the imagepol tool using the open function.  Also, use this
function when you are finished analyzing the current \imagefile\ and
want to attach to another one.  This function detaches the \imagetool\
from the current \imagefile, if one exists, and reattaches it (opens)
to the new \imagefile.

The input image file may be in native \casa, \fits, or Miriad  
format.  Look \htmlref{here}{IMAGES:FOREIGNIMAGES}  for more
information on foreign images.

The input image must have a Stokes axis. The exact collection of
Stokes that the image has, determines what the Imagepol tool can
compute. Stokes I, Q, U, and V refer to total intensity, two
components of linear polarization, and circular polatization,
respectively. Therefore, if you ask for linear polarization and the
image only has Stokes I and V, you will get an error.

The input image may contain data at many frequencies. For example, the
image may be a 4D image with axes RA, DEC, Stokes and Frequency (order
not important) where the Frequency axis is regularly sampled. However,
the image may also contain many frequencies at irregular
intervals. Such an image may be created with the Image tool function
imageconcat. It enables you to concatenate images along an axis, and
it allows irregular coordinate values along that axis.

<!--
You could, of course, also create a new \imagetool\ and associate that
with the new \imagefile, but this saves you the \tool\ creation. 
-->

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t open Ex 1 \t----"
po.open('stokes.image')
po.close()
#
"""
\end{verbatim}
The {\stff open} function first closes the old \imagefile\ if one exists.
</example>
</method>

 

 
   <method type="function" name="pol">
   <shortdescription>Polarized quantities</shortdescription>
   
<input>
  
     <param xsi:type="string" direction="in" name="which">
     <description>Specify operation.
     One of 'lpi', 'tpi', 'lppa', 'flp', 'ftp' (case insensitive)
     </description>
     </param>
  
     <param xsi:type="bool" direction="in" name="debias">
     <description>Debias the polarized intensity ?</description>
     <value>false</value>
     </param>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
  
     <param xsi:type="double" direction="in" name="sigma">
     <description>Standard deviation of thermal noise.  Default is auto determined.</description>
     <value>-1</value>
     </param>
  
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimage"/>

<description>

This function just packages the other specific polarization
functions into one where you specify an operation with the
argument {\stfaf which} (can be useful for scripts).  
This argument can take the values:

\begin{itemize}
\item 'lpi' - linearly polarized intensity (function 
  <link anchor="images:imagepol.linpolint.function">linpolint</link>)

\item 'tpi' - total polarized intensity (function 
   <link anchor="images:imagepol.totpolint.function">totpolint</link>)

\item 'lppa' linearly polarized position angle (function 
    <link anchor="images:imagepol.linpolposang.function">linpolposang</link>)

\item 'flp' - fractional linear polarization (function 
   <link anchor="images:imagepol.fraclinpol.function">fraclinpol</link>)

\item 'ftp' - fractional total polarized intensity (function 
   <link anchor="images:imagepol.fractotpol.function">fractotpol</link>)

\end{itemize}

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t pol Ex 1 \t----"
po.open('stokes.image')
lpi = po.pol('lpi')
lpi.statistics()
lpi.done()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="rotationmeasure">
   <shortdescription>Find Rotation Measure (traditional approach)</shortdescription>
   
<input>
  
     <param xsi:type="string" direction="in" name="rm">
     <description>Output Rotation Measure image file name. Default is unset.</description>
     <value></value>
     </param>
  
     <param xsi:type="string" direction="in" name="rmerr">
     <description>Output Rotation Measure error image file name. Default is unset.</description>
     <value></value>
     </param>
  
     <param xsi:type="string" direction="in" name="pa0">
     <description>Output position angle (degrees) at zero wavelength image file name. Default is unset.</description>
     <value></value>
     </param>
  
     <param xsi:type="string" direction="in" name="pa0err">
     <description>Output position angle (degrees) at zero wavelength error image file name. Default is unset.</description>
     <value></value>
     </param>
  
     <param xsi:type="string" direction="in" name="nturns">
     <description>Output number of turns image file name. Default is unset.</description>
     <value></value>
     </param>
  
     <param xsi:type="string" direction="in" name="chisq">
     <description>Output reduced chi squared image file name. Default is unset.</description>
     <value></value>
     </param>
  
     <param xsi:type="double" direction="in" name="sigma">
     <description>Estimate of the thermal noise.  Default is auto estimate.</description>
     <value>-1</value>
     </param>
  
     <param xsi:type="double" direction="in" name="rmfg">
     <description>Foreground Rotation Measure (rad/m/m) to subtract</description>
     <value>0.0</value>
     </param>
  
     <param xsi:type="double" direction="in" name="rmmax">
     <description>Maximum Rotation Measure (rad/m/m) to solve for</description>
     <value>0.0</value>
     </param>
  
     <param xsi:type="double" direction="in" name="maxpaerr">
     <description>Maximum input position angle error (degrees) to allow</description>
     <value>1e30</value>
     </param>
  
     <param xsi:type="string" direction="in" name="plotter">
     <description>Name of plotter.  Default is none.</description>
     <value></value>
     </param>
  
     <param xsi:type="int" direction="in" name="nx">
     <description>Number of plots in x direction</description>
     <value>5</value>
     </param>
  
     <param xsi:type="int" direction="in" name="ny">
     <description>Number of plots in y direction</description>
     <value>5</value>
     </param>
</input>
<returns xsi:type="bool">T or fail</returns>

<description>

This function <!-- (short-hand name {\stff rm}) --> generates the Rotation
Measure image from a collection of different frequencies.  It will only
work if the Imagepol \tool\ is attached to an image containing
Stokes $Q$ and $U$, and a frequency axis (regular or irregular) with at
least 2 pixels.  It will work out the position angle images for you.

See also the <link anchor="images:imagepol.fourierrotationmeasure.function">fourierrotationmeasure</link> 
function for a new Fourier-based approach.

Rotation Measure algorithms that work robustly are not common.  The main
problem is in trying to account for the $n- \pi$ ambiguity (see Leahy et
al, Astronomy \& Astrophysics, 156, 234 or Killeen et al;
http://www.atnf.csiro.au/\verb+~+nkilleen/rm.ps). 

The algorithm that this function uses is that of Leahy et al.  Please
refer to their paper for details (their Appendix A.1).  But as in all
these algorithms, the basic process is that for each spatial pixel, a
vector of position angles (i.e.  at the different frequencies) is fit to
determine the Rotation Measure and the position angle at zero wavelength
(and their errors).   You can write out an image containing
the number of $n- \pi$ turns that were added to the data
at each spatial pixel and for which the best fit was found.
You can also write out the reduced chi-squared image for the
fits.

Note that as yet no assessment of curvature (i.e. deviation
from the simple linear position angle - $\lambda^2$ functional form)
is made.  

You can write out any or all of the output images.   

The argument {\stfaf sigma} gives the themal noise in Stokes Q and U.
By default it is worked out for you from the image data.  But if it proves 
to be inaccurate (maybe not many signal-free pixels), then you can
input it here.  This is used for working out the error in the
position angles (propagation of Gaussian errors).

The argument {\stfaf maxpaerr} specifies the maximum allowable error in
the position angle that is acceptable.  The default is an infinite
value.  From the standard propagation of errors, the error in the
linearly polarized position angle is determined from the Stokes $Q$ and
$U$ images (at each spatial pixel for each frequency).  At each spatial
pixel we do a fit to the position angle vector (i.e.  at the different
frequencies) to determine the Rotation Measure.  If the position angle
error for any pixel in the vector exceeds the specified value, it is
dropped from the fit.     The process generates an error for the
fit and this is used to compute the errors in the output
images.  

Note that {\stfaf maxpaerr} is {\it not} used to specify that any pixel
for which the output position angle error exceeds this value
should be masked out.

The argument {\stfaf rmfg} is used to specify a foreground RM value.  For
example, you may know the mean RM in some direction out of the Galaxy,
then including this can aid the algorithm (reduces ambiguity).

The argument {\stfaf rmmax} specifies the maximum absolute RM that
should be solved for.  This quite an important parameter.  If you leave
it at the default, zero, basically, no ambiguity handling will be
invoked.  So you must have some idea of what you are looking for - this
is the basic problem with Rotation Measure algorithms. 

<!--
Plotting can be done with a PGPLOT device via argument {\stfaf plotter}.
The syntax is {\stfaf plotter=name/type}.  For example {\stfaf
plotter='plot1.ps/ps'} (disk postscript file) or {\stfaf plotter='1/xs'}
(X-windows device) or {\stfaf plotter='plot/glish'} (\glish\ PGplotter).
The plots show the used position angle data (after ambiguity
adjustments), position angle errors, and fit for each profile. 
The pixel location of each profile and reduced chi-square of
the fit are written in the title.
-->

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t rotationmeasure Ex 1 \t----"
#im = ia.imageconcat(outfile='stokes.image', 
#                    infiles="im.f1 im.f2 im.f3 im.f4 im.f5", axis=4)
po.open('stokes.image')
ok = po.rotationmeasure(rm='rm', rmerr='rmerr', rmmax=800, maxpaerr=10)
#
"""
\end{verbatim}

Say we have 5 images, each with axes RA, DEC, Stokes, and Frequency in
that order.  We use the Image \tool\ to concatenate these images
along the frequency axis - you have ordered them in increasing or
decreasing frequency order. <!-- Note that the Image \tool\ is virtual - it
is not written to an output file.  This just means that the data are
read from the input images as needed.  The Imagepol \tool\ is then made
from this virtual Image \tool.-->   We then compute the Rotation Measure
and Rotation Measure error images with the traditional method and
write them out to disk.

</example>
</method>

 
   <method type="function" name="sigma">
   <shortdescription>Find best guess at thermal noise</shortdescription>
   
<input>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
</input>
<returns xsi:type="double">Float or fail</returns>

<description> 

This function returns the standard deviation from V, Q\&U or I in that
order of precedence.  It is attempting to give you the best estimate of
the thermal noise it can from the data.  Outliers from the mean are
clipped at the specified level. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t sigma Ex 1 \t----"
po.open('stokes.image')
sigma = po.sigma()
print "sigma=", sigma
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="sigmadepolratio">
   <shortdescription>Error in linear depolarization ratio</shortdescription>
   
<input>
  
     <param xsi:type="string" direction="in" name="infile">
     <description>Other image.  Required input.</description>
     </param>
  
     <param xsi:type="bool" direction="in" name="debias">
     <description>Debias the linearly polarized intensity ?</description>
     <value>false</value>
     </param>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
  
     <param xsi:type="double" direction="in" name="sigma">
     <description>Standard deviation of thermal noise.  Default is auto determined.</description>
     <value>-1</value>
     </param>
  
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimage"/>

<description> 

This function <!-- (short-hand name {\stff edr}) --> returns the error
in the linear depolarization ratio computed from two frequencies; this
is the ratio of the fractional linear polarization at the two
frequencies.  Generally this is done when you have generated two
images, each at a different frequency (continuum work). Thus if the
fractional linear polarzation images are $m1$ and $m2$ then the
depolarization ratio is $m1/m2$.

This function operates with two images; the first is attached
to the Imagepol \tool.  The second is supplied via the
argument {\stfaf infile}, which is <!-- can be an Image \tool, or--> a String
holding the name of the \imagefile.

In generating the depolarization ratio, and hence its error, you may
optionally debias the  linearly polarized intensity.  This requires the
standard deviation of the thermal noise.  You can either supply it if
you know it, or it will be worked out for you with outliers from the
mean clipped at the specified level. 

You can get the depolarization ratio image with function
<link anchor="images:imagepol.depolratio.function">depolratio</link>.

</description>

<example>
\begin{verbatim}
\begin{verbatim}
"""
#
#print "\t----\t sigmadepolratio Ex 1 \t----"
#po.open('stokes.4800')
#dpr = po.depolratio('stokes.8300')
#edpr = po.sigmadepolratio('stokes.8300');
#dpr.done()
#edpr.done()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="sigmafraclinpol">
   <shortdescription>Error in fractional linear polarization</shortdescription>
   
<input>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
  
     <param xsi:type="double" direction="in" name="sigma">
     <description>Standard deviation of thermal noise.  Default is auto determined.</description>
     <value>-1</value>
     </param>
  
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimage"/>

<description> 

This function <!-- (short-hand name {\stff sflp}) --> returns the
error (standard deviation) of the fractional linear polarization.
This result comes from standard propagation of errors.  The result is
an on-the-fly Image tool as the error is signal-to-noise ratio
dependent.

This function requires the standard deviation of the thermal noise.  You
can either supply it if you know it, or it will be worked out for you
with outliers from the mean clipped at the specified level. 

</description>

<example>
\begin{verbatim}
\begin{verbatim}
"""
#
print "\t----\t sigmafraclinpol Ex 1 \t----"
po.open('stokes.image')
sigflp = po.sigmafraclinpol()
sigflp.statistics()
sigflp.done()           # free up resources
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="sigmafractotpol">
   <shortdescription>Error in fractional total polarization</shortdescription>
   
<input>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
  
     <param xsi:type="double" direction="in" name="sigma">
     <description>Standard deviation of thermal noise.  Default is auto determined.</description>
     <value>-1</value>
     </param>
  
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimage"/>

<description> 

This function <!-- (short-hand name {\stff sftp}) --> returns the
error (standard deviation) of the fractional total polarization.  This
result comes from standard propagation of errors.  The result is an
on-the-fly Image tool as the error is signal-to-noise ratio dependent.

This function requires the standard deviation of the thermal noise.  You
can either supply it if you know it, or it will be worked out for you
with outliers from the mean clipped at the specified level. 

</description>

<example>
\begin{verbatim}
\begin{verbatim}
"""
#
print "\t----\t sigmafractotpol Ex 1 \t----"
po.open('stokes.image')
sigftp = po.sigmafractotpol()
sigftp.statistics()
sigftp.done()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="sigmalinpolint">
   <shortdescription>Error in linearly polarized intensity</shortdescription>
   
<input>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
  
     <param xsi:type="double" direction="in" name="sigma">
     <description>Standard deviation of thermal noise.  Default is auto determined.</description>
     <value>-1</value>
     </param>
  
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="double">Float or fail</returns>

<description> 

This function <!-- (short-hand name {\stff slpi}) --> returns the error (standard
deviation) of the linearly polarized intensity; $\sqrt{(Q^2+U^2)}$. 
This result comes from standard propagation of statistical errors.
The result is a float as the error is not signal-to-noise
ratio dependent

This function requires the standard deviation of the thermal noise.  You
can either supply it if you know it, or it will be worked out for you
with outliers from the mean clipped at the specified level. 

</description>

<example>
\begin{verbatim}
\begin{verbatim}
"""
#
print "\t----\t sigmalinpolint Ex 1 \t----"
po.open('stokes.image')
siglpi = po.sigmalinpolint()
print "siglpi=", siglpi
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="sigmalinpolposang">
   <shortdescription>Error in linearly polarized position angle</shortdescription>
   
<input>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
  
     <param xsi:type="double" direction="in" name="sigma">
     <description>Standard deviation of thermal noise.  Default is auto determined.</description>
     <value>-1</value>
     </param>
  
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimage"/>

<description> 

This function <!-- (short-hand name {\stff slppa}) --> returns the
error (standard deviation) of the linearly polarized position angle
($0.5 \tan^{-1}(U/Q)$$\sqrt{(Q^2+U^2)}$) in degrees.  This result
comes from standard propagation of errors.  The result is an
on-the-fly Image tool as the error is signal-to-noise ratio dependent.

This function requires the standard deviation of the thermal noise.  You
can either supply it if you know it, or it will be worked out for you
with outliers from the mean clipped at the specified level. 

</description>

<example>
\begin{verbatim}
\begin{verbatim}
"""
#
print "\t----\t sigmalinpolposang Ex 1 \t----"
po.open('stokes.image')
siglppa = po.sigmalinpolposang()
siglppa.statistics()
siglppa.done()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="sigmastokes">
   <shortdescription>Find standard deviation of specified Stokes data</shortdescription>
   
<input>
  
     <param xsi:type="string" direction="in" name="which">
     <description>Must specify Stokes parameter.
     One of 'I', 'Q', 'U', 'V' (case insensitive)
     </description>
     </param>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
</input>
<returns xsi:type="double">Float or fail</returns>

<description> 

This function <!-- (short-hand name {\stff ss}) --> returns the standard
deviation of the noise for the specified Stokes.  Outliers from the mean
are clipped at the specified level. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t sigmastokes Ex 1 \t----"
po.open('stokes.image')
sigq = po.sigmastokes('q', 10.0)
print "sigq=", sigq
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="sigmastokesi">
   <shortdescription>Find standard deviation of Stokes I data</shortdescription>
   
<input>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
</input>
<returns xsi:type="double">Float or fail</returns>

<description> 

This function <!-- (short-hand name {\stff ssi}) --> returns the standard deviation of the noise for the
Stokes I data.  Outliers from the mean are clipped at the specified
level. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t sigmastokesi Ex 1 \t----"
po.open('stokes.image')
sigi = po.sigmastokesi(10.0)
print "sigi=", sigi
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="sigmastokesq">
   <shortdescription>Find standard deviation of Stokes Q data</shortdescription>
   
<input>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
</input>
<returns xsi:type="double">Float or fail</returns>

<description> 

This function <!-- (short-hand name {\stff ssq}) --> returns the standard deviation of the noise for the
Stokes Q data.  Outliers from the mean are clipped at the specified
level. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t sigmastokesq Ex 1 \t----"
po.open('stokes.image')
sigq = po.sigmastokesq(10.0)
print "sigq=", sigq
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="sigmastokesu">
   <shortdescription>Find standard deviation of Stokes U data</shortdescription>
   
<input>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
</input>
<returns xsi:type="double">Float or fail</returns>

<description> 

This function <!-- (short-hand name {\stff ssu}) --> returns the standard
deviation of the noise for the Stokes U data.  Outliers from the mean
are clipped at the specified level. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t sigmastokesu Ex 1 \t----"
po.open('stokes.image')
sigu = po.sigmastokesu(10.0)
print "sigu=", sigu
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="sigmastokesv">
   <shortdescription>Find standard deviation of Stokes V data</shortdescription>
   
<input>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
</input>
<returns xsi:type="double">Float or fail</returns>

<description> 

This function <!-- (short-hand name {\stff ssv}) --> returns the standard
deviation of the noise for the Stokes V data.  Outliers from the mean
are clipped at the specified level. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t sigmastokesv Ex 1 \t----"
po.open('stokes.image')
sigv = po.sigmastokesv(10.0)
print "sigv=", sigv
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="sigmatotpolint">
   <shortdescription>Error in total polarized intensity</shortdescription>
   
<input>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
  
     <param xsi:type="double" direction="in" name="sigma">
     <description>Standard deviation of thermal noise.  Default is auto determined.</description>
     <value>-1</value>
     </param>
</input>
<returns xsi:type="double">Float or fail</returns>

<description> 

This function <!-- (short-hand name {\stff stpi}) --> returns the error (standard
deviation) of the total polarized intensity; $\sqrt{(Q^2+U^2+V^2)}$. 
This result comes from standard propagation of statistical errors.
The result is a float as the error is not signal-to-noise
ratio dependent

This function requires the standard deviation of the thermal noise.  You
can either supply it if you know it, or it will be worked out for you
with outliers from the mean clipped at the specified level. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t sigmastotpolint Ex 1 \t----"
po.open('stokes.image')
sigtpi = po.sigmatotpolint()
print "sigtpi=", sigtpi
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="stokes">
   <shortdescription>Stokes</shortdescription>
   
<input>
  
     <param xsi:type="string" direction="in" name="which">
     <description>Must specify Stokes.
     One of 'I', 'Q', 'U', 'V' (case insensitive)
     </description>
     </param>
  
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimage"/>

<description>

This function returns an on-the-fly image tool containing the
specified Stokes only.  This interface can be useful for scripts.

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t stokes Ex 1 \t----"
po.open('stokes.image')
q = po.stokes('q')
q.statistics()
q.done()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="stokesi">
   <shortdescription>Stokes I</shortdescription>
   
<input>
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimage"/>

<description>
This function returns an on-the-fly image tool containing Stokes I only.
</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t stokesi Ex 1 \t----"
po.open('stokes.image')
i = po.stokesi()
i.statistics()
i.done()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="stokesq">
   <shortdescription>Stokes Q</shortdescription>
   
<input>
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimage"/>

<description>
This function returns an on-the-fly image tool containing Stokes Q only.
</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t stokesq Ex 1 \t----"
po.open('stokes.image')
q = po.stokesq()
q.statistics()
q.done()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="stokesu">
   <shortdescription>Stokes U</shortdescription>
   
<input>
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimage"/>

<description>
This function returns an on-the-fly image tool containing Stokes U only.
</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t stokesu Ex 1 \t----"
po.open('stokes.image')
u = po.stokesu()
u.statistics()
u.done()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="stokesv">
   <shortdescription>Stokes V</shortdescription>
   
<input>
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimage"/>

<description>
This function returns an on-the-fly image tool containing Stokes V only.
</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t stokesv Ex 1 \t----"
po.open('stokes.image')
v = po.stokesv()
v.statistics()
v.done()
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="summary">
   <shortdescription>Summarise Imagepol tool</shortdescription>
   

<returns xsi:type="bool"/>

<description>

This function just lists a summary of the Imagepol \tool\ to the logger. 
Currently it just summarizes the image to which the tool is attached.


</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t summary Ex 1 \t----"
po.open('stokes.image')
po.summary()
#
#Image name       : stokes.image
#Object name      :
#Image type       : PagedImage
#Image quantity   : Intensity
#Pixel mask(s)    : None
#Region(s)        : None
#
#Direction reference : J2000
#Spectral  reference : TOPO
#Velocity  type      : RADIO
#Rest frequency      : 1.4e+09 Hz
#Telescope           : UNKNOWN
#Observer            : UNKNOWN
#Date observation    : UNKNOWN
#
#Axis Coord Type      Name             Proj Shape Tile   Coord value at pixel    Coord incr Units
#------------------------------------------------------------------------------------------------
#0    0     Direction Right Ascension   SIN    32   32  00:00:00.000    16.00 -6.000000e+01 arcsec
#1    0     Direction Declination       SIN    32   32 +00.00.00.000    16.00  6.000000e+01 arcsec
#2    1     Stokes    Stokes                    4    4       I Q U V
#3    2     Spectral  Frequency                32   32       1.4e+09    16.00  4.000000e+06 Hz
#                     Velocity                                     0    16.00 -8.565499e+02 km/s
#
#
"""
\end{verbatim}
</example>
</method>

 
   <method type="function" name="totpolint">
   <shortdescription>Total polarized intensity</shortdescription>
   
<input>
  
     <param xsi:type="bool" direction="in" name="debias">
     <description>Debias the total polarized intensity ?</description>
     <value>false</value>
     </param>
  
     <param xsi:type="double" direction="in" name="clip">
     <description>Clip level for auto-sigma determination</description>
     <value>10.0</value>
     </param>
  
     <param xsi:type="double" direction="in" name="sigma">
     <description>Standard deviation of thermal noised.  Default is auto determined.</description>
     <value>-1</value>
     </param>
  
     <param xsi:type="string" direction="in" name="outfile">
     <description>Output image file name.  Default is unset.</description>
     <value></value>
     </param>
</input>
<returns xsi:type="casaimage"/>

<description> This function <!-- (short-hand name {\stff tpi}) -->
returns the total polarized intensity; $\sqrt{(Q^2+U^2+V^2)}$. 
If your image contains only Q and U, or only V, then just
those components contribute to the total polarized intensity.

You may optionally debias the polarized intensity.  This requires the
standard deviation of the thermal noise.  You can either supply it if
you know it, or it will be worked out for you with outliers from the
mean clipped at the specified level. 

</description>

<example>
\begin{verbatim}
"""
#
print "\t----\t totpolint Ex 1 \t----"
po.open('stokes.image')
tpi = po.totpolint()
tpi.statistics()
tpi.done()
#
"""
\end{verbatim}
</example>
</method>


<!-- 
   <task name="is\_imagepol">
   <shortdescription>Is this variable an imagepol tool ?</shortdescription>
   
<input>
  
     <param xsi:type="unknown" direction="in" name="thing">
     <description>Variable to test</description>
     <value>Glish variable</value>
     <value>None</value>
     </param>
</input>   

<returns xsi:type="unknown">T or F</returns>
<description>

Determine if this variable is an Imagepol \tool.  This is very useful in
scripts; for robustness, you check that the tool is an Imagepol \tool\
before you start applying Imagepol \toolfunctions.

</description>  

<example>
\begin{verbatim}
- p=imagepol('myimage')
- is_imagepol(p)
T 
- p2 = [10,20]   
- is_imagepol(p2)   
F
\end{verbatim}
</example>
</task>
 
   <task name="imagepoltest">
   <shortdescription>Run test suite for Imagepol tool</shortdescription>
   
<input>
  
     <param xsi:type="unknown" direction="in" name="which">
     <description>Which test to run</description>
     <value>Integer or vector of integers</value>
     <value>All tests</value>
     </param>
</input>
<returns xsi:type="unknown">T or Fail</returns>

<description>

Self-test of the Imagepol \tool.  This function includes forced errors. 
As long as the function finally returns with a {\cf T}, it has succeeded
(regardless of what error messages you might see). 

The function invokes many tests.  The first one is a general test of
everything at a basic level.  The succeeding tests work on individual
areas of the image module. 

Since the number of tests keeps growing, we don't tell you how many
there are here or exactly what they do ! If you give too large a value,
nothing will happen.  {\stfaf which} can be an integer or a vector of
integers. 

</description>

<example>
\begin{verbatim}
- imagepoltest()
\end{verbatim}

Runs all tests.
</example>

<example>
\begin{verbatim}
- imagepoltest(2)
\end{verbatim}

Runs test 2.
"""
#
exit()
#
"""
</example>

</task>

-->

</tool>


</casaxml>
"""
