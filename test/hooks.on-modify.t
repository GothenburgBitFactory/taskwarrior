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


class TestHooksOnModify(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t.activate_hooks()

    def test_onmodify_builtin_accept(self):
        """on-modify-accept - a well-behaved, successful, on-modify hook."""
        hookname = 'on-modify-accept'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t("add foo")
        code, out, err = self.t("1 modify +tag")

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

        code, out, err = self.t("1 info")
        self.assertIn("Description   foo", out)
        self.assertIn("Tags          tag", out)

    def test_onmodify_builtin_reject(self):
        """on-modify-reject - a well-behaved, failing, on-modify hook."""
        hookname = 'on-modify-reject'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t("add foo")
        code, out, err = self.t.runError("1 modify +tag")

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(1)

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

    def test_onmodify_builtin_misbehave2(self):
        """on-modify-misbehave2 - does not emit JSON."""
        hookname = 'on-modify-misbehave2'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t("add foo")
        code, out, err = self.t.runError("1 modify +tag")
        self.assertIn("Hook Error: Expected 1 JSON task(s), found 0", err)

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

    def test_onmodify_builtin_misbehave3(self):
        """on-modify-misbehave3 - emits additional JSON."""
        hookname = 'on-modify-misbehave3'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t("add foo")
        code, out, err = self.t.runError("1 modify +tag")
        self.assertIn("Hook Error: Expected 1 JSON task(s), found 2", err)

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

    def test_onmodify_builtin_misbehave4(self):
        """on-modify-misbehave4 - emits different task JSON."""
        hookname = 'on-modify-misbehave4'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t("add foo")
        code, out, err = self.t.runError("1 modify +tag")
        self.assertIn("Hook Error: JSON must be for the same task:", err)

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

    def test_onmodify_builtin_misbehave5(self):
        """on-modify-misbehave5 - emits syntactically wrong JSON."""
        hookname = 'on-modify-misbehave5'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t("add foo")
        code, out, err = self.t.runError("1 modify +tag")
        self.assertIn("Hook Error: JSON syntax error in: {\"}", err)

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

    def test_onmodify_builtin_misbehave6(self):
        """on-modify-misbehave6 - emits incomplete JSON."""
        hookname = 'on-modify-misbehave6'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t("add foo")
        code, out, err = self.t.runError("1 modify +tag")
        self.assertIn("Hook Error: JSON Object missing 'uuid' attribute.", err)

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

        logs = hook.get_logs()
        self.assertEqual(logs["output"]["msgs"][0], "FEEDBACK")

    def test_onmodify_revert_changes(self):
        """on-modify-revert - revert all user modifications."""
        hookname = 'on-modify-revert'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t("add foo")
        before = self.t.export()
        self.t.faketime("+5s")
        code, out, err = self.t("1 modify bar")
        after = self.t.export()

        # There should be absolutely _no_ changes.
        self.assertEqual(before, after)

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
