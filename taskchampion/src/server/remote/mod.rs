use crate::server::{AddVersionResult, GetVersionResult, HistorySegment, Server, VersionId};
use failure::{format_err, Fallible};
use std::convert::TryInto;
use uuid::Uuid;

mod crypto;
use crypto::{HistoryCiphertext, HistoryCleartext, Secret};

pub struct RemoteServer {
    origin: String,
    client_id: Uuid,
    encryption_secret: Secret,
    agent: ureq::Agent,
}

/// A RemoeServer communicates with a remote server over HTTP (such as with
/// taskchampion-sync-server).
impl RemoteServer {
    /// Construct a new RemoteServer.  The `origin` is the sync server's protocol and hostname
    /// without a trailing slash, such as `https://tcsync.example.com`.  Pass a client_id to
    /// identify this client to the server.  Multiple replicas synchronizing the same task history
    /// should use the same client_id.
    pub fn new(origin: String, client_id: Uuid, encryption_secret: Vec<u8>) -> RemoteServer {
        RemoteServer {
            origin,
            client_id,
            encryption_secret: encryption_secret.into(),
            agent: ureq::agent(),
        }
    }
}

/// Convert a ureq::Response to an Error
fn resp_to_error(resp: ureq::Response) -> failure::Error {
    return format_err!(
        "error {}: {}",
        resp.status(),
        resp.into_string()
            .unwrap_or_else(|e| format!("(could not read response: {})", e))
    );
}

/// Read a UUID-bearing header or fail trying
fn get_uuid_header(resp: &ureq::Response, name: &str) -> Fallible<Uuid> {
    let value = resp
        .header(name)
        .ok_or_else(|| format_err!("Response does not have {} header", name))?;
    let value = Uuid::parse_str(value)
        .map_err(|e| format_err!("{} header is not a valid UUID: {}", name, e))?;
    Ok(value)
}

impl Server for RemoteServer {
    fn add_version(
        &mut self,
        parent_version_id: VersionId,
        history_segment: HistorySegment,
    ) -> Fallible<AddVersionResult> {
        let url = format!(
            "{}/client/{}/add-version/{}",
            self.origin, self.client_id, parent_version_id
        );
        let history_cleartext = HistoryCleartext {
            parent_version_id,
            history_segment,
        };
        let history_ciphertext = history_cleartext.seal(&self.encryption_secret)?;
        let resp = self
            .agent
            .post(&url)
            .timeout_connect(10_000)
            .timeout_read(60_000)
            .set(
                "Content-Type",
                "application/vnd.taskchampion.history-segment",
            )
            .send_bytes(history_ciphertext.as_ref());
        if resp.ok() {
            let version_id = get_uuid_header(&resp, "X-Version-Id")?;
            Ok(AddVersionResult::Ok(version_id))
        } else if resp.status() == 409 {
            let parent_version_id = get_uuid_header(&resp, "X-Parent-Version-Id")?;
            Ok(AddVersionResult::ExpectedParentVersion(parent_version_id))
        } else {
            Err(resp_to_error(resp))
        }
    }

    fn get_child_version(&mut self, parent_version_id: VersionId) -> Fallible<GetVersionResult> {
        let url = format!(
            "{}/client/{}/get-child-version/{}",
            self.origin, self.client_id, parent_version_id
        );
        let resp = self
            .agent
            .get(&url)
            .timeout_connect(10_000)
            .timeout_read(60_000)
            .call();

        if resp.ok() {
            let parent_version_id = get_uuid_header(&resp, "X-Parent-Version-Id")?;
            let version_id = get_uuid_header(&resp, "X-Version-Id")?;
            let history_ciphertext: HistoryCiphertext = resp.try_into()?;
            let history_segment = history_ciphertext
                .open(&self.encryption_secret, parent_version_id)?
                .history_segment;
            Ok(GetVersionResult::Version {
                version_id,
                parent_version_id,
                history_segment,
            })
        } else if resp.status() == 404 {
            Ok(GetVersionResult::NoSuchVersion)
        } else {
            Err(resp_to_error(resp))
        }
    }
}
