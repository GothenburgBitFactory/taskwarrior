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

quick_main!(run);

fn run() -> Result<()> {
    let input = stdin();
    parse(input.lock())?.iter().for_each(|t| {
        println!("{:?}", t);
    });
    Ok(())
}
