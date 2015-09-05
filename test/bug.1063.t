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
import re
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestBug1063(TestCase):
    def setUp(self):
        self.t = Task()

        self.t.config("uda.foo.type", "numeric")
        self.t.config("uda.foo.label", "Foo")
        self.t.config("report.bar.columns", "foo,description")
        self.t.config("report.bar.description", "Bar")
        self.t.config("report.bar.labels", "Foo,Desc")
        self.t.config("report.bar.sort", "foo-")

    def test_sortable_uda(self):
        """numeric UDA fields are sortable

        Reported as bug 1063
        """

        self.t("add four foo:4")
        self.t("add one foo:1")
        self.t("add three foo:3")
        self.t("add two foo:2")

        code, out, err = self.t("bar")
        expected = re.compile("4.+3.+2.+1", re.DOTALL)  # dot matches \n too
        self.assertRegexpMatches(out, expected)

        code, out, err = self.t("bar rc.report.bar.sort=foo+")
        expected = re.compile("1.+2.+3.+4", re.DOTALL)  # dot matches \n too
        self.assertRegexpMatches(out, expected)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
