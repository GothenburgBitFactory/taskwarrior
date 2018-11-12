error_chain!{
    foreign_links {
        Io(::std::io::Error);
        StrFromUtf8(::std::str::Utf8Error);
        StringFromUtf8(::std::string::FromUtf8Error);
        StringFromUtf16(::std::string::FromUtf16Error);
    }

    errors {
        ParseError(filename: String, line: u64) {
            description("TDB2 parse error"),
            display("TDB2 parse error at {}:{}", filename, line),
        }
    }
}
