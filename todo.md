# TODO

## To finish the semester

* Store hash_str in ref (what does this mean?)

Server:
* Connect the server to the data management layer
* Test that multiple threads are actually managing the incoming connections

Data storage:
* Add file locks we actually writing the data
* Switch data.c implementation to write data to a socket instead of a buffer first.
  * So when a request comes in to:
	1. write some data, we feed it straight from the socket to a file
	2. read some data, we feed it straight from the file and into the socket
  * This means we have no worries over the size of the data they want to store, it can be much greater than the memory we use.

Client
* Improve the client for testing purposes
  * have it construct simple requests
  * have it retrieve the response from the server


Limit the client server to talking over Unix sockets for now.


## Required!!!

* catalog
* history

detached thread (no join)
	pthread_detach or attribute

* nice demo (2-3 pages)

## After the semester

* Write a simple client in Python
* Add simple compression to the data files
* Look into only storing diffs, instead of explicitly storing each version of some data

