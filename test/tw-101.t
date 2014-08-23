#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest
import math
# Ensure python finds the local simpletap and basetest modules
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestBug101(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        # Define report with truncated_count style
        with open(self.t.taskrc, 'a') as fh:
            fh.write("report.bug101.columns=description.truncated_count\n")
        #Find screen width in order to generate long enough string
        command = ("_get", "context.width")
        code, out, err = self.t(command)
        self.width = int(out)
        #Since task strips leading and trailing spaces, for the purposes
        #of these tests, ensure description contains no spaces so we know
        #exactly what string we are expecting
        self.short_description = "A_task_description_"
        #Generate long string
        self.long_description = self.short_description * int(math.ceil(float(self.width)/len(self.short_description)))

    def test_short_no_count(self):
        """Check short description with no annotations"""
        command = ("add", self.short_description)
        self.t(command)

        command = ("bug101",)
        code, out, err = self.t(command)

        expected = self.short_description
        self.assertIn(expected, out)

    def test_short_with_count(self):
        """Check short description with annotations"""
        command = ("add", self.short_description)
        self.t(command)

        command = ("1", "annotate", "A task annotation")
        self.t(command)

        command = ("bug101",)
        code, out, err = self.t(command)

        expected = self.short_description + " [1]"
        self.assertIn(expected, out)

    def test_long_no_count(self):
        """Check long description with no annotations"""
        command = ("add", self.long_description)
        self.t(command)

        command = ("bug101",)
        code, out, err = self.t(command)

        expected = self.long_description[:(self.width - 3)] + "..."
        self.assertIn(expected, out)

    def test_long_with_count(self):
        """Check long description with annotations"""
        command = ("add", self.long_description)
        self.t(command)

        command = ("1", "annotate", "A task annotation")
        self.t(command)

        command = ("bug101",)
        code, out, err = self.t(command)

        expected = self.long_description[:(self.width - 7)] + "... [1]"
        self.assertIn(expected, out)

    def test_long_with_double_digit_count(self):
        """Check long description with double digit amount of annotations"""
        command = ("add", self.long_description)
        self.t(command)

        for a in range(10):
            command = ("1", "annotate", "A task annotation")
            self.t(command)

        command = ("bug101",)
        code, out, err = self.t(command)

        expected = self.long_description[:(self.width - 8)] + "... [10]"
        self.assertIn(expected, out)

    def tearDown(self):
        command = ("1", "delete")
        self.t(command, "y\n")


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
