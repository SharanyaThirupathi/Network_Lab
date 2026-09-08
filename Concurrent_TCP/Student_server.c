#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/file.h>
#include <signal.h>
#include <time.h>

#define PORT 8090
#define MAX 1024

void log_activity(char *username, char *activity)
{
    FILE *fp;
    time_t now;

    fp = fopen("exam_log.txt", "a");

    if (fp == NULL)
    {
        perror("fopen");
        return;
    }

    flock(fileno(fp), LOCK_EX);

    time(&now);

    fprintf(fp, "%s - %s - %s",
            username, activity, ctime(&now));

    flock(fileno(fp), LOCK_UN);

    fclose(fp);
}

int authenticate(char *username, char *password)
{
    FILE *fp;
    char u[50], p[50];

    fp = fopen("users.txt", "r");

    if (fp == NULL)
        return 0;

    while (fscanf(fp, "%s %s", u, p) != EOF)
    {
        if (strcmp(username, u) == 0 &&
            strcmp(password, p) == 0)
        {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

void handle_client(int client)
{
    char username[50];
    char password[50];
    char buffer[MAX];

    int score = 0;
    int answer;

    send(client,
         "\n===== ONLINE EXAM =====\n",
         26, 0);

    send(client,
         "Username: ",
         10, 0);

    recv(client, username, sizeof(username), 0);
    username[strcspn(username, "\n")] = '\0';

    send(client,
         "Password: ",
         10, 0);

    recv(client, password, sizeof(password), 0);
    password[strcspn(password, "\n")] = '\0';

    if (authenticate(username, password))
    {
        send(client,
             "\nLogin successful!\n",
             19, 0);

        log_activity(username, "Login successful");
    }
    else
    {
        send(client,
             "\nInvalid username or password.\n",
             33, 0);

        log_activity(username, "Login failed");

        close(client);
        exit(0);
    }

    /* Question 1 */
    strcpy(buffer,
           "\nQ1. What is the capital of India?\n"
           "1. Mumbai\n"
           "2. Delhi\n"
           "3. Chennai\n"
           "4. Kolkata\n"
           "Enter answer: ");

    send(client, buffer, strlen(buffer), 0);

    recv(client, &answer, sizeof(answer), 0);

    if (answer == 2)
        score++;

    /* Question 2 */
    strcpy(buffer,
           "\nQ2. Which language is used for system programming?\n"
           "1. HTML\n"
           "2. CSS\n"
           "3. C\n"
           "4. SQL\n"
           "Enter answer: ");

    send(client, buffer, strlen(buffer), 0);

    recv(client, &answer, sizeof(answer), 0);

    if (answer == 3)
        score++;

    /* Question 3 */
    strcpy(buffer,
           "\nQ3. Which protocol is connection oriented?\n"
           "1. TCP\n"
           "2. UDP\n"
           "3. IP\n"
           "4. ARP\n"
           "Enter answer: ");

    send(client, buffer, strlen(buffer), 0);

    recv(client, &answer, sizeof(answer), 0);

    if (answer == 1)
        score++;

    /* Store result safely */
    FILE *fp = fopen("results.txt", "a");

    if (fp != NULL)
    {
        flock(fileno(fp), LOCK_EX);

        time_t now;
        time(&now);

        fprintf(fp,
                "Student: %s | Score: %d/3 | %s",
                username,
                score,
                ctime(&now));

        flock(fileno(fp), LOCK_UN);

        fclose(fp);
    }

    sprintf(buffer,
            "\n===== EXAM COMPLETED =====\n"
            "Student: %s\n"
            "Final Score: %d / 3\n",
            username,
            score);

    send(client, buffer, strlen(buffer), 0);

    log_activity(username, "Exam completed");

    close(client);
    exit(0);
}

int main()
{
    int server, client;
    struct sockaddr_in server_addr, client_addr;
    socklen_t size;

    signal(SIGCHLD, SIG_IGN);

    server = socket(AF_INET, SOCK_STREAM, 0);

    if (server < 0)
    {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    listen(server, 5);

    printf("====================================\n");
    printf(" Concurrent Online Exam Server\n");
    printf("====================================\n");
    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        size = sizeof(client_addr);

        client = accept(server,
                        (struct sockaddr *)&client_addr,
                        &size);

        if (client < 0)
        {
            perror("accept");
            continue;
        }

        printf("Student connected.\n");

        if (fork() == 0)
        {
            close(server);

            handle_client(client);
        }

        close(client);
    }

    close(server);

    return 0;
}
