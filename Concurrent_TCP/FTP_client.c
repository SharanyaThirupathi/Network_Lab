#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9090
#define BUFFER_SIZE 1024

int main()
{
    int client;
    struct sockaddr_in server_addr;

    char filename[100];
    char buffer[BUFFER_SIZE];

    FILE *fp;

    int bytes_received;

    // Create socket
    client = socket(AF_INET, SOCK_STREAM, 0);

    if (client < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Same system
    server_addr.sin_addr.s_addr =
        inet_addr("127.0.0.1");

    // For different system, use server IP:
    // inet_addr("192.168.1.10");

    // Connect
    if (connect(client,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        return 1;
    }

    printf("=================================\n");
    printf("        FILE TRANSFER CLIENT\n");
    printf("=================================\n");

    printf("Connected to server.\n");

    // Ask filename
    printf("Enter filename: ");
    scanf("%s", filename);

    // Send filename
    send(client, filename,
         strlen(filename) + 1, 0);

    // Receive file status
    memset(buffer, 0, BUFFER_SIZE);

    recv(client, buffer,
         BUFFER_SIZE, 0);

    if (strcmp(buffer, "NOT_FOUND") == 0)
    {
        printf("File not found on server.\n");

        close(client);
        return 0;
    }

    // Create received file
    fp = fopen(filename, "wb");

    if (fp == NULL)
    {
        printf("Cannot create file.\n");
        close(client);
        return 1;
    }

    // Receive file
    while ((bytes_received =
            recv(client, buffer,
                 BUFFER_SIZE, 0)) > 0)
    {
        fwrite(buffer, 1,
               bytes_received, fp);
    }

    printf("File received successfully.\n");

    fclose(fp);

    close(client);

    return 0;
}
