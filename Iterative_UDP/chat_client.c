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

    char message[100];

    if (argc != 2)
    {
        printf("Usage: ./c1 <server_ip>\n");
        return 0;
    }

    printf("\n=============== CHAT CLIENT ===============\n");

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        printf("Socket creation failed!\n");
        return 0;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(argv[1]);

    serverSize = sizeof(server);

    /*
       Send READY automatically
       This is only to tell server our address
    */

    strcpy(message, "READY");

    sendto(sockfd,
           message,
           strlen(message) + 1,
           0,
           (struct sockaddr *)&server,
           serverSize);

    /*
       Wait for server to start the chat
    */

    recvfrom(sockfd,
             message,
             sizeof(message),
             0,
             (struct sockaddr *)&server,
             &serverSize);

    printf("\nServer : %s\n", message);

    if (strcmp(message, "exit") == 0)
    {
        printf("\nServer ended the chat.\n");
        close(sockfd);
        return 0;
    }

    while (1)
    {
        /*
           Client sends message
        */

        printf("Client : ");
        scanf(" %[^\n]", message);

        sendto(sockfd,
               message,
               strlen(message) + 1,
               0,
               (struct sockaddr *)&server,
               serverSize);

        if (strcmp(message, "exit") == 0)
        {
            printf("\nClient ended the chat.\n");
            break;
        }

        /*
           Receive server reply
        */

        recvfrom(sockfd,
                 message,
                 sizeof(message),
                 0,
                 (struct sockaddr *)&server,
                 &serverSize);

        printf("Server : %s\n", message);

        if (strcmp(message, "exit") == 0)
        {
            printf("\nServer ended the chat.\n");
            break;
        }
    }

    close(sockfd);

    return 0;
}
