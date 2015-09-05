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
import platform

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class Test1469(TestCase):
    def setUp(self):
        self.t = Task()
        self.t('add foo')
        self.t('add "neue Badmöbel kaufen"')

    def test_implicit_search_sensitive_regex(self):
        """Implicit search, case sensitive, regex """
        code, out, err = self.t('list /möbel/ rc.search.case.sensitive=yes rc.regex=on')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('möbel', out)
        self.assertNotIn('foo', out)

    def test_implicit_search_sensitive_noregex(self):
        """Implicit search, case sensitive, no regex """
        code, out, err = self.t('list /möbel/ rc.search.case.sensitive=yes rc.regex=off')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('möbel', out)
        self.assertNotIn('foo', out)

    @unittest.skipIf('CYGWIN' in platform.system(), 'Skipping regex case-insensitive test for Cygwin')
    def test_implicit_search_insensitive_regex(self):
        """Implicit search, case insensitive, regex """
        code, out, err = self.t('list /möbel/ rc.search.case.sensitive=no rc.regex=on')
        self.assertEqual(0, code,
                         "Exit code was non-zero ({0})".format(code))
        self.assertIn('möbel', out)
        self.assertNotIn('foo', out)

    def test_implicit_search_insensitive_noregex(self):
        """Implicit search, case insensitive, no regex """
        code, out, err = self.t('list /möbel/ rc.search.case.sensitive=no rc.regex=off')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('möbel', out)
        self.assertNotIn('foo', out)

    def test_explicit_search_sensitive_regex(self):
        """Explicit search, case sensitive, regex """
        code, out, err = self.t('list /möbel/ rc.search.case.sensitive=yes rc.regex=on')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('möbel', out)
        self.assertNotIn('foo', out)

    def test_explicit_search_sensitive_noregex(self):
        """Explicit search, case sensitive, no regex """
        code, out, err = self.t('list /möbel/ rc.search.case.sensitive=yes rc.regex=off')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('möbel', out)
        self.assertNotIn('foo', out)

    @unittest.skipIf('CYGWIN' in platform.system(), 'Skipping regex case-insensitive test for Cygwin')
    def test_explicit_search_insensitive_regex(self):
        """Explicit search, case insensitive, regex """
        code, out, err = self.t('list /möbel/ rc.search.case.sensitive=no rc.regex=on')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('möbel', out)
        self.assertNotIn('foo', out)

    def test_explicit_search_insensitive_noregex(self):
        """Explicit search, case insensitive, no regex """
        code, out, err = self.t('list /möbel/ rc.search.case.sensitive=no rc.regex=off')
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn('möbel', out)
        self.assertNotIn('foo', out)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
