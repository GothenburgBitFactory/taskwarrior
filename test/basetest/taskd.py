# -*- coding: utf-8 -*-

import os
import tempfile
import shutil
import signal
from time import sleep
from subprocess import Popen
from .utils import find_unused_port, release_port, port_used, run_cmd_wait

try:
    from subprocess import DEVNULL
except ImportError:
    DEVNULL = open(os.devnull, 'w')

# Location relative to current script location
_curdir = os.path.dirname(os.path.abspath(__file__))
DEFAULT_CERT_PATH = os.path.abspath(os.path.join(_curdir, "..", "test_certs"))


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
    def __init__(self, taskd="taskd", certpath=None, address="127.0.0.1"):
        """Initialize a Task server that runs in the background and stores data
        in a temporary folder

        :arg taskd: Taskd binary to launch the server (defaults: taskd in PATH)
        :arg certpath: Folder where to find all certificates needed for taskd
        :arg address: Address to bind to
        """
        self.taskd = taskd
        # Will hold the taskd subprocess if it's running
        self.proc = None
        self.datadir = tempfile.mkdtemp()
        self.tasklog = os.path.join(self.datadir, "taskd.log")
        self.taskpid = os.path.join(self.datadir, "taskd.pid")

        # Make sure no TASKDDATA is defined
        try:
            del os.environ["TASKDDATA"]
        except KeyError:
            pass

        if certpath is None:
            certpath = DEFAULT_CERT_PATH
        self.certpath = certpath

        self.address = address
        self.port = find_unused_port()

        # Keep all certificate paths public for access by TaskClients
        self.client_cert = os.path.join(self.certpath, "client.cert.pem")
        self.client_key = os.path.join(self.certpath, "client.key.pem")
        self.server_cert = os.path.join(self.certpath, "server.cert.pem")
        self.server_key = os.path.join(self.certpath, "server.key.pem")
        self.server_crl = os.path.join(self.certpath, "server.crl.pem")
        self.ca_cert = os.path.join(self.certpath, "ca.cert.pem")

        # Initialize taskd
        cmd = (self.taskd, "init", "--data", self.datadir)
        run_cmd_wait(cmd)

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

    def config(self, var, value):
        """Run setup `var` as `value` in taskd config
        """
        cmd = (self.taskd, "config", "--force", "--data", self.datadir, var,
               value)
        run_cmd_wait(cmd)

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

        if not port_used(port=self.port):
            return False

        return True

    def start(self):
        """Start the taskd server if it's not running.
        If it's already running OSError will be raised
        """
        if self.proc is None:
            cmd = (self.taskd, "server", "--data", self.datadir)
            self.proc = Popen(cmd, stdout=DEVNULL, stdin=DEVNULL)
        else:
            raise OSError("Taskd server is still running or crashed")

        # Wait for server to listen by checking connectivity in the port
        # Wait up to 5 minutes checking once second
        minutes = 5
        for i in range(minutes * 60):
            if not self.status():
                sleep(1)
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
        self.destroy = self.__destroyed

    def __destroyed(self, *args, **kwargs):
        raise AttributeError("Taskd instance has been destroyed. "
            "Create a new instance if you need a new server.")

# vim: ai sts=4 et sw=4
