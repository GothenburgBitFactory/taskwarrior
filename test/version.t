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
import re
from datetime import datetime
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase
from basetest.utils import run_cmd_wait


class TestVersion(TestCase):
    def setUp(self):
        self.t = Task()

        self.t.config("default.command", "")

    def test_usage_command(self):
        """no_command = usage - reports failure"""
        code, out, err = self.t.runError()

        self.assertIn("You must specify a command or a task to modify", err)

    def test_copyright_up_to_date(self):
        """Copyright is current"""
        code, out, err = self.t("version")

        expected = "Copyright \(C\) \d{4} - %d" % (datetime.now().year,)
        self.assertRegexpMatches(out, expected)

    def slurp(self, file="../CMakeLists.txt"):
        number = "\.".join(["[0-9]+"] * 3)
        ver = re.compile("^set \(PROJECT_VERSION \"({0}[^\"]*)\"\)$".format(number))
        with open(file) as fh:
            for line in fh:
                if "PROJECT_VERSION" in line:
                    match = ver.match(line)
                    if match:
                        return match.group(1)
        raise ValueError("Couldn't find matching version in {0}".format(file))

    def test_version(self):
        """version command outputs expected version and license"""
        code, out, err = self.t("version")

        expected = "task {0}".format(self.slurp())
        self.assertIn(expected, out)
        self.assertIn("MIT license", out)
        self.assertIn("http://taskwarrior.org", out)

    def slurp_git(self):
        git_cmd = ("git", "rev-parse", "--short", "--verify", "HEAD")
        _, hash, _ = run_cmd_wait(git_cmd)
        return hash.rstrip("\n")

    def test_under_version(self):
        """_version outputs expected version and syntax"""
        code, out, err = self.t("_version")

        # version = "x.x.x (git-hash)" or simply "x.x.x"
        # corresponding to "compiled from git" or "compiled from tarball"
        version = out.split()

        if 2 >= len(version) > 0:
            git = version[1]
            git_expected = "({0})".format(self.slurp_git())
            self.assertEqual(git_expected, git)
        else:
            raise ValueError("Unexpected output from _version '{0}'".format(
                out))

        ver = version[0]
        ver_expected = self.slurp()
        self.assertEqual(ver_expected, ver)

    def test_task_git_version(self):
        """Task binary matches the current git commit"""
        expected = "Commit: {0}".format(self.slurp_git())

        code, out, err = self.t.diag()
        self.assertIn(expected, out)

    def test_version_option(self):
        """Verify that  'task --version' returnes something valid"""
        code, out, err = self.t("--version")
        self.assertRegexpMatches(out, r'^\d\.\d+\.\d+(\.\w+)?$')


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
