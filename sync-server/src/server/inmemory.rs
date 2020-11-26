use super::{
    AddVersionResult, ClientId, GetVersionResult, HistorySegment, SyncServer, VersionId,
    NO_VERSION_ID,
};
use failure::Fallible;
use std::collections::HashMap;
use std::sync::{Mutex, RwLock};
use taskchampion::Uuid;

/// An in-memory server backend that can be useful for testing.
pub(crate) struct InMemorySyncServer {
    clients: RwLock<HashMap<ClientId, Mutex<Client>>>,
}

struct Version {
    version_id: VersionId,
    history_segment: HistorySegment,
}

struct Client {
    latest_version_id: VersionId,
    // NOTE: indexed by parent_version_id!
    versions: HashMap<VersionId, Version>,
}

impl InMemorySyncServer {
    pub(crate) fn new() -> Self {
        Self {
            clients: RwLock::new(HashMap::new()),
        }
    }
}

impl SyncServer for InMemorySyncServer {
    fn get_child_version(
        &self,
        client_id: ClientId,
        parent_version_id: VersionId,
    ) -> Fallible<Option<GetVersionResult>> {
        let clients = self.clients.read().expect("poisoned lock");
        if let Some(client) = clients.get(&client_id) {
            let client = client.lock().expect("poisoned lock");
            if let Some(version) = client.versions.get(&parent_version_id) {
                return Ok(Some(GetVersionResult {
                    version_id: version.version_id,
                    parent_version_id,
                    history_segment: version.history_segment.clone(),
                }));
            }
        }
        Ok(None)
    }

    fn add_version(
        &self,
        client_id: ClientId,
        parent_version_id: VersionId,
        history_segment: HistorySegment,
    ) -> Fallible<AddVersionResult> {
        let mut clients = self.clients.write().expect("poisoned lock");
        if let Some(client) = clients.get_mut(&client_id) {
            let mut client = client.lock().expect("poisoned lock");
            if client.latest_version_id != NO_VERSION_ID {
                if parent_version_id != client.latest_version_id {
                    return Ok(AddVersionResult::ExpectedParentVersion(
                        client.latest_version_id,
                    ));
                }
            }

            // invent a new ID for this version
            let version_id = Uuid::new_v4();

            client.versions.insert(
                parent_version_id,
                Version {
                    version_id,
                    history_segment,
                },
            );
            client.latest_version_id = version_id;

            Ok(AddVersionResult::Ok(version_id))
        } else {
            // new client, so insert a client with just this new version

            let latest_version_id = Uuid::new_v4();
            let mut versions = HashMap::new();
            versions.insert(
                parent_version_id,
                Version {
                    version_id: latest_version_id,
                    history_segment,
                },
            );

            clients.insert(
                client_id,
                Mutex::new(Client {
                    latest_version_id,
                    versions,
                }),
            );

            Ok(AddVersionResult::Ok(latest_version_id))
        }
    }
}
