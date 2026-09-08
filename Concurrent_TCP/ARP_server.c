#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8083

struct ARP
{
    char ip[20];
    char mac[20];
};

int main()
{
    int server, client;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len;

    struct ARP table[5];

    char search_ip[20];
    char reply[50];

    int i;
    int found = 0;

    /* ---------------- ARP TABLE ---------------- */

    strcpy(table[0].ip, "192.168.1.1");
    strcpy(table[0].mac, "AA:BB:CC:DD:EE:01");

    strcpy(table[1].ip, "192.168.1.2");
    strcpy(table[1].mac, "AA:BB:CC:DD:EE:02");

    strcpy(table[2].ip, "192.168.1.3");
    strcpy(table[2].mac, "AA:BB:CC:DD:EE:03");

    strcpy(table[3].ip, "192.168.1.4");
    strcpy(table[3].mac, "AA:BB:CC:DD:EE:04");

    strcpy(table[4].ip, "192.168.1.5");
    strcpy(table[4].mac, "AA:BB:CC:DD:EE:05");


    /* ---------------- DISPLAY ARP TABLE ---------------- */

    printf("\n========================================\n");
    printf("              ARP TABLE\n");
    printf("========================================\n");

    printf("IP Address\t\tMAC Address\n");
    printf("----------------------------------------\n");

    for (i = 0; i < 5; i++)
    {
        printf("%-20s %s\n",
               table[i].ip,
               table[i].mac);
    }

    printf("========================================\n");


    /* ---------------- SOCKET CREATION ---------------- */

    server = socket(AF_INET, SOCK_STREAM, 0);

    if (server < 0)
    {
        perror("Socket creation failed");
        return 1;
    }


    /* ---------------- SERVER ADDRESS ---------------- */

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);


    /* ---------------- BIND ---------------- */

    if (bind(server,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        return 1;
    }


    /* ---------------- LISTEN ---------------- */

    listen(server, 5);

    printf("\nARP Server started...\n");
    printf("Waiting for client...\n");


    /* ---------------- ACCEPT CLIENT ---------------- */

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


    /* ---------------- RECEIVE IP ---------------- */

    memset(search_ip, 0, sizeof(search_ip));

    recv(client,
         search_ip,
         sizeof(search_ip),
         0);

    printf("\nClient requested IP: %s\n",
           search_ip);


    /* ---------------- SEARCH ARP TABLE ---------------- */

    for (i = 0; i < 5; i++)
    {
        if (strcmp(search_ip, table[i].ip) == 0)
        {
            sprintf(reply,
                    "MAC Address: %s",
                    table[i].mac);

            found = 1;
            break;
        }
    }


    /* ---------------- SEND REPLY ---------------- */

    if (found == 1)
    {
        send(client,
             reply,
             strlen(reply) + 1,
             0);

        printf("ARP Reply sent: %s\n",
               reply);
    }
    else
    {
        strcpy(reply,
               "IP Address not found");

        send(client,
             reply,
             strlen(reply) + 1,
             0);

        printf("ARP Reply sent: %s\n",
               reply);
    }


    /* ---------------- CLOSE ---------------- */

    close(client);
    close(server);

    return 0;
}
