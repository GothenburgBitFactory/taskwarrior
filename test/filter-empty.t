#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase

class TestEmptyFilter(TestCase):
    def setUp(self):
        self.t = Task()

    def test_empty_filter_warning(self):
        """Modify tasks with no filter."""

        self.t(("add", "foo"))
        self.t(("add", "bar"))

        code, out, err = self.t.runError(("modify", "rc.allow.empty.filter=yes", "rc.confirmation=no", "priority:H"))
        self.assertIn("Command prevented from running.", err)

    def test_empty_filter_error(self):
        """Modify tasks with no filter, and disallowed confirmation."""

        self.t(("add", "foo"))
        self.t(("add", "bar"))

        code, out, err = self.t.runError(("modify", "rc.allow.empty.filter=no", "priority:H"))
        self.assertIn("You did not specify a filter, and with the 'allow.empty.filter' value, no action is taken.", err)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
