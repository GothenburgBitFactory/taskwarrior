use anyhow::{anyhow, bail, Result};
use toml::value::Table;

/// Check that the input is a table and contains no keys not in the given list, returning
/// the table.
pub(super) fn table_with_keys<'a>(cfg: &'a toml::Value, keys: &[&str]) -> Result<&'a Table> {
    let table = cfg.as_table().ok_or_else(|| anyhow!("not a table"))?;

    for tk in table.keys() {
        if !keys.iter().any(|k| k == tk) {
            bail!("unknown table key `{}`", tk);
        }
    }
    Ok(table)
}

#[cfg(test)]
mod test {
    use super::*;
    use toml::toml;

    #[test]
    fn test_dissect_table_missing() {
        let val = toml! { bar = true };
        let diss = table_with_keys(&val, &["foo", "bar"]).unwrap();
        assert_eq!(diss.get("bar"), Some(&toml::Value::Boolean(true)));
        assert_eq!(diss.get("foo"), None);
    }

    #[test]
    fn test_dissect_table_extra() {
        let val = toml! { nosuch = 10 };
        assert!(table_with_keys(&val, &["foo", "bar"]).is_err());
    }

    #[test]
    fn test_dissect_table_not_a_table() {
        let val = toml::Value::Array(vec![]);
        assert!(table_with_keys(&val, &["foo", "bar"]).is_err());
    }
}
