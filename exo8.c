#include <ncurses.h>
#include <stdlib.h>
#include <ctype.h>

int P4WIDTH;
int P4HEIGHT;
int PLAYERS;
char *CHIP;
char **board;

void INIT_SCREEN(void) {
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
}

void DONE_SCREEN(void) {
    endwin();
    exit(0);
}

void init_board(void) {
    for (int r = 0; r < SIZE; r++)
        for (int c = 0; c < SIZE; c++)  
            board[r][c] = ' ';
}

void draw_board(void) {
    move(0, 0);
    for (int r = SIZE - 1; r >= 0; r--) {
        printw("   ");
        for (int c = 0; c < SIZE; c++) {
            printw("|%c", board[r][c]);
        }
        printw("|\n");
    }

    printw("   ");
    for (int c = 0; c < SIZE; c++)
        printw("++");
    printw("+\n");

    printw("   ");
    for (int c = 0; c < SIZE; c++)
        printw(" %c", 'A' + c);
    printw("\n");

    refresh();
}

int get_col(void) {
    int ch;

    while (1) {
        ch = getch();
        if (ch == 27)
            DONE_SCREEN();
        if (ch == KEY_BACKSPACE || ch == 127)
            return -1;

        if (isalpha(ch)) {
            ch = toupper(ch);
            int col = ch - 'A';
            if (col >= 0 && col < SIZE)
                return col;
        }
    }
}

int add_coin(int col, int player) {
    for (int r = 0; r < SIZE; r++) {
        if (board[r][col] == ' ') {
            board[r][col] = CHIP[player];
            return (player + 1) % PLAYERS;
        }
    }
    return player;
}

static int check_dir(int r, int c, int dr, int dc) {
    char p = board[r][c];
    if (p == ' ') return 0;

    for (int k = 1; k < 4; k++) {
        int nr = r + dr * k;
        int nc = c + dc * k;

        if (nr < 0 || nr >= SIZE || nc < 0 || nc >= SIZE)
            return 0;
        if (board[nr][nc] != p)
            return 0;
    }
    return p;
}

int game_over(void) {
    int full = 1;

    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {

            if (board[r][c] == ' ')
                full = 0;

            int res;
            if ((res = check_dir(r, c, 0, 1))) return res;
            if ((res = check_dir(r, c, 1, 0))) return res;
            if ((res = check_dir(r, c, 1, 1))) return res;
            if ((res = check_dir(r, c, 1, -1))) return res;
        }
    }

    if (full) return 1;
    return 0;
}

void play(void) {
    int player = 0;
    int status = 0;

    while ((status = game_over()) == 0) {
        draw_board();
        printw("\nplayer %d (%c)? ", player + 1, CHIP[player]);
        refresh();

        int col = get_col();
        if (col == -1) continue;

        player = add_coin(col, player);
    }

    draw_board();

    if (status == 1)
        printw("\nMatch nul !\n");
    else
        printw("\nLe gagnant est : %c\n", status);

    refresh();
}

int main(int argc[] , char *argv[]) {
    
    
    char *val = getenv("SIZE");   // lire la variable d'environnement SIZE
    SIZE = (val != NULL) ? atoi(val) : 16;
    INIT_SCREEN();
    init_board();
    play();
    getch();
    DONE_SCREEN();
}
