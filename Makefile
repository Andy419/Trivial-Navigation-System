# -----------------------------
#   Name: Andrew Samoil
#   ID: 1621231
#   CMPUT 275, Winter 2021
#  
#  Assignment 1: Trivial Navigation System
# ------------------------------

all: server client

client: client/client.o
	cd client && g++ -o client client.o

client.o: client/client.cpp 
	cd client && g++ -c client.cpp

server: server/server.o server/dijkstra.o server/digraph.o
	cd server && g++ -o server server.o dijkstra.o digraph.o

server.o: server/server.cpp server/edmonton-roads-2.0.1.txt
	cd server && g++ -c server.cpp

dijkstra.o: server/dijkstra.cpp server/dijkstra.h server/heap.h
	cd server && g++ -c dijkstra.cpp

digraph.o: server/digraph.cpp server/digraph.h server/wdigraph.h
	cd server && g++ -c digraph.cpp

clean:
	rm -f inpipe outpipe client/client client/client.o server/server server/server.o server/dijkstra.o server/digraph.o
