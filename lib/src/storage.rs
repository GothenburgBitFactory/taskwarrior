use taskchampion::{storage::Storage, StorageConfig};

pub struct StoragePtr(Box<dyn Storage>);

#[no_mangle]
pub extern "C" fn storage_new_in_memory() -> *mut StoragePtr {
    // TODO: this is a box containing a fat pointer
    Box::into_raw(Box::new(StoragePtr(
        StorageConfig::InMemory.into_storage().unwrap(),
    )))
}

#[no_mangle]
pub extern "C" fn storage_free(storage: *mut StoragePtr) {
    drop(unsafe { Box::from_raw(storage) });
}
