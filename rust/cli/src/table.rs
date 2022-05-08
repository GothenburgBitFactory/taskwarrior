use prettytable::format;

pub(crate) fn format() -> format::TableFormat {
    format::FormatBuilder::new()
        .column_separator(' ')
        .borders(' ')
        .build()
}
