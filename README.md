# cs118_proj2

## Task 

### Step 1
Assume there is no packet loss. Just have the server send a packet, the receiver respond with an ACK. 
Critical points:
- Header generation.
- Three-way handshaking and disconnection.
Implementation details:
- Header file for abstraction and constant definition
- Print out debugging info. (debugging helper functions)

### Step 2
Introduce a large file transmission. Divide the file into multiple packets and transmit the packets based on the current congestion window size.
Critical points:
- Here we may only use fixed window size for simplicity.
- Make sure `window` field works fine.

### Step 3
Introduce packet loss. Now you have to add a timer at the first sent and unacked packet. There should be one timeout whenever data segments are sent out.
Citical points:
- Correctly handle packet loss in all kinds of scenario. 
- Possibe best way to do is to draw a finite-state-machine.
- We may need to refer to RFC and list all possible corner cases that we can think of. Make sure to write testing script for this step.

### Step 4
Congestion control feature.
- TCP Tahoe congestion window adjustment.
- Bonus: adaptive RTO.

## Uncertainty
1. Shuaiyi: How to implement the TCP congestion window in terms of data structure? 
