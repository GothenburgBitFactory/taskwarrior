# -*- coding: utf-8 -*-

from __future__ import division
import os
from sys import stderr
import shutil
import stat
try:
    import simplejson as json
except ImportError:
    import json

from datetime import datetime
from .utils import DEFAULT_HOOK_PATH
from .exceptions import HookError


class InvalidJSON(object):
    """Object representing the original unparsed JSON string and the JSON error
    """
    def __init__(self, original, error):
        self.original = original
        self.error = error


def json_decoder(string):
    """Attempt to decode a JSON string and in case of error return an
    InvalidJSON object
    """
    decoder = json.JSONDecoder().decode

    try:
        return decoder(string)
    except ValueError as e:
        return InvalidJSON(string, str(e))


class Hooks(object):
    """Abstraction to help interact with hooks (add, remove) during tests and
    keep track of which are active.
    """
    def __init__(self, datadir):
        """Initialize hooks container which keeps track of active hooks and

        :arg datadir: Temporary location where task is running (/tmp/...)
        """
        self.hookdir = os.path.join(datadir, "hooks")
        self._hooks = {}

        # Check if the hooks dir already exists
        if not os.path.isdir(self.hookdir):
            os.mkdir(self.hookdir)

    def __repr__(self):
        enabled = []
        disabled = []

        for hook in self:
            if hook.is_active():
                enabled.append(hook)
            else:
                disabled.append(hook)

        enabled = ", ".join(enabled) or None
        disabled = ", ".join(disabled) or None

        return "<Hooks: enabled: {0} | disabled: {1}>".format(enabled,
                                                              disabled)

    def __getitem__(self, name):
        return self._hooks[name]

    def __setitem__(self, key, value):
        self._hooks[key] = value

    def __delitem__(self, key):
        del self._hooks[key]

    def __iter__(self):
        for item in self._hooks:
            yield item

    def __len__(self):
        return len(self._hooks)

    def add(self, hookname, content, log=False):
        """Register hook with name 'hookname' and given file content.

        :arg hookname: Should be a string starting with one of:
            - on-launch
            - on-add
            - on-exit
            - on-modify

        :arg content: Content of the file as a (multi-line) string
        :arg log: If we require checking input/output of the hook
        """
        if log:
            self[hookname] = LoggedHook(hookname, self.hookdir, content)
        else:
            self[hookname] = Hook(hookname, self.hookdir, content)

        self[hookname].enable()

    def add_default(self, hookname, log=False):
        """Register a pre-built hook that exists in the folder containing hooks
        used for testing.
        If not explicitly passed hooks folder defaults to DEFAULT_HOOK_PATH

        :arg hookname: Name of the default hook
        :arg log: If we require checking input/output of the hook
        """
        if log:
            self[hookname] = LoggedHook(hookname, self.hookdir, default=True)
        else:
            self[hookname] = Hook(hookname, self.hookdir, default=True)

        # Finally enable this hook
        self[hookname].enable()

    def remove(self, hook):
        """Remove the hook matching given hookname"""
        try:
            hookname = hook.hookname
        except AttributeError:
            hookname = hook

        hook = self[hookname]

        try:
            del self[hookname]
        except KeyError:
            raise HookError("Hook {0} is not on record".format(hookname))

        hook._delete()

    def clear(self):
        """Remove all existing hooks and empty the hook registry
        """
        self._hooks = {}

        # Remove any existing hooks
        try:
            shutil.rmtree(self.hookdir)
        except OSError as e:
            # If the hookdir folder doesn't exist, no harm done and keep going
            if e.errno != 2:
                raise

        os.mkdir(self.hookdir)


class Hook(object):
    """Represents a hook script and provides methods to enable/disable hooks
    """
    def __init__(self, hookname, hookdir, content=None, default=False,
                 default_hookpath=None):
        """Initialize and create the hook

        This class supports creating hooks in two ways:
          * by specifying default=True in which case hookname will be
            searched on the hookpath and linked to the destination
          * by specifying content="some text" in which case the hook will be
            created with given content

        :arg hookname: Name of the hook e.g.: on-add.foobar
        :arg hookdir: Hooks directory under temporary task/ folder
        :arg content: What should be written to the hookfile
        :arg default: If True hookname is looked up on default_hookpath
        :arg default_hookpath: Default location where to look for preset hooks
        """
        self.hookname = hookname
        self.hookdir = hookdir
        self.hookfile = os.path.join(self.hookdir, self.hookname)

        if default_hookpath is None:
            self.default_hookpath = DEFAULT_HOOK_PATH
        else:
            self.default_hookpath = default_hookpath

        self._check_hook_type()
        self._check_hook_not_exists(self.hookfile)

        if not default and content is None:
            raise HookError("Cannot create hookfile {0} without content. "
                            "If using a builtin hook pass default=True"
                            .format(self.hookname))

        if os.path.isfile(self.hookfile):
            raise HookError("Hook with name {0} already exists. "
                            "Did you forget to remove() it before recreating?"
                            .format(self.hookname))

        if default:
            self.default_hookfile = os.path.join(self.default_hookpath,
                                                 self.hookname)
            self._check_hook_exists(self.default_hookfile)
            # Symlinks change permission of source file, cannot use one here
            shutil.copy(self.default_hookfile, self.hookfile)
        else:
            self.default_hookfile = None
            with open(self.hookfile, 'w') as fh:
                fh.write(content)

    def __eq__(self, other):
        try:
            if self.hookname == other.hookname:
                return True
        except AttributeError:
            pass

        return False

    def __ne__(self, other):
        try:
            if self.hookname != other.hookname:
                return True
        except AttributeError:
            pass

        return False

    def __hash__(self):
        return self.hookname.__hash__()

    def __repr__(self):
        return "<Hook '{0}'>".format(self.hookname)

    def __str__(self):
        return self.hookname

    def _check_hook_exists(self, hookfile):
        """Checks if the file pointed to by the current hook exists"""
        if not os.path.isfile(hookfile) and not os.path.islink(hookfile):
            raise HookError("Hook {0} doesn't exist.".format(hookfile))

    def _check_hook_not_exists(self, hookfile):
        """Checks if the file pointed to by the current hook doesn't exist"""
        try:
            self._check_hook_exists(hookfile)
        except HookError:
            return
        else:
            raise HookError("Hook {0} already exists.".format(hookfile))

    def _check_hook_type(self):
        """Check if the hookname is valid and if another hook with the same
        name was already created.
        """
        for hooktype in ("on-launch", "on-add", "on-exit", "on-modify"):
            if self.hookname.startswith(hooktype):
                break
        else:
            stderr.write("WARNING: {0} is not a valid hook type. "
                         "It will not be triggered\n".format(self.hookname))

    def _remove_file(self, file):
        try:
            os.remove(file)
        except OSError as e:
            if e.errno == 2:
                raise HookError("Hook with name {0} was not found on "
                                "hooks/ folder".format(file))
            else:
                raise

    def _delete(self):
        """Remove the hook from disk

        Don't call this method directly. Use Hooks.remove(hook) instead
        """
        self._remove_hookfile(self.hookfile)

    def enable(self):
        """Make hookfile executable to allow triggering
        """
        os.chmod(self.hookfile, stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)

    def disable(self):
        """Remove hookfile executable bit to deny triggering
        """
        os.chmod(self.hookfile, stat.S_IREAD | stat.S_IWRITE)

    def is_active(self):
        """Check if hook is active by verifying the execute bit
        """
        return os.access(self.hookfile, os.X_OK)


class LoggedHook(Hook):
    """A variant of a Hook that allows checking that the hook was called, what
    was received via STDIN and what was answered to STDOUT
    """
    def __init__(self, *args, **kwargs):
        super(LoggedHook, self).__init__(*args, **kwargs)

        # The wrapper will replace the hookfile
        # The original file will be 'wrappedfile'

        # NOTE If the prefix "original_" is changed here, update wrapper.sh
        self.wrappedname = "original_" + self.hookname
        self.wrappedfile = os.path.join(self.hookdir, self.wrappedname)

        self.original_wrapper = os.path.join(self.default_hookpath,
                                             "wrapper.sh")

        self.hooklog_in = self.wrappedfile + ".log.in"
        self.hooklog_out = self.wrappedfile + ".log.out"

        # Cache is used to avoid parsing the logfiles everytime it's needed
        self._cache = {}

        # Setup wrapper pointing to the correct hook name
        self._setup_wrapper()

    def __repr__(self):
        return "<LoggedHook '{0}'>".format(self.hookname)

    def _delete(self):
        """Remove the hook from disk

        Don't call this method directly. Use Task.hooks.remove(hook) instead
        """
        super(LoggedHook, self)._delete()
        self._remove_file(self.wrappedfile)
        self._remove_file(self.hooklog_in)
        self._remove_file(self.hooklog_out)

    def _setup_wrapper(self):
        """Setup wrapper shell script to allow capturing input/output of hook
        """
        # Create empty hooklog to allow checking that hook executed
        open(self.hooklog_in, 'w').close()
        open(self.hooklog_out, 'w').close()

        # Rename the original hook to the name that will be used by wrapper
        self._check_hook_not_exists(self.wrappedfile)
        os.rename(self.hookfile, self.wrappedfile)

        # Symlinks change permission of source file, cannot use one here
        shutil.copy(self.original_wrapper, self.hookfile)

    def _get_log_stat(self):
        """Return the most recent change timestamp and size of both logfiles
        """
        stdin = os.stat(self.hooklog_in)
        stdout = os.stat(self.hooklog_out)

        last_change = max((stdin.st_mtime, stdout.st_mtime))
        return last_change, stdin.st_size, stdout.st_size

    def _use_cache(self):
        """Check if log files were changed since last check
        """
        try:
            last_change = self._cache["last_change"]
        except KeyError:
            # No cache available
            return False
        else:
            change = self._get_log_stat()

            if last_change != change:
                # Cache is outdated
                return False
            else:
                # Cache is up to date
                return True

    def enable(self):
        """Make hookfile executable to allow triggering
        """
        super(LoggedHook, self).enable()
        os.chmod(self.wrappedfile, stat.S_IREAD | stat.S_IWRITE | stat.S_IEXEC)

    def disable(self):
        """Remove hookfile executable bit to deny triggering
        """
        super(LoggedHook, self).disable()
        os.chmod(self.wrappedfile, stat.S_IREAD | stat.S_IWRITE)

    def is_active(self):
        """Check if hook is active by verifying the execute bit
        """
        parent_is_active = super(LoggedHook, self).disable()
        return parent_is_active and os.access(self.wrappedfile, os.X_OK)

    def get_logs(self):
        """Parse the logs generated by the hook and return a dictionary
        containing the logs collected with the wrapper in a python friendly
        format:
        * JSON is parsed as python dictionaries
        * timestamps are parsed as datetime objects

        It should look something like this:

        ## STDIN file
        % Called at 1414874711 with 'arg1 arg2 ...'
        {... JSON received by the hook ... }
        {... more JSON ...}

        ## STDOUT file
        {... JSON emitted by the hook ... }
        Logged messages
        {... more JSON ...}
        ! Exit code: 1
        """
        if self._use_cache():
            return self._cache["log"]

        log = {"calls": [],
               "input": {
                   "json": [],
                   },
               "output": {
                   "json": [],
                   "msgs": [],
                   },
               "exitcode": None,
               }

        with open(self.hooklog_in) as fh:
            for i, line in enumerate(fh):
                line = line.rstrip("\n")
                if line.startswith("%"):
                    tstamp, args = line.split(" with ")
                    # Timestamp includes nanosecond resolution
                    timestamp = tstamp.split(" ")[-1]
                    # convert timestamp to python datetime object
                    log["calls"].append({
                        "timestamp": datetime.fromtimestamp(float(timestamp)),
                        "args": args,
                    })
                elif line.startswith("{"):
                    # Decode json input (to hook)
                    log["input"]["json"].append(json_decoder(line))
                else:
                    raise IOError("Unexpected content on STDIN line {0}: {1}"
                                  .format(i, line))

        with open(self.hooklog_out) as fh:
            for line in fh:
                line = line.rstrip("\n")
                if line.startswith("!"):
                    exitcode = int(line.split(" ")[-1])
                    log["exitcode"] = exitcode
                elif line.startswith("{"):
                    # Decode json output (from hook)
                    log["output"]["json"].append(json_decoder(line))
                else:
                    log["output"]["msgs"].append(line)

        # NOTE convert all lists to tuples to prevent tampering?

        self._cache["log"] = log

        # Update last modification timestamp in cache
        self._cache["last_change"] = self._get_log_stat()

        return self._cache["log"]

    def assertTriggeredCount(self, count):
        """Check if current hook file was triggered/used by taskwarrior and
        how many times.
        """
        log = self.get_logs()

        assert len(log["calls"]) == count, ("{0} calls expected for {1} but "
                                            "found {2}".format(
                                                count,
                                                self.hookname,
                                                log["calls"]
                                            ))

    def assertExitcode(self, exitcode):
        """Check if current hook finished with the expected exit code
        """
        log = self.get_logs()

        assert log["exitcode"] == exitcode, ("Expected exit code {0} for {1} "
                                             "but found {2}".format(
                                                 exitcode,
                                                 self.hookname,
                                                 log["exitcode"]
                                             ))

    def assertValidJSONOutput(self):
        """Check if current hook output is valid JSON in all expected replies
        """
        log = self.get_logs()

        for i, out in enumerate(log["output"]["json"]):
            assert not isinstance(out, InvalidJSON), ("Invalid JSON found at "
                                                      "reply number {0} with "
                                                      "content {1}".format(
                                                          i + 1,
                                                          out.original
                                                      ))

    def assertInvalidJSONOutput(self):
        """Check if current hook output is invalid JSON in any expected reply
        """
        log = self.get_logs()

        for i, out in enumerate(log["output"]["json"]):
            assert isinstance(out, InvalidJSON), ("Valid JSON found at reply "
                                                  "number {0} with content "
                                                  "{1}".format(
                                                      i + 1,
                                                      out.original
                                                  ))

# vim: ai sts=4 et sw=4
