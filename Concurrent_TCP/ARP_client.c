#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8083

int main()
{
    int client;
    struct sockaddr_in server_addr;

    char ip[20];
    char reply[50];


    /* ---------------- SOCKET CREATION ---------------- */

    client = socket(AF_INET, SOCK_STREAM, 0);

    if (client < 0)
    {
        perror("Socket creation failed");
        return 1;
    }


    /* ---------------- SERVER ADDRESS ---------------- */

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    /*
       Same computer:
       127.0.0.1

       Different computer:
       Use SERVER IP
    */

    server_addr.sin_addr.s_addr =
        inet_addr("127.0.0.1");


    /* ---------------- CONNECT ---------------- */

    if (connect(client,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        return 1;
    }


    printf("\n========================================\n");
    printf("              ARP CLIENT\n");
    printf("========================================\n");

    printf("Connected to ARP Server.\n");


    /* ---------------- GET IP ---------------- */

    printf("\nEnter IP Address: ");
    scanf("%s", ip);


    /* ---------------- SEND IP ---------------- */

    send(client,
         ip,
         strlen(ip) + 1,
         0);


    /* ---------------- RECEIVE MAC ---------------- */

    memset(reply, 0, sizeof(reply));

    recv(client,
         reply,
         sizeof(reply),
         0);


    /* ---------------- DISPLAY REPLY ---------------- */

    printf("\n========================================\n");
    printf("             ARP REPLY\n");
    printf("========================================\n");

    printf("%s\n", reply);

    printf("========================================\n");


    /* ---------------- CLOSE ---------------- */

    close(client);

    return 0;
}
