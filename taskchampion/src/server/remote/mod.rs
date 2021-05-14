use crate::server::{AddVersionResult, GetVersionResult, HistorySegment, Server, VersionId};
use std::convert::TryInto;
use std::time::Duration;
use uuid::Uuid;

mod crypto;
use crypto::{HistoryCiphertext, HistoryCleartext, Secret};

pub struct RemoteServer {
    origin: String,
    client_key: Uuid,
    encryption_secret: Secret,
    agent: ureq::Agent,
}

/// A RemoeServer communicates with a remote server over HTTP (such as with
/// taskchampion-sync-server).
impl RemoteServer {
    /// Construct a new RemoteServer.  The `origin` is the sync server's protocol and hostname
    /// without a trailing slash, such as `https://tcsync.example.com`.  Pass a client_key to
    /// identify this client to the server.  Multiple replicas synchronizing the same task history
    /// should use the same client_key.
    pub fn new(origin: String, client_key: Uuid, encryption_secret: Vec<u8>) -> RemoteServer {
        RemoteServer {
            origin,
            client_key,
            encryption_secret: encryption_secret.into(),
            agent: ureq::AgentBuilder::new()
                .timeout_connect(Duration::from_secs(10))
                .timeout_read(Duration::from_secs(60))
                .build(),
        }
    }
}

/// Read a UUID-bearing header or fail trying
fn get_uuid_header(resp: &ureq::Response, name: &str) -> anyhow::Result<Uuid> {
    let value = resp
        .header(name)
        .ok_or_else(|| anyhow::anyhow!("Response does not have {} header", name))?;
    let value = Uuid::parse_str(value)
        .map_err(|e| anyhow::anyhow!("{} header is not a valid UUID: {}", name, e))?;
    Ok(value)
}

impl Server for RemoteServer {
    fn add_version(
        &mut self,
        parent_version_id: VersionId,
        history_segment: HistorySegment,
    ) -> anyhow::Result<AddVersionResult> {
        let url = format!(
            "{}/v1/client/add-version/{}",
            self.origin, parent_version_id
        );
        let history_cleartext = HistoryCleartext {
            parent_version_id,
            history_segment,
        };
        let history_ciphertext = history_cleartext.seal(&self.encryption_secret)?;
        match self
            .agent
            .post(&url)
            .set(
                "Content-Type",
                "application/vnd.taskchampion.history-segment",
            )
            .set("X-Client-Key", &self.client_key.to_string())
            .send_bytes(history_ciphertext.as_ref())
        {
            Ok(resp) => {
                let version_id = get_uuid_header(&resp, "X-Version-Id")?;
                Ok(AddVersionResult::Ok(version_id))
            }
            Err(ureq::Error::Status(status, resp)) if status == 409 => {
                let parent_version_id = get_uuid_header(&resp, "X-Parent-Version-Id")?;
                Ok(AddVersionResult::ExpectedParentVersion(parent_version_id))
            }
            Err(err) => Err(err.into()),
        }
    }

    fn get_child_version(
        &mut self,
        parent_version_id: VersionId,
    ) -> anyhow::Result<GetVersionResult> {
        let url = format!(
            "{}/v1/client/get-child-version/{}",
            self.origin, parent_version_id
        );
        match self
            .agent
            .get(&url)
            .set("X-Client-Key", &self.client_key.to_string())
            .call()
        {
            Ok(resp) => {
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
            }
            Err(ureq::Error::Status(status, _)) if status == 404 => {
                Ok(GetVersionResult::NoSuchVersion)
            }
            Err(err) => Err(err.into()),
        }
    }
}
