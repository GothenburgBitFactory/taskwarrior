#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import json
import sys
import os
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase, Taskd, ServerTestCase

class Test1481(TestCase):
    def setUp(self):
        self.t = Task()
        self.t(('add', 'parent'))
        self.t(('add', 'child'))
        self.t(('add', 'child2'))
        self.child1_uuid = json.loads(self.t(('2', 'export'))[1].strip(','))['uuid']
        self.child2_uuid = json.loads(self.t(('3', 'export'))[1].strip(','))['uuid']

    def test_set_dependency_on_first_completed_task(self):
        """Sets dependency on task which has been just completed."""
        self.t(('2', 'done'))

        # Trigger the GC to clear up IDs
        self.t(('next', ))

        # Set the dependency
        self.t(('1', 'modify', 'depends:%s' % self.child1_uuid))

    def test_set_dependency_on_second_completed_task(self):
        """
        Sets dependency on task which has been completed
        before most recently completed task.
        """

        self.t(('2', 'done'))
        self.t(('3', 'done'))

        # Trigger the GC to clear up IDs
        self.t(('next', ))

        # Set the dependencies
        self.t(('1', 'modify', 'depends:%s' % self.child2_uuid))

    def test_set_dependency_on_two_completed_tasks(self):
        """ Sets dependency on two most recent completed tasks. """
        self.t(('2', 'done'))
        self.t(('3', 'done'))

        # Trigger the GC to clear up IDs
        self.t(('next', ))

        # Set the dependencies
        self.t(('1', 'modify', 'depends:%s,%s' % (self.child1_uuid, self.child2_uuid)))

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
