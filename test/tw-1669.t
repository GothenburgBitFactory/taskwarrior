#!/usr/bin/env python3
# -*- coding: utf-8 -*-
###############################################################################
#
# Copyright 2006 - 2021, Tomas Babej, Paul Beckingham, Federico Hernandez.
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
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase

# Test methods available:
#     self.assertEqual(a, b)
#     self.assertNotEqual(a, b)
#     self.assertTrue(x)
#     self.assertFalse(x)
#     self.assertIs(a, b)
#     self.assertIsNot(a, b)
#     self.assertIsNone(x)
#     self.assertIsNotNone(x)
#     self.assertIn(a, b)
#     self.assertNotIn(a, b)
#     self.assertIsInstance(a, b)
#     self.assertNotIsInstance(a, b)
#     self.assertRaises(e)
#     self.assertRegex(t, r)
#     self.assertNotRegex(t, r)
#     self.tap("")

class TestBugXXXX(TestCase):
    def setUp(self):
        self.t = Task()

    def test_escaped_escape_char_in_annotation(self):
        """Annotate a task using double backslashes
        and check it displaying correctly."""
        self.t("add test")
        self.t("""annotate 1 'Lorem \\n ipsum \\\\n dolor \\\\\\n sit
 amet'""")
        code, out, err = self.t("_get 1.annotations.1.description")
        self.tap(out)
        self.tap(err)

        expected = """Lorem 
 ipsum \\n dolor \\
 sit
 amet
"""

        self.assertEqual(out, expected)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
