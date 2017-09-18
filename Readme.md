# VersionDB

## What is this?

VersionDB is essentially a versioned data storage class built on top of Git.
It is a simple python class that provides easy storage and retrieval for basic JSON data where tracking changes is essential.
It has a simple API where you can either write some data, or get some data.
Optionally, you may pass in a Python `datetime` object to retrieve the data at any point in time.
The envisioned used case for it is to subclass this and use it for more specfic problems. 
An example of this would be to override the `_get_history_obj` method so that `get_data_history` returns an object useful for your specific case.

Another use case is for syncing JSON data between many clients. 
VersionDB provides the capability to add a `remote_url` which allows data to be tracked over time, with modifications being made in a distributed fashion.
For example, you can set up a remote Github repository to act as your data storage, and then just give the url and credentials to a `VersionDB` instance, and it will automatically push and pull to keep your data synchronized.

## Where is it going?

Right now VersionDB is going through a rewrite to C. 
The new version will be a standalone server that serves the same function as the library.
This will be built without any external dependencies which will make this easy to build and deploy.
