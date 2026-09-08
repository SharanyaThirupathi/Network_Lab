#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define PORT 8080

int main(int argc, char *argv[])
{
    int sockfd;

    struct sockaddr_in server;
    socklen_t serverSize;

    char clientName[100];

    int requestedSubnet;

    char ip[50];
    char mask[50];
    char gateway[50];

    if (argc != 2)
    {
        printf("Usage: ./dhcp_client <server_ip>\n");
        return 0;
    }

    printf("\n================ DHCP CLIENT ================\n\n");

    printf("Enter Client Name : ");
    scanf(" %[^\n]", clientName);

    printf("Enter Required Subnet Number : ");
    scanf("%d", &requestedSubnet);

    sockfd =
        socket(AF_INET,
               SOCK_DGRAM,
               0);

    if (sockfd < 0)
    {
        printf("Socket creation failed!\n");
        return 0;
    }

    server.sin_family =
        AF_INET;

    server.sin_port =
        htons(PORT);

    server.sin_addr.s_addr =
        inet_addr(argv[1]);

    serverSize =
        sizeof(server);

    /*
       Send client name
    */

    sendto(sockfd,
           clientName,
           strlen(clientName) + 1,
           0,
           (struct sockaddr *)&server,
           serverSize);

    /*
       Send subnet number
    */

    sendto(sockfd,
           &requestedSubnet,
           sizeof(requestedSubnet),
           0,
           (struct sockaddr *)&server,
           serverSize);

    printf("\nClient Request Sent to DHCP Server...\n");

    /*
       Receive IP
    */

    recvfrom(sockfd,
             ip,
             sizeof(ip),
             0,
             (struct sockaddr *)&server,
             &serverSize);

    if (strcmp(ip, "Invalid Subnet") == 0)
    {
        printf("\nInvalid Subnet Number!\n");
        close(sockfd);
        return 0;
    }

    /*
       Receive mask
    */

    recvfrom(sockfd,
             mask,
             sizeof(mask),
             0,
             (struct sockaddr *)&server,
             &serverSize);

    /*
       Receive gateway
    */

    recvfrom(sockfd,
             gateway,
             sizeof(gateway),
             0,
             (struct sockaddr *)&server,
             &serverSize);

    printf("\n");
    printf("Client Name       : %s\n",
           clientName);

    printf("Requested Subnet  : %d\n",
           requestedSubnet);

    printf("\nIP Address Assigned Successfully!\n");

    printf("\nClient Name       : %s\n",
           clientName);

    printf("Assigned IP       : %s\n",
           ip);

    printf("Subnet Mask       : %s\n",
           mask);

    printf("Gateway           : %s\n",
           gateway);

    close(sockfd);

    return 0;
}
