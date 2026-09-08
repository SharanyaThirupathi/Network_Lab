#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9090
#define BUFFER_SIZE 1024

int main()
{
    int server, client;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len;

    char filename[100];
    char buffer[BUFFER_SIZE];

    FILE *fp;
    int bytes_read;

    // Create socket
    server = socket(AF_INET, SOCK_STREAM, 0);

    if (server < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Allow port reuse
    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(server,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        return 1;
    }

    // Listen
    listen(server, 5);

    printf("=================================\n");
    printf("       FILE TRANSFER SERVER\n");
    printf("=================================\n");
    printf("Server started...\n");
    printf("Waiting for client...\n");

    // Accept client
    len = sizeof(client_addr);

    client = accept(server,
                    (struct sockaddr *)&client_addr,
                    &len);

    if (client < 0)
    {
        perror("Accept failed");
        return 1;
    }

    printf("Client connected.\n");

    // Receive filename
    memset(filename, 0, sizeof(filename));

    recv(client, filename,
         sizeof(filename), 0);

    printf("Requested file: %s\n", filename);

    // Open file
    fp = fopen(filename, "rb");

    if (fp == NULL)
    {
        printf("File not found!\n");

        strcpy(buffer, "NOT_FOUND");

        send(client, buffer,
             strlen(buffer) + 1, 0);

        close(client);
        close(server);

        return 0;
    }

    // Tell client file exists
    strcpy(buffer, "FILE_FOUND");

    send(client, buffer,
         strlen(buffer) + 1, 0);

    // Send file
    while ((bytes_read = fread(buffer, 1,
                              BUFFER_SIZE, fp)) > 0)
    {
        send(client, buffer,
             bytes_read, 0);
    }

    printf("File sent successfully.\n");

    fclose(fp);

    close(client);
    close(server);

    return 0;
}
