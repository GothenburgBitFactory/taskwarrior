#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

/// The status of a task, as defined by the task data model.
enum TCStatus {
  TC_STATUS_PENDING,
  TC_STATUS_COMPLETED,
  TC_STATUS_DELETED,
  /// Unknown signifies a status in the task DB that was not
  /// recognized.
  TC_STATUS_UNKNOWN,
};

/// A replica represents an instance of a user's task data, providing an easy interface
/// for querying and modifying that data.
struct TCReplica;

/// TCString supports passing strings into and out of the TaskChampion API.
///
/// Unless specified otherwise, functions in this API take ownership of a TCString when it appears
/// as a function argument, and transfer ownership to the caller when the TCString appears as a
/// return value or otput argument.
struct TCString;

/// A task, as publicly exposed by this library.
///
/// A task carries no reference to the replica that created it, and can
/// be used until it is freed or converted to a TaskMut.
struct TCTask;

/// TCUuid is used as a task identifier.  Uuids do not contain any pointers and need not be freed.
/// Uuids are typically treated as opaque, but the bytes are available in big-endian format.
///
struct TCUuid {
  uint8_t bytes[16];
};

extern "C" {

extern const uintptr_t TC_UUID_STRING_BYTES;

/// Create a new TCReplica.
///
/// If path is NULL, then an in-memory replica is created.  Otherwise, path is the path to the
/// on-disk storage for this replica.  The path argument is no longer referenced after return.
///
/// Returns NULL on error; see tc_replica_error.
///
/// TCReplicas are not threadsafe.
TCReplica *tc_replica_new(TCString *path);

/// Create a new task.  The task must not already exist.
///
/// Returns the task, or NULL on error.
TCTask *tc_replica_new_task(TCReplica *rep, TCStatus status, TCString *description);

/// Undo local operations until the most recent UndoPoint.
///
/// Returns -1 on error, 0 if there are no local operations to undo, and 1 if operations were
/// undone.
int32_t tc_replica_undo(TCReplica *rep);

/// Get the latest error for a replica, or NULL if the last operation succeeded.
///
/// The returned string is valid until the next replica operation.
const char *tc_replica_error(TCReplica *rep);

/// Free a TCReplica.
void tc_replica_free(TCReplica *rep);

/// Create a new TCString referencing the given C string.  The C string must remain valid until
/// after the TCString is freed.  It's typically easiest to ensure this by using a static string.
TCString *tc_string_new(const char *cstr);

/// Create a new TCString by cloning the content of the given C string.
TCString *tc_string_clone(const char *cstr);

/// Create a new TCString containing the given string with the given length. This allows creation
/// of strings containing embedded NUL characters.  If the given string is not valid UTF-8, this
/// function will return NULL.
TCString *tc_string_clone_with_len(const char *buf, uintptr_t len);

/// Get the content of the string as a regular C string.  The given string must not be NULL.  The
/// returned value may be NULL if the string contains NUL bytes.
/// This function does _not_ take ownership of the TCString.
const char *tc_string_content(TCString *tcstring);

/// Free a TCString.
void tc_string_free(TCString *string);

/// Get a task's UUID.
TCUuid tc_task_get_uuid(const TCTask *task);

/// Get a task's status.
TCStatus tc_task_get_status(const TCTask *task);

/// Get a task's description, or NULL if the task cannot be represented as a C string (e.g., if it
/// contains embedded NUL characters).
TCString *tc_task_get_description(const TCTask *task);

/// Free a task.
void tc_task_free(TCTask *task);

/// Create a new, randomly-generated UUID.
TCUuid tc_uuid_new_v4();

/// Create a new UUID with the nil value.
TCUuid tc_uuid_nil();

/// Write the string representation of a TCUuid into the given buffer, which must be
/// at least TC_UUID_STRING_BYTES long.  No NUL terminator is added.
void tc_uuid_to_str(TCUuid uuid, char *out);

/// Parse the given value as a UUID.  The value must be exactly TC_UUID_STRING_BYTES long.  Returns
/// false on failure.
bool tc_uuid_from_str(const char *val, TCUuid *out);

} // extern "C"
