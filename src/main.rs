#![recursion_limit = "1024"]

extern crate chrono;
extern crate uuid;
#[macro_use]
extern crate error_chain;

mod tdb2;
mod task;
mod errors;

use tdb2::parse;
use std::io::stdin;

use errors::*;

fn main() {
    if let Err(ref e) = run() {
        use std::io::Write;
        let stderr = &mut ::std::io::stderr();
        let errmsg = "Error writing to stderr";

        writeln!(stderr, "error: {}", e).expect(errmsg);

        for e in e.iter().skip(1) {
            writeln!(stderr, "caused by: {}", e).expect(errmsg);
        }

        // The backtrace is not always generated. Try to run this example
        // with `RUST_BACKTRACE=1`.
        if let Some(backtrace) = e.backtrace() {
            writeln!(stderr, "backtrace: {:?}", backtrace).expect(errmsg);
        }

        ::std::process::exit(1);
    }
}

fn run() -> Result<()> {
    let input = stdin();
    parse("<stdin>".to_string(), input.lock())?
        .iter()
        .for_each(|t| {
            println!("{:?}", t);
        });
    Ok(())
}
