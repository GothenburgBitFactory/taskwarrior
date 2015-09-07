#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
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
from basetest.utils import mkstemp_exec


class TestTaskEdit(TestCase):
    def setUp(self):
        self.t = Task()

        # Workaround to always assume changes were introduced via "task edit"
        self.editor = mkstemp_exec("echo '' >> $1\n")

        self.t.env["VISUAL"] = self.editor

    def test_newline_description_edit(self):
        """task edit - parsing entries containing multiline descriptions"""

        self.t('add "Hello\nLost"')

        code, out, err = self.t()
        self.assertIn("Lost", out)

        # Newlines may not be correctly parsed
        code, out, err = self.t("1 edit")

        code, out, err = self.t()
        self.assertIn("Lost", out)

    def test_newline_annotation_edit(self):
        """task edit - parsing entries containing multiline annotations"""

        self.t("add Hello")
        self.t('1 annotate "Something\nLost"')

        code, out, err = self.t()
        self.assertIn("Lost", out)

        # Newlines may not be correctly parsed
        code, out, err = self.t("1 edit")

        code, out, err = self.t()
        self.assertIn("Lost", out)

    def test_fully_loaded_task_edit(self):
        """task edit - exercise all attributes possible"""

        self.t("add foo project:P +tag priority:H active:now due:eom wait:eom scheduled:eom recur:P1M until:eoy")
        self.t("1 annotate bar", input="n\n")

        # Does not fail
        self.t("1 edit")


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
