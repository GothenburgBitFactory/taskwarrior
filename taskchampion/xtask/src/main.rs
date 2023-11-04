//! This executable defines the `cargo xtask` subcommands.
//!
//! At the moment it is very simple, but if this grows more subcommands then
//! it will be sensible to use `clap` or another similar library.

//! The const MSRV_FILE_PATHS_REGEX in /xtask/main.rs is an array of tuples
//!  of the form (PATH, REGEX) where PATH and REGEX are literals where
//!  PATH is a file that updates the Minimum Supported Rust Version
//!  and REGEX is the pattern to find the appropriate line in the file

use regex::Regex;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::io::{Seek, Write};
use std::path::{Path, PathBuf};

// Increment length of array when adding tuples of (PATH, REGEX).
const MSRV_PATH_REGEX: [(&str, &str); 2] = [
    (
        "../../.github/workflows/rust-tests.yml",
        r#"- "[0-9]+("|\.|[0-9])+"#,
    ),
    (
        "../../.github/workflows/checks.yml",
        r#"toolchain: "[0-9]+("|\.|[0-9])+"#,
    ),
];

pub fn main() -> anyhow::Result<()> {
    let arguments: Vec<String> = env::args().collect();
    if arguments.len() < 2 {
        anyhow::bail!("xtask: Valid arguments are: `codegen`, `msrv <version x.y>`");
    }

    println!("{:#?}", env::current_dir());

    match arguments[1].as_str() {
        "codegen" => codegen(),
        "msrv" => msrv(arguments),
        _ => anyhow::bail!("xtask: unknown xtask"),
    }
}

/// `cargo xtask codegen`
///
/// This uses ffizz-header to generate `lib/taskchampion.h`.
fn codegen() -> anyhow::Result<()> {
    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let workspace_dir = manifest_dir.parent().unwrap();
    let lib_crate_dir = workspace_dir.join("lib");
    let mut file = File::create(lib_crate_dir.join("taskchampion.h")).unwrap();
    write!(&mut file, "{}", ::taskchampion_lib::generate_header()).unwrap();

    Ok(())
}

/// `cargo xtask msrv (X.Y)`
///
/// This checks and updates the Minimum Supported Rust Version for all files specified in MSRV_PATH_REGEX`.
/// Each line where the regex matches will have all values of the form `#.##` replaced with the given MSRV.
fn msrv(args: Vec<String>) -> anyhow::Result<()> {
    if args.len() != 2 {
        // check that (X.Y) argument is (mostly) valid:
        if !args[2].chars().all(|c| c.is_numeric() || c == '.') {
            anyhow::bail!("xtask: Invalid argument format. Xtask MSRV argument takes the form \"X.Y(y)\", where XYy are numbers. eg: `cargo run xtask MSRV 1.68`");
        }

        // set regex for replacing version number only within the pattern found within a line
        let re_msrv_version = Regex::new(r"([0-9]+(\.|[0-9]+|))+").unwrap();

        // for each file in const paths vector
        for msrv_file in MSRV_PATH_REGEX {
            let path: &Path = Path::new(&msrv_file.0);
            let mut is_pattern_in_file = false;

            // check if path exists, if not, skip then continue
            if !path.exists() {
                println!(
                    "xtask: Warning: xtask: Path \"{}\" not found. Skipping.",
                    path.display()
                );
                continue;
            }

            let mut file: File = File::options().read(true).write(true).open(path)?;
            let reader = BufReader::new(&file);

            // set search string and the replacement string for version number content
            let re_msrv_pattern = Regex::new(msrv_file.1).unwrap();
            let version_replacement_string = &args[2];

            // for each line in file
            let mut file_string = String::new();
            for line in reader.lines() {
                let line_ref = &line.as_ref().unwrap();

                // if rust version pattern is found and is different, update it
                if let Some(pattern_offset) = re_msrv_pattern.find(line_ref) {
                    if pattern_offset.as_str() != version_replacement_string {
                        println!(
                            "{}",
                            format!(
                                "{}\n",
                                re_msrv_version
                                    .replace(line_ref, version_replacement_string)
                                    .to_string()
                                    .as_str()
                            )
                            .as_str()
                        );

                        file_string += format!(
                            "{}\n",
                            re_msrv_version
                                .replace(line_ref, version_replacement_string)
                                .to_string()
                                .as_str()
                        )
                        .as_str();

                        is_pattern_in_file = true;
                        println!(
                            "xtask: MSRV pattern found in {:#?}; updating MSRV to {}",
                            path.as_os_str(),
                            args[2]
                        );

                        continue;
                    } else {
                        println!("xtask: MSRV pattern found in {:#?} and is same as specified version in xtask argument.", path.as_os_str());
                    }
                }

                file_string += line_ref;
                file_string += "\n";
            }

            // if pattern was found and updated, write to disk
            if is_pattern_in_file {
                // Prepare to write the file to disk

                //  Set the file length to the file_string length
                let _ = file.set_len(file_string.len() as u64);

                //  set the cursor to the beginning of the file and write
                let _ = file.seek(std::io::SeekFrom::Start(0));
                let file_write_result = file.write(file_string.as_bytes());

                // if error, print error messege and exit
                if file_write_result.is_err() {
                    anyhow::bail!("xtask: unable to write file to disk: {}", &path.display());
                }
            } else {
                println!("xtask: No changes to file: \"{}\"", &path.display());
            }
        }
    } else {
        anyhow::bail!("xtask: Wrong number of arguments.");
    }

    Ok(())
}
