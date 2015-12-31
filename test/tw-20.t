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
from basetest.utils import mkstemp_exec


class TestBug20(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        # Used to initialize objects that should be re-initialized or
        # re-created for each individual test
        self.t = Task()

        # Workaround to always assume changes were introduced via "task edit"
        self.editor = mkstemp_exec("echo '' >> $1\n")

        self.t.env["VISUAL"] = self.editor

    def test_annotate_edit_does_not_delete(self):
        """ edit annotation should not delete then add untouched annotations """
        self.t("add tw-20")

        self.t("1 annotate 1st annotation")
        self.t("1 annotate 2nd annotation")

        code, _timestamp1a, err = self.t("_get 1.annotations.1.entry")
        code, _timestamp2a, err = self.t("_get 1.annotations.2.entry")

        self.t("1 edit")

        code, _timestamp1b, err = self.t("_get 1.annotations.1.entry")
        code, _timestamp2b, err = self.t("_get 1.annotations.2.entry")

        self.assertEqual( _timestamp1a, _timestamp1b )
        self.assertEqual( _timestamp2a, _timestamp2b )

        code, out, err = self.t("info")

        self.assertNotIn("Annotation '1st annotation' deleted.", out)
        self.assertNotIn("Annotation '2nd annotation' deleted.", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
