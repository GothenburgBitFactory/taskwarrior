use failure::Fallible;
use taskchampion::Replica;

pub(crate) fn execute(replica: &mut Replica) -> Fallible<()> {
    replica.gc()?;
    println!("garbage collected.");
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::invocation::cmd::test::test_replica;

    #[test]
    fn test_gc() {
        let mut replica = test_replica();
        execute(&mut replica).unwrap();
        // this mostly just needs to not fail!
    }
}
