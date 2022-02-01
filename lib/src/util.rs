use crate::string::TCString;

pub(crate) fn err_to_tcstring(e: impl std::string::ToString) -> TCString<'static> {
    TCString::from(e.to_string())
}
