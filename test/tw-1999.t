#!/usr/bin/env python
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2019, Paul Beckingham, Federico Hernandez.
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
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase, Taskd, ServerTestCase


class TestBug1999(TestCase):
    """Bug 1999: Taskwarrior reports wrong active tim
    """
    def setUp(self):
        self.t = Task()

    def test_correct_active_time(self):
        """Ensure correct active time locally
        """
        desc = "Testing task"
        self.t(("add", desc))
        self.t(("start", "1"))
        self.t.faketime("+10m")
        self.t(("stop", "1"))

        code, out, err = self.t(("info", "1"))
        self.assertIn("duration: 0:10:00", out)


class TestBug1999Server(ServerTestCase):
    @classmethod
    def setUpClass(cls):
        cls.taskd = Taskd()
        # This takes a while...
        cls.taskd.start()

    def setUp(self):
        self.t1 = Task(taskd=self.taskd)
        self.t2 = Task(taskd=self.taskd)

    def test_correct_active_time(self):
        """Ensure correct active time across different clients
        """
        desc = "Testing task"
        self.t1(("add", desc))
        self.t1(("start", "1"))
        self.t1.faketime("+10m")
        self.t1(("stop", "1"))

        self.t1("sync")

        code, out, err = self.t1(("info", "1"))
        self.assertIn("duration: 0:10:00", out)

        self.t2("sync")

        code2, out2, err2 = self.t2(("info", "1"))
        self.assertIn("duration: 0:10:00", out2)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
