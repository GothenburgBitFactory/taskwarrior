#!/usr/bin/env python3
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2024, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
# https://www.opensource.org/licenses/mit-license.php
#
###############################################################################

import sys
import os
import unittest
import time
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase
from basetest.utils import mkstemp


class TestImport(TestCase):
    def setUp(self):
        self.t = Task()
        # Set up local sync within the TASKDATA directory, so that it will be
        # deleted properly.
        self.t.config("sync.local.server_dir", self.t.datadir)

    def exists(self, uuid):
        code, out, err = self.t(f"_get {uuid}.status")
        return out.strip() != ""

    def test_expiration(self):
        """Only tasks that are deleted and have a modification in the past are expired."""
        yesterday = int(time.time()) - 3600 * 24
        last_year = int(time.time()) - 265 * 3600 * 24
        old_pending = "a1111111-a111-a111-a111-a11111111111"
        old_completed = "a2222222-a222-a222-a222-a22222222222"
        new_deleted = "a3333333-a333-a333-a333-a33333333333"
        old_deleted = "a4444444-a444-a444-a444-a44444444444"
        task_data = f"""[
    {{"uuid":"{old_pending}","status":"pending","modified":"{last_year}","description":"x"}},
    {{"uuid":"{old_completed}","status":"completed","modified":"{last_year}","description":"x"}},
    {{"uuid":"{new_deleted}","status":"deleted","modified":"{yesterday}","description":"x"}},
    {{"uuid":"{old_deleted}","status":"deleted","modified":"{last_year}","description":"x"}}
]
"""
        code, out, err = self.t("import -", input=task_data)
        self.assertIn("Imported 4 tasks", err)

        # By default, expiration does not occur.
        code, out, err = self.t("sync")
        self.assertTrue(self.exists(old_pending))
        self.assertTrue(self.exists(old_completed))
        self.assertTrue(self.exists(new_deleted))
        self.assertTrue(self.exists(old_deleted))

        # Configure expiration on sync. The old_deleted task
        # should be removed.
        self.t.config("expiration.on-sync", "1")
        code, out, err = self.t("sync")
        self.assertTrue(self.exists(old_pending))
        self.assertTrue(self.exists(old_completed))
        self.assertTrue(self.exists(new_deleted))
        self.assertFalse(self.exists(old_deleted))

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
