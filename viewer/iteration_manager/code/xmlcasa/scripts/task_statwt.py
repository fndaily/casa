from taskinit import mstool, tbtool, casalog, write_history

def statwt(vis, dorms, byantenna, sepacs, fitspw, fitcorr, combine,
           timebin, minsamp, field, spw, antenna, timerange, scan, intent,
           array, correlation, obs, datacolumn):
    """
    Sets WEIGHT and SIGMA using the scatter of the visibilities.
    """
    casalog.origin('statwt')
    retval = True
    try:
        myms = mstool.create()
        mytb = tbtool.create()
        
        datacol = 'DATA'
        mytb.open(vis)
        colnames  = mytb.colnames()
        mytb.close()
        for datacol in ['CORRECTED_DATA', 'DATA', 'junk']:
            if datacol in colnames:
                break
        if datacol == 'junk':
            raise ValueError(vis + " does not have a data column")        

        if datacolumn.lower()=='corrected' and datacol.split('_')[0].lower() != datacolumn: # no CORRECTED_DATA case (fall back to DATA)
           casalog.post("No %s column found, using %s column" % (datacolumn.upper()+'_DATA', datacol),'WARN')
           datacolumn = datacol

        if ':' in spw:
            casalog.post('The channel selection part of spw will be ignored.', 'WARN')
        
        # for debugging 
        casalog.post('data column used:'+datacolumn,'INFO2')
        myms.open(vis, nomodify=False)
        retval = myms.statwt(dorms, byantenna, sepacs, fitspw, fitcorr, combine,
                             timebin, minsamp, field, spw, antenna, timerange, scan, intent,
                             array, correlation, obs, datacolumn)
        myms.close()
    except Exception, e:
        casalog.post("Error setting WEIGHT and SIGMA for %s:" % vis, 'SEVERE')
        casalog.post("%s" % e, 'SEVERE')
        if False:  # Set True for debugging.
            for p in statwt.func_code.co_varnames[:statwt.func_code.co_argcount]:
                v = eval(p)
                print p, "=", v, ", type =", type(v)
        retval = False

    if retval:
        try:
            param_names = statwt.func_code.co_varnames[:statwt.func_code.co_argcount]
            param_vals = [eval(p) for p in param_names]
            retval &= write_history(myms, vis, 'statwt', param_names, param_vals,
                                    casalog)
        except Exception, instance:
            casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                         'WARN')        
    return retval
        
    