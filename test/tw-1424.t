#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
from datetime import datetime
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase, Taskd, ServerTestCase


class Test1424(TestCase):
    def setUp(self):
        self.t = Task()

    def test_1824_days(self):
        """Check that due:1824d works"""
        self.t(('add', 'foo', 'due:1824d'))
        code, out, err = self.t(('_get', '1.due.year'))
        self.assertEqual(out, "%d\n" % (datetime.now().year + 5))

    def test_3648_days(self):
        """Check that due:3648d works"""
        self.t(('add', 'foo', 'due:3648d'))
        code, out, err = self.t(('_get', '1.due.year'))
        self.assertEqual(out, "%d\n" % (datetime.now().year + 10))

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
