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
from contextlib import contextmanager
# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase
from basetest.utils import BIN_PREFIX

TASKSH = os.path.abspath(os.path.join(BIN_PREFIX, "..", "scripts/bash/task.sh"))


@contextmanager
def tasksh(t):
    cmd_backup = t._command

    # Use "bash task.sh task" as command
    t._command = ["bash", t.tasksh_script, t._command[0]]
    yield

    # Restore the default "task" command
    t._command = cmd_backup


def prepare_tasksh(t):
    """Prepare task.sh to be used in tests"""
    tasksh = []

    # Ensure the task binary used is the same configured for the test
    with open(TASKSH) as fh:
        for line in fh:
            line = line.rstrip()

            if line == "taskcommand='task rc.verbose:nothing rc.confirmation:no rc.hooks:off'":
                line = "taskcommand='{0} rc.verbose:nothing rc.confirmation:no rc.hooks:off rc:{1}'".format(t.taskw, t.taskrc)

            tasksh.append(line)

    tasksh.extend([
        'COMP_WORDS=("$@")',
        'COMP_CWORD=$(($#-1))',
        '_task',
        'for reply_iter in "${COMPREPLY[@]}"; do',
        '  echo $reply_iter',
        'done',
    ])

    return '\n'.join(tasksh)


class TestBashCompletionBase(TestCase):
    def setUp(self):
        self.t = Task()

        # Used also in tasksh script
        self.t.config("alias.samplealias", "long")
        self.t.config("abbreviation.minimum", "5")

        self.t.tasksh_script = os.path.join(self.t.datadir, "task.sh")

        with open(self.t.tasksh_script, 'w') as tasksh:
            tasksh.write(prepare_tasksh(self.t))


class TestBashCompletion(TestBashCompletionBase):
    def test_aliases(self):
        """aliases should be expanded"""
        with tasksh(self.t):
            code, out, err = self.t("sampleali")

            self.assertIn("samplealias", out)

    def test_commands(self):
        """commands should be expanded"""
        with tasksh(self.t):
            code, out, err = self.t("m")

            # Expansion of 'm' should contain modify
            self.assertIn("modify", out)


class TestProject(TestBashCompletionBase):
    def setUp(self):
        super(TestProject, self).setUp()

        code, out, err = self.t("add testing project expansion project:todd")
        self.assertIn("Created task 1", out)

        # Ensure project was created
        code, out, err = self.t("projects")
        self.assertIn("todd", out)

    def test_project_non_matching(self):
        """project: expansion fails on non matching attribute"""
        with tasksh(self.t):
            code, out, err = self.t("projeABC : to")
            self.assertNotIn("todd", out)
            self.assertNotIn("todd", err)

    def test_project_abbreviation_minimum_match(self):
        """project: expansion expanded after rc.abbreviation.minimum"""
        with tasksh(self.t):
            code, out, err = self.t("proje : to")
            self.assertIn("todd", out)

    def test_project_abbreviation_minimum_shorter(self):
        """project: fails if shorter than rc.abbreviation.minimum"""
        with tasksh(self.t):
            code, out, err = self.t("proj : to")
            self.assertNotIn("todd", out)
            self.assertNotIn("todd", err)

    def test_project(self):
        """project: should be expanded and dependent on abbreviation minimum"""
        with tasksh(self.t):
            # TASKRC=test.rc is passed by the test suite by default
            code, out, err = self.t("ad to")
            # no taskrc override message should be shown
            self.assertNotIn("override", out)
            self.assertNotIn("override", err)

    def test_gc_bash_completion(self):
        """no gc coming from bash completion"""

        code, out, err = self.t("add this task should be number 2 and stay number 2")
        self.assertIn("Created task 2", out)

        code, out, err = self.t("1 delete", input="y\n")
        self.assertIn("Deleted 1 task", out)

        with tasksh(self.t):
            code, out, err = self.t("depends :")
            self.assertNotIn("todd", out)
            self.assertNotIn("todd", err)

        code, out, err = self.t("2 modify shouldreplacetext")
        # Success = no gc was run
        self.assertIn("shouldreplacetext", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
