use failure::Fallible;

pub(crate) fn execute(command_name: String, summary: bool) -> Fallible<()> {
    println!(
        "TaskChampion {}: Personal task-tracking",
        env!("CARGO_PKG_VERSION")
    );
    if !summary {
        println!();
        println!("USAGE: {} [args]\n(help output TODO)", command_name); // TODO
    }
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_summary() {
        execute("task".to_owned(), true).unwrap();
    }

    #[test]
    fn test_long() {
        execute("task".to_owned(), false).unwrap();
    }
}
