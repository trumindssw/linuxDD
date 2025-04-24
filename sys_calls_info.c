Internet domain sockets
       #include <netinet/in.h>

       struct sockaddr_in {
           sa_family_t     sin_family;    //family ipv4 or ipv6 family choose     AF_INET
           in_port_t       sin_port;       //port number                          port number
           struct in_addr  sin_addr;      //chosen family address                 IPV4 address
       };

struct in_addr {
           in_addr_t s_addr;
       };

Internet domain sockets
       sockaddr_in
              Describes  an IPv4 Internet domain socket address.  The sin_port
              and sin_addr members are stored in network byte order.


 #include <sys/socket.h>

       int socket(int domain, int type, int protocol);

DESCRIPTION
       socket()  creates an endpoint for communication and returns a file descriptor that refers to that endpoint.  The file
       descriptor returned by a successful call will be the lowest-numbered file  descriptor  not  currently  open  for  the
       process.

       The  domain argument specifies a communication domain; this selects the protocol family which will be used for commu‐
       nication.

      Name         Purpose                                    Man page
       AF_UNIX      Local communication                        unix(7)
       AF_LOCAL     Synonym for AF_UNIX
       AF_INET      IPv4 Internet protocols                    ip(7)
       AF_AX25      Amateur radio AX.25 protocol               ax25(4)
       AF_IPX       IPX - Novell protocols
       AF_APPLETALK AppleTalk                                  ddp(7)
       AF_X25       ITU-T X.25 / ISO/IEC 8208 protocol         x25(7)
       AF_INET6     IPv6 Internet protocols                    ipv6(7)

The socket has the indicated type, which specifies the communication semantics.  Currently defined types are:

       SOCK_STREAM     Provides sequenced, reliable, two-way, connection-based byte streams.  An out-of-band data  transmis‐
                       sion mechanism may be supported.

       SOCK_DGRAM      Supports datagrams (connectionless, unreliable messages of a fixed maximum length).

The protocol specifies a particular protocol to be used with the socket.  Normally only a single protocol  exists  to
       support  a particular socket type within a given protocol family, in which case protocol can be specified as 0.  How‐
       ever, it is possible that many protocols may exist, in which case a particular protocol must  be  specified  in  this
       manner.   The  protocol  number  to  use  is specific to the “communication domain” in which communication is to take
       place; see protocols(5). 

 covert host to network address:
____________________________________
The htons() function converts the unsigned short integer hostshort from host byte order to network byte order.

The  inet_addr()  function converts the Internet host address cp from IPv4 numbers-and-dots notation into binary data
       in network byte order.  If the input is invalid, INADDR_NONE (usually -1) is returned.  Use of this function is prob‐
       lematic because -1 is a valid address (255.255.255.255).  Avoid its use in favor  of  inet_aton(),  inet_pton(3),  or
       getaddrinfo(3), which provide a cleaner way to indicate error return.


sendto:
_______
ssize_t sendto(int sockfd, const void buf[.len], size_t len, int flags,
                      const struct sockaddr *dest_addr, socklen_t addrlen);

eg:
sendto(sockfd, buf, len, flags, NULL, 0);

       The argument sockfd is the file descriptor of the sending socket.

       If  sendto()  is  used on a connection-mode (SOCK_STREAM, SOCK_SEQPACKET) socket, the arguments dest_addr and addrlen
       are ignored (and the error EISCONN may be returned when they are not NULL and 0), and the error ENOTCONN is  returned
       when  the socket was not actually connected.  Otherwise, the address of the target is given by dest_addr with addrlen
       specifying its size.

       For send() and sendto(), the message is found in buf and has length len. 

recvfrom:
ssize_t recvfrom(int sockfd, void buf[restrict .len], size_t len,int flags,
                        struct sockaddr *_Nullable restrict src_addr,
                        socklen_t *_Nullable restrict addrlen);

The  only difference between recv() and read(2) is the presence of flags.  With a zero flags argument, recv() is gen‐
       erally equivalent to read(2) (but see NOTES).  Also, the following call

           recv(sockfd, buf, len, flags);

       is equivalent to

           recvfrom(sockfd, buf, len, flags, NULL, NULL);

       All three calls return the length of the message on successful completion.  If a message is too long to  fit  in  the
       supplied buffer, excess bytes may be discarded depending on the type of socket the message is received from.

       If no messages are available at the socket, the receive calls wait for a message to arrive, unless the socket is non‐
       blocking  (see  fcntl(2)), in which case the value -1 is returned and errno is set to EAGAIN or EWOULDBLOCK.  The re‐
       ceive calls normally return any data available, up to the requested amount, rather than waiting for  receipt  of  the
       full amount requested.

       -->  shmctl(id,IPC_STAT,&buff);
_________________________________________

       IPC_STAT
              Copy  information  from the kernel data structure associated with shmid into the shmid_ds structure pointed to
              by buf.  The caller must have read permission on the shared memory segment.

       IPC_SET
              Write the values of some members of the shmid_ds structure pointed to by buf to the kernel data structure  as‐
              sociated with this shared memory segment, updating also its shm_ctime member.

              The  following  fields  are  updated:  shm_perm.uid,  shm_perm.gid,  and  (the  least  significant  9 bits of)
              shm_perm.mode.

              The effective UID of the calling process must match the owner (shm_perm.uid) or creator (shm_perm.cuid) of the
              shared memory segment, or the caller must be privileged.

	    -->  shmctl(id,IPC_STAT,&buff);

