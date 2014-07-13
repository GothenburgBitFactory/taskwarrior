# -*- coding: utf-8 -*-

import os
import tempfile
import shutil
import atexit
from .utils import run_cmd_wait, run_cmd_wait_nofail
from .exceptions import CommandError


class Task(object):
    """Manage a task warrior instance

    A temporary folder is used as data store of task warrior.
    This class can be instanciated multiple times if multiple taskw clients are
    needed.

    This class can be given a Taskd instance for simplified configuration.

    A taskw client should not be used after being destroyed.
    """
    def __init__(self, taskw="task", taskd=None):
        """Initialize a Task warrior (client) that can interact with a taskd
        server. The task client runs in a temporary folder.

        :arg taskw: Task binary to use as client (defaults: task in PATH)
        :arg taskd: Taskd instance for client-server configuration
        """
        self.taskw = taskw
        self.taskd = taskd

        # Configuration of the isolated environment
        self._original_pwd = os.getcwd()
        self.datadir = tempfile.mkdtemp()
        self.taskrc = os.path.join(self.datadir, "test.rc")

        # Ensure any instance is properly destroyed at session end
        atexit.register(lambda: self.destroy())

        # Copy all env variables to avoid clashing subprocess environments
        self.env = os.environ.copy()

        # Make sure no TASKDDATA is isolated
        self.env["TASKDATA"] = self.datadir
        # As well as TASKRC
        self.env["TASKRC"] = self.taskrc

        # Cannot call self.config until confirmation is disabled
        with open(self.taskrc, 'w') as rc:
            rc.write("data.location={0}\n"
                     "confirmation=no\n".format(self.datadir))

        # Setup configuration to talk to taskd automatically
        if self.taskd is not None:
            self.bind_taskd_server(self.taskd)

    def __repr__(self):
        txt = super(Task, self).__repr__()
        return "{0} running from {1}>".format(txt[:-1], self.datadir)

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

        self.credentials = "/".join((org, user, userkey))
        self.config("taskd.credentials", self.credentials)

    def config(self, var, value):
        """Run setup `var` as `value` in taskd config
        """
        # Add -- to avoid misinterpretation of - in things like UUIDs
        cmd = (self.taskw, "config", "--", var, value)
        return run_cmd_wait(cmd, env=self.env)

    def runSuccess(self, args=(), input=None, merge_streams=True):
        """Invoke task with the given arguments

        Use runError if you want exit_code to be tested automatically and
        *not* fail if program finishes abnormally.

        If you wish to pass instructions to task such as confirmations or other
        input via stdin, you can do so by providing a input string.
        Such as input="y\ny".

        If merge_streams=True stdout and stderr will be merged into stdout.

        Returns (exit_code, stdout, stderr)
        """
        command = [self.taskw]
        command.extend(args)

        return run_cmd_wait(command, input,
                            merge_streams=merge_streams, env=self.env)

    def runError(self, args=(), input=None, merge_streams=True):
        """Same as runSuccess but Invoke task with the given arguments

        Use runSuccess if you want exit_code to be tested automatically and
        *fail* if program finishes abnormally.

        If you wish to pass instructions to task such as confirmations or other
        input via stdin, you can do so by providing a input string.
        Such as input="y\ny".

        If merge_streams=True stdout and stderr will be merged into stdout.

        Returns (exit_code, stdout, stderr)
        """
        command = [self.taskw]
        command.extend(args)

        output = run_cmd_wait_nofail(command, input,
                                     merge_streams=merge_streams, env=self.env)

        # output[0] is the exit code
        if output[0] == 0:
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

    def diag(self, out):
        """Diagnostics are just lines preceded with #.
        """
        print '# --- diag start ---'
        for line in out.split("\n"):
            print '#', line
        print '# --- diag end ---'

# vim: ai sts=4 et sw=4
