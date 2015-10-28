# -*- coding: utf-8 -*-

import atexit
import json
import os
import shlex
import shutil
import tempfile
import unittest
from .exceptions import CommandError
from .hooks import Hooks
from .utils import run_cmd_wait, run_cmd_wait_nofail, which, task_binary_location
from .compat import STRING_TYPE


class Task(object):
    """Manage a task warrior instance

    A temporary folder is used as data store of task warrior.
    This class can be instanciated multiple times if multiple taskw clients are
    needed.

    This class can be given a Taskd instance for simplified configuration.

    A taskw client should not be used after being destroyed.
    """
    DEFAULT_TASK = task_binary_location()

    def __init__(self, taskd=None, taskw=DEFAULT_TASK):
        """Initialize a Task warrior (client) that can interact with a taskd
        server. The task client runs in a temporary folder.

        :arg taskd: Taskd instance for client-server configuration
        :arg taskw: Task binary to use as client (defaults: task in PATH)
        """
        self.taskw = taskw
        self.taskd = taskd

        # Used to specify what command to launch (and to inject faketime)
        self._command = [self.taskw]

        # Configuration of the isolated environment
        self._original_pwd = os.getcwd()
        self.datadir = tempfile.mkdtemp(prefix="task_")
        self.taskrc = os.path.join(self.datadir, "test.rc")

        # Ensure any instance is properly destroyed at session end
        atexit.register(lambda: self.destroy())

        self.reset_env()

        with open(self.taskrc, 'w') as rc:
            rc.write("data.location={0}\n"
                     "hooks=off\n"
                     "".format(self.datadir))

        # Setup configuration to talk to taskd automatically
        if self.taskd is not None:
            self.bind_taskd_server(self.taskd)

        # Hooks disabled until requested
        self.hooks = None

    def __repr__(self):
        txt = super(Task, self).__repr__()
        return "{0} running from {1}>".format(txt[:-1], self.datadir)

    def __call__(self, *args, **kwargs):
        "aka t = Task() ; t() which is now an alias to t.runSuccess()"
        return self.runSuccess(*args, **kwargs)

    def activate_hooks(self):
        """Enable self.hooks functionality and activate hooks on config
        """
        self.config("hooks", "on")
        self.hooks = Hooks(self.datadir)

    def reset_env(self):
        """Set a new environment derived from the one used to launch the test
        """
        # Copy all env variables to avoid clashing subprocess environments
        self.env = os.environ.copy()

        # Make sure no TASKDDATA is isolated
        self.env["TASKDATA"] = self.datadir
        # As well as TASKRC
        self.env["TASKRC"] = self.taskrc

    def bind_taskd_server(self, taskd):
        """Configure the present task client to talk to given taskd server

        Note that this can be performed automatically by passing taskd when
        creating an instance of the current class.
        """
        self.taskd = taskd

        cert = os.path.join(self.taskd.certpath, "test_client.cert.pem")
        key = os.path.join(self.taskd.certpath, "test_client.key.pem")
        self.config("taskd.certificate", cert)
        self.config("taskd.key", key)
        self.config("taskd.ca", self.taskd.ca_cert)

        address = ":".join((self.taskd.address, str(self.taskd.port)))
        self.config("taskd.server", address)

        # Also configure the default user for given taskd server
        self.set_taskd_user()

    def set_taskd_user(self, taskd_user=None, default=True):
        """Assign a new user user to the present task client

        If default==False, a new user will be assigned instead of reusing the
        default taskd user for the corresponding instance.
        """
        if taskd_user is None:
            if default:
                user, group, org, userkey = self.taskd.default_user
            else:
                user, group, org, userkey = self.taskd.create_user()
        else:
            user, group, org, userkey = taskd_user

        credentials = "/".join((org, user, userkey))
        self.config("taskd.credentials", credentials)

        self.credentials = {
            "user": user,
            "group": group,
            "org": org,
            "userkey": userkey,
        }

    def config(self, var, value):
        """Run setup `var` as `value` in taskd config
        """
        # Add -- to avoid misinterpretation of - in things like UUIDs
        cmd = (self.taskw, "config", "--", var, value)
        return run_cmd_wait(cmd, env=self.env, input="y\n")

    def del_config(self, var):
        """Remove `var` from taskd config
        """
        cmd = (self.taskw, "config", var)
        return run_cmd_wait(cmd, env=self.env, input="y\n")

    @property
    def taskrc_content(self):
        """
        Returns the contents of the taskrc file.
        """

        with open(self.taskrc, "r") as f:
            return f.readlines()

    def export(self, export_filter=None):
        """Run "task export", return JSON array of exported tasks."""
        if export_filter is None:
            export_filter = ""

        code, out, err = self.runSuccess("rc.json.array=1 {0} export"
                                         "".format(export_filter))

        return json.loads(out)

    def export_one(self, export_filter=None):
        """
        Return a dictionary representing the exported task. Will
        fail if mutliple tasks match the filter.
        """

        result = self.export(export_filter=export_filter)

        if len(result) != 1:
            descriptions = [task.get('description') or '[description-missing]'
                            for task in result]

            raise ValueError(
                "One task should match the '{0}' filter, '{1}' "
                "matches:\n    {2}".format(
                    export_filter or '',
                    len(result),
                    '\n    '.join(descriptions)
                ))

        return result[0]

    @property
    def latest(self):
        return self.export_one("+LATEST")

    @staticmethod
    def _split_string_args_if_string(args):
        """Helper function to parse and split into arguments a single string
        argument. The string is literally the same as if written in the shell.
        """
        # Enable nicer-looking calls by allowing plain strings
        if isinstance(args, STRING_TYPE):
            args = shlex.split(args)

        return args

    def runSuccess(self, args="", input=None, merge_streams=False,
                   timeout=5):
        """Invoke task with given arguments and fail if exit code != 0

        Use runError if you want exit_code to be tested automatically and
        *not* fail if program finishes abnormally.

        If you wish to pass instructions to task such as confirmations or other
        input via stdin, you can do so by providing a input string.
        Such as input="y\ny\n".

        If merge_streams=True stdout and stderr will be merged into stdout.

        timeout = number of seconds the test will wait for every task call.
        Defaults to 1 second if not specified. Unit is seconds.

        Returns (exit_code, stdout, stderr) if merge_streams=False
                (exit_code, output) if merge_streams=True
        """
        # Create a copy of the command
        command = self._command[:]

        args = self._split_string_args_if_string(args)
        command.extend(args)

        output = run_cmd_wait_nofail(command, input,
                                     merge_streams=merge_streams,
                                     env=self.env,
                                     timeout=timeout)

        if output[0] != 0:
            raise CommandError(command, *output)

        return output

    def runError(self, args=(), input=None, merge_streams=False, timeout=5):
        """Invoke task with given arguments and fail if exit code == 0

        Use runSuccess if you want exit_code to be tested automatically and
        *fail* if program finishes abnormally.

        If you wish to pass instructions to task such as confirmations or other
        input via stdin, you can do so by providing a input string.
        Such as input="y\ny\n".

        If merge_streams=True stdout and stderr will be merged into stdout.

        timeout = number of seconds the test will wait for every task call.
        Defaults to 1 second if not specified. Unit is seconds.

        Returns (exit_code, stdout, stderr) if merge_streams=False
                (exit_code, output) if merge_streams=True
        """
        # Create a copy of the command
        command = self._command[:]

        args = self._split_string_args_if_string(args)
        command.extend(args)

        output = run_cmd_wait_nofail(command, input,
                                     merge_streams=merge_streams,
                                     env=self.env,
                                     timeout=timeout)

        # output[0] is the exit code
        if output[0] == 0 or output[0] is None:
            raise CommandError(command, *output)

        return output

    def destroy(self):
        """Cleanup the data folder and release server port for other instances
        """
        try:
            shutil.rmtree(self.datadir)
        except OSError as e:
            if e.errno == 2:
                # Directory no longer exists
                pass
            else:
                raise

        # Prevent future reuse of this instance
        self.runSuccess = self.__destroyed
        self.runError = self.__destroyed

        # self.destroy will get called when the python session closes.
        # If self.destroy was already called, turn the action into a noop
        self.destroy = lambda: None

    def __destroyed(self, *args, **kwargs):
        raise AttributeError("Task instance has been destroyed. "
                             "Create a new instance if you need a new client.")

    def diag(self, merge_streams_with=None):
        """Run task diagnostics.

        This function may fail in which case the exception text is returned as
        stderr or appended to stderr if merge_streams_with is set.

        If set, merge_streams_with should have the format:
        (exitcode, out, err)
        which should be the output of any previous process that failed.
        """
        try:
            output = self.runSuccess("diag")
        except CommandError as e:
            # If task diag failed add the error to stderr
            output = (e.code, None, str(e))

        if merge_streams_with is None:
            return output
        else:
            # Merge any given stdout and stderr with that of "task diag"
            code, out, err = merge_streams_with
            dcode, dout, derr = output

            # Merge stdout
            newout = "\n##### Debugging information (task diag): #####\n{0}"
            if dout is None:
                newout = newout.format("Not available, check STDERR")
            else:
                newout = newout.format(dout)

            if out is not None:
                newout = out + newout

            # And merge stderr
            newerr = "\n##### Debugging information (task diag): #####\n{0}"
            if derr is None:
                newerr = newerr.format("Not available, check STDOUT")
            else:
                newerr = newerr.format(derr)

            if err is not None:
                newerr = err + derr

            return code, newout, newerr

    def faketime(self, faketime=None):
        """Set a faketime using libfaketime that will affect the following
        command calls.

        If faketime is None, faketime settings will be disabled.
        """
        cmd = which("faketime")
        if cmd is None:
            raise unittest.SkipTest("libfaketime/faketime is not installed")

        if self._command[0] == cmd:
            self._command = self._command[3:]

        if faketime is not None:
            # Use advanced time format
            self._command = [cmd, "-f", faketime] + self._command

# vim: ai sts=4 et sw=4
