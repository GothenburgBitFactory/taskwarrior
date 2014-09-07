#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest
from datetime import datetime
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestMath(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.t = Task()
        cls.t.config("verbose", "nothing")
        cls.t.config("dateformat", "YYYY-MM-DD")

        # YYYY-12-21.
        cls.when = "%d-12-21T23:59:59\n" % datetime.now().year

        # Different ways of specifying YYYY-12-21.
        cls.t(('add', 'one',   "due:eoy-10days"))
        cls.t(('add', 'two',   "due:'eoy-10days'"))
        cls.t(('add', 'three', "'due:eoy-10days'"))
        cls.t(('add', 'four',  "due:'eoy - 10days'"))
        cls.t(('add', 'five',  "'due:eoy - 10days'"))
        cls.t(('add', 'six',   "'due:%d-12-31T23:59:59 - 10days'" % datetime.now().year))

    def test_compact_unquoted(self):
        """compact unquoted"""
        code, out, err = self.t(('_get', '1.due'))
        self.assertEqual(out, self.when)

    def test_compact_value_quoted(self):
        """compact value quoted"""
        code, out, err = self.t(('_get', '2.due'))
        self.assertEqual(out, self.when)

#    def test_compact_arg_quoted(self):
#        """compact arg quoted"""
#        code, out, err = self.t(('_get', '3.due'))
#        self.assertEqual(out, self.when)

    def test_sparse_value_quoted(self):
        """sparse value quoted"""
        code, out, err = self.t(('_get', '4.due'))
        self.assertEqual(out, self.when)

#    def test_sparse_arg_quoted(self):
#        """sparse arg quoted"""
#        code, out, err = self.t(('_get', '5.due'))
#        self.assertEqual(out, self.when)

#    def test_sparse_arg_quoted_literal(self):
#        """sparse arg quoted literal"""
#        code, out, err = self.t(('_get', '6.due'))
#        self.assertEqual(out, self.when)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
