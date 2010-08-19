import numpy
import re
from taskinit import me, qa
from dict_to_table import dict_to_table

# Possible columns, as announced by their column titles.
# The data is scooped up by 'pat'.  Either use ONE group named by the column
# key, or mark it as unwanted.  '-' is not valid in group names.
# Leading and trailing whitespace will be taken care of later.
# Sample lines:
#  Date__(UT)__HR:MN     R.A.___(ICRF/J2000.0)___DEC Ob-lon Ob-lat Sl-lon Sl-lat   NP.ang   NP.dist               r        rdot            delta      deldot    S-T-O   L_s
#  2010-May-01 00:00     09 01 43.1966 +19 04 28.673 286.52  18.22 246.99  25.34 358.6230      3.44  1.661167637023  -0.5303431 1.28664311447968  15.7195833  37.3033   84.50
cols = {
    'date': {'header': r'Date__\(UT\)__HR:MN',
             'comment': 'date',
             'pat':     r'(?P<date>\d+-\w+-\d+ \d+:\d+)'},
    'ra': {'header': r'R.A._+\([^)]+',
           'comment': 'Right Ascension (J2000)',
           'pat':    r'(?P<ra>(\d+ \d+ )?\d+\.\d+)'}, # require a . for safety
    'dec': {'header': r'\)_+DEC.',
            'comment': 'Declination (J2000)',
            'pat':    r'(?P<dec>([-+]?\d+ \d+ )?[-+]?\d+\.\d+)'},
    'illu': {'header': r'Illu%',
             'comment': 'Illumination',
             'pat':    r'(?P<illu>[0-9.]+)',
             'unit': r'%'},
    'ob_lon': {'header': r'Ob-lon',
               'comment': 'Sub-observer longitude',
               'pat':    r'(?P<ob_lon>[0-9.]+|n\.a\.)',
               'unit': 'deg'},
    'ob_lat': {'header': r'Ob-lat',
               'comment': 'Sub-observer longitude',
               'pat':    r'(?P<ob_lat>[-+0-9.]+|n\.a\.)',
               'unit': 'deg'},
    'sl_lon': {'header': r'Sl-lon',
               'comment': 'Sub-Solar longitude',
               'pat':    r'(?P<sl_lon>[0-9.]+|n\.a\.)',
               'unit': 'deg'},
    'sl_lat': {'header': r'Sl-lat',
               'comment': 'Sub-Solar longitude',
               'pat':    r'(?P<sl_lat>[-+0-9.]+|n\.a\.)',
               'unit': 'deg'},
    'np_ang': {'header': r'NP.ang',
               'comment': 'North Pole position angle',
               'pat':    r'(?P<np_ang>[-+0-9.]+)',
               'unit': 'deg'},
    'np_dist': {'header': r'NP.dist',
                'comment': 'North Pole distance from sub-observer point',
                # Negative distance means the N.P. is on the hidden hemisphere.
                'pat':    r'(?P<np_dist>[-+0-9.]+)',
                'unit': 'deg'},
    'r': {'header': 'r',
          'comment': 'heliocentric distance',
          'unit':    'AU',
          'pat':     r'(?P<r>[0-9.]+)'},
    'rdot': {'header': 'rdot',
             'pat': r'[-+0-9.]+',
             'unwanted': True},
    'delta': {'header': 'delta',
              'comment': 'geocentric distance',
              'unit':    'AU',
              'pat':     r'(?P<delta>[0-9.]+)'},
    'deldot': {'header': 'deldot',
               'pat': r'[-+0-9.]+',
               'unwanted': True},
    'phang': {'header':  'S-T-O',
              'comment': 'phase angle',
              'unit':    'deg',
              'pat':     r'(?P<phang>[0-9.]+)'},
    'L_s': {'header': 'L_s',  # 08/2010: JPL does not supply this and
            'unit': 'deg',    # says they cannot.  Ask Bryan Butler.
            'comment': 'Season angle',
            'pat': r'(?P<L_s>[-+0-9.]+)'}
    }

def readJPLephem(fmfile):
    """
    Reads a JPL Horizons text file (see
    http://ssd.jpl.nasa.gov/horizons.cgi#top ) for a solar system object and
    returns various quantities in a dictionary.  The dict will be blank ({}) if
    there is a failure.
    """
    retdict = {}
    casalog.origin('readJPLephem')

    # Try opening fmfile now, because otherwise there's no point continuing.
    try:
        ephem = open(fmfile)
    except IOError:
        casalog.post("Could not open ephemeris file " + fmfile,
                     priority="SEVERE")
        return {}

    # Setup the regexps.

    # Headers (one time only things)
    
    # Dictionary of quantity label: regexp pattern pairs that will be searched
    # for once.  The matching quantity will go in retdict[label].  Only a
    # single quantity (group) will be retrieved per line.
    headers = {
        'body': {'pat': r'^Target body name:\s+\d*\s*(\w+)'},   # object name, w.o. number
        'ephtype': {'pat': r'\?s_type=1#top>\]\s*:\s+\*(\w+)'}, # e.g. OBSERVER
        'obsloc': {'pat': r'^Center-site name:\s+(\w+)'},        # e.g. GEOCENTRIC
        'meanrad': {'pat': r'Mean radius \(km\)\s*=\s*([0-9.]+)',
                    'unit': 'km'},
        'radii': {'pat': r'Target radii\s*:\s*([0-9.]+\s*x\s*[0-9.]+\s*x\s*[0-9.]+)\s*km.*Equator, meridian, pole',
                  'unit': 'km'},
        'T_mean': {'pat': r'Mean Temperature \(K\)\s*=\s*([0-9.]+)',
                   'unit': 'K'},
        }
    for hk in headers:
        headers[hk]['pat'] = re.compile(headers[hk]['pat'])

    # Data ("the rows of the table")
    
    # need date, r (heliocentric distance), delta (geocentric distance), and phang (phase angle).
    # (Could use the "dot" time derivatives for Doppler shifting, but it's
    # likely unnecessary.)
    datapat = r'^\s*'

    stoppat = r'\$\$EOE$'  # Signifies the end of data.

    # Read fmfile into retdict.
    num_cols = 0
    in_data = False
    comp_mismatches = []
    print_datapat = False
    for line in ephem:
        if in_data:
            if re.match(stoppat, line):
                break
            matchobj = re.search(datapat, line)
            if matchobj:
                gdict = matchobj.groupdict()
                for col in gdict:
                    retdict['data'][col]['data'].append(gdict[col])
                if len(gdict) < num_cols:
                    print "Partially mismatching line:"
                    print line
                    print "Found:"
                    print gdict
                    print_datapat = True
            else:
                print_datapat = True
                # Chomp trailing whitespace.
                comp_mismatches.append(re.sub(r'\s*$', '', line))
        elif re.match(r'^\s*' + cols['date']['header'] + r'\s+'
                      + cols['ra']['header'], line):
            # See what columns are present, and finish setting up datapat and
            # retdict.
            havecols = []
            # Chomp trailing whitespace.
            myline = re.sub(r'\s*$', '', line)
            titleline = myline
            remaining_cols = cols.keys()
            found_col = True
            # This loop will terminate one way or another.
            while myline and remaining_cols and found_col:
                found_col = False
                #print "myline = '%s'" % myline
                #print "remaining_cols =", ', '.join(remaining_cols)
                for col in remaining_cols:
                    if re.match(r'^\s*' + cols[col]['header'], myline):
                        #print "Found", col
                        havecols.append(col)
                        remaining_cols.remove(col)
                        myline = re.sub(r'^\s*' + cols[col]['header'],
                                        '', myline)
                        found_col = True
                        break
            datapat += r'\s+'.join([cols[col]['pat'] for col in havecols])
            sdatapat = datapat
            print "Found columns:", ', '.join(havecols)
            datapat = re.compile(datapat)
            retdict['data'] = {}
            for col in havecols:
                if not cols[col].get('unwanted'):
                    retdict['data'][col] = {'comment': cols[col]['comment'],
                                            'data':    []}
            num_cols = len(retdict['data'])
        elif re.match(r'^\$\$SOE\s*$', line):  # Start of ephemeris
            #print "Starting to read data."
            in_data = True
        else:
            #print "line =", line
            #print "looking for", 
            for hk in headers:
                if not retdict.has_key(hk):
                    #print hk,
                    matchobj = re.search(headers[hk]['pat'], line)
                    if matchobj:
                        retdict[hk] = matchobj.group(1) # 0 is the whole line
                        #if hk == 'isOK':
                        #    print "Found isOK =", retdict[hk]
                        #print "found", retdict[hk]
                        break
    ephem.close()

    # If there were errors, provide debugging info.
    if comp_mismatches:
        print "Completely mismatching lines:"
        print "\n".join(comp_mismatches)
    if print_datapat:
        print "The apparent title line is:"
        print titleline
        print "datapat = r'%s'" % sdatapat

    # Convert numerical strings into actual numbers.
    try:
        retdict['earliest'] = datestr_to_epoch(retdict['data']['date']['data'][0])
        retdict['latest'] = datestr_to_epoch(retdict['data']['date']['data'][-1])
    except Exception, e:
        print "Error!"
        if retdict.has_key('data'):
            if retdict['data'].has_key('date'):
                if retdict['data']['date'].has_key('data'):
                    #print "retdict['data']['date']['data'] =", retdict['data']['date']['data']
                    print "retdict['data'] =", retdict['data']
                else:
                    print "retdict['data']['date'] has no 'data' key."
                    print "retdict['data']['date'].keys() =", retdict['data']['date'].keys()
            else:
                print "retdict['data'] has no 'date' key."
                print "retdict['data'].keys() =", retdict['data'].keys()
        else:
            print "retdict has no 'data' key."
        raise e

    for hk in headers:
        if headers[hk].has_key('unit') and retdict.has_key(hk):
            if hk == 'radii':
                radii = retdict[hk].split('x')
                a, b, c = [float(r) for r in radii]
                retdict[hk] = {'unit': headers[hk]['unit'],
                               'value': (a, b, c)}
                retdict['meanrad'] = {'unit': headers[hk]['unit'],
                                      'value': mean_radius(a, b, c)}
            else:
                try:
                    # meanrad might already have been converted.
                    if type(retdict[hk]) != dict:
                        retdict[hk] = {'unit': headers[hk]['unit'],
                                       'value': float(retdict[hk])}
                except Exception, e:
                    print "Error converting header", hk, "to a Quantity."
                    print "retdict[hk] =", retdict[hk]
                    raise e
    retdict['data']['date'] = datestrs_to_epochs(retdict['data']['date']['data'])
    for dk in retdict['data']:
        if cols[dk].has_key('unit'):
            retdict['data'][dk]['data'] = {'unit': cols[dk]['unit'],
                                           'value': numpy.array([float(s) for s in retdict['data'][dk]['data']])}
    
    return retdict

def mean_radius(a, b, c):
    """
    Return the average apparent mean radius of an ellipsoid with semiaxes
    a >= b >= c.
    "average" means average over time naively assuming the pole orientation
    is uniformly distributed over the whole sphere, and "apparent mean radius"
    means a radius that would give the same area as the apparent disk.
    """
    # This is an approximation, but it's not bad if b ~= a.
    return numpy.sqrt(0.5 * a * (a + (2.0 * b + c) / 3.0))

def datestr_to_epoch(datestr):
    """
    Given a UT date like "2010-May-01 00:00", returns an epoch measure.
    """
    return me.epoch(rf='UTC', v0=qa.totime(datestr))

def datestrs_to_epochs(datestrlist):
    """
    Like datestr_to_epoch, but more so.  All of the date strings must have the
    same reference frame (i.e. UT).
    """
    timeq = {}
    # Do first conversion to get unit.
    firsttime = qa.totime(datestrlist[0])
    timeq['unit'] = firsttime['unit']
    timeq['value'] = [firsttime['value']]
    for i in xrange(1, len(datestrlist)):
        timeq['value'].append(qa.totime(datestrlist[i])['value'])

    # me.epoch doesn't take array values, so make a vector epoch measure manually.
    #return me.epoch(rf='UT', v0=timeq)
    return {'m0': {'unit': timeq['unit'],
                   'value': numpy.array(timeq['value'])},
            'refer': 'UTC',
            'type': 'epoch'}


def ephem_dict_to_table(fmdict, tablepath=''):
    """
    Converts a dictionary from readJPLephem() to a CASA table, and attempts to
    save it to tablepath.  Returns whether or not it was successful.
    """
    if not tablepath:
        tablepath = "ephem_JPL-Horizons_%s_%.0f-%.0f%s%s.tab" % (fmdict['body'],
                                                              fmdict['earliest']['m0']['value'],
                                                              fmdict['latest']['m0']['value'],
                                                              fmdict['latest']['m0']['unit'],
                                                              fmdict['latest']['refer'])
        print "Writing to", tablepath
        
    retval = True
    try:
        outdict = fmdict.copy() # Yes, I want a shallow copy.
        kws = fmdict.keys()
        kws.remove('data')
        cols = outdict['data'].keys()
        clashing_cols = []
        for c in cols:
            if c in kws:
                clashing_cols.append(c)
        if clashing_cols:
            raise ValueError, 'The input dictionary lists' + ', '.join(clashing_cols) + 'as both keyword(s) and column(s)'

        # This promotes the keys in outdict['data'] up one level, and removes
        # 'data' as a key of outdict.
        outdict.update(outdict.pop('data'))

        retval = dict_to_table(outdict, tablepath, kws, cols)
    except Exception, e:
        print 'Error', e, 'trying to export an ephemeris dict to', tablepath
        retval = False

    return retval
