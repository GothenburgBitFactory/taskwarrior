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

import json
import sys
import os
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class Test1481(TestCase):
    def setUp(self):
        self.t = Task()
        self.t('add parent')
        self.t('add child')
        self.t('add child2')
        self.child1_uuid = self.t.export_one(2)['uuid']
        self.child2_uuid = self.t.export_one(3)['uuid']

    @unittest.expectedFailure
    def test_set_dependency_on_first_completed_task(self):
        """Sets dependency on task which has been just completed."""
        self.t('2 done')

        # Trigger the GC to clear up IDs
        self.t('next')

        # Set the dependency
        self.t('1 modify depends:%s' % self.child1_uuid)

    @unittest.expectedFailure
    def test_set_dependency_on_second_completed_task(self):
        """
        Sets dependency on task which has been completed
        before most recently completed task.
        """

        self.t('2 done')
        self.t('3 done')

        # Trigger the GC to clear up IDs
        self.t('next')

        # Set the dependencies
        self.t('1 modify depends:%s' % self.child2_uuid)

    @unittest.expectedFailure
    def test_set_dependency_on_two_completed_tasks(self):
        """ Sets dependency on two most recent completed tasks. """
        self.t('2 done')
        self.t('3 done')

        # Trigger the GC to clear up IDs
        self.t('next')

        # Set the dependencies
        self.t('1 modify depends:%s,%s' % (self.child1_uuid,
                                           self.child2_uuid))

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
