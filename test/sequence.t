#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2016, Paul Beckingham, Federico Hernandez.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# http://www.opensource.org/licenses/mit-license.php
#
###############################################################################

import sys
import os
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestSequences(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t("add one mississippi")
        self.t("add two mississippi")

    def test_sequence_done(self):
        """Test sequences in done"""
        self.t("1,2 done")
        code, out, err = self.t("_get 1.status 2.status")
        self.assertEqual("completed completed\n", out)

    def test_sequence_delete(self):
        """Test sequences in delete"""
        self.t("1,2 delete", input="y\ny\n")
        code, out, err = self.t("_get 1.status 2.status")
        self.assertEqual("deleted deleted\n", out)

    def test_sequence_start_stop(self):
        """Test sequences in start/stop"""
        self.t("1,2 start")
        code, out, err = self.t("_get 1.start 2.start")
        self.assertRegexpMatches(out, "\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2} \d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\n")

        self.t("1,2 stop")
        code, out, err = self.t("_get 1.start 2.start")
        self.assertEqual(" \n", out)  # Space separating the two blank values.

    def test_sequence_modify(self):
        """Test sequences in modify"""
        self.t("1,2 modify +xyz")
        code, out, err = self.t("_get 1.tags 2.tags")
        self.assertEqual("xyz xyz\n", out)

    def test_sequence_info(self):
        """Test sequences in info"""
        self.t("1,2 info")
        code, out, err = self.t("_get 1.description 2.description")
        self.assertEqual(out.count("miss"), 2)

    def test_sequence_duplicate(self):
        """Test sequences in duplicate"""
        self.t("1,2 duplicate priority:H")
        code, out, err = self.t("_get 3.priority 4.priority")
        self.assertEqual("H H\n", out)

    def test_sequence_annotate(self):
        """Test sequences in annotate"""
        self.t("1,2 annotate note")
        code, out, err = self.t("_get 1.annotations.1.description 2.annotations.1.description")
        self.assertEqual("note note\n", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
