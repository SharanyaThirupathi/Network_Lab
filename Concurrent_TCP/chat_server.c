#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, client_socket[MAX_CLIENTS];
    int max_sd, activity, new_socket;
    int client_count = 0;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen;
    fd_set readfds;

    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    // Initialize client sockets
    for (int i = 0; i < MAX_CLIENTS; i++)
        client_socket[i] = 0;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        exit(1);
    }

    // Listen
    if (listen(server_fd, MAX_CLIENTS) < 0)
    {
        perror("Listen failed");
        exit(1);
    }

    printf("=================================\n");
    printf(" Concurrent TCP Chat Server\n");
    printf("=================================\n");
    printf("Server started...\n");
    printf("Port : %d\n", PORT);
    printf("Waiting for clients...\n");

    while (1)
    {
        FD_ZERO(&readfds);

        // Add server socket
        FD_SET(server_fd, &readfds);

        // Add STDIN
        FD_SET(0, &readfds);

        max_sd = server_fd;

        // Add client sockets
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_socket[i] > 0)
                FD_SET(client_socket[i], &readfds);

            if (client_socket[i] > max_sd)
                max_sd = client_socket[i];
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0)
        {
            perror("Select error");
            continue;
        }

        // New client connection
        if (FD_ISSET(server_fd, &readfds))
        {
            addrlen = sizeof(client_addr);

            new_socket = accept(server_fd,
                                (struct sockaddr *)&client_addr,
                                &addrlen);

            if (new_socket < 0)
            {
                perror("Accept failed");
                continue;
            }

            printf("\nNew client connected: %s\n",
                   inet_ntoa(client_addr.sin_addr));

            // Add client
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    client_count++;

                    // SERVER STARTS CHAT
                    strcpy(message,
                           "Hello! Server connected. Please reply.");

                    send(new_socket, message,
                         strlen(message), 0);

                    printf("Client %d connected.\n", i + 1);
                    break;
                }
            }
        }

        // Server types message
        if (FD_ISSET(0, &readfds))
        {
            fgets(message, BUFFER_SIZE, stdin);

            // Format:
            // client_number message
            //
            // Example:
            // 1 Hello Client 1

            int client_no;
            char *msg_start;

            client_no = atoi(message);

            msg_start = strchr(message, ' ');

            if (msg_start != NULL)
            {
                msg_start++;

                if (client_no >= 1 &&
                    client_no <= MAX_CLIENTS &&
                    client_socket[client_no - 1] > 0)
                {
                    send(client_socket[client_no - 1],
                         msg_start,
                         strlen(msg_start), 0);
                }
                else
                {
                    printf("Invalid client number!\n");
                }
            }
            else
            {
                printf("Use: client_number message\n");
                printf("Example: 1 Hello Client\n");
            }
        }

        // Receive messages from clients
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = client_socket[i];

            if (sd > 0 && FD_ISSET(sd, &readfds))
            {
                int bytes_received;

                memset(buffer, 0, BUFFER_SIZE);

                bytes_received = recv(sd, buffer,
                                      BUFFER_SIZE - 1, 0);

                if (bytes_received <= 0)
                {
                    printf("Client %d disconnected.\n", i + 1);

                    close(sd);
                    client_socket[i] = 0;
                    client_count--;
                }
                else
                {
                    buffer[bytes_received] = '\0';

                    printf("\nClient %d: %s",
                           i + 1, buffer);
                }
            }
        }
    }

    close(server_fd);

    return 0;
}
