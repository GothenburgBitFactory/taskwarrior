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


class TestNagging(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls.t = Task()
        cls.t.config("nag", "NAG")

    def test_nagging(self):
        """Verify that nagging works when tasks are done in the 'wrong' order"""
        self.t("add due:yesterday one")
        self.t("add due:tomorrow two")
        self.t("add priority:H three")
        self.t("add priority:M four")
        self.t("add priority:L five")
        self.t("add six")
        self.t("add seven +nonag")

        code, out, err = self.t("7 done")
        self.assertNotIn("NAG", err)

        code, out, err = self.t("6 done")
        self.assertIn("NAG", err)

        code, out, err = self.t("5 done")
        self.assertIn("NAG", err)

        code, out, err = self.t("4 done")
        self.assertIn("NAG", err)

        code, out, err = self.t("3 done")
        self.assertIn("NAG", err)

        code, out, err = self.t("2 done")
        self.assertIn("NAG", err)

        code, out, err = self.t("1 done")
        self.assertNotIn("NAG", err)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
