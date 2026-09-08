#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int sock;
    struct sockaddr_in server_addr;

    fd_set readfds;

    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    /*
       CHANGE THIS IP
       to the SERVER computer IP
    */
    server_addr.sin_addr.s_addr =
        inet_addr("172.29.84.214");

    // Connect
    if (connect(sock,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        exit(1);
    }

    printf("=================================\n");
    printf(" Connected to Chat Server\n");
    printf("=================================\n");

    while (1)
    {
        FD_ZERO(&readfds);

        // Server messages
        FD_SET(sock, &readfds);

        // Keyboard
        FD_SET(0, &readfds);

        select(sock + 1, &readfds, NULL, NULL, NULL);

        // Message from server
        if (FD_ISSET(sock, &readfds))
        {
            int bytes_received;

            memset(buffer, 0, BUFFER_SIZE);

            bytes_received = recv(sock,
                                  buffer,
                                  BUFFER_SIZE - 1,
                                  0);

            if (bytes_received <= 0)
            {
                printf("Server disconnected.\n");
                break;
            }

            buffer[bytes_received] = '\0';

            printf("\nServer: %s", buffer);
        }

        // Message from client keyboard
        if (FD_ISSET(0, &readfds))
        {
            fgets(message, BUFFER_SIZE, stdin);

            if (strncmp(message, "exit", 4) == 0)
            {
                send(sock, message,
                     strlen(message), 0);

                break;
            }

            send(sock, message,
                 strlen(message), 0);
        }
    }

    close(sock);

    return 0;
}
