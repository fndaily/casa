import os
import sys
import shutil
import string
import listing as lt
from __main__ import default
from tasks import *
from taskinit import *
import unittest

'''
Unit tests for task listobs. It tests the following parameters:
    vis:        wrong and correct values
    verbose     true or false
    listfile:   save on a file
    
'''

# Reference files
refpath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/listobs/'
reffile = refpath+'reflistobs'

# Input and output names
msfile1 = 'ngc5921_ut.ms'
msfile2 = 'uid___X02_X3d737_X1_01_small.ms'

class listobs_test1(unittest.TestCase):

    def setUp(self):
        self.res = None
        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/listobs/'
        if (not os.path.exists(msfile1)):            
            shutil.copytree(datapath+msfile1, msfile1)

        if (not os.path.exists(msfile2)):            
            shutil.copytree(datapath+msfile2, msfile2)
        
        default(listobs)
        
    
    def tearDown(self):
        pass        

    def test1(self):
        '''Listobs 1: Default values'''
        self.res = listobs()
        self.assertFalse(self.res,'Default parameters should return False')
        
    def test2(self):
        '''Listobs 2: Input MS'''
        self.res = listobs(vis=msfile1)
        self.assertEqual(self.res, None, "Return value should be None")

    def test3(self):
        '''Listobs 3: CSV-591. Check if long field names are fully displayed'''
        ms.open(msfile1)
        res = ms.summary(True)
        ms.close()
        name = res['header']['field_0']['name']
        self.assertFalse(name.__contains__('*'), "Field name contains a *")
        name = res['header']['scan_7']['0']['FieldName']
        self.assertFalse(name.__contains__('*'), "Field name contains a *")
        
    def test4(self):
        '''Listobs 4: CAS-2751. Check that ALMA MS displays one row per scan'''
        ms.open(msfile2)
        res = ms.summary(True)
        ms.close()
        # Begin and end times should be different
        btime = res['header']['scan_1']['0']['BeginTime']
        etime = res['header']['scan_1']['0']['EndTime']
        self.assertNotEqual(btime, etime, "Begin and End times of scan=1 should not be equal")
        
        # Only one row of scan=1 should be printed
        output = 'listobs4.txt'
        out = "newobs4.txt"
        reference = reffile+'4'
        diff = "difflistobs4"
        
        listobs(vis=msfile2, verbose=True, listfile=output)
#        # Remove the name of the MS from output before comparison
        os.system("sed '1,3d' "+ output+ ' > '+ out)    
        os.system("diff "+reference+" "+out+" > "+diff)    
        self.assertTrue(lt.compare(out,reference),
                        'New and reference files are different. %s != %s. '
                        'See the diff file %s.'%(out,reference, diff))
                    
    def test5(self):
        '''Listobs 5: Save on a file, verbose=False'''
        output = 'listobs5.txt'
        out = "newobs5.txt"
        reference = reffile+'5'
        diff1 = "diff1listobs5"
        diff2 = "diff2listobs5"
        
#        # Run it twice to check for the precision change
        self.res = listobs(vis=msfile1, verbose = False, listfile=output)
#        # Remove the name of the MS from output before comparison
        os.system("sed '1,3d' "+ output+ ' > '+ out)    
        os.system("diff "+reference+" "+out+" > "+diff1)    
        self.assertTrue(lt.compare(out,reference),
                        'New and reference files are different in first run. %s != %s. '
                        'See the diff file %s.'%(out,reference, diff1))
        
        os.system('rm -rf '+output+ " "+out)
        self.res = listobs(vis=msfile1, verbose = False, listfile=output)
#        # Remove the name of the MS from output before comparison
        os.system("sed '1,3d' "+ output+ ' > '+ out)        
        os.system("diff "+reference+" "+out+" > "+diff2)    
        self.assertTrue(lt.compare(out,reference),
                        'New and reference files are different in second run. %s != %s. '
                        'See the diff file %s.'%(out,reference,diff2))
        
    def test6(self):
        '''Listobs 6: Save on a file, verbose=True'''
        output = 'listobs6.txt'
        out = "newobs6.txt"
        diff = "diff1listobs6"
        reference = reffile+'6'
        self.res = listobs(vis=msfile1, listfile=output, verbose = True)
#        # Remove the name of the MS from output before comparison
        os.system("sed '1,3d' "+ output+ ' > '+ out)        
        os.system("diff "+reference+" "+out+" > "+diff)    
        self.assertTrue(lt.compare(out,reference),
                        'New and reference files are different. %s != %s. '
                        'See the diff file %s.'%(out,reference,diff))
        

class listobs_cleanup(unittest.TestCase):

    def setUp(self):
        pass
    
    def tearDown(self):
        # It will ignore errors in case the files don't exist
        shutil.rmtree(msfile1,ignore_errors=True)
        shutil.rmtree(msfile2,ignore_errors=True)
        os.system('rm -rf ' + 'listobs*.txt')
        
    def test_run(self):
        '''Listobs: Cleanup'''
        pass
        
def suite():
    return [listobs_test1,listobs_cleanup]