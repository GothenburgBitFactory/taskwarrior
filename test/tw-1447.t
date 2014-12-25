#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
from datetime import datetime
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase, Taskd, ServerTestCase

class Test1447(TestCase):
    def setUp(self):
        self.t = Task()

    def test_filter_uda(self):
        """Verify single-word aliases"""
        self.t.config('uda.sep.type', 'string')
        self.t(('add', 'one'))
        self.t(('add', 'two', 'sep:foo'))
        code, out, err = self.t(('sep:', 'list'))
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('one', out)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
