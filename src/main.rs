extern crate chrono;
extern crate uuid;

mod tdb2;
mod task;

use tdb2::parse;
use std::io::stdin;

fn main() {
    let input = stdin();
    parse(input.lock()).unwrap().iter().for_each(|t| {
        println!("{:?}", t);
    });
}
