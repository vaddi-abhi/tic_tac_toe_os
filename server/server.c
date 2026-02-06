#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <time.h>

int server_fd, client[2];
char board[9] = {'1','2','3','4','5','6','7','8','9'};
int turn;

void cleanup() {
    for(int i=0;i<2;i++) {
        if(client[i] > 0) {
            kill(client[i], SIGTERM);
            close(client[i]);
        }
    }
    close(server_fd);
    printf("\nServer shutting down cleanly.\n");
    exit(0);
}

void sigint_handler(int sig) {
    printf("\nSIGINT received. Terminating game...\n");
    cleanup();
}

void print_board(char *buf) {
    sprintf(buf,
        "\n %c | %c | %c\n"
        "---+---+---\n"
        " %c | %c | %c\n"
        "---+---+---\n"
        " %c | %c | %c\n",
        board[0],board[1],board[2],
        board[3],board[4],board[5],
        board[6],board[7],board[8]);
}

int check_win() {
    int w[8][3]={{0,1,2},{3,4,5},{6,7,8},
                 {0,3,6},{1,4,7},{2,5,8},
                 {0,4,8},{2,4,6}};
    for(int i=0;i<8;i++)
        if(board[w[i][0]]==board[w[i][1]] &&
           board[w[i][1]]==board[w[i][2]])
            return 1;
    return 0;
}

int main() {
    struct sockaddr_in addr;
    char buffer[1024];
    int move;
    srand(time(NULL));
    turn = rand()%2;

    if (turn == 0) {
    write(client[0], "You start first\n", 16);
    write(client[1], "Opponent starts first\n", 22);
} else {
    write(client[1], "You start first\n", 16);
    write(client[0], "Opponent starts first\n", 22);
}

    

    signal(SIGINT, sigint_handler);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 2);

    printf("Server running. Waiting for players...\n");

    for(int i=0;i<2;i++) {
        client[i] = accept(server_fd, NULL, NULL);
        write(client[i], i==0 ? "X" : "O", 1);
    }

    while(1) {
        memset(buffer, 0, sizeof(buffer));
        print_board(buffer);

        write(client[0], buffer, strlen(buffer));
        write(client[1], buffer, strlen(buffer));

        write(client[turn], "Your move: ", 11);
        read(client[turn], &move, sizeof(move));

        board[move-1] = turn==0 ? 'X' : 'O';

        if(check_win()) {
            print_board(buffer);
            write(client[0], buffer, strlen(buffer));
            write(client[1], buffer, strlen(buffer));

            write(client[turn], "You win!\n", 9);
            write(client[1-turn], "You lose!\n", 10);

            kill(client[0], SIGUSR1);
            kill(client[1], SIGUSR1);
            break;
        }
        turn = 1 - turn;
    }
    cleanup();
    return 0;
}
