/**
 * TaskChampion
 *
 * This file defines the C interface to libtaskchampion.  This is a thin
 * wrapper around the Rust `taskchampion` crate.  Refer to the documentation
 * for that crate at https://docs.rs/taskchampion/latest/taskchampion/ for API
 * details.  The comments in this file focus mostly on the low-level details of
 * passing values to and from TaskChampion.
 *
 * # Overview
 *
 * This library defines two major types used to interact with the API, that map directly
 * to Rust types.
 *
 *  * TCReplica - see https://docs.rs/taskchampion/latest/taskchampion/struct.Replica.html
 *  * TCTask - see https://docs.rs/taskchampion/latest/taskchampion/struct.Task.html
 *  * TCServer - see https://docs.rs/taskchampion/latest/taskchampion/trait.Server.html
 *  * TCWorkingSet - see https://docs.rs/taskchampion/latest/taskchampion/struct.WorkingSet.html
 *
 * It also defines a few utility types:
 *
 *  * TCString - a wrapper around both C (NUL-terminated) and Rust (always utf-8) strings.
 *  * TC…List - a list of objects represented as a C array
 *  * see below for the remainder
 *
 * # Safety
 *
 * Each type contains specific instructions to ensure memory safety.
 * The general rules are as follows.
 *
 * No types in this library are threadsafe.  All values should be used in only
 * one thread for their entire lifetime.  It is safe to use unrelated values in
 * different threads (for example, different threads may use different
 * TCReplica values concurrently).
 *
 * ## Pass by Pointer
 *
 * Several types such as TCReplica and TCString are "opaque" types and always
 * handled as pointers in C. The bytes these pointers address are private to
 * the Rust implemetation and must not be accessed from C.
 *
 * Pass-by-pointer values have exactly one owner, and that owner is responsible
 * for freeing the value (using a `tc_…_free` function), or transferring
 * ownership elsewhere.  Except where documented otherwise, when a value is
 * passed to C, ownership passes to C as well.  When a value is passed to Rust,
 * ownership stays with the C code.  The exception is TCString, ownership of
 * which passes to Rust when it is used as a function argument.
 *
 * The limited circumstances where one value must not outlive another, due to
 * pointer references between them, are documented below.
 *
 * ## Pass by Value
 *
 * Types such as TCUuid and TC…List are passed by value, and contain fields
 * that are accessible from C.  C code is free to access the content of these
 * types in a _read_only_ fashion.
 *
 * Pass-by-value values that contain pointers also have exactly one owner,
 * responsible for freeing the value or transferring ownership.  The tc_…_free
 * functions for these types will replace the pointers with NULL to guard
 * against use-after-free errors.  The interior pointers in such values should
 * never be freed directly (for example, `tc_string_free(tcuda.value)` is an
 * error).
 *
 * TCUuid is a special case, because it does not contain pointers.  It can be
 * freely copied and need not be freed.
 *
 * ## Lists
 *
 * Lists are a special kind of pass-by-value type.  Each contains `len` and
 * `items`, where `items` is an array of length `len`.  Lists, and the values
 * in the `items` array, must be treated as read-only.  On return from an API
 * function, a list's ownership is with the C caller, which must eventually
 * free the list.  List data must be freed with the `tc_…_list_free` function.
 * It is an error to free any value in the `items` array of a list.
 */
