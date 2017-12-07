# TODO

## To finish the semester

* Store hash_str in ref (what does this mean?)

* Switch protocol to plaintext protocol?

Server:
* Write the worker process loop

Data storage:
* Add file locks we actually writing the data
* Switch data.c implementation to write data to a socket instead of a buffer first.
  * So when a request comes in to:
	1. write some data, we feed it straight from the socket to a file
	2. read some data, we feed it straight from the file and into the socket
  * This means we have no worries over the size of the data they want to store, it can be much greater than the memory we use.

Client
* Write a simple client in C


Limit the client server to talking over Unix sockets for now.

## After the semester

* Write a simple client in Python
* Add simple compression to the data files
* Look into only storing diffs, instead of explicitly storing each version of some data




# Data flow

How data is added:
1. Take a name and some data
2. Hash name and data, write to object database
3. Create commit object that points to previous commit for this name and the hash
   * Commit includes unix epoch timestamp
4. Store commit in object database
5. Update ref for that name

How current data is retrieved:
1. Get commit hash for name from ref
2. Get data hash from commit
3. Read and return data

How past data is retrieved:
1. Get commit hash for name from ref
2. Traverse commits until you find the first commit with timestamp less than given timestamp
3. Get data hash from that commit
4. Read and return data

