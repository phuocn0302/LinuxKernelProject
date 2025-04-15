#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int sock;

void clear_input_line() {
    printf("\r\033[K");
    fflush(stdout);
}

void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            clear_input_line();
            printf("Disconnected from server.\n");
            exit(0);
        }

        if (strcmp(buffer, "Server is shutting down. Goodbye!\n") == 0) {
            clear_input_line();
            printf("%s", buffer);
            exit(0);
        }

        clear_input_line();
        printf("%s", buffer);
        printf("You: ");
        fflush(stdout);
    }
    return NULL;
}

int main() {
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    pthread_t recv_thread;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect error");
        exit(1);
    }

    printf("Connected to server.\n");
    pthread_create(&recv_thread, NULL, receive_messages, NULL);

    while (1) {
        printf("You: ");
        fflush(stdout);
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) continue;
        buffer[strcspn(buffer, "\n")] = 0;
        send(sock, buffer, strlen(buffer), 0);
    }

    close(sock);
    return 0;
}

