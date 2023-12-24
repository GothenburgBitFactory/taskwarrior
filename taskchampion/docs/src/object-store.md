# Object Store Representation

TaskChampion also supports use of a generic key-value store to synchronize replicas.

In this case, the salt used in key derivation is the SHA256 hash of the string "TaskChampion".

The details of the mapping from this protocol to keys an values are private to the implementation.
Other applications should not access the key-value store directly.
