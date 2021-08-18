#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
import unittest
import re
import json

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestExport(TestCase):
    def setUp(self):
        self.t = Task()

        self.t("add one")
        self.t("add two project:strange")
        self.t("add task1 +home project:A")
        self.t("add task2 +work project:A")

        self.t.config("report.foo.description", "DESC")
        self.t.config("report.foo.labels", "ID,DESCRIPTION")
        self.t.config("report.foo.columns", "id,description")
        self.t.config("report.foo.sort", "urgency+")
        self.t.config("report.foo.filter", "project:A")
        self.t.config("urgency.user.tag.home.coefficient", "15")

    def assertTaskEqual(self, t1, t2):
        keys = [k for k in sorted(t1.keys()) if k not in ["entry", "modified", "uuid"]]
        for k in keys:
            self.assertEqual(t1[k], t2[k])

    def test_exports(self):
        """Verify exports work"""
        code, out, err = self.t("export")
        out = json.loads(out)

        self.assertEqual(len(out), 4)

        self.assertTaskEqual(out[0], {"id": 1, "description": "one", "status": "pending", "urgency": 0})
        self.assertTaskEqual(out[1], {"id": 2, "description": "two", "project": "strange", "status": "pending", "urgency": 1})
        self.assertTaskEqual(out[2], {"id": 3, "description": "task1", "status": "pending", "project": "A", "tags": ["home"], "urgency": 16.8})
        self.assertTaskEqual(out[3], {"id": 4, "description": "task2", "status": "pending", "project": "A", "tags": ["work"], "urgency": 1.8})

    def test_exports_filter(self):
        """Verify exports with filter work"""
        code, out, err = self.t("one export")
        out = json.loads(out)

        self.assertEqual(len(out), 1)

        self.assertTaskEqual(out[0], {"id": 1, "description": "one", "status": "pending", "urgency": 0})

    def test_exports_with_limits_and_filter(self):
        """Verify exports with limits and filter work"""
        code, out, err = self.t("task limit:4 export")
        out = json.loads(out)

        self.assertEqual(len(out), 2)

        self.assertTaskEqual(out[0], {"id": 3, "description": "task1", "status": "pending", "project": "A", "tags": ["home"], "urgency": 16.8})
        self.assertTaskEqual(out[1], {"id": 4, "description": "task2", "status": "pending", "project": "A", "tags": ["work"], "urgency": 1.8})

        code, out, err = self.t("task limit:1 export")
        out = json.loads(out)

        self.assertEqual(len(out), 1)

        self.assertTaskEqual(out[0], {"id": 3, "description": "task1", "status": "pending", "project": "A", "tags": ["home"], "urgency": 16.8})

    def test_exports_report(self):
        """Verify exports with report work"""
        code, out, err = self.t("export foo")
        out = json.loads(out)

        self.assertEqual(len(out), 2)

        self.assertTaskEqual(out[0], {"id": 4, "description": "task2", "status": "pending", "project": "A", "tags": ["work"], "urgency": 1.8})
        self.assertTaskEqual(out[1], {"id": 3, "description": "task1", "status": "pending", "project": "A", "tags": ["home"], "urgency": 16.8})


if __name__ == "__main__":
    from simpletap import TAPTestRunner

    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 ft=python syntax=python
