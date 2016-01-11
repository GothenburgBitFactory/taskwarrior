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


class TestHooksOnLaunch(TestCase):
    def setUp(self):
        """Executed before each test in the class"""
        self.t = Task()
        self.t.activate_hooks()

    def test_onlaunch_builtin_env(self):
        """on-launch-env - a well-behaved, successful, on-launch hook that echoes its env."""
        hookname = 'on-launch-good-env'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t("version")    # Arbitrary command that generates output.

        hook = self.t.hooks[hookname]
        hook.assertTriggeredCount(1)
        hook.assertExitcode(0)

        logs = hook.get_logs()
        taskenv = {k:v for k, v in (line.split(":", 1) for line in logs["output"]["msgs"])}

        self.assertEqual('api'     in taskenv, True, 'api:...')
        self.assertEqual('args'    in taskenv, True, 'args:...')
        self.assertEqual('command' in taskenv, True, 'command:...')
        self.assertEqual('rc'      in taskenv, True, 'rc:...')
        self.assertEqual('data'    in taskenv, True, 'data:...')
        self.assertEqual('version' in taskenv, True, 'version:...')

    def test_onlaunch_builtin_env_diag(self):
        """Verify that 'diagnostics' can see hook details"""
        hookname = 'on-launch-good-env'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t("diagnostics")
        self.assertRegexpMatches(out, r"on-launch-good-env\s+\(executable\)")

    def test_onlaunch_builtin_env_debug(self):
        """Verify that 'debug.hooks' shows hook details"""
        hookname = 'on-launch-good-env'
        self.t.hooks.add_default(hookname, log=True)

        code, out, err = self.t("version rc.debug.hooks:2")
        self.assertIn("api:", err)
        self.assertIn("Found hook script", err)
        self.assertIn("Hook: Completed with status 0", err)
        self.assertIn("Hook: input", err)
        self.assertIn("Hook: output", err)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
