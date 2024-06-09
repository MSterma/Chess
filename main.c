#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

#define w 10
#define b 20

enum pieces { P = 1, N, B, R, Q, K };
bool castling[6] = { false,false,false,false, false, false }; //0- wk 1- first wk rook 2- second white rook 3-bk 4-1st black rook 5-2nd black rook


typedef struct piece {
    int code;// first digit  0-empty 1-white 2-black, second digit piece type
    ALLEGRO_BITMAP* bitmap;
    bool (*can_move)(int, int, int, int);
} piece;

int board[8][8] = {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}
};

piece code_to_piece[2][6];

int wkPos = 74;
int bkPos = 04;

void must_init(bool test, const char* description) {
    if (test) return;
    printf("couldn't initialize %s\n", description);
    exit(1);
}
piece* new_piece(int code, ALLEGRO_BITMAP* bitmap, bool (*can_move)(int, int, int, int)) {
    piece* p = malloc(sizeof(piece));
    if (p == NULL) {
        printf("new piece");
        exit(1);
    }
    p->code = code;
    p->bitmap = bitmap;
    p->can_move = can_move;
    must_init((p->bitmap), "bitmap");
    return p;

}

bool pawn_move(int from_row, int from_col, int to_row, int to_col) {
    int delta_row = to_row - from_row;
    int delta_col = to_col - from_col;
    int color = board[from_row][from_col] / 10; // 0 - empty, 1 - white, 2 - black
    int target_color = board[to_row][to_col] / 10;
    bool start_position = color && from_row == 6 || from_row == 1;
    if (color == 2) {
        return ((delta_row == 1 || (start_position && delta_row == 2 && board[to_row - 1][to_col] == 0)) && delta_col == 0 && target_color == 0)
            || (delta_row == 1 && (delta_col == -1 || delta_col == 1) && target_color == 1);
    }
    else if (color == 1) {
        return ((delta_row == -1 || (start_position && delta_row == -2 && board[to_row + 1][to_col] == 0)) && delta_col == 0 && target_color == 0)
            || (delta_row == -1 && (delta_col == -1 || delta_col == 1) && target_color == 2);
    }
    else return false;


}
bool knight_move(int from_row, int from_col, int to_row, int to_col) {
    int delta_row = to_row - from_row;
    int delta_col = to_col - from_col;
    int color = board[from_row][from_col] / 10; // 0 - empty, 1 - white, 2 - black
    int target_color = board[to_row][to_col] / 10;
    return ((delta_row == 2 && (delta_col == -1 || delta_col == 1)) || (delta_row == -2 && (delta_col == -1 || delta_col == 1))
        || (delta_row == 1 && (delta_col == -2 || delta_col == 2)) || (delta_row == -1 && (delta_col == -2 || delta_col == 2)))
        && color != target_color;
}
bool bishop_move(int from_row, int from_col, int to_row, int to_col) {
    int delta_row = to_row - from_row;
    int delta_col = to_col - from_col;
    int color = board[from_row][from_col] / 10; // 0 - empty, 1 - white, 2 - black
    int target_color = board[to_row][to_col] / 10;
    int min_row, min_col, count;
    count = (delta_row > 0) ? delta_row : delta_row * (-1);
    if (from_row < to_row) {
        min_row = from_row;
        min_col = from_col;
    }
    else {
        min_row = to_row;
        min_col = to_col;
    }
    if (color == target_color) return false;
    if (delta_row == delta_col && delta_row != 0) { // top left to bottom right, bottom right to top left
        for (int i = 1; i < count; i++) {
            if (board[min_row + i][min_col + i] != 0) return false;
        }
    }
    else if (delta_row == delta_col * (-1) && delta_row != 0) { // top right to bottom left, bottom left to top tight
        for (int i = 1; i < count; i++) {
            if (board[min_row + i][min_col - i] != 0) return false;
        }
    }
    else return false;
    return true;
}
bool rook_move(int from_row, int from_col, int to_row, int to_col) {
    int delta_row = to_row - from_row;
    int delta_col = to_col - from_col;
    int color = board[from_row][from_col] / 10; // 0 - empty, 1 - white, 2 - black
    int target_color = board[to_row][to_col] / 10;
    int min_row, min_col;
    min_row = (from_row < to_row) ? from_row : to_row;
    min_col = (from_col < to_col) ? from_col : to_col;
    if (delta_row == 0 && delta_col == 0 || color == target_color) return false;
    if (delta_row == 0) {
        for (int i = 1; i < ((delta_col > 0) ? delta_col : delta_col * (-1)); i++) {
            if (board[min_row][min_col + i] != 0) return false;
        }
    }
    else if (delta_col == 0) {
        for (int i = 1; i < ((delta_row > 0) ? delta_row : delta_row * (-1)); i++) {
            if (board[min_row + i][min_col] != 0) return false;
        }
    }
    else return false;
    return true;
}
bool queen_move(int from_row, int from_col, int to_row, int to_col) {
    int delta_row = to_row - from_row;
    int delta_col = to_col - from_col;
    int color = board[from_row][from_col] / 10; // 0 - empty, 1 - white, 2 - black
    int target_color = board[to_row][to_col] / 10;
    int min_row, min_col, count;
    if (delta_row == 0 || delta_col == 0) {
        min_row = (from_row < to_row) ? from_row : to_row;
        min_col = (from_col < to_col) ? from_col : to_col;
    }
    else if (from_row < to_row) {
        min_row = from_row;
        min_col = from_col;
    }
    else {
        min_row = to_row;
        min_col = to_col;
    }
    count = (delta_row > 0) ? delta_row : delta_row * (-1);
    if (delta_row == 0 && delta_col == 0 || color == target_color) return false;
    if (delta_row == delta_col) { // top left to bottom right, bottom right to top left
        for (int i = 1; i < count; i++) {
            if (board[min_row + i][min_col + i] != 0) return false;
        }
    }
    else if (delta_row == delta_col * (-1)) { // top right to bottom left, bottom left to top tight
        for (int i = 1; i < count; i++) {
            if (board[min_row + i][min_col - i] != 0) return false;
        }
    }
    else if (delta_row == 0) {
        for (int i = 1; i < ((delta_col > 0) ? delta_col : delta_col * (-1)); i++) {
            if (board[min_row][min_col + i] != 0) return false;
        }
    }
    else if (delta_col == 0) {
        for (int i = 1; i < ((delta_row > 0) ? delta_row : delta_row * (-1)); i++) {
            if (board[min_row + i][min_col] != 0) return false;
        }
    }
    else return false;
    return true;
}
bool is_check(int king_pos, int turn) {
    int king_color = turn + 1;
    //printf("KROL %d %d\n", king_color, king_pos);
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            int code = board[row][col];
            if (code / 10 && code / 10 != king_color) {
                piece p = code_to_piece[code / 10 - 1][code % 10 - 1];
                if (p.can_move(row, col, king_pos / 10, king_pos % 10)) {
                    //printf("%d ATTACKS KING %d\n", 10 * row + col, king_pos);
                    return true;
                }
            }
        }
    }
    return false;
}
bool king_move(int from_row, int from_col, int to_row, int to_col) {
    int delta_row = to_row - from_row;
    int delta_col = to_col - from_col;
    int color = board[from_row][from_col] / 10; // 0 - empty, 1 - white, 2 - black
    int target_color = board[to_row][to_col] / 10;
    if (delta_row == 0 && delta_col == 0 || color == target_color) return false;
    if (delta_col==-2){
        if (color == 1) {
            if (castling[0] == false && castling[1] == false) {
                for (int i = 0; i < 3; i++) {
                    if (is_check(74 - i, 0)) {
                        return false;
                    }
                    if (board[7][4 - i-1] ) {
                        return false;
                    }
                }
                return true;
            }
        }
        else if (color==2){
            if (castling[3] == false && castling[4] == false) {
                for (int i = 0; i < 3; i++) {
                    if (is_check(04 - i, 1)) {
                        return false;
                    }
                    if (board[0][4 - i-1]) {
                        return false;
                    }
                }
                return true;

            }
        }

    }else if (delta_col == 2) {
        if (color == 1) {
            if (castling[0] == false && castling[2] == false) {
                for (int i = 0; i < 3; i++) {
                    if (is_check(74 + i, 0)) {
                        return false;
                    }
                    if (board[7][4 + i]!=0 && i!=0) {
                        return false;
                    }
                }
                return true;
            }
        }
        else {
            if (castling[3] == false && castling[5] == false) {
                for (int i = 0; i < 3; i++) {
                    if (is_check(04 +i, 1)) {
                        return false;
                    }
                    if (board[0][4 + i] && i) {
                        return false;
                    }
                }
                return true;

            }
        }

        
    }
    return delta_row >= -1 && delta_row <= 1 && delta_col >= -1 && delta_col <= 1;
}
bool check_after_move(int from_row, int from_col, int to_row, int to_col) {
    int temp1 = board[from_row][from_col];
    int temp2 = board[to_row][to_col];
    int temp3 = wkPos;
    int temp4 = bkPos;

    int piece_code = temp1;
    piece p = code_to_piece[temp1 / 10 - 1][temp1 % 10 - 1];
    if (p.code == w + K) {
        wkPos = to_row * 10 + to_col;
    }
    else if (p.code == b + K) {
        bkPos = to_row * 10 + to_col;
    }
    board[from_row][from_col] = 0;
    board[to_row][to_col] = temp1;
    bool check = (temp1 / 10 == 2) ? is_check(bkPos, temp1 / 10 - 1) : is_check(wkPos, temp1 / 10 - 1);
    //printf("SZACH PO RUCHU KOLOR %d ??? %d PRZED %d PO %d \n", temp1 / 10, check, 10 * from_row + from_col, 10 * to_row + to_col);
    board[from_row][from_col] = temp1;
    board[to_row][to_col] = temp2;
    wkPos = temp3;
    bkPos = temp4;
    return check;
}
bool can_any_move(int color) {
    color += 1;
    int king_pos;
    if (color == 1) king_pos = wkPos;
    else if (color == 2) king_pos = bkPos;
    else exit(1);
    for (int from_row = 0; from_row < 8; from_row++) {
        for (int from_col = 0; from_col < 8; from_col++) {
            for (int to_row = 0; to_row < 8; to_row++) {
                for (int to_col = 0; to_col < 8; to_col++) {
                    int code = board[from_row][from_col];
                    piece p = code_to_piece[code / 10 - 1][code % 10 - 1];
                    if (code / 10 == color && p.can_move(from_row, from_col, to_row, to_col)) {
                        bool check = is_check(king_pos, color - 1);
                        bool still_check = check_after_move(from_row, from_col, to_row, to_col);

                        if (!check && !still_check) return true;
                        else if (check && !still_check) return true;

                        
                    }
                    /*if (code / 10 == color && code % 10 == 6 && p.can_move(from_row, from_col, to_row, to_col)) {
                        if (!is_check(10 * to_row + to_col, color, 0)) {
                            //printf("KING CAN MOVE %d\n", 10 * to_row + to_col);
                            return true;
                        }
                    }
                    if (code / 10 == color && p.can_move(from_row, from_col, to_row, to_col)) {
                        if (check_after_move(from_row, from_col, to_row, to_col)) return false;
                        //printf("%d SOMETHING CAN MOVE FROM %d TO %d\n",color, 10 * from_row + from_col, 10 * to_row + to_col);
                        return true;
                    }*/
                }
            }
        }
    }
    return false;
}
bool is_stalemate(int king_pos, int turn) {
    if (is_check(king_pos, turn)) return false;
    int king_code = board[king_pos / 10][king_pos % 10];
    piece king = code_to_piece[king_code / 10 - 1][king_code % 10 - 1];
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int temp = king_pos + 10 * i + j;
            if (!(i == 0 && j == 0) && temp / 10 >= 0 && temp / 10 <= 7 && temp % 10 >= 0 && temp % 10 <= 7) {
                bool king_can_move = king.can_move(king_pos / 10, king_pos % 10, temp / 10, temp % 10);
                bool check = check_after_move(king_pos / 10, king_pos % 10, temp / 10, temp % 10);
                //printf("King %d temp %d check %d  can move %d\n", king_code, temp, check, king_can_move);
                if (!check && king_can_move) return false;
            }

        }
    }
    //printf("KING %d CANT MOVE\n",king_code/10);
    return true;
}
int main() {
    must_init(al_init(), "allegro");
    must_init(al_install_keyboard(), "keyboard");
    must_init(al_init_primitives_addon(), "primitives");
    must_init(al_install_mouse(), "mouse");

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 30.0);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    must_init(queue, "queue");

    ALLEGRO_MONITOR_INFO aminfo;
    al_get_monitor_info(0, &aminfo);
    int desktop_width = aminfo.x2 - aminfo.x1 + 1;
    int desktop_height = aminfo.y2 - aminfo.y1 + 1;


    al_init_image_addon();
    //al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    ALLEGRO_DISPLAY* disp = al_create_display(desktop_width, desktop_height);
    must_init(disp, "display");

    int board_size = (desktop_height < desktop_width) ? desktop_height : desktop_width;
    int tile_size = board_size / 10;

    al_init_font_addon();
    al_init_ttf_addon();
    ALLEGRO_FONT* font = al_load_ttf_font("ariblk.ttf", tile_size / 2, 0);
    must_init(font, "font");

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_mouse_event_source());

    ALLEGRO_EVENT event;

    al_grab_mouse(disp);
    al_start_timer(timer);

    bool done = false;
    bool redraw = true;
    bool first_click = false;
    bool second_click = false;
    bool menu_click = false;
    bool in_menu_click = false;

    int x_offset = 0, y_offset = 0;
    if (desktop_height < desktop_width) x_offset = (desktop_width - board_size) / 2;
    else y_offset = (desktop_height - board_size) / 2;

    int click_counter = 0;
    int pos_x = 0, pos_y = 0;
    int from_row = -1, from_col = -1, to_row = -1, to_col = -1;
    int turn = 0; // %2==0 - white, %2==1 black
    int game_state = 0; // 0 - paused, 1 - ongoing, 2 - finished
    bool stalemate = false;
    bool any_move = true;
    int mate = 0;
    piece wP = *new_piece(w + P, al_load_bitmap("graphics/pawn_white.png"), pawn_move);
    piece bP = *new_piece(b + P, al_load_bitmap("graphics/pawn_black.png"), pawn_move);

    piece wN = *new_piece(w + N, al_load_bitmap("graphics/knight_white.png"), knight_move);
    piece bN = *new_piece(b + N, al_load_bitmap("graphics/knight_black.png"), knight_move);

    piece wB = *new_piece(w + B, al_load_bitmap("graphics/bishop_white.png"), bishop_move);
    piece bB = *new_piece(b + B, al_load_bitmap("graphics/bishop_black.png"), bishop_move);

    piece wR = *new_piece(w + R, al_load_bitmap("graphics/rook_white.png"), rook_move);
    piece bR = *new_piece(b + R, al_load_bitmap("graphics/rook_black.png"), rook_move);

    piece wQ = *new_piece(w + Q, al_load_bitmap("graphics/queen_white.png"), queen_move);
    piece bQ = *new_piece(b + Q, al_load_bitmap("graphics/queen_black.png"), queen_move);

    piece wK = *new_piece(w + K, al_load_bitmap("graphics/king_white.png"), king_move);
    piece bK = *new_piece(b + K, al_load_bitmap("graphics/king_black.png"), king_move);

    //put pieces on board
    for (int i = 0; i < 8; i++) {
        board[1][i] = bP.code;
        board[6][i] = wP.code;
    }

    board[0][0] = board[0][7] = bR.code;
    board[0][1] = board[0][6] = bN.code;
    board[0][2] = board[0][5] = bB.code;
    board[0][3] = bQ.code;
    board[0][4] = bK.code;

    board[7][0] = board[7][7] = wR.code;
    board[7][1] = board[7][6] = wN.code;
    board[7][2] = board[7][5] = wB.code;
    board[7][3] = wQ.code;
    board[7][4] = wK.code;

    code_to_piece[0][0] = wP;
    code_to_piece[0][1] = wN;
    code_to_piece[0][2] = wB;
    code_to_piece[0][3] = wR;
    code_to_piece[0][4] = wQ;
    code_to_piece[0][5] = wK;

    code_to_piece[1][0] = bP;
    code_to_piece[1][1] = bN;
    code_to_piece[1][2] = bB;
    code_to_piece[1][3] = bR;
    code_to_piece[1][4] = bQ;
    code_to_piece[1][5] = bK;

    int sep = 0;
    piece captured[30];
    int last_captured = 0;
    while (1) {
        al_wait_for_event(queue, &event);

        switch (event.type) {
        case ALLEGRO_EVENT_TIMER:
            redraw = true;
            break;
        case ALLEGRO_EVENT_MOUSE_AXES:
            pos_x = event.mouse.x;
            pos_y = event.mouse.y;
            break;
        case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
            if (pos_y >= 0 && pos_y <= tile_size && pos_x >= desktop_width - 2 * tile_size && pos_x <= desktop_width) {
                menu_click = true;
            }
            if (game_state == 0 && pos_x > x_offset + 3 * tile_size && pos_x < x_offset + 7 * tile_size && pos_y >= y_offset + 3 * tile_size && pos_y < y_offset + 7 * tile_size) {
                in_menu_click = true;
            }
            if (game_state == 1 && click_counter == 0) {
                first_click = true;
            }
            if (game_state == 1 && click_counter == 1) {
                second_click = true;
            }
            break;
        case ALLEGRO_EVENT_KEY_DOWN:
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            done = true;
            break;
        }

        if (done) break;

        if (redraw && al_is_event_queue_empty(queue)) {

            if (menu_click) {
                menu_click = false;
                //game_state += 1;
                //game_state %= 2;
                if (game_state == 1) game_state = 0;
                else if (game_state == 2) game_state = 0;
                else if (game_state == 0 && !stalemate) game_state = 1;
                else if (game_state == 0 && stalemate) game_state = 2;

            }
            //menu handling
            if (game_state != 1) {
                if (pos_x > x_offset + 3 * tile_size && pos_x < x_offset + 7 * tile_size) {
                    //resume
                    if (pos_y >= y_offset + 3 * tile_size && pos_y < y_offset + 4 * tile_size && in_menu_click) {
                        //game_state += 1;
                        //game_state %= 2;
                        if (game_state == 1) game_state = 0;
                        else if (game_state == 2) game_state = 0;
                        else if (game_state == 0 && !stalemate) game_state = 1;
                        else if (game_state == 0 && stalemate) game_state = 2;

                    }
                    //save
                    if (pos_y >= y_offset + 4 * tile_size && pos_y < y_offset + 5 * tile_size && in_menu_click) {
                        FILE* f = fopen("games/game.txt", "w");
                        if (f == NULL) {
                            printf("file");
                            return 1;
                        }
                        fprintf(f, "%d;", turn % 2);
                        fprintf(f, "%c;", wkPos + ' ');
                        fprintf(f, "%c;", bkPos + ' ');
                        for (int i = 0; i < 8; i++) {
                            for (int j = 0; j < 8; j++) {
                                fprintf(f, "%c;", board[i][j] + '@');
                            }
                        }
                        fclose(f);
                    }
                    //load
                    if (pos_y >= y_offset + 5 * tile_size && pos_y < y_offset + 6 * tile_size && in_menu_click) {
                        FILE* f = fopen("games/game.txt", "r");
                        if (f == NULL) {
                            printf("file");
                            return 1;
                        }
                        turn = fgetc(f);
                        fgetc(f);
                        wkPos = fgetc(f) - ' ';
                        fgetc(f);
                        bkPos = fgetc(f) - ' ';
                        char c = fgetc(f);
                        int row = 0;
                        int col = 0;
                        while (c != EOF) {
                            c = fgetc(f);
                            board[row][col] = c - '@';
                            c = fgetc(f);
                            col += 1;
                            if (col == 8) {
                                row += 1;
                                col = 0;
                            }
                        }
                        fclose(f);
                    }
                    //exit
                    if (pos_y >= y_offset + 6 * tile_size && pos_y < y_offset + 7 * tile_size && in_menu_click) {
                        done = true;
                    }
                    in_menu_click = false;
                }
            }
            int y = y_offset;
            int x = x_offset;
            int color_flag = 0;

            ALLEGRO_COLOR tile_color = al_map_rgb(222, 184, 135);

            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_filled_rectangle(0, 0, desktop_width, desktop_height, al_map_rgb(128, 128, 128));
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 0, 0, "X: %d", pos_x);
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, tile_size / 2, 0, "kps: %d", wkPos);
            al_draw_filled_rectangle(desktop_width - 2 * tile_size, 0, desktop_width, tile_size, al_map_rgb(0, 0, 0));
            al_draw_textf(font, al_map_rgb(255, 255, 255), desktop_width - 9 * tile_size / 5, tile_size / 8, 0, "MENU");

            //move 
            int row = (pos_y - y_offset) / tile_size - 1;
            int col = (pos_x - x_offset) / tile_size - 1;

            if (game_state == 1) {
                bool black_stalemate = is_stalemate(bkPos, turn);
                bool white_stalemate = is_stalemate(wkPos, turn);
                any_move = can_any_move(turn % 2);

                stalemate = !any_move && (white_stalemate || black_stalemate);
                printf("CHECKED %d CAN MOVE %d", is_check(wkPos, turn % 2) || is_check(bkPos, turn % 2), any_move);
                if (!any_move) {
                    if (is_check(wkPos, turn % 2)) mate = 1;
                    else if (is_check(bkPos, turn % 2)) mate = 2;
                    else if ((white_stalemate || black_stalemate)) stalemate = true;
                }
                if (stalemate || mate) game_state = 2;

            }
            if (game_state == 1) {
                int temp = 0;
                // check if first click on board
                if (row >= 0 && row < 8 && col >= 0 && col < 8 && first_click && (board[row][col] == 0 || ((board[row][col] / 10) - 1) != turn % 2)) {
                    //printf("AAA\n");
                    first_click = false;
                    click_counter = 0;
                    from_row = -1;
                    from_col = -1;
                }
                // check if second click on board
                if ((first_click || second_click) && (row < 0 || row >= 8 || col < 0 || col >= 8)) {
                    //printf("BBB\n");
                    first_click = false;
                    second_click = false;
                    click_counter = 0;
                    from_row = -1;
                    from_col = -1;
                    to_row = -1;
                    to_col = -1;
                }
                //set sourxe tile clicked on
                if (first_click) {
                    //printf("OK1\n");
                    from_row = row;
                    from_col = col;
                    click_counter += 1;
                    first_click = false;
                }
                //set destination tile clicked on
                if (second_click) {
                    //printf("OK2\n");
                    to_row = row;
                    to_col = col;
                    second_click = false;
                    click_counter = 0;
                }
                //check if legal move
                //printf("SPRAWDZ SZACHA\n");
                bool check = (turn % 2) ? is_check(bkPos, turn % 2) : is_check(wkPos, turn % 2 + 1);
                //printf("KROL %d JEST SZACHOWANY %d\n",turn%2+1,check);
                //printf("RUCH %d SZACH %d PAT %d RUCH %d\n",turn%2,check, stalemate, any_move);
                if (from_col != -1 && from_row != -1 && to_col != -1 && to_row != -1) {

                    bool flag = false;
                    piece p = code_to_piece[board[from_row][from_col] / 10 - 1][board[from_row][from_col] % 10 - 1]; // [piece color][piece type]
                    flag = p.can_move(from_row, from_col, to_row, to_col);
                    //printf("FLAGA %d KOLOR %d\n", flag, turn % 2 + 1);
                    if (flag) { // is king  checked after curr move?
                        bool still_check = check_after_move(from_row, from_col, to_row, to_col);
                        //printf("KROL JEST DALEJ SZACHOWANY %d\n", still_check);

                        if (check && !still_check) check = false;
                        else if (!check && still_check) check = true;
                        else if (check && still_check) check = true;
                        else check = false;

                    }

                    if (flag && !check) {

                        if (p.code == w + K) {
                            wkPos = to_row * 10 + to_col;
                        }
                        else if (p.code == b + K) {
                            bkPos = to_row * 10 + to_col;
                        }
                        piece p = code_to_piece[board[to_row][to_col] / 10 - 1][board[to_row][to_col] % 10 - 1];
                        temp = board[from_row][from_col];
                        if (temp == wR.code) {
                            if (to_col == 0) {
                                castling[1] = true;
                            }
                            else {
                                castling[2] = true;

                            }

                        }
                        if (temp == bR.code) {
                            if (to_col == 0) {
                                castling[1] = true;
                            }
                            else {
                                castling[2] = true;
                            }
                        }

                        if (temp == wK.code) {
                            if (from_col - to_col == -2) {
                                board[from_row][from_col + 1] = board[7][7];
                                board[7][7] = 0;
                                castling[1] = true;
                                castling[2] = true;
                                wkPos = 76;
                            }
                            if (from_col - to_col == 2) {
                                board[from_row][from_col - 1] = board[7][0];
                                board[7][0] = 0;;
                                castling[1] = true;
                                castling[2] = true;
                                wkPos = 72;
                            }
                            castling[0] = true;
                        }
                        if (temp == bK.code) {
                            if (from_col - to_col == -2) {
                                board[from_row][from_col + 1] = board[0][7];
                                board[0][7] = 0;
                                castling[3] = true;
                                castling[4] = true;
                                bkPos =  (from_col -2);

                            }
                            if (from_col - to_col == 2) {
                                board[from_row][from_col - 1] = board[0][0];
                                board[0][0] = 0;
                                castling[4] = true;
                                castling[3] = true;
                                bkPos =   (from_col + 2);
                            }
                            castling[3] = true;
                        }
                        if (p.code % 10 != 6) {
                            board[from_row][from_col] = 0;
                            if (board[to_row][to_col]) {
                                captured[last_captured] = p;
                                last_captured++;
                            }
                            board[to_row][to_col] = temp;
                            printf("-----MADE MOVE------\n");
                        }
                        from_col = -1;
                        from_row = -1;
                        to_row = -1;
                        to_col = -1;
                        turn += 1;
                    }
                    else {
                        to_row = -1;
                        to_col = -1;
                        from_row = -1;
                        from_col = -1;
                        click_counter = 0;
                    }
                }
                //printf("RUCH %d SZACH  %d PAT %d RUCH %d\n", turn % 2, check, stalemate, any_move);
            }
            //draw info
            char pos[4] = "Out";
            row = (pos_y - y - tile_size) / tile_size;
            col = (pos_x - x - tile_size) / tile_size;
            if (row<8 && row>-1 && col <8 && col>-1) {
                pos[0] = '1' + row;
                pos[1] = 'A' + col;
                pos[2] = ' ';
            }

            //draw info
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 2 * tile_size / 2, 0, "Position: %s", pos);
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 3 * tile_size / 2, 0, "From: %d %d", from_row, from_col);
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 4 * tile_size / 2, 0, "To: %d %d", to_row, to_col);
            //printf("STILL  !!  %d\n", game_state);
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 5 * tile_size / 2, 0, "Game: %s", (game_state == 2 || stalemate) ? "End" : "On ");
            char won[4] = "   ";
            if (stalemate) {
                won[0] = '1';
                won[1] = '/';
                won[2] = '2';
            }
            if (mate == 1) {
                won[0] = 'B';
                won[1] = won[2] = ' ';
            }
            else if (mate == 2) {
                won[0] = 'W';
                won[1] = won[2] = ' ';
            }
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 6 * tile_size / 2, 0, "Won?: %s", won);
            if (!(turn % 2)) {
                al_draw_scaled_bitmap(wK.bitmap, 0, 0, 200, 200, 5 * tile_size / 2, desktop_height / 2 - 100, tile_size, tile_size, 0);
            }
            else {
                al_draw_scaled_bitmap(bK.bitmap, 0, 0, 200, 200, 5 * tile_size / 2, desktop_height / 2 - 100, tile_size, tile_size, 0);

            }
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 7 * tile_size / 2, 0, "Pause: %c", (game_state == 0) ? 'Y' : 'N');
            int black_sep = 2;
            int white_sep = 2;
            for (int i = 0; i < last_captured; i++) {
                if (captured[i].code / 10 == 1) {
                    al_draw_scaled_bitmap(captured[i].bitmap, 0, 0, 200, 200, desktop_width - (tile_size * white_sep++) / 2, tile_size * 3, tile_size / 2, tile_size / 2, 0);

                }
                else {
                    al_draw_scaled_bitmap(captured[i].bitmap, 0, 0, 200, 200, desktop_width - (tile_size * black_sep++) / 2, desktop_height - tile_size * 3, tile_size / 2, tile_size / 2, 0);
                }

            }

            //draw corners
            al_draw_filled_rectangle(x, y, x + tile_size, y + tile_size, tile_color);
            al_draw_filled_rectangle(x + 9 * tile_size, y, x + board_size, y + tile_size, tile_color);
            al_draw_filled_rectangle(x, y + 9 * tile_size, x + tile_size, y + board_size, tile_color);
            al_draw_filled_rectangle(x + 9 * tile_size, y + 9 * tile_size, x + board_size, y + board_size, tile_color);

            //draw sides
            char letter[2] = "@";
            char digit[2] = "9";
            int checkY = 0;
            int checkX = 0;
            for (y += tile_size; y < y_offset + 9 * tile_size; y += tile_size) {
                digit[0] -= 1;
                al_draw_filled_rectangle(x, y, x + tile_size, y + tile_size, tile_color);
                al_draw_text(font, al_map_rgb(128, 0, 0), x + tile_size / 3, y + tile_size / 6, 0, digit);
                al_draw_filled_rectangle(x + 9 * tile_size, y, x + board_size, y + tile_size, tile_color);
                al_draw_text(font, al_map_rgb(128, 0, 0), x + tile_size / 3 + tile_size * 9, y + tile_size / 6, 0, digit);
            }
            y = y_offset;
            x = x_offset;
            for (x += tile_size; x < x_offset + 9 * tile_size; x += tile_size) {
                letter[0] += 1;
                al_draw_filled_rectangle(x, y, x + tile_size, y + tile_size, tile_color);
                al_draw_text(font, al_map_rgb(128, 0, 0), x + tile_size / 3, y + tile_size / 6, 0, letter);
                al_draw_filled_rectangle(x, y + 9 * tile_size, x + tile_size, y + board_size, tile_color);
                al_draw_text(font, al_map_rgb(128, 0, 0), x + tile_size / 3, y + tile_size / 6 + tile_size * 9, 0, letter);

            }

            //draw center
            y = y_offset;
            x = x_offset;
            for (y += tile_size; y < y_offset + 9 * tile_size; y += tile_size) {
                for (x += tile_size; x < x_offset + 9 * tile_size; x += tile_size) {
                    //draw tile
                    tile_color = (color_flag++ % 2) ? al_map_rgb(255, 228, 196) : al_map_rgb(139, 69, 19);
                    al_draw_filled_rectangle(x, y, x + tile_size, y + tile_size, tile_color);
                    row = (y - y_offset) / tile_size - 1;
                    col = (x - x_offset) / tile_size - 1;
                    //highlight selected tiles
                    if (row == from_row && col == from_col)
                        al_draw_filled_rectangle(x, y, x + tile_size, y + tile_size, al_map_rgb(0, 255, 0));
                    //draw piece

                    if (board[row][col]) {
                        piece p = code_to_piece[board[row][col] / 10 - 1][board[row][col] % 10 - 1]; // [piece color][piece type]
                        al_draw_scaled_bitmap(p.bitmap, 0, 0, 200, 200, x, y, tile_size, tile_size, 0);
                    }
                }
                x = x_offset;
                color_flag += 1;
            }
            //draw menu
            y = y_offset + 3 * tile_size;
            x = x_offset + 3 * tile_size;
            if (game_state == 0) {
                al_draw_filled_rectangle(x, y, x + 4 * tile_size, y + tile_size, al_map_rgb(0, 0, 0));
                al_draw_textf(font, al_map_rgb(255, 255, 255), x, y + tile_size / 8, 0, "RESUME");
                y += tile_size;
                al_draw_filled_rectangle(x, y, x + 4 * tile_size, y + tile_size, al_map_rgb(0, 0, 0));
                al_draw_textf(font, al_map_rgb(255, 255, 255), x, y + tile_size / 8, 0, "SAVE");
                y += tile_size;
                al_draw_filled_rectangle(x, y, x + 4 * tile_size, y + tile_size, al_map_rgb(0, 0, 0));
                al_draw_textf(font, al_map_rgb(255, 255, 255), x, y + tile_size / 8, 0, "LOAD");
                y += tile_size;
                al_draw_filled_rectangle(x, y, x + 4 * tile_size, y + tile_size, al_map_rgb(0, 0, 0));
                al_draw_textf(font, al_map_rgb(255, 255, 255), x, y + tile_size / 8, 0, "EXIT");
                y += tile_size;

            }
            al_flip_display();

            redraw = false;
        }
    }
    for (int i = 0; i < 2; i++) for (int j = 0; j < 6; j++) al_destroy_bitmap(code_to_piece[i][j].bitmap);
    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}
