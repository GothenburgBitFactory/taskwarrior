# -*- coding: utf-8 -*-

import sys
import os
# Ensure python finds the local taprunner module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

import unittest
from subprocess import Popen, PIPE, STDOUT
from datetime import datetime


class TestVersion(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        # Empty rc file
        open("version.rc", 'w').close()

    def setUp(self):
        """Executed before each test in the class"""

    def testVersion(self):
        """Copyright is current"""
        command = ["../src/task", "rc:version.rc", "version"]

        # Merge STDOUT and STDERR
        p = Popen(command, stdout=PIPE, stderr=STDOUT)
        out, err = p.communicate()

        expected = "Copyright \(C\) \d{4} - %d" % (datetime.now().year,)
        self.assertRegexpMatches(out.decode("utf8"), expected)

    def testFailVersion(self):
        """Copyright is one year old"""
        command = ["../src/task", "rc:version.rc", "version"]

        # Merge STDOUT and STDERR
        p = Popen(command, stdout=PIPE, stderr=STDOUT)
        out, _ = p.communicate()

        expected = "Copyright \(C\) \d{4} - %d" % (datetime.now().year - 1,)
        self.assertRegexpMatches(out.decode("utf8"), expected)

    def tearDown(self):
        """Executed after each test in the class"""

    @classmethod
    def tearDownClass(cls):
        """Executed once after all tests in the class"""
        os.remove("version.rc")


if __name__ == "__main__":
    from taprunner import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
