#!/usr/bin/env python3
###############################################################################
#
# Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# https://www.opensource.org/licenses/mit-license.php
#
###############################################################################

import sys
import os
import platform
import time
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestReadOnly(TestCase):
    def setUp(self):
        self.t = Task()
        self.t("add foo")

        # set the mtime of the taskdb to an hour ago, so we can see any changes
        self.taskdb = self.t.datadir + "/taskchampion.sqlite3"
        os.utime(self.taskdb, (time.time() - 3600,) * 2)

    def assertNotModified(self):
        self.assertLess(os.stat(self.taskdb).st_mtime, time.time() - 1800)

    def assertModified(self):
        self.assertGreater(os.stat(self.taskdb).st_mtime, time.time() - 1800)

    def test_read_only_command(self):
        code, out, err = self.t("reports")
        self.assertNotModified()

    def test_report(self):
        code, out, err = self.t("list")
        self.assertModified()

    def test_burndown(self):
        code, out, err = self.t("burndown")
        self.assertModified()

    def test_report_gc_0(self):
        self.t.config("gc", "0")
        code, out, err = self.t("list")
        self.assertNotModified()

    def test_burndown_gc_0(self):
        self.t.config("gc", "0")
        code, out, err = self.t("burndown")
        self.assertNotModified()


if __name__ == "__main__":
    from simpletap import TAPTestRunner

    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
