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


class TestHooksOnExit(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t.activate_hooks()

    def test_onexit_builtin_good(self):
        """on-exit-good - a well-behaved, successful, on-exit hook."""
        hookname = 'on-exit-good'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t("version")
        self.assertIn("Taskwarrior", out)

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

    def test_onexit_builtin_bad(self):
        """on-exit-bad - a well-behaved, failing, on-exit hook."""
        hookname = 'on-exit-bad'
        self.t.hooks.add_default(hookname, log=True)

        # Failing hook should prevent processing.
        code, out, err = self.t.runError("version")
        self.assertIn("Taskwarrior", out)

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(1)

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

    def test_onexit_builtin_misbehave1(self):
        """on-exit-misbehave1 - Does not consume input."""
        hookname = 'on-exit-misbehave1'
        self.t.hooks.add_default(hookname, log=True)

        # Failing hook should prevent processing.
        code, out, err = self.t("version")
        self.assertIn("Taskwarrior", out)

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

    def test_onexit_builtin_misbehave2(self):
        """on-exit-misbehave2 - Emits unexpected JSON."""
        hookname = 'on-exit-misbehave2'
        self.t.hooks.add_default(hookname, log=True)

        # Failing hook should prevent processing.
        code, out, err = self.t.runError("version")
        self.assertIn("Hook Error: Expected 0 JSON task(s), found 1", err)

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
