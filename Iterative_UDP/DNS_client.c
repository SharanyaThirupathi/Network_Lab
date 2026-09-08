#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define PORT 8080

int main(int argc, char *argv[])
{
    int clientSocket;

    struct sockaddr_in serverAddress;

    socklen_t serverLength;

    char domain[50];
    char ip[30];

    if (argc != 2)
    {
        printf("Usage: ./dns_client <server_ip>\n");
        return 0;
    }

    clientSocket =
        socket(AF_INET,
               SOCK_DGRAM,
               0);

    if (clientSocket < 0)
    {
        printf("Socket creation failed!\n");
        return 0;
    }

    serverAddress.sin_family =
        AF_INET;

    serverAddress.sin_port =
        htons(PORT);

    serverAddress.sin_addr.s_addr =
        inet_addr(argv[1]);

    serverLength =
        sizeof(serverAddress);

    printf("\n");
    printf("=============== DNS CLIENT ===============\n\n");

    printf("Enter Domain Name : ");
    scanf("%s", domain);

    /*
       Send domain name
    */

    sendto(clientSocket,
           domain,
           strlen(domain) + 1,
           0,
           (struct sockaddr *)&serverAddress,
           serverLength);

    printf("\nDNS Request Sent to Server...\n");

    /*
       Receive IP address
    */

    recvfrom(clientSocket,
             ip,
             sizeof(ip),
             0,
             (struct sockaddr *)&serverAddress,
             &serverLength);

    printf("\n");
    printf("Domain Name : %s\n",
           domain);

    printf("IP Address  : %s\n",
           ip);

    close(clientSocket);

    return 0;
}
