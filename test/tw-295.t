#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest
from datetime import datetime
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestBugNumber(TestCase):
    @classmethod
    def setUp(self):
        self.t = Task()

    def test_subst_with_slashes(self):
        """Test substitution containing slashes"""
        self.t(('add', '--', 'one/two/three'))
        self.t(('1', 'modify', '/\\/two\\//TWO/'))
        code, out, err = self.t(('list',))
        self.assertIn('oneTWOthree', out)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
