# Object Store Representation

TaskChampion also supports use of a generic key-value store to synchronize replicas.

In this case, the salt used in key derivation is a random 16-byte value, stored
in the object store and retrieved as needed.

The details of the mapping from this protocol to keys and values are private to the implementation.
Other applications should not access the key-value store directly.
