#!/usr/bin/env python
"""Searches the 'testfiles' directory for all '.mhtest' test files and
passes them to the mhegtest.test_file() function."""
import sys
import os
import mhegtest

find_tests_command = 'find testfiles -type f -name "*.mhtest"'

find_command = os.popen(find_tests_command)
file_list = find_command.readlines()
find_command.close()

failures = []

for file in file_list:
    file = file.strip()
    try:
        mhegtest.Test(file).run();
    except mhegtest.TestFailed, message:
        failures.append((file, message.value))

if len(failures) > 0:
    print "FAILURES: "
    for test in failures:
        print '%s:' % test[0]
        print test[1]
else:
    print "ALL (%d) TESTS PASSED" % len(file_list)

