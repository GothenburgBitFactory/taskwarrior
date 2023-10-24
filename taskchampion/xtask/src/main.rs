//! This executable defines the `cargo xtask` subcommands.
//!
//! At the moment it is very simple, but if this grows more subcommands then
//! it will be sensible to use `clap` or another similar library.

//! const MSRV_FILE_PATHS in /xtask/main.rs is a list of relative file paths that the
//! Minimum Supported Rust Version should be commented into

use regex::Regex;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::io::{Seek, Write};
use std::path::{Path, PathBuf};

// Increment length of array when adding more paths.
const MSRV_FILE_PATHS: [&str; 1] =
    ["/home/ike/projects/taskwarrior.git/taskchampion/xtask/src/main.rs"];

pub fn main() -> anyhow::Result<()> {
    let arguments: Vec<String> = env::args().collect();
    if arguments.len() < 2 {
        anyhow::bail!("xtask: Valid arguments are: `codegen`, `msrv <version x.y>`");
    }

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
/// This checks and updates the Minimum Supported Rust Version for all files specified in the `MSRV_FILE_PATHS` const in `xtask/src/main.rs` where the pattern `MSRV = "X.Y"` is found in the file.
/// The argument "X.Y" will replace the text any place the MSRV is found.
fn msrv(args: Vec<String>) -> anyhow::Result<()> {
    if args.len() != 2 {
        // check that (X.Y) argument is (mostly) valid:
        if !args[2].chars().all(|c| c.is_numeric() || c == '.') {
            anyhow::bail!("xtask: Invalid argument format. Xtask MSRV argument takes the form \"X.Y(y)\", where XYy are numbers. eg: `cargo run xtask MSRV 1.68`");
        }

        // for each file in const paths vector
        for path in MSRV_FILE_PATHS {
            let path: &Path = Path::new(&path);
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

            // set the file_extension if it exists and is valid UTF-8
            let file_extension = path
                .extension()
                .unwrap_or_default()
                .to_str()
                .unwrap()
                .to_string();

            // set the comment glyph (#, //, --) pattern to search for based on file type
            let comment_glyph: String = match file_extension.as_str() {
                "rs" => "//".to_string(),
                "toml" | "yaml" | "yml" => "#".to_string(),
                _ => anyhow::bail!("xtask: Support for file extension {} is not yet implemented in `cargo xtask msrv` command.", file_extension)
            };

            // set search string and the replacement string
            let re = Regex::new(r#"(#|/{2,}) *MSRV *= *"*([0-9]+\.*)+"*.*"#).unwrap();
            let replacement_string = format!("{} MSRV = \"{}\"", comment_glyph, args[2]);

            // for each line in file
            let mut file_string = String::new();
            for line in reader.lines() {
                let line_ref = &line.as_ref().unwrap();

                // if a pre-existing MSRV pattern is found and is different, update it.
                if let Some(pattern_offset) = re.find(line_ref) {
                    if pattern_offset.as_str() != replacement_string {
                        file_string += format!("{}\n", &replacement_string).as_str();
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
