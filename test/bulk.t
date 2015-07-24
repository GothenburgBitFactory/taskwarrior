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


class TestBulk(TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""

    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_bulk_confirmations(self):
        """Exercise bulk and non-bulk confirmations for 'delete' and 'modify'"""
        self.t.config("bulk", "3")

        self.t("add one")
        self.t("add two")
        self.t("add three")
        self.t("add four")
        self.t("add five")
        self.t("add six")
        self.t("add seven")
        self.t("add eight")
        self.t("add nine")
        self.t("add ten")
        self.t("add eleven")
        self.t("add twelve")
        self.t("add thirteen")
        self.t("add fourteen")
        self.t("add fifteen")
        self.t("add sixteen")
        self.t("add seventeen")
        self.t("add eighteen")

        # The 'delete' command is used, but it could be any write command.
        # Note that 'y' is passed to task despite rc.confirmation=off.  This allows
        # failing tests to complete without blocking on input.

        # 'yes' tests:

        # Test with 1 task.  1 is a special case.
        code, out, err = self.t("1 delete rc.confirmation:off")
        self.assertNotIn("(yes/no)", out)
        self.assertNotIn("(yes/no/all/quit)", out)
        self.assertIn("Deleting task", out)

        code, out, err = self.t("2 delete rc.confirmation:on", input="y\n")
        self.assertIn("(yes/no)", out)
        self.assertNotIn("(yes/no/all/quit)", out)
        self.assertIn("Deleting task", out)

        # Test with 2 tasks.  2 is greater than 1 and less than bulk.
        code, out, err = self.t("3-4 delete rc.confirmation:off")
        self.assertNotIn("(yes/no)", out)
        self.assertNotIn("(yes/no/all/quit)", out)
        self.assertIn("Deleting task", out)

#        code, out, err = self.t("5-6 delete rc.confirmation:on", input="y\ny\n")
#        self.assertNotIn("(yes/no)", out)
#        self.assertIn("(yes/no/all/quit)", out)
#        self.assertIn("Deleting task", out)

#        # Test with 3 tasks.  3 is considered bulk.
#        code, out, err = self.t("7-9 delete rc.confirmation:off", input="y\n")
#        self.assertNotIn("(yes/no)", out)
#        self.assertIn("(yes/no/all/quit)", out)
#        self.assertIn("Deleting task", out)

#        code, out, err = self.t("10-12 delete rc.confirmation:on", input="y\ny\ny\n")
#        self.assertNotIn("(yes/no)", out)
#        self.assertIn("(yes/no/all/quit)", out)
#        self.assertIn("Deleting task", out)

        # 'no' tests:

        # Test with 1 task, denying delete.
        code, out, err = self.t.runError("13 delete rc.confirmation:on", input="n\n")
        self.assertIn("(yes/no)", out)
        self.assertNotIn("(yes/no/all/quit)", out)
        self.assertNotIn("Deleting task", out)

#        # Test with 2 tasks, denying delete.
#        code, out, err = self.t.runError("13-14 delete rc.confirmation:on", input="n\nn\n")
#        self.assertNotIn("(yes/no)", out)
#        self.assertIn("(yes/no/all/quit)", out)
#        self.assertNotIn("Deleting task", out)

#        # Test with 3 tasks, denying delete.
#        code, out, err = self.t.runError("13-15 delete rc.confirmation:on", input="n\nn\nn\n")
#        self.assertNotIn("(yes/no)", out)
#        self.assertIn("(yes/no/all/quit)", out)
#        self.assertNotIn("Deleting task", out)

        # 'all' tests:

        code, out, err = self.t("13-15 delete rc.confirmation:on", input="all\n")
        self.assertNotIn("(yes/no)", out)
        self.assertIn("(yes/no/all/quit)", out)
        self.assertIn("Deleting task", out)

        # 'quit' tests:

        code, out, err = self.t.runError("16-18 delete rc.confirmation:on", input="quit\n")
        self.assertNotIn("(yes/no)", out)
        self.assertIn("(yes/no/all/quit)", out)
        self.assertIn("Deleted 0 tasks", out)
        self.assertNotIn("Deleting task", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
