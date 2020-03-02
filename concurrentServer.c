#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define PORT 8080

int valread;
int buffer[1024] = {0};
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *socket_thread(void *arg) {
    int new_socket = *((int*) arg);
    int number = 0;

    valread = read(new_socket, buffer, 1024);

    pthread_mutex_lock(&lock);

    if (buffer[0] == -1)
        return;
    number = buffer[0];
    printf("Client Number: %d\n", number);
    ++number;
    send(new_socket, &number, sizeof(number), 0);

    pthread_mutex_unlock(&lock);
    sleep(1);
    close(new_socket);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int server_socket, new_socket;
    struct sockaddr_in server_address;
    struct sockaddr_storage server_storage;
    socklen_t addr_size;
    pthread_t thread_id[10];
    int i = 0;

    server_socket = socket(PF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        addr_size = sizeof server_storage;
        new_socket = accept(server_socket, (struct sockaddr *) &server_storage, &addr_size);

        if (pthread_create(&thread_id[i], NULL, socket_thread, &new_socket) != 0)
            printf("Failed to create thread");

        if (i >= 10) {
            i = 0;
            while (i < 10) {
                pthread_join(thread_id[i++], NULL);
            }
            i = 0;
        }
    }

    return 0;
}