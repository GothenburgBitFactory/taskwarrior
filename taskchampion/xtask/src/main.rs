//! This executable defines the `cargo xtask` subcommands.
//!
//! At the moment it is very simple, but if this grows more subcommands then
//! it will be sensible to use `clap` or another similar library.

//! const MSRV_FILE_PATHS in /xtask/main.rs is a list of relative file paths that the
//! Minimum Supported Rust Version should be commented into

use std::env;
use std::fs::File;
use std::io::Write;
use std::io::{BufRead, BufReader};
use std::path::{Path, PathBuf};

//#[cfg(target_os = "unix")]
//use std::os::unix::prelude::FileExt;

//#[cfg(target_os = "windows")]
//use std::os::windows::fs::FileExt;

// Increment length of array when adding more paths.
const MSRV_FILE_PATHS: [&str; 2] = [r"main.rs", r"src/main.rs"];

pub fn main() -> anyhow::Result<()> {
    let arguments: Vec<String> = env::args().collect();
    if arguments.len() < 2 {
        anyhow::bail!("xtask: Valid arguments are: `codegen`, `msrv <version x.y>`");
    }

    match arguments[1].as_str() {
        "codegen" => codegen(),
        "msrv" => msrv(arguments),
        arg => anyhow::bail!("xtask: unknown xtask: {}", arg),
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
/// This updates all of the places in the repo where the MSRV occurs to <arg2> for the list of file paths defined as const in Xtask main.rs.
/// If no pre-existing Xtask-formatted MSRV is found, inserts the MSRV as first comment block after the last `//!` library-doc comments.
fn msrv(args: Vec<String>) -> anyhow::Result<()> {
    match args.len(){
        0 ..= 2 => anyhow::bail!("Error: xtask: Command `caro run xtask msrv` expects at least one argument, none detected."),
        3 => {

                // check that (X.Y) argument is (mostly) valid:
                for xy in & mut args[2].chars() {
                    if !xy.is_numeric() && xy != '.'{
                        anyhow::bail!("xtask: Invalid argument format. Xtask MSRV argument takes the form \"X.Y(y)\", where XYy are numbers. eg: `cargo run xtask MSRV 1.68`");
                    }
                }

                // for each file in const paths vector
                for path in MSRV_FILE_PATHS {
                    let path : & Path = Path::new(& path);

                    // check if path exists, if not, skip then continue
                    if ! path.exists() {
                        println!("xtask: Warning: xtask: Path `{}` not found. Skipping.", path.display());
                        continue;
                    }

                    let mut file : File = File::options().read(true).write(true).open(path).unwrap();
                    let reader = BufReader::new(&file);

                    // set pattern to search for and the replacement string
                    let msrv_pattern = "///  MSRV = \"";
                    let replacement_string = format!("///  MSRV = \"{}\"\n", args[2]);

                    // for each line in file
                    let mut file_string = String::new();
                    let mut msrv_pattern_found = false;
                    let mut comment_pattern_last_offset : usize = 0;
                    for line in reader.lines() {
                        let pattern_offset = & line.as_ref().unwrap().find(msrv_pattern);

                        // if a pre-existing MSRV pattern is found, update it.
                        if pattern_offset.is_some() {
                            msrv_pattern_found = true;
                            file_string += & replacement_string;
                            println!("xtask: MSRV pattern found in {:#?}; updating MSRV to {}", path.as_os_str(), args[2]);
                            continue;
                        }

                        file_string += & line.as_ref().unwrap();
                        file_string += "\n";

                        // if the line starts with a library-doc comments (which must precede regular comments), update index
                        // used iff MSRV isn't found
                        if line.as_ref().unwrap().starts_with("//!") {
                            comment_pattern_last_offset = file_string.len();
                        }

                    }

                    // insert MSRV block if no pre-existing block is found
                    if !msrv_pattern_found {
                        println!("xtask: MSRV pattern not found in {:#?}; inserting MSRV: {}", & path.display(), & replacement_string);
                        let mut insert_string = format!(concat!(
                            "/// Xtask Auto-Generated Comments:\n",
                            "///  Minimum Supported Rust Version for this crate:\n",
                            "{}"), & replacement_string);

                        // if inserting at beginning of file, no need for new line, else need new line before inserting
                        if comment_pattern_last_offset > 0 {
                            insert_string = format!("\n{}",& insert_string);
                        }

                        // if there is not a new line after a comment block already, then insert one with the new block comment
                        if ! file_string.chars().nth(comment_pattern_last_offset).unwrap().is_ascii_whitespace() {
                            insert_string = format!("{}\n",& insert_string);
                        }

                        file_string.insert_str(comment_pattern_last_offset, & insert_string);
                    }

                    // write the updated file to disk
                    //let file_write_result = file.write_at(file_string.as_bytes(),0);
                    let file_write_result = file.write(file_string.as_bytes()); // appends file instead of replacing contents. 

                    // if error, print error messege and exit
                    if file_write_result.is_err() {
                        anyhow::bail!("xtask: unable to write file to disk: {}", & path.display());
                    }
                }

        },
        _ => anyhow::bail!("xtask: Valid arguments are `codegen`, `msrv <version x.y>`")
    }

    Ok(())
}
