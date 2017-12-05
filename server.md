
# Server Architecture

The server takes in a number of workers, forks these into separate processes.

One idea was that the main server process waits and listens for requests. When it gets a request:
1. It finds one of its worker processes, and send it the initial request from the client.
2. The main server goes back to listening for responses.
3. The worker gets the request object, which tells us what socket we're talking to the client over. We process the client's request.
4. We now tell the main loop we're ready to receive another request.


This doesn't seem correct because if we get a lot of requests and don't have enough worker processes, then we are stuck waiting and clients can't connect to the server.

We will make use of the producer consumer model, where the main server if producing requests, then each process is consuming requests.

1. Server receives a request, adds it to the queue of requests.
2. A worker grabs the request and processes it.


