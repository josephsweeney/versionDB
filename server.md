
# Server Architecture

The server takes in a number of workers, makes this many worker threads.

The server also binds and starts listening on a socket. Each worker thread enters a loop where they all `accept` on that socket. This means one of them will get each request while the others continue to wait.

The main downside of this approach is it gives us less control over how many clients can be queued before we start rejecting new requests, with our only control is setting the limit in our call to `listen`.

This could be reworked in the future to have a single thread accepting requests, throwing them into a queue and then have workers pull from this queue. This would gives us the control we're lacking in the current implementation.


## Server Protocol

For this protocol, a simple plaintext option was chosen to keep complexity to a minimum. The requirements it must meet are simple so the protocol itself should be simple.

There are 3 types of requests: `WRITE`, `READ`, and `READ_TIME`. 

Once a connection is established we send over 2 things separated by commas:

1. "`TYPE:{type}`" where `type` can be either `WRITE`, `READ`, or `READ_TIME`.
2. "`ID:{id}`" where `id` is the id of the data we are tracking. (NOTE: Currently an arbitrary bound of 1024 characters is placed on `id`, if more than that are sent, we consider the request bad.)

In addition to these there are 2 extra pieces of information that may be sent.

If the request is a `WRITE`, then we must also send over the number of bytes that we are going to send. This is of the form `BYTES:{number of bytes}`.

If the request is a `READ_TIME`, then we send the time. This time specifies which version of your data that you want. It is of the form `TIME:{time}` where `time` is the number of milliseconds from the Unix epoch in UTC.

Here are a few examples of valid requests:

`TYPE:READ,ID:great_data` 

This grabs the current version of "great_data".

`TYPE:WRITE,ID:my_data,BYTES:1024` 

This would be followed by 1024 bytes that you want stored.

`TYPE:READ_TIME,ID:great_data,TIME:1356048000000`

This would get you what the state of "great_data" was on Dec 21 2012 00:00:00 or the original state of the data if it was added after that time.

NOTE: We are also making the assumption that these messages are null terminated strings.


## Future

A `READ_ALL` feature is planned that will give a start time and end time, and return all the versions of the data within that time.

A `DELETE` feature is also planned that will delete all data associated with an id, within whatever time range is given.


