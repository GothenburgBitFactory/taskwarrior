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
 * # Error Handling
 *
 * When a `tc_replica_..` function that returns a TCResult returns TC_RESULT_ERROR, then
 * `tc_replica_error` will return the error message.
 *
 * # Safety
 *
 * The `*TCReplica` returned from `tc_replica_new…` functions is owned by the caller and
 * must later be freed to avoid a memory leak.
 *
 * Any function taking a `*TCReplica` requires:
 *  - the pointer must not be NUL;
 *  - the pointer must be one previously returned from a tc_… function;
 *  - the memory referenced by the pointer must never be modified by C code; and
 *  - except for `tc_replica_free`, ownership of a `*TCReplica` remains with the caller.
 *
 * Once passed to `tc_replica_free`, a `*TCReplica` becmes invalid and must not be used again.
 *
 * TCReplicas are not threadsafe.
 */
typedef struct TCReplica TCReplica;

/**
 * TCString supports passing strings into and out of the TaskChampion API.
 *
 * # Rust Strings and C Strings
 *
 * A Rust string can contain embedded NUL characters, while C considers such a character to mark
 * the end of a string.  Strings containing embedded NULs cannot be represented as a "C string"
 * and must be accessed using `tc_string_content_and_len` and `tc_string_clone_with_len`.  In
 * general, these two functions should be used for handling arbitrary data, while more convenient
 * forms may be used where embedded NUL characters are impossible, such as in static strings.
 *
 * # UTF-8
 *
 * TaskChampion expects all strings to be valid UTF-8. `tc_string_…` functions will fail if given
 * a `*TCString` containing invalid UTF-8.
 *
 * # Safety
 *
 * When a `*TCString` appears as a return value or output argument, ownership is passed to the
 * caller.  The caller must pass that ownerhsip back to another function or free the string.
 *
 * Any function taking a `*TCReplica` requires:
 *  - the pointer must not be NUL;
 *  - the pointer must be one previously returned from a tc_… function; and
 *  - the memory referenced by the pointer must never be modified by C code.
 *
 * Unless specified otherwise, TaskChampion functions take ownership of a `*TCString` when it is
 * given as a function argument, and the pointer is invalid when the function returns.  Callers
 * must not use or free TCStrings after passing them to such API functions.
 *
 * TCStrings are not threadsafe.
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

/**
 * TCStrings represents a list of strings.
 *
 * The content of this struct must be treated as read-only.
 */
typedef struct TCStrings {
  /**
   * number of strings in items
   */
  size_t len;
  /**
   * total size of items (internal use only)
   */
  size_t _capacity;
  /**
   * TCStrings representing each string. these remain owned by the TCStrings instance and will
   * be freed by tc_strings_free.  This pointer is never NULL for a valid TCStrings, and the
   * *TCStrings at indexes 0..len-1 are not NULL.
   */
  struct TCString *const *items;
} TCStrings;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Create a new TCReplica with an in-memory database.  The contents of the database will be
 * lost when it is freed.
 */
struct TCReplica *tc_replica_new_in_memory(void);

/**
 * Create a new TCReplica with an on-disk database having the given filename. On error, a string
 * is written to the `error_out` parameter (if it is not NULL) and NULL is returned.
 */
struct TCReplica *tc_replica_new_on_disk(struct TCString *path, struct TCString **error_out);

/**
 * Get an existing task by its UUID.
 *
 * Returns NULL when the task does not exist, and on error.  Consult tc_replica_error
 * to distinguish the two conditions.
 */
struct TCTask *tc_replica_get_task(struct TCReplica *rep, struct TCUuid tcuuid);

/**
 * Create a new task.  The task must not already exist.
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
struct TCTask *tc_replica_import_task_with_uuid(struct TCReplica *rep, struct TCUuid tcuuid);

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
 * returned buffer is valid until the TCString is freed or passed to another TaskChampio
 * function.
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
 * Free a TCStrings instance.  The instance, and all TCStrings it contains, must not be used after
 * this call.
 *
 * When this call returns, the `items` pointer will be NULL, signalling an invalid TCStrings.
 */
void tc_strings_free(struct TCStrings *tcstrings);

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
 * Check if a task has the given tag.  If the tag is invalid, this function will return false, as
 * that (invalid) tag is not present. No error will be reported via `tc_task_error`.
 */
bool tc_task_has_tag(struct TCTask *task, struct TCString *tag);

/**
 * Get the tags for the task.
 *
 * The caller must free the returned TCStrings instance.  The TCStrings instance does not
 * reference the task and the two may be freed in any order.
 */
struct TCStrings tc_task_get_tags(struct TCTask *task);

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
 * Remove a tag from a mutable task.
 */
TCResult tc_task_remove_tag(struct TCTask *task, struct TCString *tag);

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
void tc_uuid_to_buf(struct TCUuid tcuuid, char *buf);

/**
 * Write the string representation of a TCUuid into the given buffer, which must be
 * at least TC_UUID_STRING_BYTES long.  No NUL terminator is added.
 */
struct TCString *tc_uuid_to_str(struct TCUuid tcuuid);

/**
 * Parse the given string as a UUID.  Returns false on failure.
 */
bool tc_uuid_from_str(struct TCString *s, struct TCUuid *uuid_out);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
