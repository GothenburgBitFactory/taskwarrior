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


class TestAliasesCompletion(TestCase):
    """Aliases should be listed by '_aliases' not '_commands' or '_zshcommands'
    reported as bug 1043
    """
    def setUp(self):
        self.t = Task()
        self.t.config("alias.samplealias", "long")

    def test__aliases(self):
        """samplealias in _aliases"""
        code, out, err = self.t("_aliases")

        self.assertIn("samplealias", out)

    def test__commands(self):
        """samplealias not in _commands"""
        code, out, err = self.t("_commands")

        self.assertIn("information", out)
        self.assertNotIn("samplealias", out)

    def test__zshcommands(self):
        """samplealias not in _zshcommands"""
        code, out, err = self.t("_zshcommands")

        self.assertIn("information", out)
        self.assertNotIn("samplealias", out)

class TestZshCompletion(TestCase):
    """Test _zshcommands and related completion subcommands"""

    def setUp(self):
        self.t = Task()
        self.t.config("report.foobar.columns", "id")

    def test_categories(self):
        """test _zshcommands categories"""
        code, out, err = self.t("_zshcommands")

        self.assertIn("\nfoobar:report:", out)
        self.assertIn("\ninformation:metadata:", out)
        self.assertIn("\nexport:migration:", out)
        self.assertNotIn(":unassigned:", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
