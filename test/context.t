#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
################################################################################
##
## Copyright 2006 - 2015, Paul Beckingham, Federico Hernandez.
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included
## in all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
## OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
## THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
##
## http://www.opensource.org/licenses/mit-license.php
##
################################################################################

import sys
import os
import unittest
import re

# Ensure python finds the local simpletap module
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from basetest import Task, TestCase

class ContextManagementTest(TestCase):
    def setUp(self):
        self.t = Task()

    def test_context_define(self):
        """
        Test simple context definition.
        """

        output = self.t(('context', 'define', 'work', 'project:Work'))[1]

        # Assert successful output
        self.assertIn("Context 'work' defined.", output)

        # Assert the config contains context definition
        self.assertIn('context.work=project:Work\n', self.t.taskrc_content)

        # Assert that it contains the definition only once
        is_context_line = lambda x: x == 'context.work=project:Work\n'
        self.assertEqual(len(filter(is_context_line, self.t.taskrc_content)), 1)

    def test_context_redefine_same_definition(self):
        """
        Test re-defining the context with the same definition.
        """

        self.t(('context', 'define', 'work', 'project:Work'))[1]
        output = self.t(('context', 'define', 'work', 'project:Work'))[1]

        # Assert successful output
        self.assertIn("Context 'work' defined.", output)

        # Assert the config contains context definition
        self.assertIn('context.work=project:Work\n', self.t.taskrc_content)

        # Assert that it contains the definition only once
        is_context_line = lambda x: x == 'context.work=project:Work\n'
        self.assertEqual(len(filter(is_context_line, self.t.taskrc_content)), 1)

    def test_context_redefine_different_definition(self):
        """
        Test re-defining the context with different definition.
        """

        self.t(('context', 'define', 'work', 'project:Work'))[1]
        output = self.t(('context', 'define', 'work', '+work'))[1]

        # Assert successful output
        self.assertIn("Context 'work' defined.", output)

        # Assert the config does not contain the old context definition
        self.assertNotIn('context.work=project:Work\n', self.t.taskrc_content)

        # Assert the config contains context definition
        self.assertIn('context.work=+work\n', self.t.taskrc_content)

        # Assert that it contains the definition only once
        is_context_line = lambda x: x == 'context.work=+work\n'
        self.assertEqual(len(filter(is_context_line, self.t.taskrc_content)), 1)

    def test_context_delete(self):
        """
        Test simple context deletion.
        """

        self.t(('context', 'define', 'work', 'project:Work'))
        output = self.t(('context', 'delete', 'work'))[1]

        # Assert correct output
        self.assertIn("Context 'work' undefined.", output)

        # Assert that taskrc does not countain context work definition
        self.assertFalse(any('context.work=' in line for line in self.t.taskrc_content))

    def test_context_delete_undefined(self):
        """
        Test deletion of undefined context.
        """

        output = self.t.runError(('context', 'delete', 'work'))[1]

        # Assert correct output
        self.assertIn("Context 'work' was not undefined.", output)

        # Assert that taskrc does not countain context work definition
        self.assertFalse(any('context.work=' in line for line in self.t.taskrc_content))

    def test_context_delete_unset_after_removal(self):
        """
        Test that context is unset if its definition has been removed.
        """

        self.t(('context', 'define', 'work', 'project:Work'))
        self.t(('context', 'work'))
        output = self.t(('context', 'delete', 'work'))[1]

        # Assert correct output
        self.assertIn("Context 'work' undefined.", output)

        # Assert that taskrc does not countain context work definition
        self.assertFalse(any('context.work=' in line for line in self.t.taskrc_content))

        # Aseert that the context is not set
        output = self.t(('context', 'show'))[1]
        self.assertIn('No context is currently applied.', output)
        self.assertFalse(any(re.search("^context=", line) for line in self.t.taskrc_content))

    def test_context_list(self):
        """
        Test the 'context list' command.
        """

        self.t(('context', 'define', 'work', 'project:Work'))[1]
        self.t(('context', 'define', 'home', '+home'))[1]

        output = self.t(('context', 'list'))[1]

        contains_work = lambda line: 'work' in line and 'project:Work' in line
        contains_home = lambda line: 'home' in line and '+home' in line

        # Assert that output contains work and home context definitions exactly
        # once
        self.assertEqual(len(filter(contains_work, output.splitlines())), 1)
        self.assertEqual(len(filter(contains_home, output.splitlines())), 1)

    def test_context_initially_empty(self):
        """
        Test that no context is set initially.
        """

        self.t(('context', 'define', 'work', 'project:Work'))[1]
        self.t(('context', 'define', 'home', '+home'))[1]

        output = self.t(('context', 'show'))[1]
        self.assertIn('No context is currently applied.', output)
        self.assertFalse(any(re.search("^context=", line) for line in self.t.taskrc_content))

    def test_context_setting(self):
        """
        Test simple context setting.
        """

        self.t(('context', 'define', 'work', 'project:Work'))[1]
        self.t(('context', 'define', 'home', '+home'))[1]

        output = self.t(('context', 'home'))[1]
        self.assertIn("Context 'home' applied.", output)
        self.assertIn("context=home\n", self.t.taskrc_content)

    def test_context_resetting(self):
        """
        Test resetting the same context.
        """

        self.t(('context', 'define', 'work', 'project:Work'))[1]
        self.t(('context', 'define', 'home', '+home'))[1]

        self.t(('context', 'home'))[1]
        output = self.t(('context', 'home'))[1]
        self.assertIn("Context 'home' applied.", output)

        contains_home = lambda line: line == "context=home\n"
        self.assertEqual(len(filter(contains_home, self.t.taskrc_content)), 1)

    def test_context_switching(self):
        """
        Test changing the context.
        """

        self.t(('context', 'define', 'work', 'project:Work'))[1]
        self.t(('context', 'define', 'home', '+home'))[1]

        contains_home = lambda line: line == "context=home\n"
        contains_work = lambda line: line == "context=work\n"

        # Switch to home context
        output = self.t(('context', 'home'))[1]
        self.assertIn("Context 'home' applied.", output)

        self.assertEqual(len(filter(contains_home, self.t.taskrc_content)), 1)

        # Switch to work context
        output = self.t(('context', 'work'))[1]
        self.assertIn("Context 'work' applied.", output)

        self.assertNotIn("context=home\n", self.t.taskrc_content)
        self.assertEqual(len(filter(contains_work, self.t.taskrc_content)), 1)

        # Switch back to home context
        output = self.t(('context', 'home'))[1]
        self.assertIn("Context 'home' applied.", output)

        self.assertNotIn("context=work\n", self.t.taskrc_content)
        self.assertEqual(len(filter(contains_home, self.t.taskrc_content)), 1)

    def test_context_unsetting(self):
        """
        Test removing the context.
        """

        self.t(('context', 'define', 'work', 'project:Work'))[1]
        self.t(('context', 'define', 'home', '+home'))[1]

        self.t(('context', 'home'))
        output = self.t(('context', 'none'))[1]

        # Assert expected output.
        self.assertIn("Context unset.", output)

        # Assert no context definition in the taskrc
        contains_any_context = lambda line: re.match('^context=', line)
        self.assertFalse(any(contains_any_context(line) for line in self.t.taskrc_content))

        # Assert no context showing up using show subcommand
        output = self.t(('context', 'show'))[1]
        self.assertIn("No context is currently applied.", output)

    def test_context_unsetting_after_switching(self):
        """
        Test unsetting the context after changing the context around.
        """

        self.t(('context', 'define', 'work', 'project:Work'))[1]
        self.t(('context', 'define', 'home', '+home'))[1]

        # Switch to contexts around
        self.t(('context', 'home'))
        self.t(('context', 'work'))
        self.t(('context', 'home'))

        # Unset the context
        output = self.t(('context', 'none'))[1]

        # Assert expected output.
        self.assertIn("Context unset.", output)

        # Assert no context definition in the taskrc
        contains_any_context = lambda line: re.match('^context=', line)
        self.assertFalse(any(contains_any_context(line) for line in self.t.taskrc_content))

        # Assert no context showing up using show subcommand
        output = self.t(('context', 'show'))[1]
        self.assertIn("No context is currently applied.", output)

    def test_context_unsetting_with_no_context_set(self):
        """
        Test removing the context when no context is set.
        """

        self.t(('context', 'define', 'work', 'project:Work'))
        self.t(('context', 'define', 'home', '+home'))

        output = self.t.runError(('context', 'none'))[1]

        # Assert expected output.
        self.assertIn("Context not unset.", output)

        # Assert no context definition in the taskrc
        contains_any_context = lambda line: re.match('^context=', line)
        self.assertFalse(any(contains_any_context(line) for line in self.t.taskrc_content))

        # Assert no context showing up using show subcommand
        output = self.t(('context', 'show'))[1]
        self.assertIn("No context is currently applied.", output)

    def test_context_completion(self):
        """
        Test the _context command.
        """

        self.t(('context', 'define', 'work', 'project:Work'))
        self.t(('context', 'define', 'home', '+home'))

        output = self.t(('_context',))[1]

        # Assert expected output.
        self.assertIn("work", output.splitlines())
        self.assertIn("home", output.splitlines())
        self.assertEqual(len(output.splitlines()), 2)

    def test_context_completion(self):
        """
        Test the _context command with some context set.
        """

        self.t(('context', 'define', 'work', 'project:Work'))
        self.t(('context', 'define', 'home', '+home'))

        # Activete some context
        self.t(('context', 'work'))

        output = self.t(('_context',))[1]

        # Assert expected output.
        self.assertIn("work", output.splitlines())
        self.assertIn("home", output.splitlines())
        self.assertEqual(len(output.splitlines()), 2)


class ContextEvaluationTest(TestCase):
    def setUp(self):
        self.t = Task()

        # Setup contexts
        self.t(('context', 'define', 'work', 'project:Work'))
        self.t(('context', 'define', 'home', '+home'))
        self.t(('context', 'define', 'today', 'due:today'))

        # Setup tasks
        self.t(('add', 'project:Work', "work task"))
        self.t(('add', '+home', "home task"))
        self.t(('add', 'project:Work', 'due:today', 'work today task'))
        self.t(('add', '+home', 'due:today', 'home today task'))

    def test_context_evaluation(self):
        """
        Test the context applied with report list command.
        """

        output = self.t(('list',))[1]

        # Assert all the tasks are present in the output
        self.assertIn("work task", output)
        self.assertIn("home task", output)
        self.assertIn("work today task", output)
        self.assertIn("home today task", output)

        # Set the home context and rerun the report
        self.t(('context', 'home'))
        output = self.t(('list',))[1]

        # Assert all the tasks with the home tag are present in the output
        self.assertNotIn("work task", output)
        self.assertIn("home task", output)
        self.assertNotIn("work today task", output)
        self.assertIn("home today task", output)

    def test_context_evaluation_switching(self):
        """
        Test swtiching context using the list report.
        """

        output = self.t(('list',))[1]

        # Assert all the tasks are present in the output
        self.assertIn("work task", output)
        self.assertIn("home task", output)
        self.assertIn("work today task", output)
        self.assertIn("home today task", output)

        # Set the home context and rerun the report
        self.t(('context', 'home'))
        output = self.t(('list',))[1]

        # Assert all the tasks with the home tag are present in the output
        self.assertNotIn("work task", output)
        self.assertIn("home task", output)
        self.assertNotIn("work today task", output)
        self.assertIn("home today task", output)

        # Set the work context and rerun the report
        self.t(('context', 'work'))
        output = self.t(('list',))[1]

        # Assert all the tasks with the home tag are present in the output
        self.assertIn("work task", output)
        self.assertNotIn("home task", output)
        self.assertIn("work today task", output)
        self.assertNotIn("home today task", output)

        # Set the today context and rerun the report
        self.t(('context', 'today'))
        output = self.t(('list',))[1]

        # Assert all the tasks with the home tag are present in the output
        self.assertNotIn("work task", output)
        self.assertNotIn("home task", output)
        self.assertIn("work today task", output)
        self.assertIn("home today task", output)

    def test_context_evaluation_unset(self):
        """
        Test unsetting context with report list command.
        """

        self.t(('context', 'home'))
        output = self.t(('list',))[1]

        # Assert all the tasks home tagged tasks are present
        self.assertNotIn("work task", output)
        self.assertIn("home task", output)
        self.assertNotIn("work today task", output)
        self.assertIn("home today task", output)

        # Set the context to none
        self.t(('context', 'none'))
        output = self.t(('list',))[1]

        # Assert all the tasks are present in the output
        self.assertIn("work task", output)
        self.assertIn("home task", output)
        self.assertIn("work today task", output)
        self.assertIn("home today task", output)

    def test_context_evaluation_with_user_filters(self):
        """
        Test the context applied with report list command
        combined with user filters.
        """

        # Set the home context
        self.t(('context', 'home'))
        output = self.t(('list', 'due:today'))[1]

        # Assert all the tasks are present in the output
        self.assertNotIn("work task", output)
        self.assertNotIn("home task", output)
        self.assertNotIn("work today task", output)
        self.assertIn("home today task", output)

        # Set the work context and rerun the report
        self.t(('context', 'work'))
        output = self.t(('list', 'due:today'))[1]

        # Assert all the tasks are present in the output
        self.assertNotIn("work task", output)
        self.assertNotIn("home task", output)
        self.assertIn("work today task", output)
        self.assertNotIn("home today task", output)


if __name__ == "__main__":
    from simpletap import TAPTestRunner
    unittest.main(testRunner=TAPTestRunner())

# vim: ai sts=4 et sw=4 syntax=python
