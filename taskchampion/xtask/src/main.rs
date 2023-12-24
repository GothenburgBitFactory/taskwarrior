//! This executable defines the `cargo xtask` subcommands.
//!
//! At the moment it is very simple, but if this grows more subcommands then
//! it will be sensible to use `clap` or another similar library.

use regex::Regex;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader, Seek, Write};
use std::path::{Path, PathBuf};

/// Tuples of the form (PATH, REGEX) where PATH and REGEX are literals where PATH is a file that
/// conains the Minimum Supported Rust Version and REGEX is the pattern to find the appropriate
/// line in the file. PATH is relative to the `taskchampion/` directory in the repo.
const MSRV_PATH_REGEX: &[(&str, &str)] = &[
    (
        "../.github/workflows/checks.yml",
        r#"toolchain: "[0-9.]+*" # MSRV"#,
    ),
    ("../.github/workflows/rust-tests.yml", r#""[0-9.]+" # MSRV"#),
    (
        "taskchampion/src/lib.rs",
        r#"Rust version [0-9.]* and higher"#,
    ),
    ("taskchampion/Cargo.toml", r#"^rust-version = "[0-9.]"#),
];

pub fn main() -> anyhow::Result<()> {
    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR")?);
    let workspace_dir = manifest_dir.parent().unwrap();
    let arguments: Vec<String> = env::args().collect();

    if arguments.len() < 2 {
        anyhow::bail!("xtask: Valid arguments are: `codegen`, `msrv <version x.y>`");
    }

    match arguments[1].as_str() {
        "codegen" => codegen(workspace_dir),
        "msrv" => msrv(arguments, workspace_dir),
        _ => anyhow::bail!("xtask: unknown xtask"),
    }
}

/// `cargo xtask codegen`
///
/// This uses ffizz-header to generate `lib/taskchampion.h`.
fn codegen(workspace_dir: &Path) -> anyhow::Result<()> {
    let lib_crate_dir = workspace_dir.join("lib");
    let mut file = File::create(lib_crate_dir.join("taskchampion.h")).unwrap();
    write!(&mut file, "{}", ::taskchampion_lib::generate_header()).unwrap();

    Ok(())
}

/// `cargo xtask msrv (X.Y)`
///
/// This checks and updates the Minimum Supported Rust Version for all files specified in MSRV_PATH_REGEX`.
/// Each line where the regex matches will have all values of the form `#.##` replaced with the given MSRV.
fn msrv(args: Vec<String>, workspace_dir: &Path) -> anyhow::Result<()> {
    // check that (X.Y) argument is (mostly) valid:
    if args.len() < 3 || !args[2].chars().all(|c| c.is_numeric() || c == '.') {
        anyhow::bail!("xtask: Invalid argument format. Xtask msrv argument takes the form \"X.Y(y)\", where XYy are numbers. eg: `cargo run xtask msrv 1.68`");
    }
    let version_replacement_string = &args[2];

    // set regex for replacing version number only within the pattern found within a line
    let re_msrv_version = Regex::new(r"([0-9]+(\.|[0-9]+|))+")?;

    // for each file in const paths tuple
    for msrv_file in MSRV_PATH_REGEX {
        let mut is_pattern_in_file = false;

        let path = workspace_dir.join(msrv_file.0);
        let path = Path::new(&path);
        if !path.exists() {
            anyhow::bail!("xtask: path does not exist {}", &path.display());
        };

        let mut file = File::options().read(true).write(true).open(path)?;
        let reader = BufReader::new(&file);

        // set search string and the replacement string for version number content
        let re_msrv_pattern = Regex::new(msrv_file.1)?;

        // for each line in file
        let mut file_string = String::new();
        for line in reader.lines() {
            let line = &line?;

            // if rust version pattern is found and is different, update it
            if let Some(pattern_offset) = re_msrv_pattern.find(line) {
                if !pattern_offset.as_str().contains(version_replacement_string) {
                    file_string += &re_msrv_version.replace(line, version_replacement_string);

                    file_string += "\n";

                    is_pattern_in_file = true;
                    continue;
                }
            }

            file_string += line;
            file_string += "\n";
        }

        // if pattern was found and updated, write to disk
        if is_pattern_in_file {
            //  Set the file length to the file_string length
            file.set_len(file_string.len() as u64)?;

            //  set the cursor to the beginning of the file and write
            file.seek(std::io::SeekFrom::Start(0))?;
            file.write_all(file_string.as_bytes())?;

            // notify user this file was updated
            println!(
                "xtask: Updated MSRV in {}",
                re_msrv_version.replace(msrv_file.0, version_replacement_string)
            );
        }
    }

    Ok(())
}
