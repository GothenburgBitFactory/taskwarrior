//! This executable defines the `cargo xtask` subcommands.
//!
//! At the moment it is very simple, but if this grows more subcommands then
//! it will be sensible to use `clap` or another similar library.

//! const MSRV_FILE_PATHS in /xtask/main.rs is a list of relative file paths that the
//! Minimum Supported Rust Version should be commented into

use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::io::{Seek, Write};
use std::path::{Path, PathBuf};
use regex::Regex;

// Increment length of array when adding more paths.
const MSRV_FILE_PATHS: [&str; 2] = [r"main.rs", r"src/main.rs"];
// TODO: Per @djmitche : Cargo sets $CARGO_MANIFEST_DIR to the taskchampion/xtask directory,
//  so you might be able to build paths relative to there, and then use that environment variable to make xtask msrv
//  independent of the current directory.

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

            // check if path exists, if not, skip then continue
            if !path.exists() {
                println!(
                    "xtask: Warning: xtask: Path `{}` not found. Skipping.",
                    path.display()
                );
                continue;
            }

            let mut file: File = File::options().read(true).write(true).open(path).unwrap();
            let reader = BufReader::new(&file);
            
            // set specify the comment glyph (#, //, --) pattern to search for based on file type
            let file_extension = path.extension().unwrap().to_str().unwrap();
            let mut comment_glyph = String::new();
            match file_extension {
                "rs" => comment_glyph = "//".to_string(),
                "toml" | "yaml" => comment_glyph = "#".to_string(),
                _ => anyhow::bail!("xtask: Support for file extension {} is not yet implemented in `cargo xtask msrv` command.", file_extension)
            }

            // set search string and the replacement string
            // set regex
            let re = Regex::new(r#"(#|/{2,}) *MSRV *= *"*([0-9]+\.*)+"*.*"#).unwrap();
            let replacement_string = format!("{} MSRV = \"{}\"\n", comment_glyph, args[2]);

            // for each line in file
            let mut file_string = String::new();
            for line in reader.lines() {
                let line_ref = &line.as_ref().unwrap();
                let pattern_offset = re.find(line_ref);

                // if a pre-existing MSRV pattern is found, update it.
                if pattern_offset.is_some() {
                    file_string += &replacement_string;
                    println!(
                        "xtask: MSRV pattern found in {:#?}; updating MSRV to {}",
                        path.as_os_str(),
                        args[2]
                    );
                    continue;
                }

                file_string += line_ref;
                file_string += "\n";
            }

            // write the updated file to disk
            let _ = file.seek(std::io::SeekFrom::Start(0));
            let file_write_result = file.write(file_string.as_bytes());

            // TODO: solve for case where updating the MSRV to a different length version number (eg `1.0` -> `1.1.1` or vice versa)
            //  results in a longer or shorter file. Can be problematic for the latter case resulting in the inclusion of an
            //  extra closing bracket.

            // if error, print error messege and exit
            if file_write_result.is_err() {
                anyhow::bail!("xtask: unable to write file to disk: {}", &path.display());
            }
        }
    } else {
        anyhow::bail!("xtask: Wrong number of arguments.");
    }

    Ok(())
}