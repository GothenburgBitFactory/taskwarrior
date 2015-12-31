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


class TestCommands(TestCase):
    def setUp(self):
        self.t = Task()

    def test_command_dna(self):
        """Verify 'add', 'modify', 'list' dna"""
        code, out, err = self.t("commands")
        self.assertRegexpMatches(out, "add\s+operation\s+RW\s+Mods\s+Adds a new task")
        self.assertRegexpMatches(out, "list\s+report\s+RO\s+ID\s+GC\s+Ctxt\s+Filt\s+Most details of")
        self.assertRegexpMatches(out, "modify\s+operation\s+RW\s+Filt\s+Mods\s+Modifies the")

    def test_command_dna_color(self):
        """Verify 'add', 'modify', 'list' dna"""
        code, out, err = self.t("commands rc._forcecolor:on")
        self.assertRegexpMatches(out, "add\s+operation\s+RW\s+Mods\s+Adds a new task")
        self.assertRegexpMatches(out, "list\s+report\s+RO\s+ID\s+GC\s+Ctxt\s+Filt\s+Most details of")
        self.assertRegexpMatches(out, "modify\s+operation\s+RW\s+Filt\s+Mods\s+Modifies the")


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
