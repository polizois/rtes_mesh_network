# Simple Mesh Network
Final assignement for the course named "Real Time Embedded Systems" of Aristotle University's "Electrical and Computer Engineering" faculty.

Message exchanging program meant to run on multiple raspberry PIs located in the same area creating a basic mesh network.  
The program generates message with destination certain devices. It also connects with the available devices in the network and exchanges these messsages with them. A messsage circulates from device to device in the network until it reaches it's destination.

**Notes:**
* Each device running the program should be assigned a device id in the range of 7000 to 9999 wich will be reflected in the device's ip in the following way: If the device's id is, for example, **8535** it's ip should be set to "10.0.**85**.**35**".
* In order to discover each other in the network the devices ping the broadcast ip. So the devices have to be able to respond to ICMP packets. In rasbian OS this can be allowed by writing **0 to */proc/sys/net/ipv4/icmp_echo_ignore_broadcasts***.

**Settings:**  
The program doesn't accept arguements. You can change the following options found in ***include/definitions.h*** before compilation.
* **SERVER_PORT** : The port on which the device will be listening for incomming activity and on which it will try to reach other devices.
* **NETWORK_DEVICE** : The name of the network device that will be used by the program ("eth0" or "wlan0" or "usb0").
* **PING_PACKETS** : The number of ICMP packets sent to ping the broadcast ip (in order to find the available devices in the network).
* **MAX_SOCKETS** : The maximum allowed number of connected devices.
* **CIRC_BUFFER_SIZE** : The maximum number of temporary messages the device can store before the start getting replaced by the newer ones.
* **MSG_SIZE** : The maximum size of a complete message "origin_destination_timestamp_actualMessage".

**Compile:**  
Type "***make***" to compile (arm-linux-gnueabihf-gcc needs to be installed).

**Run:**  
Type "***./routerPI.out***" to run the compiled program.

**Terminate:**  
Terminate the program by sending a **SIGINT** signal (or with **Ctrl+c**).
