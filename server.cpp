#include <iostream>
#include <fstream>

#include <vector>
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
#include <sys/wait.h>

//information of each line of input file will be stored in a Huffman node
struct HuffmanNode {
    char c;
    int freq;
    std::string code;
    HuffmanNode* left;
    HuffmanNode* right ;

    HuffmanNode(int freq, char c = '\0'){

        this->c = c;
        this->freq = freq;
        this->left = nullptr;
        this->right = nullptr;
    }

    //returns code based on position of node on Huffman Tree
    char decode(std::string str){
        if (str == "")
            return c;
        else{
            char temp  = str.at(0);
            if (temp == '0')
                return this->left->decode(str.substr(1));
            return this->right->decode(str.substr(1));
        }
    }
};

//comparator for strong ordering of nodes contained in vector
struct HuffmanNodeComparison{
   bool operator()( const HuffmanNode* a, const HuffmanNode* b ) const{
        if( a->freq != b->freq)
            return ( a->freq > b->freq);
        return ( a->c > b->c);
   }
};

//uses algorithm to build Huffman tree based on the nodes in the vector passed in parameter list
void build_huffman_tree(std::vector<HuffmanNode*> &huffmanNodeVector){
    std::string line;
    char c;
    int freq;

    //alphabet information is read of the input file
    while(getline(std::cin, line)){

        c = line.at(0);
        freq = stoi(line.substr(2));
        huffmanNodeVector.push_back(new HuffmanNode(freq ,c));
    }

    sort(huffmanNodeVector.begin(), huffmanNodeVector.end(), HuffmanNodeComparison());

    //Huffman tree is generated based on the vector of nodes
    while(huffmanNodeVector.size() > 1){

        HuffmanNode* left = huffmanNodeVector[huffmanNodeVector.size()-1];
        HuffmanNode* right = huffmanNodeVector[huffmanNodeVector.size()-2];

        huffmanNodeVector.pop_back();
        huffmanNodeVector.pop_back();

        HuffmanNode* root = new HuffmanNode(left->freq + right->freq);

        root->left = left;
        root->right = right;

        huffmanNodeVector.push_back(root);

        sort(huffmanNodeVector.begin(), huffmanNodeVector.end(), HuffmanNodeComparison());
    }
}

//creates the codes for each node in Huffman tree
void create_huffman_code(HuffmanNode* root, std::string code){
    if(root == nullptr)
        return;
    if(root->left == nullptr && root->right == nullptr)
        root->code = code;
    if(root->left)
        create_huffman_code(root->left, code + '0');
    if(root->right)
        create_huffman_code(root->right, code + '1');
}

//prints nodes of Huffman tree in preorder traversal
void print_huffman_tree(HuffmanNode* root){
    if(root == nullptr)
        return;
    if(root->c)
        std::cout << "Symbol: " << root->c << ", Frequency: " << root->freq << ", Code: " << root->code << std::endl;
    print_huffman_tree(root->left);
    print_huffman_tree(root->right);
}

void fireman(int){
    while(waitpid(-1, NULL, WNOHANG) > 0)
        std::cout << "A child processes ended" << std::endl;
}

void create_huffman_server(HuffmanNode* root, int portnoParameter){
    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    sockfd = socket(AF_INET/*family of protocols (INTERNET*/, SOCK_STREAM/*type of socket -empty socket) (e.g SOCKET STREAM, DATAGRAM)*/, 0); //create an empty socket like we did in client
    if(sockfd < 0)
        exit(1);
    //populating another struct that is a soft address struct
    bzero((char*) &serv_addr, sizeof(serv_addr)); //initializing \0 to all bytes in server address
    portno = portnoParameter; // transform portno into integer and set it as port no -- next need ip of server
    serv_addr.sin_family = AF_INET; //set family for socket address structure to be AF INET (internet protocol family)
    serv_addr.sin_addr.s_addr = INADDR_ANY; //in client we copy address from hostent struct, but here we are allowing our server to recieve info from ANY
    serv_addr.sin_port = htons(portno);

    if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        exit(1);

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    signal(SIGCHLD, fireman);

    while(true){
        newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, (socklen_t*) &clilen); //creating a new sockfd thats handling requests from client
        if(fork() == 0){
            if(newsockfd < 0)
                exit(1);
            bzero(buffer, 256); //char array all char \0
            n = recv(newsockfd, buffer, sizeof(buffer) / sizeof(char), 0);
            if(n < 0)
                exit(1);

            char test;
            char my_char = root->decode(buffer);
            memset(buffer, 0, sizeof(buffer));
            n = send(newsockfd, &my_char, sizeof(my_char), 0);
            if(n < 0)
                exit(1);
            close(newsockfd);
            _exit(0);
        }
        wait(0); //server for single request
    }

    close(sockfd); //close socket
}

int main(int argc/*2 arguments from command line*/, char**argv/*file name, port of server*/){


    if(argc < 2 /*server only needs 2 command line arguments*/){
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(1);
    }

    int portno = atoi(argv[1]);
    std::vector<HuffmanNode*> huffmanNodeVector;

    //1. Generate Huffman tree using a sorted vector as a priority queue
    build_huffman_tree(huffmanNodeVector);
    //2. Huffman codes are created from the generated tree
    create_huffman_code(huffmanNodeVector[0], "");
    //3. Huffman codes are printed from the generated tree
    print_huffman_tree(huffmanNodeVector[0]);
    //4. Create server that recieves codes and responds with corresponding character
    create_huffman_server(huffmanNodeVector[0], portno);

    return 0;
}
