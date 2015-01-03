#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
from datetime import datetime
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase, Taskd, ServerTestCase

class Test1486(TestCase):
    def setUp(self):
        self.t = Task()

    def test_waiting(self):
        """Verify waiting report shows waiting tasks"""
        self.t.config('uda.sep.type', 'string')

        self.t(('add', 'regular'))
        self.t(('add', 'waited', 'wait:later'))

        code, out, err = self.t(('waiting',))
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('waited', out)
        self.assertNotIn('regular', out)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
