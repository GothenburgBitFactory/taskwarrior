#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase, Taskd, ServerTestCase

class Test1468(TestCase):
    def setUp(self):
        self.t = Task()
        self.t(('add', 'project:home', 'buy milk'))
        self.t(('add', 'project:home', 'mow the lawn'))

    def test_single_attribute_filter(self):
        """Single attribute filter (project:home)"""
        code, out, err = self.t(('list', 'project:home'))
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn ('buy milk', out)
        self.assertIn ('mow the lawn', out)

    def test_attribute_and_implicit_search_filter(self):
        """Attribute and implicit search filter (project:home lawn)"""
        code, out, err = self.t(('list', 'project:home', 'lawn'))
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertNotIn ('buy milk', out)
        self.assertIn ('mow the lawn', out)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
