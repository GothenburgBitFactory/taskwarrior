use assert_cmd::prelude::*;
use predicates::prelude::*;
use std::process::Command;

// This tests that the task binary is running and parsing arguments.  The details of subcommands
// are handled with unit tests.

#[test]
fn help() -> Result<(), Box<dyn std::error::Error>> {
    let mut cmd = Command::cargo_bin("task")?;

    cmd.arg("--help");
    cmd.assert()
        .success()
        .stdout(predicate::str::contains("Personal task-tracking"));

    Ok(())
}

#[test]
fn version() -> Result<(), Box<dyn std::error::Error>> {
    let mut cmd = Command::cargo_bin("task")?;

    cmd.arg("--version");
    cmd.assert()
        .success()
        .stdout(predicate::str::contains("TaskChampion"));

    Ok(())
}

#[test]
fn invalid_option() -> Result<(), Box<dyn std::error::Error>> {
    let mut cmd = Command::cargo_bin("task")?;

    cmd.arg("--no-such-option");
    cmd.assert()
        .failure()
        .stderr(predicate::str::contains("USAGE"));

    Ok(())
}
