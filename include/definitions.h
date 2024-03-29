#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

// Edit these definitions to control the basic settings of the progran

#define SERVER_PORT 2288       // The port on which the device will be listening for incomming activity and on which it will try to reach other devices
#define NETWORK_DEVICE "wlan0" // The name of the network device that will be used by the program ("eth0" or "wlan0" or "usb0")
#define PING_PACKETS 20        // The number of packets sent to ping the broadcast ip (in order to find the available devices in the netwirk)
#define MAX_SOCKETS 30         // The maximum allowed number of connected devices
#define CIRC_BUFFER_SIZE 2000  // The maximum number of temporary messages the device can store before the start getting replaced by the newer ones
#define MSG_SIZE 299           // The maximum size of a complete message "origin_destination_timestamp_actualMessage"
#define TRUE 1                 // True
#define FALSE 0                // False

#endif
