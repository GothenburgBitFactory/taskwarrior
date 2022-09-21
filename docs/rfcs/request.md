---
title: "Taskwarrior - Request"
---

# Taskserver Message Format

The Taskserver accepts and emits only messages.
These messages look somewhat like email, as defined in [RFC821](https://tools.ietf.org/html/rfc821), [RFC2822](https://tools.ietf.org/html/rfc2822).

The message format allows for data, metadata, and extensibility.
This combination allows the Taskserver to accommodate current and future needs.
This document describes the message format, and the supported message types.

## Requirements

In this document, we adopt the convention discussed in Section 1.3.2 of [RFC1122](https://tools.ietf.org/html/rfc1122#page-16) of using the capitalized words MUST, REQUIRED, SHOULD, RECOMMENDED, MAY, and OPTIONAL to define the significance of each particular requirement specified in this document.

In brief: "MUST" (or "REQUIRED") means that the item is an absolute requirement of the specification; "SHOULD" (or "RECOMMENDED") means there may exist valid reasons for ignoring this item, but the full implications should be understood before doing so; and "MAY" (or "OPTIONAL") means that this item is optional, and may be omitted without careful consideration.

## Encoding

All messages are UTF8-encoded text.

## Message Format

This format is based on [RFC2822](https://tools.ietf.org/html/rfc2822), 'Internet Message Format'.
Here is an example of the format:

    <SIZE>
    name: value
    name2: value2

    payload

There are three sections.
The first is the size, which is a 4-byte, big- Endian, binary byte count of the length of the message, including the 4 bytes for the size.

The header section is a set of name/value pairs separated by newline characters (U+000D).
The name is separated from the value by ': ' (colon U+003A, space U+0020) The header section is terminated by two consecutive newline (U+000D) characters.
All text is UTF8-encoded.

The payload section is arbitrary, and message type-specific.
However, it is still UTF8-encoded text.


## Message Requirements

Messages SHALL contain particular headers.
Those are:

-   type
-   protocol
-   client

The 'type' value is what determines the interpretation of the payload.

The 'protocol' value should be 'v1', or any subsequently published protocol version.

The 'client' represent the client identifier, so that any special cases can be handled.
For example, an emergency fix that is client version-specific could be released, to support users that have not updated their client, or perhaps the client has not released a fix.
The form of the 'version' value is:

    <product identifier> <version number>

As an example:

    taskwarrior 2.3.0

DO NOT spoof any other software using this client value.
If another client is spoofed, then patches addressing protocol errors may break working software.


## Auth Data

Every request from the client SHALL contain "auth" information, which involves these header entries:

    org: <organization>
    user: <user>
    key: <key>

The user and org fields uniquely identify a user.

The key field is generated when a new server account is set up.
It is a shared secret, equivalent to a password, and should be protected.

Authentication failure can result in these errors:

-   430 Authentication failed
-   431 Account suspended


## Status Data

Every response from the Taskserver SHALL contain status data:

    code: <code>
    status: <status text>

The code is a numeric status indicator defined in the [Sync Protocol](/docs/design/protocol).


## Payload Data

Payload data is optional, arbitrary and message type dependent.
It is always UTF8-encoded text.


## Message Types

The Taskserver supports several message types, thus providing a set of primitives for use by clients.

It is expected that the number of supported ticket types will increase over time.


## Sync Message

The "sync" message always originates from the client, but the response will contain data from the server.
A sync is therefore a single request with a single response.

The "sync" message type MUST contain the following headers:

-   type
-   org
-   user
-   key
-   client
-   protocol

The "sync" message payload has this format:

    <uuid>
    <JSON task 1>
    <JSON task 2>
    ...
    <JSON task N>

Here is an example of a sync message:

    <size>type: sync
    org: <organization>
    user: <user>
    key: <key>
    client: task 2.3.0
    protocol: v1

    2e4685f8-34bc-4f9b-b7ed-399388e182e1
    {"description":"Test data","entry":"20130602T002341Z","status":"pending"}

The request contains the proper auth section, and the body contains the current sync key followed by a newline characters (U+000D), then a list of JSON-formatted tasks \[2\] each separated by a newline character (U+000D).

An example response message might be:

    <size>type: response
    client: taskd 1.0.0
    protocol: v1
    code: 200
    status: Ok

    45da7110-1bcc-4318-d33e-12267a774e0f

The status indicates success, and the payload contains zero remote task modifications, followed by a sync key.


## Statistics Message

The message format іs simply:

    <size>type: statistics
    org: <Organization>
    user: <User>
    key: <Key>
    client: taskd 1.0.0
    protocol: v1

There is no payload.
An example response message might be:

    <size>type: response
    client: taskd 1.0.0
    protocol: v1
    code: 200
    status: Ok
    average request bytes: 0
    average response bytes: 0
    average response time: 0.000000
    errors: 0
    idle: 1.000000
    maximum response time: 0.000000
    total bytes in: 0
    total bytes out: 0
    tps: 0.000000
    transactions: 1
    uptime: 28

There is no payload, and the results are in the header variables.

Note that the statistics gathered by the server are growing, which means new values are occasionally added to the response message.
Existing values will not be removed.
