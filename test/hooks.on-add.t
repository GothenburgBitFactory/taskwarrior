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


class TestHooksOnAdd(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t.activate_hooks()

    def test_onadd_builtin_accept(self):
        """on-add-accept - a well-behaved, successful, on-add hook."""
        hookname = 'on-add-accept'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t("add foo")

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

        code, out, err = self.t("1 info")
        self.assertIn("Description   foo", out)

    def test_onadd_builtin_reject(self):
        """on-add-reject - a well-behaved, failing, on-add hook."""
        hookname = 'on-add-reject'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t.runError("add foo")

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(1)

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

    def test_onadd_builtin_misbehave1(self):
        """on-add-misbehave1 - does not consume input."""
        hookname = 'on-add-misbehave1'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t.runError("add foo")

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(1)

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

    def test_onadd_builtin_misbehave2(self):
        """on-add-misbehave2 - does not emit JSON."""
        hookname = 'on-add-misbehave2'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t.runError("add foo")
        self.assertIn("Hook Error: Expected 1 JSON task(s), found 0", err)

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

    def test_onadd_builtin_misbehave3(self):
        """on-add-misbehave3 - emits additional JSON."""
        hookname = 'on-add-misbehave3'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t.runError("add foo")
        self.assertIn("Hook Error: Expected 1 JSON task(s), found 2", err)

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

    def test_onadd_builtin_misbehave4(self):
        """on-add-misbehave4 - emits different task JSON."""
        hookname = 'on-add-misbehave4'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t.runError("add foo")
        self.assertIn("Hook Error: JSON must be for the same task:", err)

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

    def test_onadd_builtin_misbehave5(self):
        """on-add-misbehave5 - emits syntactically wrong JSON."""
        hookname = 'on-add-misbehave5'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t.runError("add foo")
        self.assertIn("Hook Error: JSON syntax error in: {\"}", err)

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)
        hook.assertInvalidJSONOutput()

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

    def test_onadd_builtin_misbehave6(self):
        """on-add-misbehave6 - emits incomplete JSON."""
        hookname = 'on-add-misbehave6'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t.runError("add foo")
        self.assertIn("Hook Error: JSON Object missing 'uuid' attribute.", err)

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
