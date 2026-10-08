#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

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
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    struct Numbers n;
    struct Result r;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        return 1;
    }

    listen(server_socket, 5);

    printf("========== CONCURRENT RPC SERVER ==========\n");
    printf("Server started...\n");
    printf("Waiting for clients...\n");

    while (1)
    {
        addr_size = sizeof(client_addr);

        client_socket = accept(server_socket,
                               (struct sockaddr *)&client_addr,
                               &addr_size);

        if (client_socket < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("\nClient connected\n");

        if (fork() == 0)
        {
            close(server_socket);

            recv(client_socket, &n, sizeof(n), 0);

            printf("\nReceived numbers from client\n");
            printf("Number 1 = %.2f\n", n.num1);
            printf("Number 2 = %.2f\n", n.num2);

            /* Remote procedure calculation */

            r.addition = n.num1 + n.num2;
            r.subtraction = n.num1 - n.num2;
            r.multiplication = n.num1 * n.num2;

            if (n.num2 != 0)
                r.division = n.num1 / n.num2;
            else
                r.division = 0;

            printf("\n========== RPC RESULT ==========\n");
            printf("Addition       = %.2f\n", r.addition);
            printf("Subtraction    = %.2f\n", r.subtraction);
            printf("Multiplication = %.2f\n", r.multiplication);

            if (n.num2 == 0)
                printf("Division       = Cannot divide by zero\n");
            else
                printf("Division       = %.2f\n", r.division);

            send(client_socket, &r, sizeof(r), 0);

            printf("\nResult sent to client.\n");

            close(client_socket);
            exit(0);
        }

        close(client_socket);
    }

    close(server_socket);

    return 0;
}
