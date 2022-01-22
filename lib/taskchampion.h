#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

struct StoragePtr;

extern "C" {

StoragePtr *storage_new_in_memory();

void storage_free(StoragePtr *storage);

} // extern "C"
