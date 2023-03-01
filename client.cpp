#include <iostream>
#include <fstream>

#include <vector>
#include <unordered_map>
#include <cstring>

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


//converts the output array to a string that can be printed
std::string convertToString(char* a, int size){
    int i;
    std::string s = "";

    for (i = 0; i < size; i++)
        s = s + a[i];
    return s;
}
struct decompress_info{
    std::string str;
    std::vector<int> positions;
    char* output;
    int portno;
    const char* serverName;

    decompress_info(std::string str, std::vector<int> positions, char* output, int portno, const char* serverName){
        this->str = str;
        copy(positions.begin(), positions.end(), back_inserter(this->positions));
        this->output = output;
        this->portno = portno;
        this->serverName = serverName;
    }
};

//args function for pthread_create; adds decoded characters to final output array based on their positions
void *code_to_string_through_server(void *decompress_info_void_ptr){
    decompress_info *decompress_info_ptr = (decompress_info*) decompress_info_void_ptr;

    //sockets
    int sockfd, portno, n;
    const char* serverName;
    struct sockaddr_in serv_addr;
    struct hostent *server; //stores info about a server (ip, name, etc..)
    char buffer[256];

    serverName = decompress_info_ptr->serverName;
    portno = decompress_info_ptr->portno; // transform portno into integer and set it as port no -- next need ip of server

    sockfd = socket(AF_INET/*family of protocols (INTERNET*/, SOCK_STREAM/*type of socket -empty socket) (e.g SOCKET STREAM, DATAGRAM)*/, 0); //socket to be used later
    if(sockfd < 0)
        exit(1);

    server = gethostbyname(serverName); /*takes name of server and returns hostent structure; struct contains info about server (ip, name, other, info; populates hostent*/
    if(server == NULL /*if null; name given to program is invalid*/){
        fprintf(stderr, "Server Name NULL usage %s hostname port\n", portno);
        exit(0);
    }
    //populating another struct that is a soft address struct
    bzero((char *) &serv_addr, sizeof(serv_addr)); //initializing \0 to all bytes in server address
    serv_addr.sin_family = AF_INET; //set family for socket address structure to be AF INET (internet protocol family)
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length); //now that be know the address from the server, copy it to this location that is part of the structure = giving to structure info about the server we have (hostnstruct)
    serv_addr.sin_port = htons(portno); //assign port no to structure
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) //relate socket we created with the sock addressing structure - allows to client to connect to the sever with all the info
        exit(1);

    //connected...
    const char* cstr = decompress_info_ptr->str.c_str();
    char *message = new char[decompress_info_ptr->str.length() + 1];
    strcpy(message, cstr);
    bzero(buffer, 256); //char array all char \0

    n = send(sockfd, message, strlen(message), 0);
    if(n < 0)
        exit(1);

    n = recv(sockfd, buffer, sizeof(buffer), 0);
    if(n < 0)
        exit(1);

    //printf("$s\n", buffer);
    for(int i = 0; i < decompress_info_ptr->positions.size(); i++){
    /*stores the decompressed character (c) a total of (size of positions vector) times
    in our output array which was created in and therefore accessible by the main thread*/
        decompress_info_ptr->output[decompress_info_ptr->positions[i]] = buffer[0];
    }
    close(sockfd);
}

//compressed file is decoded using multi-threaded approach
void decompress_huffman_code(int portno, const char* serverName){
    std::string line;
    int outputLength = 0;
    std::map<std::string, std::vector<int>> positionsMap;
    static std::vector<decompress_info*> array_of_decompress_info_structs;
    static std::vector<pthread_t> tid;

    //information is read from the compressed file
    while(getline(std::cin, line)){
        std::istringstream ss(line);
        std::string code;
        std::string buffer;
        int position;
        bool getCode = true;

        while(ss >> buffer){
            if(getCode){
                code = buffer;
                getCode = false;
            }
            else{
                positionsMap[code].push_back(stoi(buffer));
                outputLength ++;
            }
        }
    }

    char output[outputLength];

    for(auto mapElements : positionsMap){
        //n POSIX threads are created (n is the number of lines in the compressed file)
        pthread_t thread;
        decompress_info* temp = new decompress_info(mapElements.first, mapElements.second, output, portno, serverName);
        /*std::cout << it.first << ": ";
        for(int i = 0; i < it.second.size(); i++)
            std::cout << it.second[i] << " ";
        std::cout << std::endl;*/
        tid.push_back(thread);
        array_of_decompress_info_structs.push_back(temp);
    }

    for(int i = 0; i < array_of_decompress_info_structs.size(); i++){
        /*
        each thread receives a pointer to a struct that contains the following:
        struct decompress_info{
            HuffmanNode* root;
            std::string str;
            std::vector<int> positions;
            char* output;
        }
        (1) a pointer to the root node of the Huffman tree
        (2) the binary code
        (3) a vector of positions in which the code should be placed in the output array
        (4) a pointer to the output array that will be mutated by each thread
        */
        if(pthread_create(&tid[i], NULL, code_to_string_through_server, (void*)array_of_decompress_info_structs[i])){
            fprintf(stderr, "Error creating thread\n");
            return;
        }
    }

    //joining threads together after their processes are finished
    for(int i = 0; i < tid.size(); i++)
        pthread_join(tid[i], NULL);
    /*After threads mutate the output array,
    and are joined, the original message is printed after*/
    std::cout << "Original message: " << convertToString(output, sizeof(output) / sizeof(char)) << std::endl;
}


int main(int argc/*3 arguments from command line*/, char**argv/*file name, name of server, port of server*/){

    if(argc < 3){
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    int portno = atoi(argv[2]);
    const char* serverName = argv[1];
    decompress_huffman_code(portno, serverName);


    return 0;
}
