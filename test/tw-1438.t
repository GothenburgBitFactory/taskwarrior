#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestBug1438(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()

    def test_recurring_tasks_shouldn_ask_for_confirmation(self):
        """rc.confirmation=off still prompts while changing recurring tasks"""
        command = ("add", "Sometimes", "due:tomorrow", "recur:daily",)
        code, out, err = self.t(command)
        self.assertIn("Created task 1", out)
        code, out, err = self.t(("list",))
        self.assertIn("Sometimes", out)

        command = ("rc.confirmation=off", "rc.recurrence.confirmation=off", "2", "mod", "/Sometimes/Everytime/")
        code, out, err = self.t(command)
        self.assertIn("Modified 1 task", out)
        code, out, err = self.t(("list",))
        self.assertIn("Everytime", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
