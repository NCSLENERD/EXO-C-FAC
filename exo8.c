#include <ncurses.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

int P4WIDTH;
int P4HEIGHT;
int P4PLAYERS;
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
    for (int r = 0; r < P4HEIGHT; r++)
        for (int c = 0; c < P4WIDTH; c++)  
            board[r][c] = ' ';
}

void draw_board(void) {
    move(0, 0);
    for (int r = P4HEIGHT - 1; r >= 0; r--) {
        printw("   ");
        for (int c = 0; c < P4WIDTH; c++) {
            printw("|%c", board[r][c]);
        }
        printw("|\n");
    }

    printw("   ");
    for (int c = 0; c < P4WIDTH; c++)
        printw("++");
    printw("+\n");

    printw("   ");
    for (int c = 0; c < P4WIDTH; c++)
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
            if (col >= 0 && col < P4WIDTH)
                return col;
        }
    }
}

int add_coin(int col, int player) {
    for (int r = 0; r < P4HEIGHT; r++) {
        if (board[r][col] == ' ') {
            board[r][col] = CHIP[player];
            return (player + 1) % P4PLAYERS;
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

        if (nr < 0 || nr >= P4HEIGHT || nc < 0 || nc >= P4WIDTH)
            return 0;
        if (board[nr][nc] != p)
            return 0;
    }
    return p;
}

int game_over(void) {
    int full = 1;

    for (int r = 0; r < P4HEIGHT; r++) {
        for (int c = 0; c < P4WIDTH; c++) {

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

int main(int argc, char *argv[]) {
    
    if (argc > 1 && (atoi(argv[1]) >= 4 && atoi(argv[1]) <= 26))
	{
		/*if (setenv("WIDTH", argv[1], 1) != 0)
			perror("setenv error");
		P4WIDTH = atoi(getenv("WIDTH"));*/
		P4WIDTH = atoi(argv[1]);
	}
	else
		P4WIDTH = 16;

	if (argc > 2 && (atoi(argv[2]) >= 4 && atoi(argv[2]) <= 26))
	{
		/*if (setenv("HEIGHT", argv[2], 1) != 0)
			perror("setenv error");
		P4HEIGHT = atoi(getenv("HEIGHT"));*/
		P4HEIGHT = atoi(argv[2]);
	}
	else
		P4HEIGHT = 16;

	if (argc > 3 && (atoi(argv[3]) >= 2 && atoi(argv[3]) <= 8))
	{
		/*if (setenv("PLAYERS", argv[3], 1) != 0)
			perror("setenv error");
		P4PLAYERS = atoi(getenv("PLAYERS"));*/
		P4PLAYERS = atoi(argv[3]);
	}
	else
		P4PLAYERS = 2;

	board = malloc(P4HEIGHT * sizeof(char *));
	for (int r = 0; r < P4HEIGHT; r++)
    	board[r] = malloc(P4WIDTH * sizeof(char));

	CHIP = "XO345678";

    INIT_SCREEN();
    init_board();
    play();
    getch();
	for (int r = 0; r < P4HEIGHT; r++)
    	free(board[r]);
	free(board);
    DONE_SCREEN();
}
