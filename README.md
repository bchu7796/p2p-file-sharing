# P2P File Sharing System

## System Description

The system consists of several clients (peers) and a central server. A peer can join the p2p network by connecting to the server. After entering the network, a peer can either register the file that it wants to share or download the file from other peers. The data being distributed are split into chunks. For each file, the server keeps track of the list of chunks each peer has. Any peer can download files from other peers directly. Moreover, any peer is capable of downloading different chunks of a file simultaneously from different peers. 

## Protocol

| Request  | Reply  |
|---|---|
|Register Request: Tells the server what files the peer wants to share with the network. |Register Reply: For each file, it advises if the file registration was a success |
|  File List Request: Asks the server for the list of files. |  File List Reply: Includes the number of files in the list; and for each file, a file name and a file length. |
| File Locations Request: Asks the server for the IP endpoints of the peers containing the requested file  | File Locations Reply: Includes number of endpoints; then for each endpoint, chunks of the file it has, an IP address and port .  |
| Chunk Register Request: Tells the server when a peer receives a new chunk of the file and becomes a source (of that chunk) for other peers.  | Chunk Register Reply: Advises if the chunk registration was a success.|
| File Chunk Request: Asks the peer to return the file chunk. Reads in a file name, chunk indicator.  | File Chunk Reply: A stream of bytes representing the requested chunk. |

## Download and Compile
``` 
git clone https://github.com/bchu7796/P2P_File_Sharing.git
cd P2P_File_Sharing.git
make
```

## Execution
Server:
``` 
./server 
```
Client:
``` 
# To start the client
./peer 

# Get list of files that can be downloaded
show

# Share files
share filenums file1, file2, ...

# Download file
download filename

```

## Reference
Penn State 2019 Fall - CSE 513 LAB1

## License

MIT License

Copyright (c) 2019 Pang-Yang Chu (Brian Chu) and contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
