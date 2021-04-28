# Compatibility & deprecation

Until TaskChampion reaches [v1.0.0](https://github.com/taskchampion/taskchampion/milestone/7), nothing is set in stone. That being said, we aim for the following:

1. Major versions represent significant change and may be incompatible with previous major release.
2. Minor versions are always backwards compatible and might add some new functionality.
3. Patch versions should not introduce any new functionality and do what name implies &mdash; fix bugs.

As there are no major releases yet, we do not support any older versions. Users are encouraged to use the latest release.

## ABI policy

1. We target stable `rustc`.
2. TaskChampion will never upgrade any storage to a non-compatible version without explicit user's request.

## API policy

1. Deprecated features return a warning at least 1 minor version prior to being removed.

    Example:

    > If support of `--bar` is to be dropped in v2.0.0, we shall announce it in v1.9.0 at latest.

2. We aim to issue a notice of newly added functionality when appropriate.

    Example:

    > "NOTICE: Since v1.1.0 you can use `--foo` in conjunction with `--bar`. Foobar!"

3. TaskChampion always uses UTF-8.

## Command-line interface

Considered to be part of the API policy.

## CLI exit codes

- `0` No errors, normal exit.
- `1` Generic error.
- `2` Never used to avoid conflicts with Bash.
- `3` Unable to execute with the given parameters.
- `4` I/O error.
- `5` Database error.

# Security

To report a vulnerability, please contact [dustin@cs.uchicago.edu](dustin@cs.uchicago.edu), you may use GPG public-key `D8097934A92E4B4210368102FF8B7AC6154E3226` which is available [here](https://keybase.io/djmitche/pgp_keys.asc?fingerprint=d8097934a92e4b4210368102ff8b7ac6154e3226). Initial response is expected within ~48h.

We kinldy ask to follow the responsible disclosure model and refrain from sharing information until:
1. Vulnerabilities are patched in TaskChampion + 60 days to coordinate with distributions.
2. 90 days since the vulnerability is disclosed to us.

We recognise the legitimacy of public interest and accept that security researchers can publish information after 90-days deadline unilaterally.

We will assist with obtaining CVE and acknowledge the vulnerabilites reported.
