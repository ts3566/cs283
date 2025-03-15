1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

_The remote client determines when a command's output is complete by looking for a special EOF character that the server sends after each command's output. The client continuously reads from the socket in a loop until it receives this EOF character. The server ensures complete message transmission by sending all command output first, followed by an explicit call to send_message_eof() which sends the EOF character._

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

_A networked shell protocol should define explicit message boundaries by either sending the message length first or using special delimiter characters to mark the end of each command and response. Without proper message boundaries, the receiver might combine multiple commands or split a single command due to TCP's streaming, leading to errors. Partial reads could also cause the shell to hang waiting for more data or execute incomplete commands._

3. Describe the general differences between stateful and stateless protocols.

_Stateful protocols maintain information about previous interactions between client and server across multiple requests, requiring the server to track session data and client context. Stateless protocols treat each request as independent and self-contained, with all necessary information included in each request, making them simpler but potentially requiring more bandwidth due to repeated information._

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

_UDP is useful when speed and low latency are more important than guaranteed delivery, such as in real-time applications like video streaming or online gaming where dropping an occasional packet is preferable to waiting for retransmission._

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

_The operating system provides the sockets API, abstracting network communication via endpoints and system calls. It supports TCP/UDP, handling addressing, data transmission, and error handling._