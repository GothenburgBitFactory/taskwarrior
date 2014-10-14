#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestBug252(TestCase):
    def setUp(self):
        self.t = Task()

    def test_done_stop(self):
        """done should also stop a task timer"""
        command = ("add", "Timeit")
        code, out, err = self.t(command)

        # Starting the newly added task
        command = ("1", "start")
        code, out, err = self.t(command)

        notexpected = "Start deleted"
        self.assertNotIn(notexpected, out)

        # Completing the task should also stop the timer
        command = ("1", "done")
        code, out, err = self.t(command)

        # Confirm that "start" was removed
        command = ("1", "info")
        code, out, err = self.t(command)

        expected = "Start deleted"
        self.assertIn(expected, out)

        # Confirm that the timer was indeed stopped
        command = ("1", "stop")
        code, out, err = self.t.runError(command)

        expected = "Task 1 'Timeit' not started."
        self.assertIn(expected, out)
        expected = "Stopped 0 tasks."
        self.assertIn(expected, out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
