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
``` git clone https://github.com/bchu7796/P2P_File_Sharing.git```

``` cd P2P_File_Sharing.git```

```make```

## Execution

``` ./server ```

``` ./peer ```

## Reference
Penn State 2019 Fall - CSE 513 LAB1