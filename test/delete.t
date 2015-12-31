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
from basetest.exceptions import CommandError


class TestDelete(TestCase):
    def setUp(self):
        self.t = Task()

    def test_add_delete_undo(self):
        """Verify that add/delete/undo yields a Pending task"""
        self.t("add one")
        code, out, err = self.t("_get 1.uuid")
        uuid = out.strip()

        self.t("1 delete", input="y\n")
        self.t.runError("list") # GC/handleRecurrence
        code, out, err = self.t("_get 1.status")
        self.assertEqual("\n", out)

        code, out, err = self.t("_get %s.status" % uuid)
        self.assertIn("deleted\n", out)

        self.t("undo", input="y\n")
        code, out, err = self.t("_get 1.status")
        self.assertIn("pending\n", out)
        code, out, err = self.t("_get %s.status" % uuid)
        self.assertIn("pending\n", out)

    def test_delete_en_passant(self):
        """Verify that en-passant works with delete"""
        self.t("add foo")
        code, out, err = self.t("1 delete project:work", input="y\n")
        self.assertIn("Deleted 1 task.", out)

        code, out, err = self.t("all rc.verbose:nothing")
        self.assertIn("work", out)

    def test_add_done_delete(self):
        """Verify that a completed task can be deleted"""
        self.t("add foo")
        code, out, err = self.t("_get 1.uuid")
        uuid = out.strip()

        code, out, err = self.t("1 done")
        self.assertIn("Completed 1 task.", out)

        self.t("all") # GC/handleRecurrence

        code, out, err = self.t("%s delete" % uuid, input="y\n")
        self.assertIn("Deleted 1 task.", out)

        code, out, err = self.t("_get %s.status" % uuid)
        self.assertIn("deleted\n", out)

    def test_delete_single_prompt_loop(self):
        """Delete prompt with closed STDIN causes infinite loop and floods stdout (single)"""
        self.t("add foo1")

        # Would expect 1 yes via input none sent
        code, out, err = self._validate_prompt_loop(input="")

        self.assertIn("Task not deleted", out)
        self.assertIn("Deleted 0 tasks", out)

        # If command was aborted, exit code should be 1
        self.assertEqual(code, 1)

    def test_delete_multiple_prompt_loop(self):
        """Delete prompt with closed STDIN causes infinite loop and floods stdout (multiple)"""
        self.t("add foo1")
        self.t("add foo2")

        # Would expect 2 yes via input only 1 sent
        code, out, err = self._validate_prompt_loop(input="y\n")
        self.assertEqual(code, 1)

    def test_delete_bulk_prompt_loop(self):
        """Delete prompt with closed STDIN causes infinite loop and floods stdout (bulk)"""
        self.t.config("confirmation", "off")
        # bulk is applied when confirmation is disabled
        self.t.config("bulk", "2")

        self.t("add foo1")
        self.t("add foo2")
        self.t("add foo3")

        # Would expect 3 yes via input only 2 sent
        code, out, err = self._validate_prompt_loop(input="y\ny\n")

        self.assertEqual(code, 1)

    def _validate_prompt_loop(self, input=""):
        """Helper method to check if task flooded stream on closed STDIN"""
        try:
            code, out, err = self.t("/foo[1-3]/ delete", input=input, timeout=0.3)
        except CommandError as e:
            # If delete fails with a timeout, don't fail the test immediately
            code, out, err = e.code, e.out, e.err

        # If task fails to notice STDIN is closed it will loop and flood for
        # confirmation until timeout
        # 500 bytes is arbitrary. Shouldn't reach this value in normal execution
        self.assertLessEqual(len(out), 500)
        self.assertLessEqual(len(err), 500)

        return code, out, err


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
