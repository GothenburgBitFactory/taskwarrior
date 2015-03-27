# -*- coding: utf-8 -*-

from __future__ import division
import os
import tempfile
import shutil
import signal
import atexit
from time import sleep
from subprocess import Popen
from .utils import (find_unused_port, release_port, port_used, run_cmd_wait,
                    which, parse_datafile, DEFAULT_CERT_PATH,
                    taskd_binary_location)
from .exceptions import CommandError

try:
    from subprocess import DEVNULL
except ImportError:
    DEVNULL = open(os.devnull, 'w')


class Taskd(object):
    """Manage a taskd instance

    A temporary folder is used as data store of taskd.
    This class can be instanciated multiple times if multiple taskd servers are
    needed.

    This class implements mechanisms to automatically select an available port
    and prevent assigning the same port to different instances.

    A server can be stopped and started multiple times, but should not be
    started or stopped after being destroyed.
    """
    DEFAULT_TASKD = taskd_binary_location()

    def __init__(self, taskd=DEFAULT_TASKD, certpath=None,
                 address="localhost"):
        """Initialize a Task server that runs in the background and stores data
        in a temporary folder

        :arg taskd: Taskd binary to launch the server (defaults: taskd in PATH)
        :arg certpath: Folder where to find all certificates needed for taskd
        :arg address: Address to bind to
        """
        self.taskd = taskd
        self.usercount = 0

        # Will hold the taskd subprocess if it's running
        self.proc = None
        self.datadir = tempfile.mkdtemp(prefix="taskd_")
        self.tasklog = os.path.join(self.datadir, "taskd.log")
        self.taskpid = os.path.join(self.datadir, "taskd.pid")

        # Ensure any instance is properly destroyed at session end
        atexit.register(lambda: self.destroy())

        self.reset_env()

        if certpath is None:
            certpath = DEFAULT_CERT_PATH
        self.certpath = certpath

        self.address = address
        self.port = find_unused_port(self.address)

        # Keep all certificate paths public for access by TaskClients
        self.client_cert = os.path.join(self.certpath, "client.cert.pem")
        self.client_key = os.path.join(self.certpath, "client.key.pem")
        self.server_cert = os.path.join(self.certpath, "server.cert.pem")
        self.server_key = os.path.join(self.certpath, "server.key.pem")
        self.server_crl = os.path.join(self.certpath, "server.crl.pem")
        self.ca_cert = os.path.join(self.certpath, "ca.cert.pem")

        # Initialize taskd
        cmd = (self.taskd, "init", "--data", self.datadir)
        run_cmd_wait(cmd, env=self.env)

        self.config("server", "{0}:{1}".format(self.address, self.port))
        self.config("log", self.tasklog)
        self.config("pid.file", self.taskpid)
        self.config("root", self.datadir)
        self.config("client.allow", "^task [2-9]")

        # Setup all necessary certificates
        self.config("client.cert", self.client_cert)
        self.config("client.key", self.client_key)
        self.config("server.cert", self.server_cert)
        self.config("server.key", self.server_key)
        self.config("server.crl", self.server_crl)
        self.config("ca.cert", self.ca_cert)

        self.default_user = self.create_user()

    def __repr__(self):
        txt = super(Taskd, self).__repr__()
        return "{0} running from {1}>".format(txt[:-1], self.datadir)

    def reset_env(self):
        """Set a new environment derived from the one used to launch the test
        """
        # Copy all env variables to avoid clashing subprocess environments
        self.env = os.environ.copy()

        # Make sure TASKDDATA points to the temporary folder
        self.env["TASKDATA"] = self.datadir

    def create_user(self, user=None, group=None, org=None):
        """Create a user/group in the server and return the user
        credentials to use in a taskw client.
        """
        if user is None:
            # Create a unique user ID
            uid = self.usercount
            user = "test_user_{0}".format(uid)

            # Increment the user_id
            self.usercount += 1

        if group is None:
            group = "default_group"

        if org is None:
            org = "default_org"

        self._add_entity("org", org, ignore_exists=True)
        self._add_entity("group", org, group, ignore_exists=True)
        userkey = self._add_entity("user", org, user)

        return user, group, org, userkey

    def _add_entity(self, keyword, org, value=None, ignore_exists=False):
        """Add an organization, group or user to the current server

        If a user creation is requested, the user unique ID is returned
        """
        cmd = (self.taskd, "add", "--data", self.datadir, keyword, org)

        if value is not None:
            cmd += (value,)

        try:
            code, out, err = run_cmd_wait(cmd, env=self.env)
        except CommandError as e:
            match = False
            for line in e.out.splitlines():
                if line.endswith("already exists.") and ignore_exists:
                    match = True
                    break

            # If the error was not "Already exists" report it
            if not match:
                raise

        if keyword == "user":
            expected = "New user key: "
            for line in out.splitlines():
                if line.startswith(expected):
                    return line.replace(expected, '')

    def config(self, var, value):
        """Run setup `var` as `value` in taskd config
        """
        cmd = (self.taskd, "config", "--force", "--data", self.datadir, var,
               value)
        run_cmd_wait(cmd, env=self.env)

        # If server is running send a SIGHUP to force config reload
        if self.proc is not None:
            try:
                self.proc.send_signal(signal.SIGHUP)
            except:
                pass

    def status(self):
        """Check the status of the server by checking if it's still running and
        listening for connections
        :returns: True if running and listening, False otherwise (including
            crashed and not started)
        """
        if self.proc is None:
            return False

        if self.proc.poll() is not None:
            return False

        if not port_used(addr=self.address, port=self.port):
            return False

        return True

    def start(self, minutes=5, tries_per_minute=2):
        """Start the taskd server if it's not running.
        If it's already running OSError will be raised
        """
        if self.proc is None:
            cmd = (self.taskd, "server", "--data", self.datadir)
            self.proc = Popen(cmd, stdout=DEVNULL, stdin=DEVNULL, env=self.env)
        else:
            raise OSError("Taskd server is still running or crashed")

        # Wait for server to listen by checking connectivity in the port
        # Default is to wait up to 5 minutes checking once every 500ms
        for i in range(minutes * 60 * tries_per_minute):
            if not self.status():
                sleep(1 / tries_per_minute)
            else:
                return

        raise OSError("Task server failed to start and listen on port {0}"
                      " after {1} minutes".format(self.port, minutes))

    def stop(self):
        """Stop the server by sending a SIGTERM and SIGKILL if fails to
        terminate.
        If it's already stopped OSError will be raised
        """
        if self.proc is None:
            raise OSError("Taskd server is not running")

        self.proc.send_signal(signal.SIGTERM)

        # Wait ~1 sec for taskd to finish and send a SIGKILL if still running
        kill = True
        for i in range(10):
            sleep(0.1)
            if self.proc.poll() is not None:
                kill = False
                break

        if kill:
            self.proc.kill()

        # Wait for process to end to avoid zombies
        self.proc.wait()

        # Unset the process to inform that no process is running
        self.proc = None

    def destroy(self):
        """Cleanup the data folder and release server port for other instances
        """
        # Ensure server is stopped first
        if self.proc is not None:
            self.stop()

        try:
            shutil.rmtree(self.datadir)
        except OSError as e:
            if e.errno == 2:
                # Directory no longer exists
                pass
            else:
                raise

        release_port(self.port)

        # Prevent future reuse of this instance
        self.start = self.__destroyed
        self.config = self.__destroyed
        self.stop = self.__destroyed

        # self.destroy will get called when the python session closes.
        # If self.destroy was already called, turn the action into a noop
        self.destroy = lambda: None

    def __destroyed(self, *args, **kwargs):
        raise AttributeError("Taskd instance has been destroyed. "
                             "Create a new instance if you need a new server.")

    @classmethod
    def not_available(cls):
        """Check if the taskd binary is available in the path"""
        if which(cls.DEFAULT_TASKD):
            return False
        else:
            return True

    def client_data(self, client):
        """Return a python list with the content of tx.data matching the given
        task client. tx.data will be parsed to string and JSON.
        """
        file = os.path.join(self.datadir,
                            "orgs",
                            client.credentials["org"],
                            "users",
                            client.credentials["userkey"],
                            "tx.data")

        return parse_datafile(file)

# vim: ai sts=4 et sw=4
