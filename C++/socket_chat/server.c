#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int client_sockets[MAX_CLIENTS];
int client_ids[MAX_CLIENTS];
int client_count = 0;
int server_sock;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
volatile int running = 1;

void clear_input_line() {
    printf("\r\033[K");
    fflush(stdout);
}

void broadcast(const char *msg, int exclude_sock) {
    pthread_mutex_lock(&lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != 0 && client_sockets[i] != exclude_sock) {
            send(client_sockets[i], msg, strlen(msg), 0);
        }
    }
    pthread_mutex_unlock(&lock);
}

void *handle_client(void *arg) {
    int index = *(int *)arg;
    free(arg);
    int sock = client_sockets[index];
    int id = client_ids[index];
    char buffer[BUFFER_SIZE], msg[BUFFER_SIZE + 64];

    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes <= 0) break;

        snprintf(msg, sizeof(msg), "Client %d: %s\n", id, buffer);
        clear_input_line();
        printf("%s", msg);
        printf("You: ");
        fflush(stdout);
        broadcast(msg, sock);
    }

    close(sock);
    pthread_mutex_lock(&lock);
    client_sockets[index] = 0;
    client_ids[index] = 0;
    pthread_mutex_unlock(&lock);
    clear_input_line();
    printf("Client %d disconnected.\n", id);
    printf("You: ");
    fflush(stdout);
    return NULL;
}

void *server_input_thread(void *arg) {
    char buffer[BUFFER_SIZE], msg[BUFFER_SIZE + 64];
    while (running) {
        printf("You: ");
        fflush(stdout);
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) continue;
        buffer[strcspn(buffer, "\n")] = 0;

        snprintf(msg, sizeof(msg), "Server: %s\n", buffer);
        broadcast(msg, -1);
    }
    return NULL;
}

void handle_shutdown(int sig) {
    running = 0;
    clear_input_line();
    printf("\nShutting down server...\n");

    broadcast("Server is shutting down. Goodbye!\n", -1);

    pthread_mutex_lock(&lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != 0) {
            close(client_sockets[i]);
            client_sockets[i] = 0;
        }
    }
    pthread_mutex_unlock(&lock);

    close(server_sock);
    exit(0);
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t tid, server_input;

    signal(SIGINT, handle_shutdown);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind error");
        exit(1);
    }

    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("Listen error");
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);
    pthread_create(&server_input, NULL, server_input_thread, NULL);

    while (running) {
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (client_sock < 0) break;

        pthread_mutex_lock(&lock);
        int index = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == 0) {
                index = i;
                client_sockets[i] = client_sock;
                client_ids[i] = ++client_count;
                break;
            }
        }
        pthread_mutex_unlock(&lock);

        if (index != -1) {
            char connect_msg[64];
            snprintf(connect_msg, sizeof(connect_msg), "Client %d connected.\n", client_ids[index]);

            clear_input_line();
            printf("%s", connect_msg);
            printf("You: ");
            fflush(stdout);

            broadcast(connect_msg, client_sock);

            int *arg = (int *)malloc(sizeof(int));
            *arg = index;
            pthread_create(&tid, NULL, handle_client, arg);
            pthread_detach(tid);
        } else {
            char *msg = "Server full. Try again later.\n";
            send(client_sock, msg, strlen(msg), 0);
            close(client_sock);
        }
    }

    return 0;
}

