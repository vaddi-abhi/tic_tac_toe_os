#include <stdio.h>
#include<string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>

int sock;

void sigterm_handler(int sig) {
    printf("\nServer terminated. Exiting...\n");
    close(sock);
    exit(0);
}

void sigusr1_handler(int sig) {
    printf("\nGame Over Signal Received.\n");
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server;
    char buffer[1024];
    int move;
    char symbol;

    signal(SIGTERM, sigterm_handler);
    signal(SIGUSR1, sigusr1_handler);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    inet_pton(AF_INET, argv[1], &server.sin_addr);

    connect(sock, (struct sockaddr*)&server, sizeof(server));

    read(sock, &symbol, 1);
    printf("You are Player %c\n", symbol);

    while(1) {
        memset(buffer, 0, sizeof(buffer));
        read(sock, buffer, sizeof(buffer));
        printf("%s", buffer);

        if(strstr(buffer, "Your move")) {
            scanf("%d", &move);
            write(sock, &move, sizeof(move));
        }
    }
    return 0;
}
