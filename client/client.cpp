// -----------------------------
//  Name: Andrew Samoil
//  ID: 1621231
//  CMPUT 275, Winter 2021
//  
//  Assignment 1: Trivial Navigation System
// ------------------------------

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>     // socket, connect
#include <arpa/inet.h>      // inet_addr, htonl, htons
#include <cassert>
#include <cstring>          // strcpy, memset
#include <string.h>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

// Add more libraries, macros, functions, and global variables if needed

using namespace std;

int create_and_open_fifo(const char * pname, int mode) {
    // creating a fifo special file in the current working directory
    // with read-write permissions for communication with the plotter
    // both proecsses must open the fifo before they can perform
    // read and write operations on it
    if (mkfifo(pname, 0666) == -1) {
        cout << "Unable to make a fifo. Ensure that this pipe does not exist already!" << endl;
        exit(-1);
    }

    // opening the fifo for read-only or write-only access
    // a file descriptor that refers to the open file description is
    // returned

    int fd = open(pname, mode);

    if (fd == -1) {
        cout << "Error: failed on opening named pipe." << endl;
        exit(-1);
    }

    return fd;
}


/*
  Description: reads from inpipe and writes to outpipe

  Arguments:
    argc (int): Number of arguments
    const *argv[] (char): const array of arguments

  Returns:
    returns 0 (by default)
*/
int main(int argc, char const *argv[]) {


    // check if two command line arguments
    if (argc != 3) { 
        cout << "This program takes two command line arguments" << endl;
        return 0;
    }

    // server ip and port number
    const char * server_ip = argv[1];  
    int port_num = atoi(argv[2]);

    // default packet and socket set-up
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    char packet[2048] = {0};

    // create the socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        cout << "Socket creation failed!" << endl;
        return 1;
    }

    serv_addr.sin_family = AF_INET;                     // IPv4 socket family
    serv_addr.sin_port = htons(port_num);               // server port number in network byte order    
    inet_aton(server_ip, &(serv_addr.sin_addr));        // server IP address in network byte order

    // connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        cout << "Connection refused!" << endl;
        return 1;
    }
    cout << "Connection request accepted from " << inet_ntoa(serv_addr.sin_addr) << ":" << ntohs(serv_addr.sin_port) << endl;


    // create inpipe and outpipe
    const char *inpipe = "inpipe";
    const char *outpipe = "outpipe";

    int in = create_and_open_fifo(inpipe, O_RDONLY);
    cout << "inpipe opened..." << endl;
    int out = create_and_open_fifo(outpipe, O_WRONLY);
    cout << "outpipe opened..." << endl;

    // loop client until Q in inpipe
    while (true) {
        string filename = "inpipe";
        ifstream inFile;
        // open file (inpipe)
        inFile.open(filename);

        string line;
        string message = "";
        bool done = false;

        // read two lines from inpipe
        for (int i=0; i < 2; i++) {

            getline(inFile, line);

            // if quit (Q) stop the client and server
            if (line == "Q") {
                packet[0] = 0;
                send(sock, packet, sizeof(packet), 0);
                done = !done;
                break;
            }

            message += line + " ";
        }

        // close file
        inFile.close();

        // check if input was Q
        if (done) {
            break;
        }

        // set memory
        memset(packet, 0, sizeof(packet));

        // copy string message into char array (c_str())
        strcpy(packet, message.c_str());

        //  send message to server
        send(sock, packet, strlen(packet), 0);


        // reset message
        message = "";
        bool nada = false;
        bool first = true;

        // reset packet
        string reset = "";
        strcpy(packet, reset.c_str());

        // recieve from server
        while (recv(sock, packet, 2048, 0)) {
            string cur;

            // convert packet to string
            cur = packet;

            // if E, stop reciving
            if (cur == "E") {
                cur += "\n";
                message += cur;
                break;
            }

            // if N0, dont recive anything
            if (cur == "N0") {
                nada = true;
                break;
            }

            // depending on coord the cur packet will be added
            if (first) {
               cur += " ";
            } else {
                cur += "\n";
            }


            first = !first;
            message += cur;
        }

        // open outpipe
        ofstream myfile ("outpipe");
        if (!nada) {
            if (myfile.is_open()) {
                // insert data
                myfile << message;
            }
        } else {
            myfile << "E\n";
        }
        
        // close file
        myfile.close();

    }

    // send server a quit message
    packet[0] = 0;
    send(sock, packet, sizeof(packet), 0);
    // close socket
    close(sock);

    close(in);
    close(out);
    unlink(inpipe);
    unlink(outpipe);
    return 0;
}
