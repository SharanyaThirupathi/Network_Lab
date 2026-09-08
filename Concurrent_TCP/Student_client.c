#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8090
#define MAX 1024

int main()
{
    int sock;
    struct sockaddr_in server_addr;

    char buffer[MAX];
    char username[50];
    char password[50];

    int answer;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect");
        exit(1);
    }

    /* Username */
    recv(sock, buffer, MAX, 0);
    printf("%s", buffer);

    fgets(username, sizeof(username), stdin);
    send(sock, username, strlen(username), 0);

    /* Password */
    recv(sock, buffer, MAX, 0);
    printf("%s", buffer);

    fgets(password, sizeof(password), stdin);
    send(sock, password, strlen(password), 0);

    /* Login result */
    memset(buffer, 0, MAX);
    recv(sock, buffer, MAX, 0);

    printf("%s", buffer);

    if (strstr(buffer, "Invalid") != NULL)
    {
        close(sock);
        return 0;
    }

    /* Q1 */
    memset(buffer, 0, MAX);
    recv(sock, buffer, MAX, 0);

    printf("%s", buffer);

    scanf("%d", &answer);
    send(sock, &answer, sizeof(answer), 0);

    /* Q2 */
    memset(buffer, 0, MAX);
    recv(sock, buffer, MAX, 0);

    printf("%s", buffer);

    scanf("%d", &answer);
    send(sock, &answer, sizeof(answer), 0);

    /* Q3 */
    memset(buffer, 0, MAX);
    recv(sock, buffer, MAX, 0);

    printf("%s", buffer);

    scanf("%d", &answer);
    send(sock, &answer, sizeof(answer), 0);

    /* Result */
    memset(buffer, 0, MAX);
    recv(sock, buffer, MAX, 0);

    printf("%s", buffer);

    close(sock);

    return 0;
}
