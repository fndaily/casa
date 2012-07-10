import sys
import inspect

from asap.parameters import rcParams
from asap.scantable import scantable


# workaround for ipython, which redirects this if banner=0 in ipythonrc
#sys.stdout = sys.__stdout__
#sys.stderr = sys.__stderr__

def version():
    print  "ASAP %s(%s)"% (__version__, __date__)

def list_scans(t = scantable):
    print "The user created scantables are: ",
    globs=inspect.currentframe().f_back.f_locals.copy()
    out = [ k for k,v in globs.iteritems() \
                 if isinstance(v, scantable) and not k.startswith("_") ]
    print out
    return out

def commands():
    x = """\
[The scan container]
    scantable           - a container for integrations/scans
                          (can open asap/rpfits/sdfits and ms files)
        copy            - returns a copy of a scan
        get_scan        - gets a specific scan out of a scantable
                          (by name or number)
        drop_scan       - drops a specific scan out of a scantable
                          (by number)
        set_selection   - set a new subselection of the data
        get_selection   - get the current selection object
        summary         - print info about the scantable contents
        stats           - get specified statistic of the spectra in
                          the scantable
        stddev          - get the standard deviation of the spectra
                          in the scantable
        get_tsys        - get the TSys
        get_time        - get the timestamps of the integrations
        get_inttime     - get the integration time
        get_sourcename  - get the source names of the scans
        get_azimuth     - get the azimuth of the scans
        get_elevation   - get the elevation of the scans
        get_parangle    - get the parallactic angle of the scans
        get_coordinate  - get the spectral coordinate for the given row,
                          which can be used for coordinate conversions
        get_weather     - get the weather condition parameters
        get_unit        - get the current unit
        set_unit        - set the abcissa unit to be used from this
                          point on
        get_abcissa     - get the abcissa values and name for a given
                          row (time)
        get_column_names - get the names of the columns in the scantable
                           for use with selector.set_query
        set_freqframe   - set the frame info for the Spectral Axis
                          (e.g. 'LSRK')
        set_doppler     - set the doppler to be used from this point on
        set_dirframe    - set the frame for the direction on the sky
        set_instrument  - set the instrument name
        set_feedtype    - set the feed type
        get_fluxunit    - get the brightness flux unit
        set_fluxunit    - set the brightness flux unit
        set_sourcetype  - set the type of the source - source or reference
        create_mask     - return an mask in the current unit
                          for the given region. The specified regions
                          are NOT masked
        get_restfreqs   - get the current list of rest frequencies
        set_restfreqs   - set a list of rest frequencies
        shift_refpix    - shift the reference pixel of the IFs
        set_spectrum    - overwrite the spectrum for a given row
        get_spectrum    - retrieve the spectrum for a given
        get_mask        - retrieve the mask for a given
        flag            - flag selected channels in the data
        lag_flag        - flag specified frequency in the data
        save            - save the scantable to disk as either 'ASAP',
                          'SDFITS' or 'ASCII'
        nbeam,nif,nchan,npol - the number of beams/IFs/Pols/Chans
        nscan           - the number of scans in the scantable
        nrow            - the number of spectra in the scantable
        history         - print the history of the scantable
        get_fit         - get a fit which has been stored witnh the data
        average_time    - return the (weighted) time average of a scan
                          or a list of scans
        average_pol     - average the polarisations together.
        average_beam    - average the beams together.
        convert_pol     - convert to a different polarisation type
        auto_quotient   - return the on/off quotient with
                          automatic detection of the on/off scans (closest
                          in time off is selected)
        mx_quotient     - Form a quotient using MX data (off beams)
        scale, *, /     - return a scan scaled by a given factor
        add, +          - return a scan with given value added
        sub, -          - return a scan with given value subtracted
        bin             - return a scan with binned channels
        resample        - return a scan with resampled channels
        smooth          - return the spectrally smoothed scan
        poly_baseline   - fit a polynomial baseline to all Beams/IFs/Pols
        auto_poly_baseline - automatically fit a polynomial baseline
        recalc_azel     - recalculate azimuth and elevation based on
                          the pointing
        gain_el         - apply gain-elevation correction
        opacity         - apply opacity correction
        convert_flux    - convert to and from Jy and Kelvin brightness
                          units
        freq_align      - align spectra in frequency frame
        invert_phase    - Invert the phase of the cross-correlation
        swap_linears    - Swap XX and YY (or RR LL)
        rotate_xyphase  - rotate XY phase of cross correlation
        rotate_linpolphase - rotate the phase of the complex
                             polarization O=Q+iU correlation
        freq_switch     - perform frequency switching on the data
        stats           - Determine the specified statistic, e.g. 'min'
                          'max', 'rms' etc.
        stddev          - Determine the standard deviation of the current
                          beam/if/pol
        get_row_selector - get the selection object for a specified row
                           number
 [Selection]
     selector              - a selection object to set a subset of a scantable
        set_scans          - set (a list of) scans by index
        set_cycles         - set (a list of) cycles by index
        set_beams          - set (a list of) beamss by index
        set_ifs            - set (a list of) ifs by index
        set_polarisations  - set (a list of) polarisations by name
                             or by index
        set_names          - set a selection by name (wildcards allowed)
        set_tsys           - set a selection by tsys thresholds
        set_query          - set a selection by SQL-like query, e.g. BEAMNO==1
        ( also  get_ functions for all these )
        reset              - unset all selections
        +                  - merge two selections

 [Math] Mainly functions which operate on more than one scantable

        average_time    - return the (weighted) time average
                          of a list of scans
        quotient        - return the on/off quotient
        simple_math     - simple mathematical operations on two scantables,
                          'add', 'sub', 'mul', 'div'
        quotient        - build quotient of the given on and off scans
                          (matched pairs and 1 off - n on are valid)
        merge           - merge a list of scantables

 [Line Catalog]
    linecatalog              - a linecatalog wrapper, taking an ASCII or
                               internal format table
        summary              - print a summary of the current selection
        set_name             - select a subset by name pattern, e.g. '*OH*'
        set_strength_limits  - select a subset by line strength limits
        set_frequency_limits - select a subset by frequency limits
        reset                - unset all selections
        save                 - save the current subset to a table (internal
                               format)
        get_row              - get the name and frequency from a specific
                               row in the table
 [Fitting]
    fitter
        auto_fit        - return a scan where the function is
                          applied to all Beams/IFs/Pols.
        commit          - return a new scan where the fits have been
                          commited.
        fit             - execute the actual fitting process
        store_fit       - store the fit parameters in the data (scantable)
        get_chi2        - get the Chi^2
        set_scan        - set the scantable to be fit
        set_function    - set the fitting function
        set_parameters  - set the parameters for the function(s), and
                          set if they should be held fixed during fitting
        set_gauss_parameters - same as above but specialised for individual
                               gaussian components
        get_parameters  - get the fitted parameters
        plot            - plot the resulting fit and/or components and
                          residual
[Plotter]
    asapplotter         - a plotter for asap, default plotter is
                          called 'plotter'
        plot            - plot a scantable
        plot_lines      - plot a linecatalog overlay
        plotazel        - plot azimuth and elevation versus time
        plotpointing    - plot telescope pointings
        save            - save the plot to a file ('png' ,'ps' or 'eps')
        set_mode        - set the state of the plotter, i.e.
                          what is to be plotted 'colour stacked'
                          and what 'panelled'
        set_selection   - only plot a selected part of the data
        set_range       - set a 'zoom' window [xmin,xmax,ymin,ymax]
        set_legend      - specify user labels for the legend indeces
        set_title       - specify user labels for the panel indeces
        set_abcissa     - specify a user label for the abcissa
        set_ordinate    - specify a user label for the ordinate
        set_layout      - specify the multi-panel layout (rows,cols)
        set_colors      - specify a set of colours to use
        set_linestyles  - specify a set of linestyles to use if only
                          using one color
        set_font        - set general font properties, e.g. 'family'
        set_histogram   - plot in historam style
        set_mask        - set a plotting mask for a specific polarization
        text            - draw text annotations either in data or relative
                          coordinates
        arrow           - draw arrow annotations either in data or relative
                          coordinates
        axhline,axvline - draw horizontal/vertical lines
        axhspan,axvspan - draw horizontal/vertical regions
        annotate        - draw an arrow with label
        create_mask     - create a scnatble mask interactively

    xyplotter           - matplotlib/pylab plotting functions

[General]
    commands            - this command
    print               - print details about a variable
    list_scans          - list all scantables created by the user
    list_files          - list all files readable by asap (default rpf)
    del                 - delete the given variable from memory
    range               - create a list of values, e.g.
                          range(3) = [0,1,2], range(2,5) = [2,3,4]
    help                - print help for one of the listed functions
    execfile            - execute an asap script, e.g. execfile('myscript')
    list_rcparameters   - print out a list of possible values to be
                          put into $HOME/.asaprc
    rc                  - set rc parameters from within asap
    mask_and,mask_or,
    mask_not            - boolean operations on masks created with
                          scantable.create_mask
    skydip              - gain opacity values from a sky dip observation
    opacity_model       - compute opacities fro given frequencies based on
                          atmospheric model

Note:
    How to use this with help:
                                     # function 'summary'
    [xxx] is just a category
    Every 'sub-level' in this list should be replaces by a '.' Period when
    using help
    Example:
        ASAP> help scantable # to get info on ths scantable
        ASAP> help scantable.summary # to get help on the scantable's
        ASAP> help average_time

        """
    if rcParams['verbose']:
        try:
            from IPython.genutils import page as pager
        except ImportError:
            from pydoc import pager
        pager(x)
    else:
        print x
    return
