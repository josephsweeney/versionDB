# VersionDB

## What is this?

VersionDB is essentially a versioned data storage solution inspired by Git. Imagine you have some data that is extremely valuable. If someone comes along and overwrites this data, most of the time you're out of luck, and you better hope you have a backup. 

With VersionDB you can store any data, and as you overwrite it, we keep track of each version of the data. If something goes wrong, all you have to do is ask for your data with some time in the past, we will return what the data was at that point in time.

For more information about the protocol that versionDB speaks, read the information in  [`server_architecture.md`](server_architecture.md).


## Building

The build is done using `make`. To build the server run `make server`, and `make client` for the simple client that is included.

The build has not been tested on anything except macOS version 10.13.2.

If there are any problems, please open an issue.


## Example Usage

Move into the `versiondb` directory and run the following commands:

```
~> make db
mkdir -p db/objects db/refs
~> make server
cc -g -std=c99 bin/server.o bin/data.o bin/sha1.o bin/commit.o -o bin/server
~> ./bin/server vdb
```

Now open a new terminal window and run:

```
~> make client
cc -g -std=c99 bin/client.o -o bin/client
~> ./bin/client vdb --help
Error, please give ./client <path for socket> <request type>
Request types:
Read: ./client <path for socket> -r <id>
Read at time: ./client <path for socket> -rt <id> <time>
Read from hash: ./client <path for socket> -rh <hash>
Write: ./client <path for socket> -w <id>
    Then pass data in through stdin
List ids: ./client <path for socket> ls
History: ./client <path for socket> -history <id>
~> ./bin/client vdb ls
```
Write from a file using a redirect:
```
~> ./bin/client vdb -w server_doc < server.md
15e85a8f15e85a9315e85a9715e85a9b15e85a9f⏎                                     
```
Write to a file from a read:
```
~> ./bin/client vdb -r server_doc > cp_server.md
~> ./bin/client vdb ls
server_doc
~> ./bin/client vdb -w readme < Readme.md
f0ec91c1f0ec91c5f0ec91c9f0ec91cdf0ec91d1⏎                                     
~> ./bin/client vdb ls
server_doc
readme
~> ./bin/client vdb -w readme
Now we're overwriting 'readme'
When you're done, just make a new line and send EOF(Ctrl-D)
5063c3825063c3865063c38a5063c38e5063c392⏎                                                                  
~> ./bin/client vdb -r readme
Now we're overwriting 'readme'
When you're done, just make a new line and send EOF(Ctrl-D)
~> ./bin/client vdb -w readme
Also access the readme through the hash that is returned.
7708cb887708cb8c7708cb907708cb947708cb98⏎                                                                  
~> ./bin/client vdb -rh 7708cb887708cb8c7708cb907708cb947708cb98
Also access the readme through the hash that is returned.
~> ./bin/client vdb -rh 5063c3825063c3865063c38a5063c38e5063c392
Now we're overwriting 'readme'
When you're done, just make a new line and send EOF(Ctrl-D)
~> ./bin/client vdb -history readme
7708cb887708cb8c7708cb907708cb947708cb98 ~ TIME:1513385202107
5063c3825063c3865063c38a5063c38e5063c392 ~ TIME:1513385143380
f0ec91c1f0ec91c5f0ec91c9f0ec91cdf0ec91d1 ~ TIME:1513385053571
~> ./bin/client vdb -rt readme 1513385143380
Now we're overwriting 'readme'
When you're done, just make a new line and send EOF(Ctrl-D)
~> ./bin/client vdb -r readme
Also access the readme through the hash that is returned.
~> ./bin/client vdb -w note
So we can access the readme from its hash, or from the time.
93cd10db93cd10df93cd10e393cd10e793cd10eb⏎                                                                  ~> ./bin/client vdb -r note
So we can access the readme from its hash, or from the time.
~> ./bin/client vdb -history note
93cd10db93cd10df93cd10e393cd10e793cd10eb ~ TIME:1513385336509
~> ./bin/client vdb -w note
done
```



## Python lib notes

The Python library is still very functional but has been untouched for some time.
It has a simple API where you can either write some data, or get some data.
Optionally, you may pass in a Python `datetime` object to retrieve the data at any point in time.
The envisioned used case for it is to subclass this and use it for more specfic problems. 
An example of this would be to override the `_get_history_obj` method so that `get_data_history` returns an object useful for your specific case.

Another use case is for syncing JSON data between many clients. 
VersionDB provides the capability to add a `remote_url` which allows data to be tracked over time, with modifications being made in a distributed fashion.
For example, you can set up a remote Github repository to act as your data storage, and then just give the url and credentials to a `VersionDB` instance, and it will automatically push and pull to keep your data synchronized.
