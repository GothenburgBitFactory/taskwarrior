use crate::traits::*;
use crate::types::*;
use crate::util::err_to_ruststring;
use taskchampion::{Server, ServerConfig};

/// TCServer represents an interface to a sync server.  Aside from new and free, a server
/// has no C-accessible API, but is designed to be passed to `tc_replica_sync`.
///
/// ## Safety
///
/// TCServer are not threadsafe, and must not be used with multiple replicas simultaneously.
pub struct TCServer(Box<dyn Server>);

impl PassByPointer for TCServer {}

impl From<Box<dyn Server>> for TCServer {
    fn from(server: Box<dyn Server>) -> TCServer {
        TCServer(server)
    }
}

impl AsMut<Box<dyn Server>> for TCServer {
    fn as_mut(&mut self) -> &mut Box<dyn Server> {
        &mut self.0
    }
}

/// Utility function to allow using `?` notation to return an error value.  
fn wrap<T, F>(f: F, error_out: *mut TCString, err_value: T) -> T
where
    F: FnOnce() -> anyhow::Result<T>,
{
    if !error_out.is_null() {
        // SAFETY:
        //  - error_out is not NULL (just checked)
        //  - properly aligned and valid (promised by caller)
        unsafe { *error_out = TCString::default() };
    }

    match f() {
        Ok(v) => v,
        Err(e) => {
            if !error_out.is_null() {
                // SAFETY:
                //  - error_out is not NULL (just checked)
                //  - properly aligned and valid (promised by caller)
                unsafe {
                    TCString::val_to_arg_out(err_to_ruststring(e), error_out);
                }
            }
            err_value
        }
    }
}

/// Create a new TCServer that operates locally (on-disk).  See the TaskChampion docs for the
/// description of the arguments.
///
/// On error, a string is written to the error_out parameter (if it is not NULL) and NULL is
/// returned.  The caller must free this string.
///
/// The server must be freed after it is used - tc_replica_sync does not automatically free it.
#[no_mangle]
pub unsafe extern "C" fn tc_server_new_local(
    server_dir: TCString,
    error_out: *mut TCString,
) -> *mut TCServer {
    wrap(
        || {
            // SAFETY:
            //  - server_dir is valid (promised by caller)
            //  - caller will not use server_dir after this call (convention)
            let mut server_dir = unsafe { TCString::val_from_arg(server_dir) };
            let server_config = ServerConfig::Local {
                server_dir: server_dir.to_path_buf_mut()?,
            };
            let server = server_config.into_server()?;
            // SAFETY: caller promises to free this server.
            Ok(unsafe { TCServer::return_ptr(server.into()) })
        },
        error_out,
        std::ptr::null_mut(),
    )
}

/// Create a new TCServer that connects to a remote server.  See the TaskChampion docs for the
/// description of the arguments.
///
/// On error, a string is written to the error_out parameter (if it is not NULL) and NULL is
/// returned.  The caller must free this string.
///
/// The server must be freed after it is used - tc_replica_sync does not automatically free it.
#[no_mangle]
pub unsafe extern "C" fn tc_server_new_remote(
    origin: TCString,
    client_key: TCUuid,
    encryption_secret: TCString,
    error_out: *mut TCString,
) -> *mut TCServer {
    wrap(
        || {
            // SAFETY:
            //  - origin is valid (promised by caller)
            //  - origin ownership is transferred to this function
            let origin = unsafe { TCString::val_from_arg(origin) }.into_string()?;

            // SAFETY:
            //  - client_key is a valid Uuid (any 8-byte sequence counts)

            let client_key = unsafe { TCUuid::val_from_arg(client_key) };
            // SAFETY:
            //  - encryption_secret is valid (promised by caller)
            //  - encryption_secret ownership is transferred to this function
            let encryption_secret = unsafe { TCString::val_from_arg(encryption_secret) }
                .as_bytes()
                .to_vec();

            let server_config = ServerConfig::Remote {
                origin,
                client_key,
                encryption_secret,
            };
            let server = server_config.into_server()?;
            // SAFETY: caller promises to free this server.
            Ok(unsafe { TCServer::return_ptr(server.into()) })
        },
        error_out,
        std::ptr::null_mut(),
    )
}

/// Free a server.  The server may not be used after this function returns and must not be freed
/// more than once.
#[no_mangle]
pub unsafe extern "C" fn tc_server_free(server: *mut TCServer) {
    debug_assert!(!server.is_null());
    // SAFETY:
    //  - server is not NULL
    //  - server came from tc_server_new_.., which used return_ptr
    //  - server will not be used after (promised by caller)
    let server = unsafe { TCServer::take_from_ptr_arg(server) };
    drop(server);
}
