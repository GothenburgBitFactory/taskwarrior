#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase, Taskd, ServerTestCase


class Test1430(TestCase):
    def setUp(self):
        self.t = Task()

    def test_project_names_with_dots(self):
        """Check that filtering works for project names with dots"""
        pro = "home.garden"
        self.t(('add', 'foo', 'project:%s' % pro))
        code, out, err = self.t(('list', 'project:%s' % pro))
        # We expect a clean exit
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))

    def test_project_names_with_slashes(self):
        """Check that filtering works for project names with slashes"""
        pro = "home/garden"
        self.t(('add', 'foo', 'project:%s' % pro))
        code, out, err = self.t(('list', 'project:%s' % pro))
        # We expect a clean exit
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
