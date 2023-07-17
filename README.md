# Centralized-Multi-User-BankAccount

To design and implement a multithreaded bank server which handles multiple and concurrent client requests.

•	We implement this server using sockets for establishing connections and mutex locks for maintaining correct information on the record files.


•	On the server, all the data that is present in the Records file is stored in an array of structures which have the variables to store the account number, name and balance of the user.


•	Based the request these values are updated and each time a new request comes we load these values into the array.


•	Server needs to handle multiple requests at the same time and this is done through a pool of threads.


•	On the client side we read each line from the file and send it to the server through multiple threads and their respective connections with the server.


•	Once we get the request, from the client ,we need to handle transactions like cash withdrawal and deposit on the server. This is decided through tokenizing the request and taking out the appropriate values.


•	If no record of the user is found or if it’s an invalid transaction, same will be intimated to the client.


•	Upon a successful transaction, a success message is sent to the client as an acknowledgement.




