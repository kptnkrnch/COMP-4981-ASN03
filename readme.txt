How to compile the project:

open a new terminal in Linux(tested to work on a system running Fedora)
navigate to folder containing the client.cpp file or server.cpp file using “cd”
type in: “g++ Wall -o client client.cpp -lpthread” and/or 
“g++ Wall -o server server.cpp” to compile the program

check there are no errors and use “ls” to to see that the .exe has been created in
the folder run the program by typing in ./client to start the client 
or ./server to start the server
optionally the client program can be set to save the chat log to a file by running the
program with ./client [filename].txt where [filename].txt is the name of the file you wish to
create or overwrite with the chat log.

How to use the program

1. Start the server program.
2. Set the port that the server will be listening on (after the prompt)
3. Start up the clients (type in a file name after ./client in order to save output
   ie ./client chatdata.txt
3. b. Optionally, start up the clients with a specified file to save the chat log too.
4. Set the IP and ports of the clients
5. Set the alias names of the clients (usernames)
6. Send messages to the server from the clients by typing in (enter sends the data).