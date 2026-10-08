#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct Numbers
{
    float num1;
    float num2;
};

struct Result
{
    float addition;
    float subtraction;
    float multiplication;
    float division;
};

int main()
{
    int client_socket;
    struct sockaddr_in server_addr;

    struct Numbers n;
    struct Result r;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        return 1;
    }

    printf("========== RPC CLIENT ==========\n\n");

    printf("Enter first number: ");
    scanf("%f", &n.num1);

    printf("Enter second number: ");
    scanf("%f", &n.num2);

    printf("\nCalling remote procedure...\n");

    send(client_socket, &n, sizeof(n), 0);

    recv(client_socket, &r, sizeof(r), 0);

    printf("\n========== RESULT ==========\n");

    printf("Addition       = %.2f\n", r.addition);
    printf("Subtraction    = %.2f\n", r.subtraction);
    printf("Multiplication = %.2f\n", r.multiplication);

    if (n.num2 == 0)
        printf("Division       = Cannot divide by zero\n");
    else
        printf("Division       = %.2f\n", r.division);

    printf("\nAll operations completed.\n");

    close(client_socket);

    return 0;
}
