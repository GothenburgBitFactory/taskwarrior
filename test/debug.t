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


class TestDebugMode(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t("add one")
        self.t("add two")

    def test_debug_output(self):
        """Verify debug mode generates interesting output"""
        code, out, err = self.t("list rc.debug=1")

        # Debug
        self.assertIn("Config::load", err)
        self.assertIn("Filtered 2 tasks --> 2 tasks [pending only]", err)
        self.assertIn("Perf task", err)

    def test_debug_parser_output(self):
        """Verify debug parser mode generates interesting output"""
        code, out, err = self.t("list rc.debug.parser=2")

        # Debug
        self.assertIn("Config::load", err)
        self.assertIn("Filtered 2 tasks --> 2 tasks [pending only]", err)
        self.assertIn("Perf task", err)

        # Parser
        self.assertIn("CLI2::prepareFilter", err)

    def test_debug_parser_eval_output(self):
        """Verify debug parser + eval mode generates interesting output"""
        code, out, err = self.t("list rc.debug.parser=3")

        # Debug
        self.assertIn("Config::load", err)
        self.assertIn("Filtered 2 tasks --> 2 tasks [pending only]", err)
        self.assertIn("Perf task", err)

        # Parser
        self.assertIn("CLI2::prepareFilter", err)
        self.assertIn("Infix parsed", err)

    def test_debug_hooks_output(self):
        """Verify debug hooks mode generates interesting output"""
        code, out, err = self.t("list rc.debug.hooks=2")

        # Debug
        self.assertIn("Config::load", err)
        self.assertIn("Filtered 2 tasks --> 2 tasks [pending only]", err)
        self.assertIn("Perf task", err)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
