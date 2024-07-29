#!/usr/bin/env python3
###############################################################################
#
# Copyright 2024, Adrian Sadłocha.
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

import os
import sys
import unittest

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestNewsNag(TestCase):
    def setUp(self):
        self.t = Task()
        self.t("add Sample")

    def test_news_nag_gets_displayed_with_default_verbosity_levels(self):
        """Default verbosity"""

        _, _, err = self.t("")
        self.assertIn("Please run 'task news'", err)

    def test_news_nag_gets_displayed_when_explicitly_toggled_on(self):
        """Explicitly toggled on"""

        # Add `footnote` so there is a sink for the nag message.
        _, _, err = self.t("rc.verbose:news,footnote")
        self.assertIn("Please run 'task news'", err)

    def test_news_nag_does_not_get_displayed_when_explicitly_toggled_off(self):
        """Explicitly toggled off"""

        # Add `footnote` so there is a sink for the nag message.
        _, _, err = self.t("rc.verbose:footnote")
        self.assertNotIn("Please run 'task news'", err)


if __name__ == "__main__":
    from simpletap import TAPTestRunner

    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
