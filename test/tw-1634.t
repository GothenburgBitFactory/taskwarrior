#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
################################################################################
##
## Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included
## in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
## OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
## THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
##
## http://www.opensource.org/licenses/mit-license.php
##
################################################################################

import sys
import os
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase

class Test1634(TestCase):
    def setUp(self):
        self.t = Task()

        # Setup some tasks due on 2015-07-07
        self.t('add due:2015-07-07T00:00:00 on due date 1')
        self.t('add due:2015-07-07T08:00:00 on due date 2')
        self.t('add due:2015-07-07T14:34:56 on due date 3')
        self.t('add due:2015-07-07T22:00:00 on due date 4')
        self.t('add due:2015-07-07T23:59:59 on due date 5')

        # Setup some tasks not due on 2015-07-07
        self.t('add due:2015-07-06T23:59:59 not on due date 1')
        self.t('add due:2015-07-08T00:00:00 not on due date 2')
        self.t('add due:2015-07-08T00:00:01 not on due date 3')
        self.t('add due:2015-07-06T00:00:00 not on due date 4')


    def test_due_match_not_exact(self):
        """Test that due:<date> matches any task that date."""

        code, out, err = self.t('due:2015-07-07 minimal')

        # Assert that each task on 2015-07-07 is listed
        for i in range(1,5):
            self.assertIn("on due date {0}".format(i), out)

        # Assert that no task not on 2015-07-07 is not listed
        for i in range(1,4):
            self.assertNotIn("not on due date {0}".format(i), out)

    def test_due_not_match_not_exact(self):
        """Test that due.not:<date> does not match any task that date."""

        code, out, err = self.t('due.not:2015-07-07 minimal')

        # Assert that every task not on 2015-07-07 is listed
        for i in range(1,4):
            self.assertIn("not on due date {0}".format(i), out)

        # Assert that each task on 2015-07-07 is listed
        for i in range(1,5):
            self.assertNotIn("on due date {0}".format(i), out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
