###############################################################################
# taskwarrior - a command line task list manager.
#
# Copyright 2006 - 2018, Paul Beckingham, Federico Hernandez.
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

# Original version by Renato Alves

import os
import sys
import unittest
import warnings
import traceback
import inspect


def color(text, c):
    """
    Add color on the keyword that identifies the state of the test
    """
    if sys.stdout.isatty():
        clear = "\033[0m"

        colors = {
            "red": "\033[1m\033[91m",
            "yellow": "\033[1m\033[93m",
            "green": "\033[1m\033[92m",
        }
        return colors[c] + text + clear
    else:
        return text


class TAPTestResult(unittest.result.TestResult):
    def __init__(self, stream, descriptions, verbosity):
        super(TAPTestResult, self).__init__(stream, descriptions, verbosity)
        self.stream = stream
        self.descriptions = descriptions
        self.verbosity = verbosity
        # Buffer stdout and stderr
        self.buffer = True

    def getDescription(self, test):
        doc_first_line = test.shortDescription()
        if self.descriptions and doc_first_line:
            return doc_first_line
        else:
            try:
                method = test._testMethodName
            except AttributeError:
                return "Preparation error on: {0}".format(test.description)
            else:
                return "{0} ({1})".format(method, test.__class__.__name__)

    def startTestRun(self, total="unk"):
        self.stream.writeln("1..{0}".format(total))

    def stopTest(self, test):
        """Prevent flushing of stdout/stderr buffers until later"""
        pass

    def _restoreStdout(self):
        """Restore sys.stdout and sys.stderr, don't merge buffered output yet
        """
        if self.buffer:
            sys.stdout = self._original_stdout
            sys.stderr = self._original_stderr

    @staticmethod
    def _do_stream(data, stream):
        """Helper function for _mergeStdout"""
        for line in data.splitlines(True):
            # newlines should be taken literally and be comments in TAP
            line = line.replace("\\n", "\n# ")

            # Add a comment sign before each line
            if line.startswith("#"):
                stream.write(line)
            else:
                stream.write("# " + line)

        if not line.endswith('\n'):
            stream.write('\n')

    def _mergeStdout(self):
        """Merge buffered output with main streams
        """

        if self.buffer:
            output = self._stdout_buffer.getvalue()
            error = self._stderr_buffer.getvalue()
            if output:
                self._do_stream(output, sys.stdout)
            if error:
                self._do_stream(error, sys.stderr)

            self._stdout_buffer.seek(0)
            self._stdout_buffer.truncate()
            self._stderr_buffer.seek(0)
            self._stderr_buffer.truncate()

        # Needed to fix the stopTest override
        self._mirrorOutput = False

    def report(self, test, status=None, err=None):
        # Restore stdout/stderr but don't flush just yet
        self._restoreStdout()

        desc = self.getDescription(test)

        try:
            exception, msg, tb = err
        except (TypeError, ValueError):
            exception_name = ""
            msg = err
            tb = None
        else:
            exception_name = exception.__name__
            msg = str(msg)

        trace_msg = ""

        # Extract line where error happened for easier debugging
        trace = traceback.extract_tb(tb)
        for t in trace:
            # t = (filename, line_number, function_name, raw_line)
            if t[2].startswith("test"):
                trace_msg = " on file {0} line {1} in {2}: '{3}'".format(*t)
                break

        # Retrieve the name of the file containing the test
        filename = os.path.basename(inspect.getfile(test.__class__))

        if status:

            if status == "SKIP":
                self.stream.writeln("{0} {1} - {2}: {3} # skip".format(
                    color("ok", "yellow"), self.testsRun, filename, desc)
                )
            elif status == "EXPECTED_FAILURE":
                self.stream.writeln("{0} {1} - {2}: {3} # TODO".format(
                    color("not ok", "yellow"), self.testsRun, filename, desc)
                )
            else:
                self.stream.writeln("{0} {1} - {2}: {3}".format(
                    color("not ok", "red"), self.testsRun, filename, desc)
                )

            if exception_name:
                self.stream.writeln("# {0}: {1}{2}:".format(
                    status, exception_name, trace_msg)
                )
            else:
                self.stream.writeln("# {0}:".format(status))

            # Magic 3 is just for pretty indentation
            padding = " " * (len(status) + 3)

            for line in msg.splitlines():
                # Force displaying new-line characters as literal new lines
                line = line.replace("\\n", "\n# ")
                self.stream.writeln("#{0}{1}".format(padding, line))
        else:
            self.stream.writeln("{0} {1} - {2}: {3}".format(
                color("ok", "green"), self.testsRun, filename, desc)
            )

        # Flush all buffers to stdout
        self._mergeStdout()

    def addSuccess(self, test):
        super(TAPTestResult, self).addSuccess(test)
        self.report(test)

    def addError(self, test, err):
        super(TAPTestResult, self).addError(test, err)
        self.report(test, "ERROR", err)

    def addFailure(self, test, err):
        super(TAPTestResult, self).addFailure(test, err)
        self.report(test, "FAIL", err)

    def addSkip(self, test, reason):
        super(TAPTestResult, self).addSkip(test, reason)
        self.report(test, "SKIP", reason)

    def addExpectedFailure(self, test, err):
        super(TAPTestResult, self).addExpectedFailure(test, err)
        self.report(test, "EXPECTED_FAILURE", err)

    def addUnexpectedSuccess(self, test):
        super(TAPTestResult, self).addUnexpectedSuccess(test)
        self.report(test, "UNEXPECTED_SUCCESS", str(test))


class TAPTestRunner(unittest.runner.TextTestRunner):
    """A test runner that displays results using the Test Anything Protocol
    syntax.

    Inherits from TextTestRunner the default runner.
    """
    resultclass = TAPTestResult

    def run(self, test):
        result = self._makeResult()
        unittest.signals.registerResult(result)
        result.failfast = self.failfast

        # TAP requires output is on STDOUT.
        # TODO: Define this at __init__ time
        result.stream = unittest.runner._WritelnDecorator(sys.stdout)

        with warnings.catch_warnings():
            if getattr(self, "warnings", None):
                # if self.warnings is set, use it to filter all the warnings
                warnings.simplefilter(self.warnings)
                # if the filter is 'default' or 'always', special-case the
                # warnings from the deprecated unittest methods to show them
                # no more than once per module, because they can be fairly
                # noisy.  The -Wd and -Wa flags can be used to bypass this
                # only when self.warnings is None.
                if self.warnings in ['default', 'always']:
                    warnings.filterwarnings(
                        'module',
                        category=DeprecationWarning,
                        message='Please use assert\w+ instead.')
            startTestRun = getattr(result, 'startTestRun', None)
            if startTestRun is not None:
                startTestRun(test.countTestCases())
            try:
                test(result)
            finally:
                stopTestRun = getattr(result, 'stopTestRun', None)
                if stopTestRun is not None:
                    stopTestRun()

        return result
