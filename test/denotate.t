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


class TestDenotate(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""

    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_denotation(self):
        """Test the various forms of denotation"""

        self.t("add one")
        self.t("1 annotate alpha")
        self.t("1 annotate beta")
        self.t("1 annotate beta")  # Deliberately identical, not a typo
        self.t("1 annotate gamma")

        self.t("add two")
        self.t("2 annotate alpha")
        self.t("2 annotate beta")

        # Exact match, one annotation
        code, out, err = self.t("1 denotate alpha")
        self.assertIn("Found annotation 'alpha' and deleted it.", out)
        code, out, err = self.t("_get 1.annotations.1.description")
        self.assertEqual("beta\n", out)

        # Partial match, one annotation
        code, out, err = self.t("1 denotate gam")
        self.assertIn("Found annotation 'gamma' and deleted it.", out)

        # Failed partial match, one annotation
        code, out, err = self.t.runError("rc.search.case.sensitive=yes 2 denotate AL")
        self.assertIn("Did not find any matching annotation to be deleted for 'AL'.", out)

        # Exact match, two annotations
        code, out, err = self.t("1 denotate beta")
        self.assertIn("Found annotation 'beta' and deleted it.", out)
        code, out, err = self.t("1 denotate beta")
        self.assertIn("Found annotation 'beta' and deleted it.", out)

        # No annotation specified
        code, out, err = self.t("2 denotate")
        self.assertIn("Found annotation 'alpha' and deleted it.", out)
        code, out, err = self.t("2 denotate")
        self.assertIn("Found annotation 'beta' and deleted it.", out)
        code, out, err = self.t.runError("2 denotate")
        self.assertIn("The specified task has no annotations that can be deleted.", err)

        # No matching task
        code, out, err = self.t.runError("999 denotate")
        self.assertIn("No tasks specified.", err)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
