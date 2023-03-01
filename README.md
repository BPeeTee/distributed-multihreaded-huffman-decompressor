# distributed-multihreaded-huffman-decompressor
multithread huffman decompressor with distributed workload (client-server)
commands

compile to executable

g++.exe -Wall -g client.cpp -o client.exe

g++.exe -Wall -g server.cpp -o server.exe

CMD line args

client:

client.exe DESKTOP-H31VE59 3490 < compressed.txt

client.exe DESKTOP-H31VE59 3490 < compressed2.txt

client.exe DESKTOP-H31VE59 3490 < compressed3.txt

AND

server:

server.exe 3490 < input.txt

server.exe 3490 < input2.txt

server.exe 3490 < input3.txt
