#!/usr/bin/env python3
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
import json

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase
from basetest.utils import mkstemp


class TestImport(TestCase):
    def setUp(self):
        self.t = Task()
        self.t.config("dateformat", "m/d/Y")

        # Multiple tasks.
        self.pending = """\
[description:"bing" due:"1734480000" entry:"1734397061" modified:"1734397061" status:"pending" uuid:"ad7f7585-bff3-4b57-a116-abfc9f71ee4a"]
[description:"baz" entry:"1734397063" modified:"1734397063" status:"pending" uuid:"591ccfee-dd8d-44e9-908a-40618257cf54"]\
"""
        self.completed = """\
[description:"foo" end:"1734397073" entry:"1734397054" modified:"1734397074" status:"deleted" uuid:"6849568f-55d7-4152-8db0-00356e39f0bb"]
[description:"bar" end:"1734397065" entry:"1734397056" modified:"1734397065" status:"completed" uuid:"51921813-7abb-412d-8ada-7c1417d01209"]\
"""

    def test_import_v2(self):
        with open(os.path.join(self.t.datadir, "pending.data"), "w") as f:
            f.write(self.pending)
        with open(os.path.join(self.t.datadir, "completed.data"), "w") as f:
            f.write(self.completed)
        code, out, err = self.t("import-v2")
        self.assertIn("Imported 4 tasks", err)

        code, out, err = self.t("list")
        self.assertIn("bing", out)
        self.assertIn("baz", out)
        self.assertNotIn("foo", out)
        self.assertNotIn("bar", out)

        code, out, err = self.t("completed")
        self.assertNotIn("bing", out)
        self.assertNotIn("baz", out)
        self.assertNotIn("foo", out)  # deleted, not in the completed report
        self.assertIn("bar", out)


if __name__ == "__main__":
    from simpletap import TAPTestRunner

    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python
