#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest
from datetime import datetime
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestPartialMatch(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.t = Task()

    def test_partial_date_match(self):
        """Partial match for dates: today=now --> true"""
        code, out, err = self.t(('calc', 'today=now'))
        self.assertIn('true', out)

    def test_exact_date_match(self):
        """Exact match for dates: today==now --> false"""
        code, out, err = self.t(('calc', 'today==now'))
        self.assertIn('false', out)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
