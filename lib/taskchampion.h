#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

/// A replica represents an instance of a user's task data, providing an easy interface
/// for querying and modifying that data.
struct Replica;

extern "C" {

/// Create a new Replica.
///
/// If path is NULL, then an in-memory replica is created.  Otherwise, path is the path to the
/// on-disk storage for this replica.  The path argument is no longer referenced after return.
///
/// Returns NULL on error; see tc_replica_error.
///
/// Replicas are not threadsafe.
Replica *tc_replica_new(const char *path);

/// temporary (testing errors)
uint32_t uhoh(Replica *rep);

/// Get the latest error for a replica, or NULL if the last operation succeeded.
///
/// The returned string is valid until the next replica operation.
const char *tc_replica_error(Replica *rep);

/// Free a Replica.
void tc_replica_free(Replica *rep);

} // extern "C"
