#include <stdbool.h>
#include <stdint.h>

/**
 * The status of a task, as defined by the task data model.
 */
typedef enum TCStatus {
  TC_STATUS_PENDING,
  TC_STATUS_COMPLETED,
  TC_STATUS_DELETED,
  /**
   * Unknown signifies a status in the task DB that was not
   * recognized.
   */
  TC_STATUS_UNKNOWN,
} TCStatus;

/**
 * A replica represents an instance of a user's task data, providing an easy interface
 * for querying and modifying that data.
 *
 * TCReplicas are not threadsafe.
 */
typedef struct TCReplica TCReplica;

/**
 * TCString supports passing strings into and out of the TaskChampion API.
 *
 * Unless specified otherwise, functions in this API take ownership of a TCString when it appears
 * as a function argument, and transfer ownership to the caller when the TCString appears as a
 * return value or output argument.
 */
typedef struct TCString TCString;

/**
 * A task, as publicly exposed by this library.
 *
 * A task carries no reference to the replica that created it, and can
 * be used until it is freed or converted to a TaskMut.
 */
typedef struct TCTask TCTask;

/**
 * TCUuid is used as a task identifier.  Uuids do not contain any pointers and need not be freed.
 * Uuids are typically treated as opaque, but the bytes are available in big-endian format.
 *
 */
typedef struct TCUuid {
  uint8_t bytes[16];
} TCUuid;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern const size_t TC_UUID_STRING_BYTES;

/**
 * Create a new TCReplica with an in-memory database.  The contents of the database will be
 * lost when it is freed.
 */
struct TCReplica *tc_replica_new_in_memory(void);

/**
 * Create a new TCReplica with an on-disk database.  On error, a string is written to the
 * `error_out` parameter (if it is not NULL) and NULL is returned.
 */
struct TCReplica *tc_replica_new_on_disk(struct TCString *path, struct TCString **error_out);

/**
 * Create a new task.  The task must not already exist.
 *
 * Returns the task, or NULL on error.
 */
struct TCTask *tc_replica_new_task(struct TCReplica *rep,
                                   enum TCStatus status,
                                   struct TCString *description);

/**
 * Undo local operations until the most recent UndoPoint.
 *
 * Returns -1 on error, 0 if there are no local operations to undo, and 1 if operations were
 * undone.
 */
int32_t tc_replica_undo(struct TCReplica *rep);

/**
 * Get the latest error for a replica, or NULL if the last operation succeeded.
 * Subsequent calls to this function will return NULL.  The caller must free the
 * returned string.
 */
struct TCString *tc_replica_error(struct TCReplica *rep);

/**
 * Free a TCReplica.
 */
void tc_replica_free(struct TCReplica *rep);

/**
 * Create a new TCString referencing the given C string.  The C string must remain valid until
 * after the TCString is freed.  It's typically easiest to ensure this by using a static string.
 */
struct TCString *tc_string_new(const char *cstr);

/**
 * Create a new TCString by cloning the content of the given C string.
 */
struct TCString *tc_string_clone(const char *cstr);

/**
 * Create a new TCString containing the given string with the given length. This allows creation
 * of strings containing embedded NUL characters.  If the given string is not valid UTF-8, this
 * function will return NULL.
 */
struct TCString *tc_string_clone_with_len(const char *buf, size_t len);

/**
 * Get the content of the string as a regular C string.  The given string must not be NULL.  The
 * returned value is NULL if the string contains NUL bytes.  The returned string is valid until
 * the TCString is freed or passed to another TC API function.
 *
 * This function does _not_ take ownership of the TCString.
 */
const char *tc_string_content(struct TCString *tcstring);

/**
 * Get the content of the string as a pointer and length.  The given string must not be NULL.
 * This function can return any string, even one including NUL bytes.  The returned string is
 * valid until the TCString is freed or passed to another TC API function.
 *
 * This function does _not_ take ownership of the TCString.
 */
const char *tc_string_content_with_len(struct TCString *tcstring, size_t *len_out);

/**
 * Free a TCString.
 */
void tc_string_free(struct TCString *string);

/**
 * Get a task's UUID.
 */
struct TCUuid tc_task_get_uuid(const struct TCTask *task);

/**
 * Get a task's status.
 */
enum TCStatus tc_task_get_status(const struct TCTask *task);

/**
 * Get a task's description, or NULL if the task cannot be represented as a C string (e.g., if it
 * contains embedded NUL characters).
 */
struct TCString *tc_task_get_description(const struct TCTask *task);

/**
 * Free a task.
 */
void tc_task_free(struct TCTask *task);

/**
 * Create a new, randomly-generated UUID.
 */
struct TCUuid tc_uuid_new_v4(void);

/**
 * Create a new UUID with the nil value.
 */
struct TCUuid tc_uuid_nil(void);

/**
 * Write the string representation of a TCUuid into the given buffer, which must be
 * at least TC_UUID_STRING_BYTES long.  No NUL terminator is added.
 */
void tc_uuid_to_str(struct TCUuid uuid, char *out);

/**
 * Parse the given value as a UUID.  The value must be exactly TC_UUID_STRING_BYTES long.  Returns
 * false on failure.
 */
bool tc_uuid_from_str(const char *val, struct TCUuid *out);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
