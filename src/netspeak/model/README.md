# `netspeak::model`

This namespace defines basic internal types that are used everywhere. All types are Netspeak specific and used across classes. These types are how our different classes communicate.


## Queries

The namespace defines 3 types of queries.

Note: For all 3 types, you directly print a whole query, one unit, or just a tag to an output stream via the `<<` operator. The string representation is guaranteed to be deterministic and unique (no two different things will have the same string representation). These print operators are mostly intended for debugging and testing purposes.

### `Query`

This is a read-only literal AST representation of a parsed query. Every unit directly corresponds with a token (or list of tokens) in the query string.

This representation is necessary to, so each unit in other query types can be traced back to a specific query unit. This traceability is important because for each phrase returned by Netspeak, it will also include which query units lead to the phrase being part of the result. This is one of the defining characteristics of Netspeak and necessary to understand the result of queries.

### `NormQuery`

A norm query is the simplest type of queries. It's a query that can be answered directly by a Netspeak index without any further processing and the be thought of the "native language" of Netspeak.

### `SimpleQuery`

A simple query is a mutable intermediate format between `Query` and `NormQuery`. It is logically equivalent to `Query` but cannot literally represent every `Query` object (equally powerful but less verbose).

This format is mainly used during the conversion from `Query` to a list of `NormQuery`s.
