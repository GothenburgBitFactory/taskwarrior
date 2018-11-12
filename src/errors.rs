error_chain!{
    links {
        Tdb2Error(::tdb2::errors::Error, ::tdb2::errors::ErrorKind);
    }
}
