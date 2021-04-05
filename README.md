# Trivial-Navigation-System
```
Included Files:
	* README
	* Makefile
		Targets:
			- all  --> calls to create server and client executables
			- client  --> creates the client executable
			- client.o --> compiles client.cpp to an object file
			- server  --> creates the server executable
			- server.o  --> compiles server.cpp to an object file
			- dijkstra.o  --> compiles dijkstra.cpp to an object file
			- digraph.o  --> compiles digraph.cpp to an object file
			- clean  --> removes all target files/executables/pipes

	* plotter
	*/client
		- client.cpp

	*/server
		- dijkstra.cpp
		- dijkstra.h
		- server.cpp
		- wdigraph.h
		- digraph.h
		- digraph.cpp

Running Instructions:
	To run the user will have to be in the same directory as the Makefile.
	typing "make" into the consol will create the client and server
	executables. Then the user must run the server executable with a port number
	in a terminal. Then the client executable with the same port number and
	an ip in another terminal. And finally run plotter in a third terminal.
	Once the map has shown up, the user can click two points on the graph and
	the program will calculate and display the shortest distances between the
	two points. clicking the x button on the display will end the program

Functionality:
	This program is essentially a makeshift google maps. Once all the terminals are
	running as described in the running instructions the operation takes flight.
	The plotter runs the UI which is a map of edmonton which can be clicked on and
	information is sent to a pipe called inpipe. The client bascially acts as a
	middleman which connects the plotter to the server. The client gets information
	by reading the data outputted by the plotter into the inpipe. The client takes this
	information and via a socket, it sends that information to the server. When the
	server recives the packet, the server performs most of the heavy lifting via 
	finding the shortest path between the two vertices given. This happens through
	dijkstra's algorithm. Once the pathway is found, the server sends the coordinates
	along the way of the path in packets back to the client. The client recives these
	packets and stores them in a string. When the flow of packets stops as indicated
	by a stop character "E", the client outputs the string to the outpipe which
	communicates with the plotter to plot the pathway of the vertices and draw a
	connecting line between each of the points displaying a path similar to google
	maps. There are several next steps to continue this project. Prettifying the UI,
	GPS location and alternate pathways are just a few. However alot of these require
	more money than is imaginable and is to fill a niche that is already filled
	(by google that is) however it is cool to see how far we have gotten with this
	trivial model and hopefully there are classes in the future that continue from
	this model.
```
