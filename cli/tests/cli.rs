use assert_cmd::prelude::*;
use predicates::prelude::*;
use std::fs;
use std::process::Command;
use tempfile::TempDir;

// NOTE: This tests that the `ta` binary is running and parsing arguments.  The details of
// subcommands are handled with unit tests.

/// These tests force config to be read via TASKCHAMPION_CONFIG so that a user's own config file
/// (in their homedir) does not interfere with tests.
fn test_cmd(dir: &TempDir) -> Result<Command, Box<dyn std::error::Error>> {
    let config_filename = dir.path().join("config.toml");
    fs::write(
        config_filename.clone(),
        format!("data_dir = {:?}", dir.path()),
    )?;

    let config_filename = config_filename.to_str().unwrap();
    let mut cmd = Command::cargo_bin("ta")?;
    cmd.env("TASKCHAMPION_CONFIG", config_filename);
    Ok(cmd)
}

#[test]
fn help() -> Result<(), Box<dyn std::error::Error>> {
    let dir = TempDir::new().unwrap();
    let mut cmd = test_cmd(&dir)?;

    cmd.arg("--help");
    cmd.assert()
        .success()
        .stdout(predicate::str::contains("Personal task-tracking"));

    Ok(())
}

#[test]
fn version() -> Result<(), Box<dyn std::error::Error>> {
    let dir = TempDir::new().unwrap();
    let mut cmd = test_cmd(&dir)?;

    cmd.arg("--version");
    cmd.assert()
        .success()
        .stdout(predicate::str::contains("TaskChampion"));

    Ok(())
}

#[test]
fn invalid_option() -> Result<(), Box<dyn std::error::Error>> {
    let dir = TempDir::new().unwrap();
    let mut cmd = test_cmd(&dir)?;

    cmd.arg("--no-such-option");
    cmd.assert()
        .failure()
        .stderr(predicate::str::contains("command line not recognized"))
        .code(predicate::eq(3));

    Ok(())
}
