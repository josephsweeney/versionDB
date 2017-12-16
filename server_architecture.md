
# Server Architecture

The server listens on a Unix socket for new connections, as we get a new connection, we spawn a detached worker thread that takes care of the requests from this client.


## Server Protocol

For this protocol, a simple plaintext option was chosen to keep complexity to a minimum. The requirements it must meet are simple so the protocol itself should be simple.

There are 6 types of requests:
```
WRITE,
READ,
READ_TIME,
LS,
HISTORY,
EXIT
```

Once a connection is established we send over our request with individual fields separated by commas:

1. "`TYPE:{type}`" where `type` is one of our types above.
2. "`ID:{id}`" where `id` is the id of the data we are tracking. (NOTE: Currently an arbitrary bound of 1024 characters is placed on `id`, if more than that are sent, we consider the request bad.)
3. If the type is `WRITE` we also send over `BYTES:{number of bytes}`. We then send that many bytes to be stored.
4. If the type is `READ_TIME` we send over `TIME:{time}` where `time` is the number of milliseconds from the Unix epoch in UTC.
5. Optionally, if you send a `READ` request, you can replace the `ID:` field with a `HASH:{hash}` where `hash` is the hash of the data you want to retrieve.

Here are a few examples of valid requests:

`TYPE:READ,ID:great_data` 

This grabs the current version of "great_data".

`TYPE:WRITE,ID:my_data,BYTES:1024` 

This would be followed by 1024 bytes that you want stored.

`TYPE:READ_TIME,ID:great_data,TIME:1356048000000`

This would get you what the state of "great_data" was on Dec 21 2012 00:00:00 UTC or the original state of the data if it was added after that time.

`TYPE:LS`

Returns a list of all the ids we are storing.

`TYPE:HISTORY,ID:other_data`

Returns the history of `other_data`, which is pairs of hashes and times for each version.

`TYPE:READ,HASH:5063c3825063c3865063c38a5063c38e5063c392`

Returns the data associated with this hash.

NOTE: We are also making the assumption that these messages are null terminated strings.


## Future

There are a couple of extra features that are planned but haven't been implemented.

A `READ_ALL` feature is planned that will give a start time and end time, and return all the versions of the data within that time.

A `DELETE` feature is also planned that will delete all data associated with an id, within whatever time range is given.


