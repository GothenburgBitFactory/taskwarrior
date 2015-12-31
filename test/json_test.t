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
from glob import glob
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import TestCase
from basetest.utils import CURRENT_DIR, run_cmd_wait_nofail
from basetest.meta import MetaTest


class MetaTestJson(MetaTest):
    """Helper metaclass to simplify test logic below (TestJson)

    Creates test_methods in the TestCase class dynamically named after the
    arguments used.
    """
    @staticmethod
    def make_function(classname, *args, **kwargs):
        filepath = args[0]
        TEST_BIN = kwargs.get("TEST_BIN")

        def test(self):
            # ### Body of the usual test_testcase ### #
            cmd = (TEST_BIN, filepath)
            code, out, err = run_cmd_wait_nofail(cmd)

            self.assertNotIn("Error", out)

        filename = os.path.basename(filepath)

        # Title of test in report
        test.__doc__ = "{0} {1}".format(classname, filename)

        return test


class TestJsonParsing(TestCase):
    __metaclass__ = MetaTestJson

    EXTRA = {}
    EXTRA["TEST_DIR"] = os.path.abspath(os.path.join(CURRENT_DIR, ".."))
    EXTRA["TEST_BIN"] = os.path.join(EXTRA["TEST_DIR"], "json_test")

    TESTS = (
        zip(glob(os.path.join(EXTRA["TEST_DIR"], "json/*.json")))
    )


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
