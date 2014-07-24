#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestBug1381(TestCase):
    def setUp(self):
        self.t = Task()

    def test_blocking(self):
        """Blocking report displays tasks that are blocking other tasks"""
        self.t(("add", "blocks"))
        self.t(("add", "dep:1", "blocked"))
        code, out, err = self.t(("blocking",))

        self.assertIn("blocks", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
