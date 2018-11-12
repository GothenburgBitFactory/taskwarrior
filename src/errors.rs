error_chain!{
    foreign_links {
        Io(::std::io::Error);
        StrFromUtf8(::std::str::Utf8Error);
        StringFromUtf8(::std::string::FromUtf8Error);
        StringFromUtf16(::std::string::FromUtf16Error);
    }

}
