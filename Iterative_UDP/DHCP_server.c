#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX 20

struct Subnet
{
    int network;
    int hostBits;
    int prefix;
    int total;
    int available;
    int block;
    int required;
};

int main()
{
    int sockfd;
    struct sockaddr_in server, client;
    socklen_t clientSize;

    struct Subnet subnet[MAX];

    int a, b, c, d;
    int prefix;
    int n;
    int i;

    char clientName[100];
    int requestedSubnet;

    char ip[50];
    char mask[50];
    char gateway[50];

    printf("\n================ DHCP SERVER ================\n\n");

    printf("Enter Base Address : ");
    scanf("%d.%d.%d.%d", &a, &b, &c, &d);

    printf("Enter Prefix       : ");
    scanf("%d", &prefix);

    printf("Enter Number of Subnets : ");
    scanf("%d", &n);

    for (i = 0; i < n; i++)
    {
        printf("\nEnter Address Space Required for Subnet %d : ",
               i + 1);

        scanf("%d", &subnet[i].required);

        subnet[i].hostBits = 0;

        while ((1 << subnet[i].hostBits) - 2
               < subnet[i].required)
        {
            subnet[i].hostBits++;
        }

        subnet[i].prefix =
            32 - subnet[i].hostBits;

        subnet[i].total =
            1 << subnet[i].hostBits;

        subnet[i].available =
            subnet[i].total - 2;

        subnet[i].block =
            subnet[i].total;
    }

    printf("\n\nCalculating Subnet Details...\n\n");

    printf("Base Network       : %d.%d.%d.%d/%d\n",
           a, b, c, d, prefix);

    printf("Number of Subnets  : %d\n", n);

    int current = d;

    for (i = 0; i < n; i++)
    {
        int x1 = a;
        int x2 = b;
        int x3 = c;
        int x4 = current;

        printf("\n---------------------------------------------\n");

        printf("Subnet Number      : %d\n",
               i + 1);

        printf("Address Space      : %d hosts\n",
               subnet[i].required);

        printf("Host Bits          : %d\n",
               subnet[i].hostBits);

        printf("Network Bits       : %d\n",
               subnet[i].prefix);

        printf("New Prefix         : /%d\n",
               subnet[i].prefix);

        printf("Total Addresses    : %d\n",
               subnet[i].total);

        printf("Available Hosts    : %d\n",
               subnet[i].available);

        if (subnet[i].prefix == 26)
        {
            printf("Subnet Mask        : 255.255.255.192\n");
        }
        else if (subnet[i].prefix == 27)
        {
            printf("Subnet Mask        : 255.255.255.224\n");
        }
        else if (subnet[i].prefix == 28)
        {
            printf("Subnet Mask        : 255.255.255.240\n");
        }
        else if (subnet[i].prefix == 29)
        {
            printf("Subnet Mask        : 255.255.255.248\n");
        }
        else if (subnet[i].prefix == 30)
        {
            printf("Subnet Mask        : 255.255.255.252\n");
        }

        printf("Network Address    : %d.%d.%d.%d/%d\n",
               x1, x2, x3, x4,
               subnet[i].prefix);

        subnet[i].network = current;

        current = current + subnet[i].block;
    }

    printf("\n---------------------------------------------\n");

    /*
       Create UDP socket
    */

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        printf("Socket creation failed!\n");
        return 0;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd,
             (struct sockaddr *)&server,
             sizeof(server)) < 0)
    {
        printf("Bind failed!\n");
        return 0;
    }

    printf("\nDHCP SERVER CONFIGURATION COMPLETED\n");
    printf("SERVER IS WAITING FOR CLIENT REQUEST...\n");

    while (1)
    {
        clientSize = sizeof(client);

        /*
           Receive client name
        */

        recvfrom(sockfd,
                 clientName,
                 sizeof(clientName),
                 0,
                 (struct sockaddr *)&client,
                 &clientSize);

        /*
           Receive subnet number
        */

        recvfrom(sockfd,
                 &requestedSubnet,
                 sizeof(requestedSubnet),
                 0,
                 (struct sockaddr *)&client,
                 &clientSize);

        printf("\n\nClient Request Received...\n");

        printf("Client Name       : %s\n",
               clientName);

        printf("Requested Subnet  : %d\n",
               requestedSubnet);

        if (requestedSubnet < 1 ||
            requestedSubnet > n)
        {
            strcpy(ip, "Invalid Subnet");

            sendto(sockfd,
                   ip,
                   strlen(ip) + 1,
                   0,
                   (struct sockaddr *)&client,
                   clientSize);

            continue;
        }

        i = requestedSubnet - 1;

        /*
           First usable IP
        */

        int assigned = subnet[i].network + 1;

        sprintf(ip,
                "%d.%d.%d.%d",
                a,
                b,
                c,
                assigned);

        /*
           Mask
        */

        if (subnet[i].prefix == 26)
        {
            strcpy(mask, "255.255.255.192");
        }
        else if (subnet[i].prefix == 27)
        {
            strcpy(mask, "255.255.255.224");
        }
        else if (subnet[i].prefix == 28)
        {
            strcpy(mask, "255.255.255.240");
        }
        else if (subnet[i].prefix == 29)
        {
            strcpy(mask, "255.255.255.248");
        }
        else
        {
            strcpy(mask, "255.255.255.252");
        }

        strcpy(gateway, ip);

        printf("\nDHCP Server Processing Request...\n");

        printf("\nSelected Network  : %d.%d.%d.%d/%d\n",
               a,
               b,
               c,
               subnet[i].network,
               subnet[i].prefix);

        printf("Subnet Mask       : %s\n",
               mask);

        printf("Available Hosts   : %d\n",
               subnet[i].available);

        printf("\nIP Address Assigned Successfully!\n");

        /*
           Send IP
        */

        sendto(sockfd,
               ip,
               strlen(ip) + 1,
               0,
               (struct sockaddr *)&client,
               clientSize);

        /*
           Send mask
        */

        sendto(sockfd,
               mask,
               strlen(mask) + 1,
               0,
               (struct sockaddr *)&client,
               clientSize);

        /*
           Send gateway
        */

        sendto(sockfd,
               gateway,
               strlen(gateway) + 1,
               0,
               (struct sockaddr *)&client,
               clientSize);
    }

    close(sockfd);

    return 0;
}
