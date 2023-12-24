/*!
* Support for cloud-service-backed sync.
*
* All of these operate using a similar approach, with specific patterns of object names. The
* process of adding a new version requires a compare-and-swap operation that sets a new version
* as the "latest" only if the existing "latest" has the expected value. This ensures a continuous
* chain of versions, even if multiple replicas attempt to sync at the same time.
*/

mod server;
mod service;

pub(in crate::server) use server::CloudServer;

#[cfg(feature = "server-gcp")]
pub(in crate::server) mod gcp;
