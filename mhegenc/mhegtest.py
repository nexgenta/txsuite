#!/usr/bin/env python

# NOTE: This test requires that you have build mhegenc and mhegasn and
# that they are in your path.

import sys
import os
from subprocess import call, Popen, PIPE, STDOUT
import re
import tempfile
from difflib import ndiff, IS_CHARACTER_JUNK
import itertools

class TestFailed(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)


interchaged_object_tag = '<InterchangedObject>'
mhtest_file_splitter = re.compile('.*?' + interchaged_object_tag)
comment_splitter = re.compile('^//.*?\n', re.MULTILINE)

def read_mhtest_file(filename):

    "Reads an mhtest file and returns the 2 parts (txt, xml)"

    content = open(filename, 'r').read()
    res = mhtest_file_splitter.split(content, 1)
    if len(res) != 2:
        raise TestFailed, 'Unable to split "%s" into txt,xml parts' % filename
    txt = comment_splitter.split(res[0])
    txt = ''.join( txt )
    xml = interchaged_object_tag + res[1]
    return (txt, xml)


def convert_txt_to_der(txt_content):

    "Converts a string containing mheg-5 text into its ASN.1 (DER) form"

    tmp_txt_filename = tempfile.mktemp('.mhtxt')
    tmp_der_filename = tempfile.mktemp('.der')
    open(tmp_txt_filename, 'w').write(txt_content)

    proc = Popen('mhegenc -o %s %s' % (tmp_der_filename, tmp_txt_filename), 
                 shell=True, stdout=PIPE, stderr=STDOUT, close_fds=True)

    retcode = proc.wait()
    cmd_output = proc.stdout.read()
    if retcode:
        raise TestFailed, 'mhegenc returned: %d"\n%s\n"' % (retcode, cmd_output)
    der_content = open(tmp_der_filename, 'r').read()
    os.unlink(tmp_txt_filename)
    os.unlink(tmp_der_filename)
    return der_content


def convert_xml_to_der(xml_content):

    "Converts a string containing mheg-5 XER into its DER form"

    tmp_xml_filename = tempfile.mktemp('.xml')
    open(tmp_xml_filename, 'w').write(xml_content)

    proc = Popen('mhegasn -ixer -oder  %s' % tmp_xml_filename, 
                 shell=True, stdout=PIPE, stderr=PIPE, close_fds=True)

    retcode = proc.wait()
    cmd_output = proc.stderr.read()
    if retcode:
        raise TestFailed, 'mhegasn returned: %d"\n%s\n"' % (retcode, cmd_output)
    der_content = proc.stdout.read()
    os.unlink(tmp_xml_filename)
    return der_content


def convert_der_to_txt(der_content):

    "Converts a string containing ASN.1 (DER) encoded MHEG-5 to text form"

    tmp_der_filename = tempfile.mktemp('.der')
    tmp_txt_filename = tempfile.mktemp('.mhtxt')
    open(tmp_der_filename, 'w').write(der_content)

    proc = Popen('mhegenc -d -o %s %s' % (tmp_txt_filename, tmp_der_filename), 
                 shell=True, stdout=PIPE, stderr=STDOUT, close_fds=True)

    retcode = proc.wait()
    cmd_output = proc.stdout.read()
    if retcode:
        raise TestFailed, 'mhegenc returned: %d"\n%s\n"' % (retcode, cmd_output)
    txt_content = open(tmp_txt_filename, 'r').read()
    os.unlink(tmp_txt_filename)
    os.unlink(tmp_der_filename)
    return txt_content


def convert_der_to_xml(der_content):

    "Converts a string containingASN.1 (DER) encoded MHEG-5 to XER form"

    tmp_der_filename = tempfile.mktemp('.der')
    open(tmp_der_filename, 'w').write(der_content)

    proc = Popen('mhegasn -iber -oxer  %s' % tmp_der_filename, 
                 shell=True, stdout=PIPE, stderr=PIPE, close_fds=True)

    retcode = proc.wait()
    cmd_output = proc.stderr.read()
    if retcode:
        raise TestFailed, 'mhegasn returned: %d"\n%s\n"' % (retcode, cmd_output)
    xml_content = proc.stdout.read()
    os.unlink(tmp_der_filename)
    return xml_content

def diff_text(txt1, txt2):

    "Compares two text strings with 'diff' ignoring whitespace"

    tmp_filename1 = tempfile.mktemp('.txt')
    tmp_filename2 = tempfile.mktemp('.txt')
    open(tmp_filename1, 'w').write(txt1)
    open(tmp_filename2, 'w').write(txt2)
    
    diff = call(['diff', '-w', tmp_filename1, tmp_filename2])
    return (diff == 0, '')
    
class Test(object):

    def __init__(self, testfile_name):
        self.testfile_name = testfile_name
        
    def run(self):
        try:
            txt, xml = read_mhtest_file(self.testfile_name)
            
            der_from_txt = convert_txt_to_der(txt)
            der_from_xml = convert_xml_to_der(xml)

            if der_from_txt != der_from_xml:
                raise TestFailed, 'DER encodings differ'
            
            txt_from_der = convert_der_to_txt(der_from_txt)

            matches, diff = diff_text(txt, txt_from_der)
            if not matches:
                raise TestFailed, 'Round-trip text does not match original:\n%s' % diff
        finally:
            self.cleanup()
        
    def cleanup(self):
        cmd = 'rm -f ' + self.testfile_name + '.*'
        os.system(cmd)
        

def main():
    """Run mhegenc on a test file and compare with the embedded results"""
    if len(sys.argv) == 2:
        testfilename = sys.argv[1]
        try:
            Test(testfilename).run();
            return 0
        except TestFailed, message:
            print 'Test "%s" Failed: %s' % (testfilename, message.value)
            return -1
    if len(sys.argv) == 3 and sys.argv[1] == 'make-mhtest':
        source = sys.argv[2]
        testfilename = source + '.mhtest'
        txt = open(source, 'r').read()
        der = convert_txt_to_der(txt)
        xer = convert_der_to_xml(der)
        testfile = open(testfilename, 'w')
        testfile.write(txt)
        testfile.write('\n')
        testfile.write(xer)
        testfile.close()
        return 0
    
    print "usage: %s <test file>" % sys.argv[0]
    return -1;
   
if __name__ == "__main__":
    sys.exit(main())
