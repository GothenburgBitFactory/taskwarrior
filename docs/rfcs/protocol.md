---
title: "Taskwarrior - Sync Protocol"
---


# Sync Protocol


## Introduction

Taskwarrior data has typically been shared in several ways.
Those include SCM (source code management) systems, directory synchronizing software (such as DropBox), and by use of the 'push', 'pull' and 'merge' commands introduced in version 1.9.3.

While these methods work, they each have problems associated with the merging of data.
In the case of directory synchronizing software, there is no merging at all - just simple file overwrite, despite many people believing that the data is somehow combined and preserved.

The Taskserver is a solution.
It is an online/cloud storage and sync service for taskwarrior data.
It performs conflict-free data merging, and minimizes bandwidth use.

The Taskserver also provides multi-client access, so that a task added using a web client could be immediately viewed using a mobile client, or modified using taskwarrior.
Choice of clients is important - people have widely varying behaviors and tastes.

The Taskserver also provides multi-user access, which introduces new capabilities, such as list sharing and delegation.
These will require later modification to this protocol.

The Taskserver protocol will be implemented by the taskd project and first used in taskwarrior 2.3.0.
Other clients will follow.


## Requirements

In this document, we adopt the convention discussed in Section 1.3.2 of [RFC1122](https://tools.ietf.org/html/rfc1122#page-16) of using the capitalized words MUST, REQUIRED, SHOULD, RECOMMENDED, MAY, and OPTIONAL to define the significance of each particular requirement specified in this document.

In brief: "MUST" (or "REQUIRED") means that the item is an absolute requirement of the specification; "SHOULD" (or "RECOMMENDED") means there may exist valid reasons for ignoring this item, but the full implications should be understood before doing so; and "MAY" (or "OPTIONAL") means that this item is optional, and may be omitted without careful consideration.


## Link Level

The Taskserver protocol assumes a reliable data stream such as provided by TCP.
When TCP is used, a Taskserver listens on a single predetermined port *for the given client* only.
This means the server may be using multiple ports to serve distinct sets of clients.

This server is only an interface between programs and the task data.
It does not perform any user interaction or presentation-level functions.


## Transactions

Each transaction is a single incoming message, with a single response message.
All communication therefore consists of a single 'send', followed by a single 'receive', then termination.
There are no sessions, and no continuously open connections.
The message format is described in the [Taskserver Message Format](/docs/design/request) document.


## Responsibilities of the Server

The server will maintain a set of transactions, in the original sequence, punctuated by sync keys which are UUIDs.
Each sync key represents a non- trivial sync operation by a client.
Each transaction is a [JSON-formatted task](/docs/design/task), followed by a newline (\\n) character.
The result is a single file that contains interleaved lines of two types: tasks and sync keys.

This design allows the server to maintain a set of deltas such that multiple clients may request a minimal set of changes since their last sync.


## Responsibilities of the Client

This describes how Taskwarrior implements sync.

All modifications to tasks (add, modify, done, delete \...) are recorded in the form of a fully-composed [JSON-formatted task](/docs/design/task).
The formatted task is added to a local backlog.data file.
If a task is modified a second time, it is added again to the backlog.data file - the lines are not combined.
Each task SHALL have a 'modified' date attribute that will help resolve conflicts.

On sync:

*   Send a 'sync' type message with the entire contents of the backlog.data, unmodified, as the message payload.

*   Receive one of the following response codes:

  *   201: This means 'no change', and there is no further action to be taken.

  *   200: This means 'success', and the message payload contains a set of tasks and a sync key:

    *   The formatted tasks are to be stored as-is.
        These tasks will either be appended to the client data or will overwrite existing client data, based on the UUID of the task.
        No merge logic is necessary.

    *   The sync key will be written to the backlog.data file, overwriting the previous contents, such that it will now contain only one line.

    *   301: Redirect to : found in the 'info' response header, will force the client to resubmit the request to the new server.

    *   3xx, 4xx, 5xx: The 'status' field contains an error message.

*   If the response contained any error or warning, the error should be shown to the user.
    This provides an opportunity for the server to announce downtime, or relocation.

If no sync key is sent, the server cannot provide an incremental delta, and so will send all task data, which should be stored as above.
This should be the case for a client making its first sync call.

If an unrecognized attribute is present in the task data, the client MUST preserve the attribute unmodified, and assume it is of type 'string'.
This permits individual clients to augment the task data without other clients stripping it meaningful data.
This is how UDAs (user defined attributes) are handled.


## Extensions

This protocol was designed so that extensions to the protocol will take the form of additional message types and status codes.


## Summary of Response Codes

Status responses indicate the server's response to the last command received from the client.
The codes consist of a 3 digit numeric code.

The first digit of the response broadly indicates the success, failure, or progress of the previous command (based generally on [RFC640](https://tools.ietf.org/html/rfc640) [RFC821](https://tools.ietf.org/html/rfc821)):

| 1yz | Positive Preliminary reply           |
| 2yz | Positive Completion reply            |
| 3yz | Positive Intermediate reply          |
| 4yz | Transient Negative Completion reply  |
| 5yz | Permanent Negative Completion reply  |

The next digit in the code indicates the response category:

| x0z |  Syntax                                           |
| x1z |  Information (e.g., help)                         |
| x2z |  Connections                                      |
| x3z |  Authentication                                   |
| x4z |  Unspecified as yet                               |
| x5z |  Taskd System (\...)                              |
| x8z |  Nonstandard (private implementation) extensions  |

A summary of all status response are:

| 200 | Success   |
| 201 | No change |
| 300 | Deprecated message type. This message will not be supported in future Taskserver releases.                                                   |
| 301 | Redirect. Further requests should be made to the specified server/port.                                                                      |
| 302 | Retry. The client is requested to wait and retry the same request. The wait time is not specified, and further retry responses are possible. |
| 400 | Malformed data |
| 401 | Unsupported encoding |
| 420 | Server temporarily unavailable |
| 421 | Server shutting down at operator request |
| 430 | Access denied |
| 431 | Account suspended |
| 432 | Account terminated |
| 500 | Syntax error in request |
| 501 | Syntax error, illegal parameters |
| 502 | Not implemented |
| 503 | Command parameter not implemented |
| 504 | Request too big |

## Security Considerations

All communication with the Taskserver uses SSL 3.0 or TLS 1.0, 1.1 or 1.2.
Encryption is mandatory.
Data is never transmitted in plain text.


## Limitations and Guidelines

Some limitations exists to reduce bandwidth and load on the server.
They are:

-   A client may only connect to a single server.
    Synchronization among a set of servers is not supported.

-   A client should attempt to minimize data bandwidth usage by maintaining a local data store, and properly using sync keys.

-   A client should minimize data transfers by limiting the frequency of sync requests.
