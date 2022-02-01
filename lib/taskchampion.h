#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/**
 * Length, in bytes, of the string representation of a UUID (without NUL terminator)
 */
#define TC_UUID_STRING_BYTES 36

/**
 * A result from a TC operation.  Typically if this value is TC_RESULT_ERROR,
 * the associated object's `tc_.._error` method will return an error message.
 */
enum TCResult
#ifdef __cplusplus
  : int32_t
#endif // __cplusplus
 {
  TC_RESULT_ERROR = -1,
  TC_RESULT_OK = 0,
};
#ifndef __cplusplus
typedef int32_t TCResult;
#endif // __cplusplus

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
 *
 * When a `tc_replica_..` function that returns a TCResult returns TC_RESULT_ERROR, then
 * `tc_replica_error` will return the error message.
 */
typedef struct TCReplica TCReplica;

/**
 * TCString supports passing strings into and out of the TaskChampion API.  A string can contain
 * embedded NUL characters.  Strings containing such embedded NULs cannot be represented as a "C
 * string" and must be accessed using `tc_string_content_and_len` and `tc_string_clone_with_len`.
 * In general, these two functions should be used for handling arbitrary data, while more
 * convenient forms may be used where embedded NUL characters are impossible, such as in static
 * strings.
 *
 * Rust expects all strings to be UTF-8, and API functions will fail if given a TCString
 * containing invalid UTF-8.
 *
 * Unless specified otherwise, functions in this API take ownership of a TCString when it is given
 * as a function argument, and free the string before returning.  Callers must not use or free
 * strings after passing them to such API functions.
 *
 * When a TCString appears as a return value or output argument, it is the responsibility of the
 * caller to free the string.
 */
typedef struct TCString TCString;

/**
 * A task, as publicly exposed by this library.
 *
 * A task begins in "immutable" mode.  It must be converted to "mutable" mode
 * to make any changes, and doing so requires exclusive access to the replica
 * until the task is freed or converted back to immutable mode.
 *
 * An immutable task carries no reference to the replica that created it, and can be used until it
 * is freed or converted to a TaskMut.  A mutable task carries a reference to the replica and
 * must be freed or made immutable before the replica is freed.
 *
 * All `tc_task_..` functions taking a task as an argument require that it not be NULL.
 *
 * When a `tc_task_..` function that returns a TCResult returns TC_RESULT_ERROR, then
 * `tc_task_error` will return the error message.
 *
 * TCTasks are not threadsafe.
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

/**
 * Create a new TCReplica with an in-memory database.  The contents of the database will be
 * lost when it is freed.
 */
struct TCReplica *tc_replica_new_in_memory(void);

/**
 * Create a new TCReplica with an on-disk database having the given filename. The filename must
 * not be NULL. On error, a string is written to the `error_out` parameter (if it is not NULL) and
 * NULL is returned.
 */
struct TCReplica *tc_replica_new_on_disk(struct TCString *path, struct TCString **error_out);

/**
 * Get an existing task by its UUID.
 *
 * Returns NULL when the task does not exist, and on error.  Consult tc_replica_error
 * to distinguish the two conditions.
 */
struct TCTask *tc_replica_get_task(struct TCReplica *rep, struct TCUuid uuid);

/**
 * Create a new task.  The task must not already exist.
 *
 * The description must not be NULL.
 *
 * Returns the task, or NULL on error.
 */
struct TCTask *tc_replica_new_task(struct TCReplica *rep,
                                   enum TCStatus status,
                                   struct TCString *description);

/**
 * Create a new task.  The task must not already exist.
 *
 * Returns the task, or NULL on error.
 */
struct TCTask *tc_replica_import_task_with_uuid(struct TCReplica *rep, struct TCUuid uuid);

/**
 * Undo local operations until the most recent UndoPoint.
 *
 * If undone_out is not NULL, then on success it is set to 1 if operations were undone, or 0 if
 * there are no operations that can be done.
 */
TCResult tc_replica_undo(struct TCReplica *rep, int32_t *undone_out);

/**
 * Get the latest error for a replica, or NULL if the last operation succeeded.  Subsequent calls
 * to this function will return NULL.  The rep pointer must not be NULL.  The caller must free the
 * returned string.
 */
struct TCString *tc_replica_error(struct TCReplica *rep);

/**
 * Free a replica.  The replica may not be used after this function returns and must not be freed
 * more than once.
 */
void tc_replica_free(struct TCReplica *rep);

/**
 * Create a new TCString referencing the given C string.  The C string must remain valid and
 * unchanged until after the TCString is freed.  It's typically easiest to ensure this by using a
 * static string.
 *
 * NOTE: this function does _not_ take responsibility for freeing the given C string.  The
 * given string can be freed once the TCString referencing it has been freed.
 *
 * For example:
 *
 * ```
 * char *url = get_item_url(..); // dynamically allocate C string
 * tc_task_annotate(task, tc_string_borrow(url)); // TCString created, passed, and freed
 * free(url); // string is no longer referenced and can be freed
 * ```
 */
struct TCString *tc_string_borrow(const char *cstr);

/**
 * Create a new TCString by cloning the content of the given C string.  The resulting TCString
 * is independent of the given string, which can be freed or overwritten immediately.
 */
struct TCString *tc_string_clone(const char *cstr);

/**
 * Create a new TCString containing the given string with the given length. This allows creation
 * of strings containing embedded NUL characters.  As with `tc_string_clone`, the resulting
 * TCString is independent of the passed buffer, which may be reused or freed immediately.
 *
 * The given length must be less than half the maximum value of usize.
 */
struct TCString *tc_string_clone_with_len(const char *buf, size_t len);

/**
 * Get the content of the string as a regular C string.  The given string must not be NULL.  The
 * returned value is NULL if the string contains NUL bytes or (in some cases) invalid UTF-8.  The
 * returned C string is valid until the TCString is freed or passed to another TC API function.
 *
 * In general, prefer [`tc_string_content_with_len`] except when it's certain that the string is
 * valid and NUL-free.
 *
 * This function does _not_ take ownership of the TCString.
 */
const char *tc_string_content(struct TCString *tcstring);

/**
 * Get the content of the string as a pointer and length.  The given string must not be NULL.
 * This function can return any string, even one including NUL bytes or invalid UTF-8.  The
 * returned buffer is valid until the TCString is freed or passed to another TC API function.
 *
 * This function does _not_ take ownership of the TCString.
 */
const char *tc_string_content_with_len(struct TCString *tcstring, size_t *len_out);

/**
 * Free a TCString.  The given string must not be NULL.  The string must not be used
 * after this function returns, and must not be freed more than once.
 */
void tc_string_free(struct TCString *tcstring);

/**
 * Convert an immutable task into a mutable task.
 *
 * The task must not be NULL. It is modified in-place, and becomes mutable.
 *
 * The replica must not be NULL. After this function returns, the replica _cannot be used at all_
 * until this task is made immutable again.  This implies that it is not allowed for more than one
 * task associated with a replica to be mutable at any time.
 *
 * Typical mutation of tasks is bracketed with `tc_task_to_mut` and `tc_task_to_immut`:
 *
 * ```c
 * tc_task_to_mut(task, rep);
 * success = tc_task_done(task);
 * tc_task_to_immut(task, rep);
 * if (!success) { ... }
 * ```
 */
void tc_task_to_mut(struct TCTask *task, struct TCReplica *tcreplica);

/**
 * Convert a mutable task into an immutable task.
 *
 * The task must not be NULL.  It is modified in-place, and becomes immutable.
 *
 * The replica passed to `tc_task_to_mut` may be used freely after this call.
 */
void tc_task_to_immut(struct TCTask *task);

/**
 * Get a task's UUID.
 */
struct TCUuid tc_task_get_uuid(struct TCTask *task);

/**
 * Get a task's status.
 */
enum TCStatus tc_task_get_status(struct TCTask *task);

/**
 * Get a task's description, or NULL if the task cannot be represented as a C string (e.g., if it
 * contains embedded NUL characters).
 */
struct TCString *tc_task_get_description(struct TCTask *task);

/**
 * Get the entry timestamp for a task (when it was created), or 0 if not set.
 */
time_t tc_task_get_entry(struct TCTask *task);

/**
 * Get the wait timestamp for a task, or 0 if not set.
 */
time_t tc_task_get_wait(struct TCTask *task);

/**
 * Get the modified timestamp for a task, or 0 if not set.
 */
time_t tc_task_get_modified(struct TCTask *task);

/**
 * Check if a task is waiting.
 */
bool tc_task_is_waiting(struct TCTask *task);

/**
 * Check if a task is active (started and not stopped).
 */
bool tc_task_is_active(struct TCTask *task);

/**
 * Check if a task has the given tag.  If the tag is invalid, this function will simply return
 * false with no error from `tc_task_error`.  The given tag must not be NULL.
 */
bool tc_task_has_tag(struct TCTask *task, struct TCString *tag);

/**
 * Set a mutable task's status.
 */
TCResult tc_task_set_status(struct TCTask *task, enum TCStatus status);

/**
 * Set a mutable task's description.
 */
TCResult tc_task_set_description(struct TCTask *task, struct TCString *description);

/**
 * Set a mutable task's entry (creation time).  Pass entry=0 to unset
 * the entry field.
 */
TCResult tc_task_set_entry(struct TCTask *task, time_t entry);

/**
 * Set a mutable task's wait timestamp.  Pass wait=0 to unset the wait field.
 */
TCResult tc_task_set_wait(struct TCTask *task, time_t wait);

/**
 * Set a mutable task's modified timestamp.  The value cannot be zero.
 */
TCResult tc_task_set_modified(struct TCTask *task, time_t modified);

/**
 * Start a task.
 */
TCResult tc_task_start(struct TCTask *task);

/**
 * Stop a task.
 */
TCResult tc_task_stop(struct TCTask *task);

/**
 * Mark a task as done.
 */
TCResult tc_task_done(struct TCTask *task);

/**
 * Mark a task as deleted.
 */
TCResult tc_task_delete(struct TCTask *task);

/**
 * Add a tag to a mutable task.
 */
TCResult tc_task_add_tag(struct TCTask *task, struct TCString *tag);

/**
 * Get the latest error for a task, or NULL if the last operation succeeded.  Subsequent calls
 * to this function will return NULL.  The task pointer must not be NULL.  The caller must free the
 * returned string.
 */
struct TCString *tc_task_error(struct TCTask *task);

/**
 * Free a task.  The given task must not be NULL.  The task must not be used after this function
 * returns, and must not be freed more than once.
 *
 * If the task is currently mutable, it will first be made immutable.
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
void tc_uuid_to_buf(struct TCUuid uuid, char *buf);

/**
 * Write the string representation of a TCUuid into the given buffer, which must be
 * at least TC_UUID_STRING_BYTES long.  No NUL terminator is added.
 */
struct TCString *tc_uuid_to_str(struct TCUuid uuid);

/**
 * Parse the given string as a UUID.  The string must not be NULL.  Returns false on failure.
 */
bool tc_uuid_from_str(struct TCString *s, struct TCUuid *uuid_out);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
