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
        self.t('add due:2015-07-07T00:00:00 ON1')
        self.t('add due:2015-07-07T14:34:56 ON2')
        self.t('add due:2015-07-07T23:59:59 ON3')

        # Setup some tasks not due on 2015-07-07
        self.t('add due:2015-07-06T23:59:59 OFF4')
        self.t('add due:2015-07-08T00:00:00 OFF5')
        self.t('add due:2015-07-08T00:00:01 OFF6')
        self.t('add due:2015-07-06T00:00:00 OFF7')

    def test_due_match_not_exact(self):
        """Test that due:<date> matches any task that date."""
        code, out, err = self.t('due:2015-07-07 minimal')

        # Asswer that only tasks ON the date are listed.
        self.assertIn("ON1", out)
        self.assertIn("ON2", out)
        self.assertIn("ON3", out)

        # Assert that tasks on other dates are not listed.
        self.assertNotIn("OFF4", out)
        self.assertNotIn("OFF5", out)
        self.assertNotIn("OFF6", out)
        self.assertNotIn("OFF7", out)

    def test_due_not_match_not_exact(self):
        """Test that due.not:<date> does not match any task that date."""
        code, out, err = self.t('due.not:2015-07-07 minimal')

        # Assert that task ON the date are not listed.
        self.assertNotIn("ON1", out)
        self.assertNotIn("ON2", out)
        self.assertNotIn("ON3", out)

        # Assert that tasks on other dates are listed.
        self.assertIn("OFF4", out)
        self.assertIn("OFF5", out)
        self.assertIn("OFF6", out)
        self.assertIn("OFF7", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
