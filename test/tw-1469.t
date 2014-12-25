#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-

import sys
import os
import unittest
import platform

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase, Taskd, ServerTestCase

class Test1469(TestCase):
    def setUp(self):
        self.t = Task()
        self.t(('add', 'foo'))
        self.t(('add', 'neue Badmöbel kaufen'))

    def test_implicit_search_sensitive_regex(self):
        """Implicit search, case sensitive, regex """
        code, out, err = self.t(('list', 'möbel', 'rc.search.case.sensitive=yes', 'rc.regex=on'))
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn ('möbel', out)
        self.assertNotIn ('foo', out)

    def test_implicit_search_sensitive_noregex(self):
        """Implicit search, case sensitive, no regex """
        code, out, err = self.t(('list', 'möbel', 'rc.search.case.sensitive=yes', 'rc.regex=off'))
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn ('möbel', out)
        self.assertNotIn ('foo', out)

    def test_implicit_search_insensitive_regex(self):
        """Implicit search, case insensitive, regex """
        if 'CYGWIN' in platform.system():
            self.diag('Skipping regex case-insensitive test for Cygwin')
        else:
            code, out, err = self.t(('list', 'möbel', 'rc.search.case.sensitive=no', 'rc.regex=on'))
            self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
            self.assertIn ('möbel', out)
            self.assertNotIn ('foo', out)

    def test_implicit_search_insensitive_noregex(self):
        """Implicit search, case insensitive, no regex """
        code, out, err = self.t(('list', 'möbel', 'rc.search.case.sensitive=no', 'rc.regex=off'))
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn ('möbel', out)
        self.assertNotIn ('foo', out)

    def test_explicit_search_sensitive_regex(self):
        """Explicit search, case sensitive, regex """
        code, out, err = self.t(('list', '/möbel/', 'rc.search.case.sensitive=yes', 'rc.regex=on'))
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn ('möbel', out)
        self.assertNotIn ('foo', out)

    def test_explicit_search_sensitive_noregex(self):
        """Explicit search, case sensitive, no regex """
        code, out, err = self.t(('list', '/möbel/', 'rc.search.case.sensitive=yes', 'rc.regex=off'))
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn ('möbel', out)
        self.assertNotIn ('foo', out)

    def test_explicit_search_insensitive_regex(self):
        """Explicit search, case insensitive, regex """
        if 'CYGWIN' in platform.system():
            self.diag('Skipping regex case-insensitive test for Cygwin')
        else:
            code, out, err = self.t(('list', '/möbel/', 'rc.search.case.sensitive=no', 'rc.regex=on'))
            self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
            self.assertIn ('möbel', out)
            self.assertNotIn ('foo', out)

    def test_explicit_search_insensitive_noregex(self):
        """Explicit search, case insensitive, no regex """
        code, out, err = self.t(('list', '/möbel/', 'rc.search.case.sensitive=no', 'rc.regex=off'))
        self.assertEqual(0, code, "Exit code was non-zero ({0})".format(code))
        self.assertIn ('möbel', out)
        self.assertNotIn ('foo', out)

if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4
