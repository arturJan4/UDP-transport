# UDP-transport
Written for Computer Networks course.

## Description
reliable UDP downloader optimized for specific (provided by the lecturer) server

- server accepts UDP request in form of `GET start length\n` and it might send `length` bytes starting from `start` byte.
- the goal is to save the request data in a file byte by byte (with seek()) and under 5MB of memory usage
- server handles only given ammount of requests, sometime sends mutiple datagrams, sometimes sends none, and there is some random delay
- program uses receiver window to handle multiple requests at once
- optimized for given server by some tricks (sending multiple request to increase probability of receiving an answer, prioritizing packets in the beginning of the window to move the window at-once and not get stuck, dynamic request size calculation etc.)
- prints live progress of the download

task description (in Polish) in file `p3.pdf`

## Usage
`make` -> build the project and create the executable  
`./transport <server address> <server port> <output file> <size in bytes>`  
`make clean`
