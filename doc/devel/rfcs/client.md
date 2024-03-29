---
title: "Taskwarrior - Creating a Taskserver Client"
---


# Creating a Taskserver Client

A Taskserver client is a todo-list manager.
It may be as simple as a program that captures a single task, as complex as Taskwarrior, or anything in between.
It can be a mobile client, a web application, or any other type of program.

This document describes how such a client would interact with the server.

A client to the Taskserver is a program that manages a task list, and wishes to exchange data with the server so that the task list may be shared.

In order to do this, a client must store tasks locally, upload local changes, download remote changes, and apply remote changes to the local tasks.

The client must consider that there may be no network connectivity, or no desire by the user to synchronize.

The client will need proper credentials to talk to the server.

## Requirements

In this document, we adopt the convention discussed in Section 1.3.2 of [RFC1122](https://tools.ietf.org/html/rfc1122#page-16) of using the capitalized words MUST, REQUIRED, SHOULD, RECOMMENDED, MAY, and OPTIONAL to define the significance of each particular requirement specified in this document.

In brief: "MUST" (or "REQUIRED") means that the item is an absolute requirement of the specification; "SHOULD" (or "RECOMMENDED") means there may exist valid reasons for ignoring this item, but the full implications should be understood before doing so; and "MAY" (or "OPTIONAL") means that this item is optional, and may be omitted without careful consideration.


## Taskserver Account

A Taskserver account must be created.
This process creates a storage area, and generates the necessary credentials.


## Credentials

A Taskserver client needs the following credentials in order to communicate with a server:

-   Server address and port
-   Organization name
-   User name
-   Password
-   Certificate
-   Key

The server address and port are the network location of the server.
An example of this value is:

    foo.example.com:53589

In addition to a DNS name, this can be an IPv4 or IPv6 address.

The organization name is an arbitrary grouping, and is typically 'PUBLIC', reflecting the individual nature of server accounts.
Future capabilities will provide functionality that support groups of users, called an organization.

The user name is the full name.
This will be the name used to identify other users in an organization, in a future release.
Example 'John Doe'.

The password is a text string generated by the server at account creation time.
It should be considered a secret.

The certificate is an X.509 PEM file generated by the server at account creation time.
This is used for authentication.
It should be considered a secret.

The key is an X.509 PEM file generated by the server at account creation time.
This is used for encryption.
It should be considered a secret.

These credentials need to be stored on the client, and used during the sync operation.


## Description of a Taskserver Client

This section describes how a client might behave in order to facilitate integration with the Taskserver.


## Encryption

The Taskserver only communicates using encryption.
Therefore all user data is encrypted while in transit.
The Taskserver currently uses [GnuTLS](https://gnutls.org) to support this encryption, and therefore supports the following protocols:

-   SSL 3.0
-   TLS 1.0
-   TLS 1.1
-   TLS 1.2

The client may use any library that supports the above.


## Configuration

The client needs to store configuration, which matches the credentials needed for Taskserver communication.
See section 2.1 "Credentials".

The credentials may not be modified by the user without losing server access.

The server:port data may need to be changed automatically following a redirect response from the server.
See section 5 "Server Errors".


## Local Storage

The client needs to store task data locally.
The client will need to be able to find tasks by their UUID and overwrite them.
Uploaded and downloaded task changes will use the [Taskwarrior Data Interchange Format](/docs/design/task).

## Local Changes

Whenever local data is modified, that change MUST be synced with the server.
But this does not have to occur immediately, in fact the client SHOULD NOT assume connectivity at any time.

A client SHOULD NOT also assume that the server is available.
If the server is not available, the local changes should be retained, and the sync operation repeated later.

Ideally the client will give the user full control over sync operations.
Automatically syncing after all local modifications is not recommended.
If a client performs too many sync operations, the server MAY revoke the certificate.

Effectively, the client should maintain a separate list of tasks changed since the last successful sync operation.

Note that tasks have a "modified" attribute, which should be updated whenever a change is made.
This attribute contributes to conflict resolution on the server.


## Remote Changes

When a server sends remote changes to a client, in the response to a sync request, the changes have already been merged by the server, and therefore the client should simply store them intact.

Based on the UUID in the task, the client can determine whether a task is new (and should be added to the local list of tasks), or whether it represents a modification (and should overwrite it's existing entry).

The client MUST NOT perform any merges.


## Sync Key

Whenever a sync is performed, the server responds by sending a sync key and any remote changes.
The sync key is important, and should be included in the next sync request.
The client is REQUIRED to store the sync key in every server response message.

If a client omits the sync key in a sync message, the response will be a complete set of all tasks and modifications.


## Data Integrity

Although a task is guaranteed to contain at least 'entry', 'description' and 'uuid' attributes, it may also contain other known fields, and unknown user-defined fields.
An example might be an attribute named 'estimate'.

If a task is received via sync that contains an attribute named 'estimate', then a client has the responsibility of preserving the attribute intact.
If that data is shown, then it is assumed to be of type 'string', which is the format used by JSON for all values.

Conversely, if a client wishes to add a custom attribute, it is guaranteed that the server and other clients will preserve that attribute.

Using this rule, two clients of differing capabilities can exchange data and still maintain custom attributes.

This is a requirement.
Any client that does not obey this requirement is broken.


## Synchronizing

Synchronizing with the Taskserver consists of a single transaction.
Once an encrypted connection is made with the server, the client MUST compose a [sync request message](/docs/design/request).
This message includes credentials and local changes.
The response message contains status and remote changes, which MUST be stored locally.


## Establishing Encrypted Connection

All communication with the Taskserver is encrypted using the certificate and key provided to each user.
Using the 'server' configuration setting, establish a connection.


## Sync Request

See [sync request message](/docs/design/request).
A sync request MUST contain a sync key if one was provided by a previous sync.
A sync request MUST contain a list of modified tasks, in JSON format (see [Task JSON](/docs/design/task)), if local modifications have been made.


## Sync Response

A sync response WILL contain a 'code' and 'status' header variable, WILL contain a sync key in the payload, and MAY contain a list of tasks from the server in JSON format (see [Task JSON](/docs/design/task)).


## Server Messages

There are cases when the server needs to inform the user of some condition.
This may be anticipated server downtime, for example.
The response message is typically not present, but may be present in the header, containing a string:

    ...
    message: Scheduled maintenance 2013-07-14 08:00UTC for 10 minutes.
    ...

If such a message is returned by the server, it SHOULD be made available to the user.
This is a recommendation, not a requirement.


## Server Errors

The server may generate many errors (See [Protocol](/docs/design/protocol)), but the following is a list of the ones most in need of special handling:

-   200 Success
-   201 No change
-   301 Redirect
-   430 Access denied
-   431 Account suspended
-   432 Account terminated
-   5xx Error

The 200 indicates success, and that a change was recorded.
The 201 indicates success but no changes were necessary.
The 301 is a redirect message indicating that the client MUST re-request from a new server.
The 43x series messages are account-related.
Any 5xx series code is a server error of some kind.
All errors consist of a code and a status message:

    code: 200
    status: Success


## Examples

Here are examples of properly formatted request and response messages.
Note that the messages are indented for clarity in this document, but is not the case in a properly formatted message.
Also note that newline characters U+000D are not shown, but are implied by the separate lines.
Because some messages have trailing newline characters, the text is delimited by the 'cut' markers:

    foo

The example above illustrates text consisting of:

    U+0066   f
    U+006F   o
    U+006F   o
    U+000D   newline
    U+000D   newline

Note that these values are left unspecified, but should be clear from the context, and the [message format](/docs/design/request) spec:

    <size>
    <organization>
    <user>
    <password>


## First Sync

The first time a client syncs, there is (perhaps) no data to upload, and no sync key from a previous sync.

    <size>type: sync
    org: <organization>
    user: <user>
    key: <password>
    client: task 2.3.0
    protocol: v1

Note the double newline character separating header from payload, with an empty payload.

## Request: Sync No Data

Ordinarily when a client syncs, there is a sync key from the previous sync response to send.
This example shows a sync with no local changes, but a sync key from a previous sync.

    <size>type: sync
    org: <organization>
    user: <user>
    key: <password>
    client: task 2.3.0
    protocol: v1

    2e4685f8-34bc-4f9b-b7ed-399388e182e1


## Request: Sync Data

This sync request shows a sync key from the previous sync, and a locally modified task.

    <size>type: sync
    org: <organization>
    user: <user>
    key: <password>
    client: task 2.3.0
    protocol: v1

    2e4685f8-34bc-4f9b-b7ed-399388e182e1
    {"description":"An example","uuid":"8ad2e3db-914d-4832-b0e6-72fa04f6e331",...}


## Response: No Data

If a sync results in no downloads to the client, the response will look like this.

    <size>type: response
    client: taskd 1.0.0
    protocol: v1
    code: 200
    status: Ok

    45da7110-1bcc-4318-d33e-12267a774e0f

Note that there is a sync key which must be stored and used in the next sync request, but there are no remote changes to store.


## Response: Remote Data

This shows a sync response providing a new sync key, and a remote change to two tasks.

    <size>type: response
    client: taskd 1.0.0
    protocol: v1
    code: 200
    status: Ok

    45da7110-1bcc-4318-d33e-12267a774e0f
    {"description":"Test data","uuid":"8ad2e3db-914d-4832-b0e6-72fa04f6e331",...}
    {"description":"Test data2","uuid":"3b6218f9-726a-44fc-aa63-889ff52be442",...}

Note that the sync key must be stored for the next sync request.

Note that the two changed tasks must be stored locally, and if the UUID in the tasks matches local tasks, then the local tasks must be overwritten.


## Response: Error

    <size>type: response
    client: taskd 1.0.0
    protocol: v1
    code: 431
    status: Account suspended

Note the double newline character separating header from payload, with an empty payload.


## Response: Relocate

    <size>type: response
    client: taskd 1.0.0
    protocol: v1
    code: 301
    status: Redirect
    info:

Note the 'info' field will contain a ':' string that should be used for all future sync requests.
This indicates that a user account was moved to another server.

Note the double newline character separating header from payload, with an empty payload.


## Response: Message

Occasionally the server will need to convey a message, and will include an additional header variable containing that message.

The server [protocol](/docs/design/protocol) states that the message SHOULD be shown to the user.
This message will be used for system event messages, used rarely, and never used for advertising or promotion.

    <size>type: response
    client: taskd 1.0.0
    protocol: v1
    code: 200
    status: Ok
    message: Scheduled maintenance 2013-07-14 08:00UTC for 10 minutes.

    45da7110-1bcc-4318-d33e-12267a774e0f

Note that the same message will likely be included in consecutive responses.


## Reference Implementation

The Taskserver 1.1.0 codebase contains a reference implementation of an SSL/TLS client and server program, which communicate text strings.

    taskd.git/src/tls/Makefile             # To build the example
    taskd.git/src/tls/README               # How to run the example
    taskd.git/src/tls/TLSClient.cpp        # TLS client code
    taskd.git/src/tls/TLSClient.h
    taskd.git/src/tls/TLSServer.cpp        # TLS Server code
    taskd.git/src/tls/TLSServer.h
    taskd.git/src/tls/c.cpp                # Client program
    taskd.git/src/tls/s.cpp                # Server program
    taskd.git/src/tls/text.cpp             # Text manipulation
    taskd.git/src/tls/text.h               # Text manipulation

The Taskwarrior codebase, version 2.4.0, is the reference implementation.

    task.git/src/TLSClient.cpp             # TLS client code
    task.git/src/TLSClient.h
    task.git/src/commands/CmdSync.cpp      # Sync implementation
    task.git/src/commands/CmdSync.h
