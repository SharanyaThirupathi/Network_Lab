#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX 20

struct DNS
{
    char domain[50];
    char ip[30];
};

int main()
{
    int serverSocket;

    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;

    socklen_t clientLength;

    struct DNS table[MAX];

    int numberOfRecords;

    char domain[50];
    char ip[30];

    int found;

    printf("\n");
    printf("=============== DNS SERVER ===============\n\n");

    printf("Enter Number of DNS Records : ");
    scanf("%d", &numberOfRecords);

    printf("\n");

    for (int i = 0; i < numberOfRecords; i++)
    {
        printf("Enter Domain Name %d : ",
               i + 1);

        scanf("%s",
              table[i].domain);

        printf("Enter IP Address %d : ",
               i + 1);

        scanf("%s",
              table[i].ip);
    }

    printf("\n");
    printf("DNS TABLE\n");
    printf("--------------------------------\n");

    for (int i = 0; i < numberOfRecords; i++)
    {
        printf("Domain : %s\n",
               table[i].domain);

        printf("IP     : %s\n",
               table[i].ip);

        printf("--------------------------------\n");
    }

    serverSocket =
        socket(AF_INET,
               SOCK_DGRAM,
               0);

    if (serverSocket < 0)
    {
        printf("Socket creation failed!\n");
        return 0;
    }

    serverAddress.sin_family =
        AF_INET;

    serverAddress.sin_port =
        htons(PORT);

    serverAddress.sin_addr.s_addr =
        INADDR_ANY;

    if (bind(serverSocket,
             (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0)
    {
        printf("Bind failed!\n");
        return 0;
    }

    printf("\nDNS SERVER IS READY...\n");
    printf("SERVER IS WAITING FOR CLIENT REQUEST...\n");

    while (1)
    {
        clientLength =
            sizeof(clientAddress);

        /*
           Receive domain name
        */

        recvfrom(serverSocket,
                 domain,
                 sizeof(domain),
                 0,
                 (struct sockaddr *)&clientAddress,
                 &clientLength);

        printf("\n");
        printf("DNS Request Received\n");

        printf("Requested Domain : %s\n",
               domain);

        found = 0;

        /*
           Search domain
        */

        for (int i = 0; i < numberOfRecords; i++)
        {
            if (strcmp(domain,
                       table[i].domain) == 0)
            {
                strcpy(ip,
                       table[i].ip);

                found = 1;

                break;
            }
        }

        /*
           Send result
        */

        if (found == 1)
        {
            printf("Domain Found\n");
            printf("IP Address : %s\n",
                   ip);

            sendto(serverSocket,
                   ip,
                   strlen(ip) + 1,
                   0,
                   (struct sockaddr *)&clientAddress,
                   clientLength);
        }
        else
        {
            strcpy(ip,
                   "Domain Not Found");

            printf("Domain Not Found\n");

            sendto(serverSocket,
                   ip,
                   strlen(ip) + 1,
                   0,
                   (struct sockaddr *)&clientAddress,
                   clientLength);
        }
    }

    close(serverSocket);

    return 0;
}
