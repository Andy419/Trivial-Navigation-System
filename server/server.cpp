// -----------------------------
//  Name: Andrew Samoil
//  ID: 1621231
//  CMPUT 275, Winter 2021
//  
//  Assignment 1: Trivial Navigation System
// ------------------------------


#include <iostream>
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <list>
#include <unistd.h>
#include <cstring>          // strcpy, memset
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "wdigraph.h"

// #include "dijkstra.cpp"
// #include "digraph.cpp"
#include "digraph.h"
#include "dijkstra.h"

using namespace std;

struct Point {
    long long lat, lon;
};

// returns the manhattan distance between two points
long long manhattan(const Point& pt1, const Point& pt2) {
  long long dLat = pt1.lat - pt2.lat, dLon = pt1.lon - pt2.lon;
  return abs(dLat) + abs(dLon);
}

// finds the point that is closest to a given point, pt
int findClosest(const Point& pt, const unordered_map<int, Point>& points) {
  pair<int, Point> best = *points.begin();

  for (const auto& check : points) {
    if (manhattan(pt, check.second) < manhattan(pt, best.second)) {
      best = check;
    }
  }
  return best.first;
}

// reads graph description from the input file and builts a graph instance
void readGraph(const string& filename, WDigraph& g, unordered_map<int, Point>& points) {
  ifstream fin(filename);
  string line;

  while (getline(fin, line)) {
    // split the string around the commas, there will be 4 substrings either way
    string p[4];
    int at = 0;
    for (auto c : line) {
      if (c == ',') {
        // starting a new string
        ++at;
      }
      else {
        // appending a character to the string we are building
        p[at] += c;
      }
    }

    if (at != 3) {
      // empty line
      break;
    }

    if (p[0] == "V") {
      // adding a new vertex
      int id = stoi(p[1]);
      assert(id == stoll(p[1])); // sanity check: asserts if some id is not 32-bit
      points[id].lat = static_cast<long long>(stod(p[2])*100000);
      points[id].lon = static_cast<long long>(stod(p[3])*100000);
      g.addVertex(id);
    }
    else {
      // adding a new directed edge
      int u = stoi(p[1]), v = stoi(p[2]);
      g.addEdge(u, v, manhattan(points[u], points[v]));
    }
  }
}


/*
  Description: reads from socket and sends to socket

  Arguments:
    argc (int): Number of arguments
    const *argv[] (char): const array of arguments

  Returns:
    returns 0 (by default)
*/
int main(int argc, char* argv[]) {
  // set port to argument
  int PORT = atoi(argv[1]);

  WDigraph graph;
  unordered_map<int, Point> points;

  // build the graph
  readGraph("server/edmonton-roads-2.0.1.txt", graph, points);

  // default packet and socket set-up
  int server_fd, new_socket_fd; 
  struct sockaddr_in address, client_address; 
  int addrlen = sizeof(struct sockaddr_in);
  char packet[2048] = {0}; 
     
  // creating a socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) { 
      cout << "socket creation failed";
      return 1;
  }

  // set up the socket (initialize socket parameters)
  address.sin_family = AF_INET; 
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK); 
  address.sin_port = htons(PORT); 
     
  // binding to port 8888
  if (::bind(server_fd, (struct sockaddr *)&address, addrlen) == -1) { 
      cout << "Unable to bind to port address" << endl; 
      return 1;
  }

  // listen for connection requests
  if (listen(server_fd, 10) == -1) { 
      cout << "Listen failed";
      return 1;
  }
  cout << "Listening to " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << endl;

  // accept a connection request
  new_socket_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*)&addrlen);
  cout << "Connection request accepted from " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << endl;

  // while the exit command isn't recived
  while (true) {
    // read packet from socket
    int valread = recv(new_socket_fd, packet, 2048, 0);
    // check if the client disconnected
    if (packet[0] == 0) break; 
    
    // to initially store start and end lat and lon
    string arr[4];
    int at = 0;
    for (auto c : packet) {

      if (c == ' ') {
        // starting a new string
        ++at;
        if (at == 4) {
          break;
        }
      } else {
        // appending a character to the string we are building
        arr[at] += c;
      }
    }

    Point sPoint, ePoint;

    // set lon and lat of start and end vertices
    sPoint.lat = static_cast<long long>(stod(arr[0])*100000);
    sPoint.lon = static_cast<long long>(stod(arr[1])*100000);
    ePoint.lat = static_cast<long long>(stod(arr[2])*100000);
    ePoint.lon = static_cast<long long>(stod(arr[3])*100000);

    // get the points closest to the two points we read
    int start = findClosest(sPoint, points);
    int end = findClosest(ePoint, points);

    // run dijkstra's, this is the unoptimized version that does not stop
    // when the end is reached but it is still fast enough
    unordered_map<int, PIL> tree;
    dijkstra(graph, start, tree);

    string message = "";
    // no path
    if (tree.find(end) == tree.end()) {
        message = "N0";
        strcpy(packet, message.c_str());
        send(new_socket_fd, packet, strlen(packet) + 1, 0);
    } else {
      // read off the path by stepping back through the search tree
      list<int> path;
      while (end != start) {
        path.push_front(end);
        end = tree[end].first;
      }
      path.push_front(start);

      // output the path
      for (int v : path) {
        string lats = to_string(points[v].lat);
        string nlats = "";

        // add decimal back
        for (int i=0; i < 8; i++) {
          if (i == 2) {
            nlats += ".";
          }
          nlats += lats[i];
        }

        // add decimal back
        string lons = to_string(points[v].lon);
        string nlons = "";
        for (int i=0; i < 9; i++) {
          if (i == 4) {
            nlons += ".";
          }
          nlons += lons[i];
        }

        // send over latitude of vertex
        message = nlats;
        strcpy(packet, message.c_str());
        send(new_socket_fd, packet, strlen(packet) + 1, 0);

        // send over longitude of vertex
        message = nlons;
        strcpy(packet, message.c_str());
        send(new_socket_fd, packet, strlen(packet) + 1, 0);      
      }

      // send over End character
      message = "E";
      strcpy(packet, message.c_str());
      send(new_socket_fd, packet, strlen(packet) + 1, 0); 
    }


  }
  // close the connection socket
  close(new_socket_fd);
  // close the listening socket
  close(server_fd);

  return 0;
}
