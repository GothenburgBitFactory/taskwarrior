use super::service::{ObjectInfo, Service};
use crate::errors::Result;
use google_cloud_storage::client::google_cloud_auth::credentials::CredentialsFile;
use google_cloud_storage::client::{Client, ClientConfig};
use google_cloud_storage::http::error::ErrorResponse;
use google_cloud_storage::http::Error as GcsError;
use google_cloud_storage::http::{self, objects};
use tokio::runtime::Runtime;

/// A [`Service`] implementation based on the Google Cloud Storage service.
pub(in crate::server) struct GcpService {
    client: Client,
    rt: Runtime,
    bucket: String,
}

/// Determine whether the given result contains an HTTP error with the given code.
fn is_http_error<T>(query: u16, res: &std::result::Result<T, http::Error>) -> bool {
    match res {
        // Errors from RPC's.
        Err(GcsError::Response(ErrorResponse { code, .. })) => *code == query,
        // Errors from reqwest (downloads, uploads).
        Err(GcsError::HttpClient(e)) => e.status().map(|s| s.as_u16()) == Some(query),
        _ => false,
    }
}

impl GcpService {
    pub(in crate::server) fn new(bucket: String, credential_path: Option<String>) -> Result<Self> {
        let rt = Runtime::new()?;

        let credentialpathstring = credential_path.clone().unwrap();
        let config: ClientConfig = if credential_path.unwrap() == "" {
            rt.block_on(ClientConfig::default().with_auth())?
        } else {
            let credentials = rt.block_on(CredentialsFile::new_from_file(credentialpathstring))?;
            rt.block_on(ClientConfig::default().with_credentials(credentials))?
        };

        Ok(Self {
            client: Client::new(config),
            rt,
            bucket,
        })
    }
}

impl Service for GcpService {
    fn put(&mut self, name: &[u8], value: &[u8]) -> Result<()> {
        let name = String::from_utf8(name.to_vec()).expect("non-UTF8 object name");
        let upload_type = objects::upload::UploadType::Simple(objects::upload::Media::new(name));
        self.rt.block_on(self.client.upload_object(
            &objects::upload::UploadObjectRequest {
                bucket: self.bucket.clone(),
                ..Default::default()
            },
            value.to_vec(),
            &upload_type,
        ))?;
        Ok(())
    }

    fn get(&mut self, name: &[u8]) -> Result<Option<Vec<u8>>> {
        let name = String::from_utf8(name.to_vec()).expect("non-UTF8 object name");
        let download_res = self.rt.block_on(self.client.download_object(
            &objects::get::GetObjectRequest {
                bucket: self.bucket.clone(),
                object: name,
                ..Default::default()
            },
            &objects::download::Range::default(),
        ));
        if is_http_error(404, &download_res) {
            Ok(None)
        } else {
            Ok(Some(download_res?))
        }
    }

    fn del(&mut self, name: &[u8]) -> Result<()> {
        let name = String::from_utf8(name.to_vec()).expect("non-UTF8 object name");
        let del_res = self.rt.block_on(self.client.delete_object(
            &objects::delete::DeleteObjectRequest {
                bucket: self.bucket.clone(),
                object: name,
                ..Default::default()
            },
        ));
        if !is_http_error(404, &del_res) {
            del_res?;
        }
        Ok(())
    }

    fn list<'a>(&'a mut self, prefix: &[u8]) -> Box<dyn Iterator<Item = Result<ObjectInfo>> + 'a> {
        let prefix = String::from_utf8(prefix.to_vec()).expect("non-UTF8 object prefix");
        Box::new(ObjectIterator {
            service: self,
            prefix,
            last_response: None,
            next_index: 0,
        })
    }

    fn compare_and_swap(
        &mut self,
        name: &[u8],
        existing_value: Option<Vec<u8>>,
        new_value: Vec<u8>,
    ) -> Result<bool> {
        let name = String::from_utf8(name.to_vec()).expect("non-UTF8 object name");
        let get_res = self
            .rt
            .block_on(self.client.get_object(&objects::get::GetObjectRequest {
                bucket: self.bucket.clone(),
                object: name.clone(),
                ..Default::default()
            }));
        // Determine the object's generation. See https://cloud.google.com/storage/docs/metadata#generation-number
        let generation = if is_http_error(404, &get_res) {
            // If a value was expected, that expectation has not been met.
            if existing_value.is_some() {
                return Ok(false);
            }
            // Generation 0 indicates that the object does not yet exist.
            0
        } else {
            get_res?.generation
        };

        // If the file existed, then verify its contents.
        if generation > 0 {
            let data = self.rt.block_on(self.client.download_object(
                &objects::get::GetObjectRequest {
                    bucket: self.bucket.clone(),
                    object: name.clone(),
                    // Fetch the same generation.
                    generation: Some(generation),
                    ..Default::default()
                },
                &objects::download::Range::default(),
            ))?;
            if Some(data) != existing_value {
                return Ok(false);
            }
        }

        // Finally, put the new value with a condition that the generation hasn't changed.
        let upload_type = objects::upload::UploadType::Simple(objects::upload::Media::new(name));
        let upload_res = self.rt.block_on(self.client.upload_object(
            &objects::upload::UploadObjectRequest {
                bucket: self.bucket.clone(),
                if_generation_match: Some(generation),
                ..Default::default()
            },
            new_value.to_vec(),
            &upload_type,
        ));
        if is_http_error(412, &upload_res) {
            // A 412 indicates the precondition was not satisfied: the given generation
            // is no longer the latest.
            Ok(false)
        } else {
            upload_res?;
            Ok(true)
        }
    }
}

/// An Iterator returning names of objects from `list_objects`.
///
/// This handles response pagination by fetching one page at a time.
struct ObjectIterator<'a> {
    service: &'a mut GcpService,
    prefix: String,
    last_response: Option<objects::list::ListObjectsResponse>,
    next_index: usize,
}

impl<'a> ObjectIterator<'a> {
    fn fetch_batch(&mut self) -> Result<()> {
        let mut page_token = None;
        if let Some(ref resp) = self.last_response {
            page_token = resp.next_page_token.clone();
        }
        self.last_response = Some(self.service.rt.block_on(self.service.client.list_objects(
            &objects::list::ListObjectsRequest {
                bucket: self.service.bucket.clone(),
                prefix: Some(self.prefix.clone()),
                page_token,
                #[cfg(test)] // For testing, use a small page size.
                max_results: Some(6),
                ..Default::default()
            },
        ))?);
        self.next_index = 0;
        Ok(())
    }
}

impl<'a> Iterator for ObjectIterator<'a> {
    type Item = Result<ObjectInfo>;
    fn next(&mut self) -> Option<Self::Item> {
        // If the iterator is just starting, fetch the first response.
        if self.last_response.is_none() {
            if let Err(e) = self.fetch_batch() {
                return Some(Err(e));
            }
        }
        if let Some(ref result) = self.last_response {
            if let Some(ref items) = result.items {
                if self.next_index < items.len() {
                    // Return a result from the existing response.
                    let obj = &items[self.next_index];
                    self.next_index += 1;
                    // It's unclear when `time_created` would be None, so default to 0 in that case
                    // or when the timestamp is not a valid u64 (before 1970).
                    let creation = obj.time_created.map(|t| t.unix_timestamp()).unwrap_or(0);
                    let creation: u64 = creation.try_into().unwrap_or(0);
                    return Some(Ok(ObjectInfo {
                        name: obj.name.as_bytes().to_vec(),
                        creation,
                    }));
                } else if result.next_page_token.is_some() {
                    // Fetch the next page and try again.
                    if let Err(e) = self.fetch_batch() {
                        return Some(Err(e));
                    }
                    return self.next();
                }
            }
        }
        None
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use uuid::Uuid;

    /// Make a service if `GCP_TEST_BUCKET` is set, as well as a function to put a unique prefix on
    /// an object name, so that tests do not interfere with one another.
    ///
    /// Set up this bucket with a lifecyle policy to delete objects with age > 1 day. While passing
    /// tests should correctly clean up after themselves, failing tests may leave objects in the
    /// bucket.
    ///
    /// When the environment variable is not set, this returns false and the test does not run.
    /// Note that the Rust test runner will still show "ok" for the test, as there is no way to
    /// indicate anything else.
    fn make_service() -> Option<(GcpService, impl Fn(&str) -> Vec<u8>)> {
        let Ok(bucket) = std::env::var("GCP_TEST_BUCKET") else {
            return None;
        };

        let Ok(credential_path) = std::env::var("GCP_TEST_CREDENTIAL_PATH") else {
            return None;
        };

        let prefix = Uuid::new_v4();
        Some((
            GcpService::new(bucket, Some(credential_path)).unwrap(),
            move |n: &_| format!("{}-{}", prefix.as_simple(), n).into_bytes(),
        ))
    }

    #[test]
    fn put_and_get() {
        let Some((mut svc, pfx)) = make_service() else {
            return;
        };
        svc.put(&pfx("testy"), b"foo").unwrap();
        let got = svc.get(&pfx("testy")).unwrap();
        assert_eq!(got, Some(b"foo".to_vec()));

        // Clean up.
        svc.del(&pfx("testy")).unwrap();
    }

    #[test]
    fn get_missing() {
        let Some((mut svc, pfx)) = make_service() else {
            return;
        };
        let got = svc.get(&pfx("testy")).unwrap();
        assert_eq!(got, None);
    }

    #[test]
    fn del() {
        let Some((mut svc, pfx)) = make_service() else {
            return;
        };
        svc.put(&pfx("testy"), b"data").unwrap();
        svc.del(&pfx("testy")).unwrap();
        let got = svc.get(&pfx("testy")).unwrap();
        assert_eq!(got, None);
    }

    #[test]
    fn del_missing() {
        // Deleting an object that does not exist is not an error.
        let Some((mut svc, pfx)) = make_service() else {
            return;
        };

        assert!(svc.del(&pfx("testy")).is_ok());
    }

    #[test]
    fn list() {
        let Some((mut svc, pfx)) = make_service() else {
            return;
        };
        let mut names: Vec<_> = (0..20).map(|i| pfx(&format!("pp-{i:02}"))).collect();
        names.sort();
        // Create 20 objects that will be listed.
        for n in &names {
            svc.put(n, b"data").unwrap();
        }
        // And another object that should not be included in the list.
        svc.put(&pfx("xxx"), b"data").unwrap();

        let got_objects: Vec<_> = svc.list(&pfx("pp-")).collect::<Result<_>>().unwrap();
        let mut got_names: Vec<_> = got_objects.into_iter().map(|oi| oi.name).collect();
        got_names.sort();
        assert_eq!(got_names, names);

        // Clean up.
        for n in got_names {
            svc.del(&n).unwrap();
        }
        svc.del(&pfx("xxx")).unwrap();
    }

    #[test]
    fn compare_and_swap_create() {
        let Some((mut svc, pfx)) = make_service() else {
            return;
        };

        assert!(svc
            .compare_and_swap(&pfx("testy"), None, b"bar".to_vec())
            .unwrap());
        let got = svc.get(&pfx("testy")).unwrap();
        assert_eq!(got, Some(b"bar".to_vec()));

        // Clean up.
        svc.del(&pfx("testy")).unwrap();
    }

    #[test]
    fn compare_and_swap_matches() {
        let Some((mut svc, pfx)) = make_service() else {
            return;
        };

        // Create the existing file, with two generations.
        svc.put(&pfx("testy"), b"foo1").unwrap();
        svc.put(&pfx("testy"), b"foo2").unwrap();
        assert!(svc
            .compare_and_swap(&pfx("testy"), Some(b"foo2".to_vec()), b"bar".to_vec())
            .unwrap());
        let got = svc.get(&pfx("testy")).unwrap();
        assert_eq!(got, Some(b"bar".to_vec()));

        // Clean up.
        svc.del(&pfx("testy")).unwrap();
    }

    #[test]
    fn compare_and_swap_expected_no_file() {
        let Some((mut svc, pfx)) = make_service() else {
            return;
        };

        svc.put(&pfx("testy"), b"foo1").unwrap();
        assert!(!svc
            .compare_and_swap(&pfx("testy"), None, b"bar".to_vec())
            .unwrap());
        let got = svc.get(&pfx("testy")).unwrap();
        assert_eq!(got, Some(b"foo1".to_vec()));

        // Clean up.
        svc.del(&pfx("testy")).unwrap();
    }

    #[test]
    fn compare_and_swap_mismatch() {
        let Some((mut svc, pfx)) = make_service() else {
            return;
        };

        // Create the existing file, with two generations.
        svc.put(&pfx("testy"), b"foo1").unwrap();
        svc.put(&pfx("testy"), b"foo2").unwrap();
        assert!(!svc
            .compare_and_swap(&pfx("testy"), Some(b"foo1".to_vec()), b"bar".to_vec())
            .unwrap());
        let got = svc.get(&pfx("testy")).unwrap();
        assert_eq!(got, Some(b"foo2".to_vec()));

        // Clean up.
        svc.del(&pfx("testy")).unwrap();
    }
}
