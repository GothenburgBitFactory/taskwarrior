use failure::Fallible;

pub(crate) fn execute() -> Fallible<()> {
    println!("TaskChampion {}", env!("CARGO_PKG_VERSION"));
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_version() {
        execute().unwrap();
    }
}
