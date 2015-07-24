# -*- coding: utf-8 -*-
from __future__ import division
import os
import sys
import socket
import signal
import functools
import atexit
import tempfile
from subprocess import Popen, PIPE, STDOUT
from threading import Thread
try:
    from Queue import Queue, Empty
except ImportError:
    from queue import Queue, Empty
from time import sleep
try:
    import simplejson as json
except ImportError:
    import json
from .exceptions import CommandError, TimeoutWaitingFor

USED_PORTS = set()
ON_POSIX = 'posix' in sys.builtin_module_names

# Directory relative to basetest module location
CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))

# Location of binary files (usually the src/ folder)
BIN_PREFIX = os.path.abspath(
    os.path.join(CURRENT_DIR, "..", "..", "src")
)

# Default location of test certificates
DEFAULT_CERT_PATH = os.path.abspath(
    os.path.join(CURRENT_DIR, "..", "test_certs")
)

# Default location of test hooks
DEFAULT_HOOK_PATH = os.path.abspath(
    os.path.join(CURRENT_DIR, "..", "test_hooks")
)


# Environment flags to control skipping of task and taskd tests
TASKW_SKIP = os.environ.get("TASKW_SKIP", False)
TASKD_SKIP = os.environ.get("TASKD_SKIP", False)
# Environment flags to control use of PATH or in-tree binaries
TASK_USE_PATH = os.environ.get("TASK_USE_PATH", False)
TASKD_USE_PATH = os.environ.get("TASKD_USE_PATH", False)

UUID_REGEXP = ("[0-9A-Fa-f]{8}-" + ("[0-9A-Fa-f]{4}-" * 3) + "[0-9A-Fa-f]{12}")


def task_binary_location(cmd="task"):
    """If TASK_USE_PATH is set rely on PATH to look for task binaries.
    Otherwise ../src/ is used by default.
    """
    return binary_location(cmd, TASK_USE_PATH)


def taskd_binary_location(cmd="taskd"):
    """If TASKD_USE_PATH is set rely on PATH to look for taskd binaries.
    Otherwise ../src/ is used by default.
    """
    return binary_location(cmd, TASKD_USE_PATH)


def binary_location(cmd, USE_PATH=False):
    """If USE_PATH is True rely on PATH to look for taskd binaries.
    Otherwise ../src/ is used by default.
    """
    if USE_PATH:
        return cmd
    else:
        return os.path.join(BIN_PREFIX, cmd)


def wait_condition(cond, timeout=1, sleeptime=.01):
    """Wait for condition to return anything other than None
    """
    # NOTE Increasing sleeptime can dramatically increase testsuite runtime
    # It also reduces CPU load significantly
    if timeout is None:
        timeout = 1

    if timeout < sleeptime:
        print("Warning, timeout cannot be smaller than", sleeptime)
        timeout = sleeptime

    # Max number of attempts until giving up
    tries = int(timeout / sleeptime)

    for i in range(tries):
        val = cond()

        if val is not None:
            break

        sleep(sleeptime)

    return val


def wait_process(pid, timeout=None):
    """Wait for process to finish
    """
    def process():
        try:
            os.kill(pid, 0)
        except OSError:
            # Process is dead
            return True
        else:
            # Process is still ticking
            return None

    return wait_condition(process, timeout)


def _queue_output(arguments, pidq, outputq):
    """Read/Write output/input of given process.
    This function is meant to be executed in a thread as it may block
    """
    kwargs = arguments["process"]
    input = arguments["input"]

    try:
        proc = Popen(**kwargs)
    except OSError as e:
        # pid None is read by the main thread as a crash of the process
        pidq.put(None)

        outputq.put((
            "",
            ("Unexpected exception caught during execution of taskw: '{0}' . "
             "If you are running out-of-tree tests set TASK_USE_PATH=1 or "
             "TASKD_USE_PATH=1 in shell env before execution and add the "
             "location of the task(d) binary to the PATH".format(e)),
            255))  # false exitcode

        return

    # Put the PID in the queue for main process to know.
    pidq.put(proc.pid)

    # Send input and wait for finish
    out, err = proc.communicate(input)

    if sys.version_info > (3,):
        out, err = out.decode('utf-8'), err.decode('utf-8')

    # Give the output back to the caller
    outputq.put((out, err, proc.returncode))


def _retrieve_output(thread, timeout, queue, thread_error):
    """Fetch output from taskw subprocess queues
    """
    # Try to join the thread on failure abort
    thread.join(timeout)
    if thread.isAlive():
        # Join should have killed the thread. This is unexpected
        raise TimeoutWaitingFor(thread_error + ". Unexpected error")

    # Thread died so we should have output
    try:
        # data = (stdout, stderr, exitcode)
        data = queue.get(timeout=timeout)
    except Empty:
        data = TimeoutWaitingFor("streams from TaskWarrior")

    return data


def _get_output(arguments, timeout=None):
    """Collect output from the subprocess without blocking the main process if
    subprocess hangs.
    """
    # NOTE Increase this value if tests fail with None being received as
    # stdout/stderr instead of the expected content
    output_timeout = 0.1  # seconds

    pidq = Queue()
    outputq = Queue()

    t = Thread(target=_queue_output, args=(arguments, pidq, outputq))
    t.daemon = True
    t.start()

    try:
        pid = pidq.get(timeout=timeout)
    except Empty:
        pid = None

    # Process crashed or timed out for some reason
    if pid is None:
        return _retrieve_output(t, output_timeout, outputq,
                                "TaskWarrior to start")

    # Wait for process to finish (normal execution)
    state = wait_process(pid, timeout)

    if state:
        # Process finished
        return _retrieve_output(t, output_timeout, outputq,
                                "TaskWarrior thread to join")

    # If we reach this point we assume the process got stuck or timed out
    for sig in (signal.SIGABRT, signal.SIGTERM, signal.SIGKILL):
        # Start with lower signals and escalate if process ignores them
        try:
            os.kill(pid, signal.SIGABRT)
        except OSError as e:
            # 3 means the process finished/died between last check and now
            if e.errno != 3:
                raise

        # Wait for process to finish (should die/exit after signal)
        state = wait_process(pid, timeout)

        if state:
            # Process finished
            return _retrieve_output(t, output_timeout, outputq,
                                    "TaskWarrior to die")

    # This should never happen but in case something goes really bad
    raise OSError("TaskWarrior stopped responding and couldn't be killed")


def run_cmd_wait(cmd, input=None, stdout=PIPE, stderr=PIPE,
                 merge_streams=False, env=os.environ, timeout=None):
    "Run a subprocess and wait for it to finish"

    if input is None:
        stdin = None
    else:
        stdin = PIPE

    if merge_streams:
        stderr = STDOUT
    else:
        stderr = PIPE

    arguments = {
        "process": {
            "args": cmd,
            "stdin": stdin,
            "stdout": stdout,
            "stderr": stderr,
            "bufsize": 1,
            "close_fds": ON_POSIX,
            "env": env,
        },
        "input": input,
    }
    out, err, exit = _get_output(arguments, timeout)

    if merge_streams:
        if exit != 0:
            raise CommandError(cmd, exit, out)
        else:
            return exit, out
    else:
        if exit != 0:
            raise CommandError(cmd, exit, out, err)
        else:
            return exit, out, err


def run_cmd_wait_nofail(*args, **kwargs):
    "Same as run_cmd_wait but silence the exception if it happens"
    try:
        return run_cmd_wait(*args, **kwargs)
    except CommandError as e:
        return e.code, e.out, e.err


def get_IPs(hostname):
    output = {}
    addrs = socket.getaddrinfo(hostname, 0, 0, 0, socket.IPPROTO_TCP)

    for family, socktype, proto, canonname, sockaddr in addrs:
        addr = sockaddr[0]
        output[family] = addr

    return output


def port_used(addr="localhost", port=None):
    "Return True if port is in use, False otherwise"
    if port is None:
        raise TypeError("Argument 'port' may not be None")

    # If we got an address name, resolve it both to IPv6 and IPv4.
    IPs = get_IPs(addr)

    # Taskd seems to prefer IPv6 so we do it first
    for family in (socket.AF_INET6, socket.AF_INET):
        try:
            addr = IPs[family]
        except KeyError:
            continue

        s = socket.socket(family, socket.SOCK_STREAM)
        result = s.connect_ex((addr, port))
        s.close()
        if result == 0:
            # connection was successful
            return True
    else:
        return False


def find_unused_port(addr="localhost", start=53589, track=True):
    """Find an unused port starting at `start` port

    If track=False the returned port will not be marked as in-use and the code
    will rely entirely on the ability to connect to addr:port as detection
    mechanism. Note this may cause problems if ports are assigned but not used
    immediately
    """
    maxport = 65535
    unused = None

    for port in xrange(start, maxport):
        if not port_used(addr, port):
            if track and port in USED_PORTS:
                continue

            unused = port
            break

    if unused is None:
        raise ValueError("No available port in the range {0}-{1}".format(
            start, maxport))

    if track:
        USED_PORTS.add(unused)

    return unused


def release_port(port):
    """Forget that given port was marked as'in-use
    """
    try:
        USED_PORTS.remove(port)
    except KeyError:
        pass


def memoize(obj):
    """Keep an in-memory cache of function results given it's inputs
    """
    cache = obj.cache = {}

    @functools.wraps(obj)
    def memoizer(*args, **kwargs):
        key = str(args) + str(kwargs)
        if key not in cache:
            cache[key] = obj(*args, **kwargs)
        return cache[key]
    return memoizer


try:
    from shutil import which
    which = memoize(which)
except ImportError:
    # NOTE: This is shutil.which backported from python-3.3.3
    @memoize
    def which(cmd, mode=os.F_OK | os.X_OK, path=None):
        """Given a command, mode, and a PATH string, return the path which
        conforms to the given mode on the PATH, or None if there is no such
        file.

        `mode` defaults to os.F_OK | os.X_OK. `path` defaults to the result
        of os.environ.get("PATH"), or can be overridden with a custom search
        path.

        """
        # Check that a given file can be accessed with the correct mode.
        # Additionally check that `file` is not a directory, as on Windows
        # directories pass the os.access check.
        def _access_check(fn, mode):
            return (os.path.exists(fn) and os.access(fn, mode) and
                    not os.path.isdir(fn))

        # If we're given a path with a directory part, look it up directly
        # rather than referring to PATH directories. This includes checking
        # relative to the current directory, e.g. ./script
        if os.path.dirname(cmd):
            if _access_check(cmd, mode):
                return cmd
            return None

        if path is None:
            path = os.environ.get("PATH", os.defpath)
        if not path:
            return None
        path = path.split(os.pathsep)

        if sys.platform == "win32":
            # The current directory takes precedence on Windows.
            if os.curdir not in path:
                path.insert(0, os.curdir)

            # PATHEXT is necessary to check on Windows.
            pathext = os.environ.get("PATHEXT", "").split(os.pathsep)
            # See if the given file matches any of the expected path
            # extensions. This will allow us to short circuit when given
            # "python.exe". If it does match, only test that one, otherwise we
            # have to try others.
            if any(cmd.lower().endswith(ext.lower()) for ext in pathext):
                files = [cmd]
            else:
                files = [cmd + ext for ext in pathext]
        else:
            # On other platforms you don't have things like PATHEXT to tell you
            # what file suffixes are executable, so just pass on cmd as-is.
            files = [cmd]

        seen = set()
        for dir in path:
            normdir = os.path.normcase(dir)
            if normdir not in seen:
                seen.add(normdir)
                for thefile in files:
                    name = os.path.join(dir, thefile)
                    if _access_check(name, mode):
                        return name
        return None


def parse_datafile(file):
    """Parse .data files on the client and server treating files as JSON
    """
    data = []
    with open(file) as fh:
        for line in fh:
            line = line.rstrip("\n")

            # Turn [] strings into {} to be treated properly as JSON hashes
            if line.startswith('[') and line.endswith(']'):
                line = '{' + line[1:-1] + '}'

            if line.startswith("{"):
                data.append(json.loads(line))
            else:
                data.append(line)
    return data


def mkstemp(data):
    """
    Create a temporary file that is removed at process exit
    """
    def rmtemp(name):
        try:
            os.remove(name)
        except OSError:
            pass

    f = tempfile.NamedTemporaryFile(delete=False)
    f.write(data)
    f.close()

    # Ensure removal at end of python session
    atexit.register(rmtemp, f.name)

    return f.name


def mkstemp_exec(data):
    """Create a temporary executable file that is removed at process exit
    """
    name = mkstemp(data)
    os.chmod(name, 0o755)

    return name

# vim: ai sts=4 et sw=4
