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


class Test1510(TestCase):
    def setUp(self):
        self.t = Task()

    def assertNoEmptyValueInBacklog(self, attribute_name):
        backlog_path = os.path.join(self.t.datadir, 'backlog.data')
        with open(backlog_path) as backlog:
            empty_value = '"%s":""' % attribute_name
            self.assertFalse(any(empty_value in line
                                 for line in backlog.readlines()))

    def test_no_empty_value_for_deleted_due_in_backlog(self):
        """
        1510: Make sure deleted due attribute does not get into
        backlog.data with empty string value
        """

        self.t('add test due:2015-05-05')
        self.t('1 mod due:')
        self.assertNoEmptyValueInBacklog('due')

    def test_no_empty_value_for_empty_priority_in_backlog(self):
        """
        1510: Make sure empty priority attribute does not get into
        backlog.data with empty string value
        """

        self.t('add test pri:""')
        self.t("add test2 pri:''")
        self.t("add test3 pri:")
        self.t("add test4 pri:H")
        self.assertNoEmptyValueInBacklog('priority')

    def test_no_empty_value_for_empty_project_in_backlog(self):
        """
        1510: Make sure empty project attribute does not get into
        backlog.data with empty string value
        """

        self.t('add test project:""')
        self.t("add test2 project:''")
        self.t("add test3 project:")
        self.t("add test4 project:random")
        self.assertNoEmptyValueInBacklog('project')

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
