#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#define PORT 8080

int main()
{
    int sockfd;

    struct sockaddr_in server;
    struct sockaddr_in client;

    socklen_t clientSize;

    char message[100];

    printf("\n=============== CHAT SERVER ===============\n");

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

    printf("\nServer is ready...\n");
    printf("Waiting for client...\n");

    clientSize = sizeof(client);

    /*
       Wait for client to connect
       Client sends READY automatically
    */

    recvfrom(sockfd,
             message,
             sizeof(message),
             0,
             (struct sockaddr *)&client,
             &clientSize);

    /*
       Now server knows client address
       Server starts the chat
    */

    printf("\nServer : ");
    scanf(" %[^\n]", message);

    sendto(sockfd,
           message,
           strlen(message) + 1,
           0,
           (struct sockaddr *)&client,
           clientSize);

    while (1)
    {
        /*
           Receive client message
        */

        recvfrom(sockfd,
                 message,
                 sizeof(message),
                 0,
                 (struct sockaddr *)&client,
                 &clientSize);

        printf("Client : %s\n", message);

        if (strcmp(message, "exit") == 0)
        {
            printf("\nClient ended the chat.\n");
            break;
        }

        /*
           Server reply
        */

        printf("Server : ");
        scanf(" %[^\n]", message);

        sendto(sockfd,
               message,
               strlen(message) + 1,
               0,
               (struct sockaddr *)&client,
               clientSize);

        if (strcmp(message, "exit") == 0)
        {
            printf("\nServer ended the chat.\n");
            break;
        }
    }

    close(sockfd);

    return 0;
}
