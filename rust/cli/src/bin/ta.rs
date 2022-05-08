use std::process::exit;

pub fn main() {
    match taskchampion_cli::main() {
        Ok(_) => exit(0),
        Err(e) => {
            eprintln!("{:?}", e);
            exit(e.exit_status());
        }
    }
}
