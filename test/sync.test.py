#!/usr/bin/env python3

import os
import sys
import unittest

sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase


class TestWorkingSetSync(TestCase):
    def setUp(self):
        self.first = Task()
        self.second = Task()
        self.server = os.path.join(self.first.datadir, "server")
        os.makedirs(self.server)
        self.first.config("sync.local.server_dir", self.server)
        self.second.config("sync.local.server_dir", self.server)

    def test_remote_completion_and_additions_are_reported(self):
        self.first("add one")
        self.first("sync")
        self.second("sync")

        self.second("1 done")
        self.second("add two")
        two_uuid = self.second("_get 2.uuid")[1].strip()
        self.second("sync")
        self.first("sync")

        code, out, err = self.first("list")
        self.assertNotIn("one", out)
        self.assertRegex(out, r"1\s+-\s+two")
        self.assertEqual(two_uuid, self.first("_get 1.uuid")[1].strip())

        self.first("1 modify two-updated")
        self.first("sync")
        self.second("sync")

        code, out, err = self.second("list")
        self.assertRegex(out, r"2\s+\S+\s+two-updated")
        self.assertEqual(two_uuid, self.second("_get 2.uuid")[1].strip())

    def test_recurrence_converges(self):
        self.first.config("recurrence.limit", "1")
        self.second.config("recurrence.limit", "1")
        self.first("add due:tomorrow recur:daily recurring")
        self.first("list")
        expected = self.first.export_one("status:pending")["uuid"]

        self.first("sync")
        self.second("sync")
        self.second("list")

        self.assertEqual(expected, self.second.export_one("status:pending")["uuid"])

        self.second("sync")
        self.first("sync")

        self.assertEqual(expected, self.first.export_one("status:pending")["uuid"])

    def test_synced_dependency_state_updates(self):
        self.second("add one")
        self.second("add two")
        self.second("add three")
        self.second("add four")
        first_uuid = self.second.export_one(1)["uuid"]
        second_uuid = self.second.export_one(2)["uuid"]
        third_uuid = self.second.export_one(3)["uuid"]
        fourth_uuid = self.second.export_one(4)["uuid"]
        self.second("3-4 modify depends:1 rc.bulk=0")

        self.second("sync")
        self.first("sync")

        third_id = self.first(f"_get {third_uuid}.id")[1].strip()
        fourth_id = self.first(f"_get {fourth_uuid}.id")[1].strip()
        self.first(
            f"{third_id},{fourth_id} modify depends:-{first_uuid} depends:{second_uuid} rc.bulk=0"
        )
        self.assertEqual("\n", self.first(f"_get {first_uuid}.tags.BLOCKING")[1])
        self.assertEqual([second_uuid], self.first.export_one(third_uuid)["depends"])
        self.assertEqual([second_uuid], self.first.export_one(fourth_uuid)["depends"])

        self.first("sync")
        self.second("sync")

        self.assertEqual("\n", self.second(f"_get {first_uuid}.tags.BLOCKING")[1])
        self.assertEqual(
            "BLOCKING\n", self.second(f"_get {second_uuid}.tags.BLOCKING")[1]
        )
        self.assertEqual([second_uuid], self.second.export_one(third_uuid)["depends"])
        self.assertEqual([second_uuid], self.second.export_one(fourth_uuid)["depends"])


if __name__ == "__main__":
    from simpletap import TAPTestRunner

    unittest.main(testRunner=TAPTestRunner())
