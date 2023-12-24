use crate::errors::Result;

/// Information about an object as returned from `Service::list`
pub(in crate::server) struct ObjectInfo {
    /// Name of the object.
    pub(in crate::server) name: Vec<u8>,
    /// Creation time of the object, in seconds since the UNIX epoch.
    pub(in crate::server) creation: u64,
}

/// An abstraction of a cloud-storage service.
///
/// The underlying cloud storage is assumed to be a map from object names to object values,
/// similar to a HashMap, with the addition of a compare-and-swap operation. Object names
/// are always simple strings from the character set `[a-zA-Z0-9-]`, no more than 100 characters
/// in length.
pub(in crate::server) trait Service {
    /// Put an object into cloud storage. If the object exists, it is overwritten.
    fn put(&mut self, name: &[u8], value: &[u8]) -> Result<()>;

    /// Get an object from cloud storage, or None if the object does not exist.
    fn get(&mut self, name: &[u8]) -> Result<Option<Vec<u8>>>;

    /// Delete an object. Does nothing if the object does not exist.
    fn del(&mut self, name: &[u8]) -> Result<()>;

    /// Enumerate objects with the given prefix.
    fn list<'a>(&'a mut self, prefix: &[u8]) -> Box<dyn Iterator<Item = Result<ObjectInfo>> + 'a>;

    /// Compare the existing object's value with `existing_value`, and replace with `new_value`
    /// only if the values match. Returns true if the replacement occurred.
    fn compare_and_swap(
        &mut self,
        name: &[u8],
        existing_value: Option<Vec<u8>>,
        new_value: Vec<u8>,
    ) -> Result<bool>;
}
