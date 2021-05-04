use std::process::exit;

pub fn main() {
    if let Err(err) = taskchampion_cli::main() {
        eprintln!("{:?}", err);
        exit(1);
    }
}
