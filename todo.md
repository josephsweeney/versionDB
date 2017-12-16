# TODO

## To finish the semester


Server:

Data storage:

Client
  
* Create a nice demo of operation of server and client
  * Probably put this in the readme


Limit the client server to talking over Unix sockets for now.


### Required!!!

* catalog
* history

detached thread (no join)
	pthread_detach or attribute
	
* nice demo (2-3 pages)

## After the semester

* Store hash_str in ref (what does this mean?)

* Write a simple client in Python
* Add simple compression to the data files
* Look into only storing diffs, instead of explicitly storing each version of some data
* Add header to data files we store:
  * should header include time added so you can store duplicates?
  
(On second thought, I don't think we can do this. We need the hash of the data to figure out which file they want, so they either have to send the data over twice (once for hashing, once for writing), or we have to store it some way while we're hashing. Either way, I'm going to push this back and we'll just store the data in memory after retrieving it from the client.)
* Switch data.c implementation to write data to a socket instead of a buffer first.
  * So when a request comes in to:
	1. write some data, we feed it straight from the socket to a file
	2. read some data, we feed it straight from the file and into the socket
  * This means we have no worries over the size of the data they want to store, it can be much greater than the memory we use.

