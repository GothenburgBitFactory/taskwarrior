#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

/// A replica represents an instance of a user's task data, providing an easy interface
/// for querying and modifying that data.
struct Replica;

/// Uuid is used as a task identifier.  Uuids do not contain any pointers and need not be freed.
struct Uuid {
  uint8_t _0[16];
};

extern "C" {

extern const uintptr_t TC_UUID_STRING_BYTES;

/// Create a new Replica.
///
/// If path is NULL, then an in-memory replica is created.  Otherwise, path is the path to the
/// on-disk storage for this replica.  The path argument is no longer referenced after return.
///
/// Returns NULL on error; see tc_replica_error.
///
/// Replicas are not threadsafe.
Replica *tc_replica_new(const char *path);

/// Undo local operations until the most recent UndoPoint.
///
/// Returns -1 on error, 0 if there are no local operations to undo, and 1 if operations were
/// undone.
int32_t tc_replica_undo(Replica *rep);

/// Get the latest error for a replica, or NULL if the last operation succeeded.
///
/// The returned string is valid until the next replica operation.
const char *tc_replica_error(Replica *rep);

/// Free a Replica.
void tc_replica_free(Replica *rep);

/// Create a new, randomly-generated UUID.
Uuid tc_uuid_new_v4();

/// Create a new UUID with the nil value.
Uuid tc_uuid_nil();

/// Write the string representation of a Uuid into the given buffer, which must be
/// at least TC_UUID_STRING_BYTES long.  No NUL terminator is added.
void tc_uuid_to_str(Uuid uuid, char *out);

/// Parse the given value as a UUID.  The value must be exactly TC_UUID_STRING_BYTES long.  Returns
/// false on failure.
bool tc_uuid_from_str(const char *val, Uuid *out);

} // extern "C"
