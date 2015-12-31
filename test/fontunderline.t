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
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestUnderline(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_version(self):
        """Verify that underlineѕ table headers are used at the right time"""

        # Test the fontunderline config variable.  The following truth table defines
        # the different results which are to be confirmed.
        #
        #   color  _forcecolor  fontunderline  result
        #   -----  -----------  -------------  ---------
        #       0            0              0  dashes
        #       0            0              1  dashes
        #       0            1              0  dashes
        #       0            1              1  underline
        #       1*           0              0  dashes
        #       1*           0              1  dashes
        #       1*           1              0  dashes
        #       1*           1              1  underline
        #
        # * When isatty (fileno (stdout)) is false, color is automatically disabled.

        self.t("add foo")
        code, out, err = self.t("1 info rc.color:off rc._forcecolor:off rc.fontunderline:off")
        self.assertIn("--------", out)

        code, out, err = self.t("1 info rc.color:off rc._forcecolor:off rc.fontunderline:on")
        self.assertIn("--------", out)

        code, out, err = self.t("1 info rc.color:off rc._forcecolor:on rc.fontunderline:off")
        self.assertIn("--------", out)

        code, out, err = self.t("1 info rc.color:off rc._forcecolor:on rc.fontunderline:on")
        self.assertNotIn("--------", out)

        code, out, err = self.t("1 info rc.color:on rc._forcecolor:off rc.fontunderline:off")
        self.assertIn("--------", out)

        code, out, err = self.t("1 info rc.color:on rc._forcecolor:off rc.fontunderline:on")
        self.assertIn("--------", out)

        code, out, err = self.t("1 info rc.color:on rc._forcecolor:on rc.fontunderline:off")
        self.assertIn("--------", out)

        code, out, err = self.t("1 info rc.color:on rc._forcecolor:on rc.fontunderline:on")
        self.assertNotIn("--------", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
