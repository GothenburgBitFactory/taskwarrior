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
import platform
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestColorCommand(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""

    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_colors_off(self):
        """ Verify 'task colors' shows an error with color:off"""
        code, out, err = self.t.runError("colors")
        self.assertIn("Color is currently turned off", out)

    def test_colors_all(self):
        """ Verify 'task colors' shows all colors"""
        code, out, err = self.t("colors rc._forcecolor:on")
        self.assertIn("Basic colors", out)
        self.assertIn("Effects", out)
        self.assertIn("color0 - color15", out)
        self.assertIn("Color cube", out)
        self.assertIn("Gray ramp gray0 - gray23 (also color232 - color255)", out)
        self.assertIn("Try running 'task color white on red'.", out)

    def test_colors_sample(self):
        """ Verify 'task colors red' shows a sample"""
        code, out, err = self.t("colors rc._forcecolor:on red")
        self.assertRegexpMatches(out, "Your sample:\n\n  .\[31mtask color red.\[0m")

    def test_colors_legend(self):
        """ Verify 'task colors legend' shows theme colors"""
        code, out, err = self.t("colors rc._forcecolor:on legend")
        self.assertRegexpMatches(out, "color.debug\s+.\[0m\s.\[38;5;4mcolor4\s+.\[0m")

    def test_colors_legend_override(self):
        """Verify 'task colors legend' obeys rc overrides"""
        code, out, err = self.t("colors rc._forcecolor:on rc.color.debug:red legend")
        self.assertRegexpMatches(out, "color.debug\s+.\[0m\s.\[31mred\s+.\[0m")

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
