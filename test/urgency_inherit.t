#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
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
# http://www.opensource.org/licenses/mit-license.php
#
###############################################################################

import sys
import os
import unittest
import json
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestUrgencyInherit(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.t = Task()

        cls.t.config("urgency.age.coefficient", "0.0")
        cls.t.config("urgency.blocked.coefficient", "0.0")
        cls.t.config("urgency.blocking.coefficient", "0.0")

        cls.t("add one")
        cls.t("add two dep:1")
        cls.t("add three dep:2 +next due:today-1year")

    def get_tasks(self):
        tasks = json.loads(self.t("rc.json.array=1 export")[1])

        r = {}
        for task in tasks:
            # Make available by ID. Discards non-pending tasks.
            if task["id"] != 0:
                r[task["id"]] = task

        return r

    def test_urgency_inherit_off(self):
        """No urgency inheritance when switched off"""
        self.t.config("urgency.inherit", "off")
        tl = self.get_tasks()
        self.assertTrue(tl[1]["urgency"] <= tl[2]["urgency"] < tl[3]["urgency"])

    def test_gc_off_mod(self):
        """Biggest urgency is inherited recursively"""
        self.t.config("urgency.inherit", "off")
        tl = self.get_tasks()
        oldmax = max(tl[1]["urgency"], tl[2]["urgency"], tl[3]["urgency"])
        self.t.config("urgency.inherit", "on")
        tl = self.get_tasks()
        self.assertTrue(oldmax <= tl[3]["urgency"])
        self.assertTrue(tl[1]["urgency"] >= tl[2]["urgency"] >= tl[3]["urgency"])


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
