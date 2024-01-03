#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#define CAN_INTERFACE "can0" // Change this to your CAN interface name

void printCANMessage(struct can_frame frame) {
    printf("ID: 0x%03X  DLC: %d  Data:", frame.can_id, frame.can_dlc);
    for (int i = 0; i < frame.can_dlc; i++) {
        printf(" %02X", frame.data[i]);
    }
    printf("\n");
}

int main() {
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;
    int s;

    // Create a socket
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket creation error");
        return 1;
    }

    // Set up the CAN interface
    strcpy(ifr.ifr_name, CAN_INTERFACE);
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("IOCTL error");
        close(s);
        return 1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Bind the socket to the CAN interface
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Binding error");
        close(s);
        return 1;
    }

    printf("Listening for CAN messages...\n");

    while (1) {
        int nbytes = read(s, &frame, sizeof(struct can_frame));

        if (nbytes < 0) {
            perror("Read error");
            close(s);
            return 1;
        }

        if ((unsigned int)nbytes < sizeof(struct can_frame)) {
            fprintf(stderr, "Incomplete CAN frame\n");
            continue;
        }

        printCANMessage(frame);
    }

    close(s);
    return 0;
}

