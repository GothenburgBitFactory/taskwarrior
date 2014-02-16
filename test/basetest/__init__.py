# -*- coding: utf-8 -*-

import os
import tempfile
import shutil
import unittest
from subprocess import Popen, PIPE, STDOUT


class BaseTestCase(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        """Executed once before any test in the class"""
        cls._original_pwd = os.getcwd()
        cls._new_pwd = tempfile.mkdtemp()

        # Chdir into the temporary folder
        os.chdir(cls._new_pwd)

        # Change environment Prepare Env
        cls.prepareEnv()

        # Make task available locally
        cls.prepareExecutable()

        # Prepare the environment
        cls.prepare()

    @classmethod
    def prepareEnv(cls):
        """Ensure that any ENV variable used by task and setup in the
        environment doesn't affect the test
        """
        try:
            del os.environ["TASKDATA"]
        except KeyError:
            pass

        try:
            del os.environ["TASKRC"]
        except KeyError:
            pass

    @classmethod
    def prepareExecutable(cls):
        """Link the binary to be tested in the temporary folder"""
        src = os.path.join(cls._original_pwd, "..", "src", "task")
        os.symlink(src, "task")

    @classmethod
    def prepare(cls):
        """Additional setup, executed only once before any test

        The current (temporary) work dir is available as cls._new_pwd
        and the original work dir as cls._original_pwd
        """
        pass

    @classmethod
    def tearDownClass(cls):
        """Executed once after all tests in the class"""
        # Finishing steps before removal of temp dir
        cls.finish()

        # Finally remove whatever's left of the temporary folder
        os.chdir(cls._original_pwd)
        shutil.rmtree(cls._new_pwd)

    @classmethod
    def finish(cls):
        """Finishing steps, executed once after all tests are executed and
        before removal of the temporary directory

        The current (temporary) work dir is available as cls._new_pwd
        and the original work dir as cls._original_pwd

        Any files not removed here or via self.addCleanup() will be deleted
        once this function completes, as part of the removal of the temporary
        directory. Still it is good practice to explicitly clean your leftovers

        NOTE: If setUpClass or any of prepare* methods fails, finish() will
        not get called.
        """
        pass

    def callTask(self, args, input=None, merge_streams=True):
        """Invoke src/task with the given arguments

        Use callTaskSuccess or callTaskError if you want exit_code to be tested
        automatically and fail if result is unexpected.

        If you wish to pass instructions to task you can do so by providing a
        string via input. Such as input="add Hello task\n1 delete".

        If merge_streams=True stdout and stderr will be merged into stdout.

        Returns (exit_code, stdout, stderr)
        """
        if merge_streams:
            stderr = STDOUT
        else:
            stderr = PIPE

        if input is not None:
            stdin = PIPE
        else:
            stdin = None

        command = ["./task"]
        command.extend(args)

        p = Popen(command, stdin=stdin, stdout=PIPE, stderr=STDOUT)
        out, err = p.communicate(input)
        # In python3 we will be able use the following instead of the previous
        # line to avoid locking if task is unexpectedly waiting for input
        #try:
        #    out, err = p.communicate(input, timeout=15)
        #except TimeoutExpired:
        #    p.kill()
        #    out, err = proc.communicate()


        return p.returncode, out, err

    def callTaskSuccess(self, args, input=None, merge_streams=True):
        """Invoke src/task with the given arguments and expect a zero exit
        code.
        Causes test to fail if exit_code != 0

        If you wish to pass instructions to task you can do so by providing a
        string via input. Such as input="add Hello task\n1 delete".

        If merge_streams=True stdout and stderr will be merged into stdout.

        Returns (exit_code, stdout, stderr)
        """
        out = self.callTask(args, input, merge_streams)

        self.assertEqual(out[0], 0, "Task finished with non-zero ({0}) exit "
            "code".format(out[0]))
        return out

    def callTaskError(self, args, input=None, merge_streams=True):
        """Invoke src/task with the given arguments and expect a non-zero exit
        code.
        Causes test to fail if exit_code == 0

        If you wish to pass instructions to task you can do so by providing a
        string via input. Such as input="add Hello task\n1 delete".

        If merge_streams=True stdout and stderr will be merged into stdout.

        Returns (exit_code, stdout, stderr)
        """
        out = self.callTask(args, input, merge_streams)

        self.assertNotEqual(out[0], 0, "Task finished with zero exit (0) code")
        return out


# vim: ai sts=4 et sw=4
