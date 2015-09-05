#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestAbbreviation(TestCase):
    def setUp(self):
        self.t = Task()
        self.t.config("abbreviation.minimum", "1")

        self.t("add project:home priority:H hasattributes")
        self.t("add noattributes")

    def verify_attribute(self, expr):
        code, out, err = self.t("list {0}".format(expr))

        self.assertIn("hasattributes", out)
        self.assertNotIn("noattributes", out)

    def test_attribute_abbreviations(self):
        "Test project attribute abbrevations"

        self.verify_attribute("project:home")
        self.verify_attribute("projec:home")
        self.verify_attribute("proje:home")
        self.verify_attribute("proj:home")
        self.verify_attribute("pro:home")

    def test_uda_abbreviations(self):
        "Test uda attribute abbrevations"
        # NOTE This will be an UDA when TW-1541 is closed, for now it's just
        # one more attribute

        self.verify_attribute("priority:H")
        self.verify_attribute("priorit:H")
        self.verify_attribute("priori:H")
        self.verify_attribute("prior:H")
        self.verify_attribute("prio:H")
        self.verify_attribute("pri:H")

    def verify_command(self, cmd):
        code, out, err = self.t(cmd)

        self.assertIn("MIT license", out)

    def test_command_abbreviations(self):
        "Test version command abbrevations"

        self.verify_command("version")
        self.verify_command("versio")
        self.verify_command("versi")
        self.verify_command("vers")
        self.verify_command("ver")
        self.verify_command("ve")
        self.verify_command("v")


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
