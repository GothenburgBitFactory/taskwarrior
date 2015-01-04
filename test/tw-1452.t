#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

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
        self.t(('add', 'task'))
        self.task_uuid = json.loads(self.t(('1', 'export'))[1].strip())['uuid']

    def test_get_task_by_uuid_with_prefix(self):
        """Tries to filter task simply by it's uuid, using uuid: prefix."""

        # Load task
        output = self.t(('uuid:%s' % self.task_uuid, 'export'))[1]

        # Sanity check it is the correct one
        self.assertEqual(json.loads(output.strip())['uuid'], self.task_uuid)

    def test_get_task_by_uuid_without_prefix(self):
        """Tries to filter task simply by it's uuid, without using uuid: prefix."""

        # Load task
        output = self.t((self.task_uuid, 'export'))[1]

        # Sanity check it is the correct one
        self.assertEqual(json.loads(output.strip())['uuid'], self.task_uuid)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
