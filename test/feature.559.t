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
import re
import unittest
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestFeature559(TestCase):
    def setUp(self):
        self.t = Task()

        self.t.config("exit.on.missing.db", "yes")

        # NOTE the framework uses TASKDATA and TASKRC to tell taskwarrior where
        # data is stored. Since these env variables take precedence over
        # command-line specified options, overriding rc.data.location has no
        # effect.
        # In order to test rc.exit.on.missing.db we must unset the env vars and
        # point taskwarrior to the configuration files via rc:override.
        del self.t.env["TASKDATA"]
        del self.t.env["TASKRC"]

        # Inject rc:taskrc before any command used in this client
        # NOTE This will break if self.t.faketime() is used
        self.t._command.append("rc:{0}".format(self.t.taskrc))

    def test_exit_on_missing_db(self):
        """Missing db causes exit when rc.exit.on.missing.db=yes"""

        self.t("add footask")

        code, out, err = self.t("list")
        self.assertIn("footask", out)
        self.assertNotIn("Error", out)
        self.assertNotIn("Error", err)

        code, out, err = self.t.runError("rc.data.location=locationdoesnotexist list")
        self.assertNotIn("footask", out)
        self.assertNotIn("Error", out)
        self.assertRegexpMatches(err, re.compile("Error:.+does not exist", re.DOTALL))


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
