#!/usr/bin/env python3

import sys
import os
import unittest
import re
import json
import string

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestExport(TestCase):
    def setUp(self):
        self.t = Task()

        # pretty arbitrary, just need several unique tasks
        for letter in string.ascii_lowercase:
            self.t(f"add test_task +{letter}")
            self.t(f"+{letter} done")

    def test_export_stability_for_multiple_id_0(self):
        exports = [self.t("export")[1] for _ in range(2)]
        json_lists = [json.loads(s.strip()) for s in exports]
        # to rule out a typo causing two failed exports
        self.assertEqual(len(json_lists[0]), len(string.ascii_lowercase))
        # for better diff view
        self.assertEqual(json_lists[0], json_lists[1])
        # the real test
        self.assertEqual(exports[0], exports[1])


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

