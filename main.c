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

typedef struct piece {
    int code;
    ALLEGRO_BITMAP* bitmap;
    bool (*can_move)(int, int, int, int);
} piece ;

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
void must_init(bool test, const char* description) {
    if (test) return;
    printf("couldn't initialize %s\n", description);
    exit(1);
}
piece* new_piece(int code, ALLEGRO_BITMAP* bitmap,bool (*can_move)(int, int, int, int)) {
    piece* p=malloc(sizeof(piece));
    if (p == NULL) {
        printf("new piece");
        return 1;
    }
    p->code = code;
    p->bitmap = bitmap;
    p->can_move = can_move;
    must_init((p->bitmap), "bitmap");
    return p;

}

bool pawn_move(int from_row, int from_col,int to_row,int to_col) {
    int delta_row = to_row - from_row;
    int delta_col = to_col - from_col;
    int color = board[from_row][from_col] / 10; // 0 - empty, 1 - white, 2 - black
    int target_color = board[to_row][to_col] / 10;
    bool start_position = color && from_row == 6 || from_row == 1;
    if (color == 2) {
        return ((delta_row == 1 || (start_position && delta_row == 2)) && delta_col == 0 && target_color == 0)
            || (delta_row == 1 && (delta_col == -1 || delta_col == 1) && target_color == 1);
    }
    else if (color == 1) {
        return ((delta_row == -1 || (start_position && delta_row == -2)) && delta_col == 0 && target_color == 0)
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
        && color!=target_color;
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
    if (delta_row == delta_col && delta_row!=0) { // top left to bottom right, bottom right to top left
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
    if (delta_row==0 && delta_col==0 || color==target_color) return false;
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
    } else if (from_row < to_row) {
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
bool king_move(int from_row, int from_col, int to_row, int to_col) {
    int delta_row = to_row - from_row;
    int delta_col = to_col - from_col;
    int color = board[from_row][from_col] / 10; // 0 - empty, 1 - white, 2 - black
    int target_color = board[to_row][to_col] / 10;
    if (delta_row == 0 && delta_col == 0 || color == target_color) return false;
    return delta_row >= -1 && delta_row <= 1 && delta_col >= -1 && delta_col <= 1;
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
    /*ALLEGRO_BITMAP* bitmaps[12];
    bitmaps[0] = al_load_bitmap("graphics/pawn_white.png");
    bitmaps[1] = al_load_bitmap("graphics/knight_white.png");
    bitmaps[2] = al_load_bitmap("graphics/bishop_white.png");
    bitmaps[3] = al_load_bitmap("graphics/rook_white.png");
    bitmaps[4] = al_load_bitmap("graphics/queen_white.png");
    bitmaps[5] = al_load_bitmap("graphics/king_white.png");
    bitmaps[6] = al_load_bitmap("graphics/pawn_black.png");
    bitmaps[7] = al_load_bitmap("graphics/knight_black.png");
    bitmaps[8] = al_load_bitmap("graphics/bishop_black.png");
    bitmaps[9] = al_load_bitmap("graphics/rook_black.png");
    bitmaps[10] = al_load_bitmap("graphics/queen_black.png");
    bitmaps[11] = al_load_bitmap("graphics/king_black.png");
    for (int i = 0; i < 12; i++) must_init(bitmaps[i], "bitmap");*/

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
    int turn = 0;
    int game_state = 0;

    /*piece wP; // white pawn
    wP.code = w + P;
    wP.bitmap = al_load_bitmap("graphics/pawn_white.png");
    wP.can_move = pawn_move;
    must_init(wP.bitmap, "bitmap");*/
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
    /*piece bP; // black pawn
    bP.code = b + P;
    bP.bitmap = al_load_bitmap("graphics/pawn_black.png");
    bP.can_move = pawn_move;
    must_init(bP.bitmap, "bitmap");

    piece wN; // white knight
    wN.code = w + N;
    wN.bitmap = al_load_bitmap("graphics/knight_white.png");
    wN.can_move = knight_move;
    must_init(wN.bitmap, "bitmap");

    piece bN; // black knight
    bN.code = b + N;
    bN.bitmap = al_load_bitmap("graphics/knight_black.png");
    bN.can_move = knight_move;
    must_init(bN.bitmap, "bitmap");

    piece wB; // white bishop
    wB.code = w + B;
    wB.bitmap = al_load_bitmap("graphics/bishop_white.png");
    wB.can_move = bishop_move;
    must_init(wB.bitmap, "bitmap");

    piece bB; // black bishop
    bB.code = b + B;
    bB.bitmap = al_load_bitmap("graphics/bishop_black.png");
    bB.can_move = bishop_move;
    must_init(bB.bitmap, "bitmap");

    piece wR; // white rook
    wR.code = w + R;
    wR.bitmap = al_load_bitmap("graphics/rook_white.png");
    wR.can_move = rook_move;
    must_init(wR.bitmap, "bitmap");

    piece bR; // black rook
    bR.code = b + R;
    bR.bitmap = al_load_bitmap("graphics/rook_black.png");
    bR.can_move = rook_move;
    must_init(bR.bitmap, "bitmap");

    piece wQ; // white queen
    wQ.code = w + Q;
    wQ.bitmap = al_load_bitmap("graphics/queen_white.png");
    wQ.can_move = queen_move;
    must_init(wQ.bitmap, "bitmap");

    piece bQ; // black queen
    bQ.code = b + Q;
    bQ.bitmap = al_load_bitmap("graphics/queen_black.png");
    bQ.can_move = queen_move;
    must_init(bQ.bitmap, "bitmap");

    piece wK; // white king
    wK.code = w + K;
    wK.bitmap = al_load_bitmap("graphics/king_white.png");
    wK.can_move = king_move;
    must_init(wK.bitmap, "bitmap");

    piece bK; // black king
    bK.code = b + K;
    bK.bitmap = al_load_bitmap("graphics/king_black.png");
    bK.can_move = king_move;
    must_init(bK.bitmap, "bitmap");*/

    /*int board[8][8] = {
        {bR.code,bN.code,bB.code,bQ.code,bK.code,bB.code,bN.code,bR.code},
        {bP.code,bP.code,bP.code,bP.code,bP.code,bP.code,bP.code,bP.code},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0},
        {wP.code,wP.code,wP.code,wP.code,wP.code,wP.code,wP.code,wP.code},
        {wR.code,wN.code,wB.code,wQ.code,wK.code,wB.code,wN.code,wR.code}
    };*/

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

    piece code_to_piece[2][8] = { //[piece color][piece type]
        {wP,wN,wB,wR,wQ,wK},
        {bP,bN,bB,bR,bQ,bK}
    };
    
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
                if (game_state%2==0 && pos_x > x_offset + 3 * tile_size && pos_x < x_offset + 7 * tile_size && pos_y >= y_offset + 3 * tile_size && pos_y < y_offset + 7 * tile_size) {
                    in_menu_click = true;
                }
                if (game_state%2==1 && click_counter == 0) {
                    first_click = true;
                }
                if (game_state % 2 == 1 && click_counter == 1) {
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
                game_state += 1;
            }
            //menu handling
            if (game_state % 2 == 0) {
                if (pos_x > x_offset + 3 * tile_size && pos_x < x_offset + 7 * tile_size) {
                    //resume
                    if (pos_y >= y_offset + 3 * tile_size && pos_y < y_offset + 4 * tile_size && in_menu_click) {
                        game_state += 1;
                    }
                    //save
                    if (pos_y >= y_offset + 4 * tile_size && pos_y < y_offset + 5 * tile_size && in_menu_click) {
                        FILE* f = fopen("games/game.txt", "w");
                        if (f == NULL) {
                            printf("file");
                            return 1;
                        }
                        fprintf(f, "%d;", turn % 2);
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
                        char c=fgetc(f);
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
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, tile_size / 2, 0, "Y: %d", pos_y);
            al_draw_filled_rectangle(desktop_width - 2 * tile_size, 0, desktop_width, tile_size, al_map_rgb(0, 0, 0));
            al_draw_textf(font, al_map_rgb(255, 255, 255), desktop_width - 9 * tile_size / 5, tile_size / 8, 0, "MENU");

            //move
            int row = (pos_y - y_offset) / tile_size - 1;
            int col = (pos_x - x_offset) / tile_size - 1;
            if (game_state % 2 == 1) {
                int temp = 0;
                
                if (row >= 0 && row < 8 && col >= 0 && col < 8 && first_click && (board[row][col] == 0 || ((board[row][col] / 10) - 1) != turn % 2)) {
                    first_click = false;
                    click_counter = 0;
                    from_row = -1;
                    from_col = -1;
                }
                if ((first_click || second_click) && (row < 0 || row >= 8 || col < 0 || col >= 8)) {
                    first_click = false;
                    second_click = false;
                    click_counter = 0;
                    from_row = -1;
                    from_col = -1;
                    to_row = -1;
                    to_col = -1;
                }
                if (first_click) {
                    from_row = row;
                    from_col = col;
                    click_counter += 1;
                    first_click = false;
                }
                if (second_click) {
                    to_row = row;
                    to_col = col;
                    second_click = false;
                    click_counter = 0;
                }
                if (from_col != -1 && from_row != -1 && to_col != -1 && to_row != -1) {
                    bool flag = false;
                    piece p = code_to_piece[board[from_row][from_col] / 10 - 1][board[from_row][from_col] % 10 - 1]; // [piece color][piece type]
                    flag = p.can_move(from_row, from_col, to_row, to_col);
                    //flag = pawn_move(from_row, from_col, to_row, to_col);
                    /*switch (board[from_row][from_col]) {
                        case w+P:
                        case b+P:
                            flag = pawn_move(from_row, from_col, to_row, to_col, board);
                            break;
                        case w+N:
                        case b+N:
                            flag = knight_move(from_row, from_col, to_row, to_col);
                            break;
                        case w+B:
                        case b+B:
                            flag = bishop_move(from_row, from_col, to_row, to_col);
                            break;
                        case w+R:
                        case b+R:
                            flag = rook_move(from_row, from_col, to_row, to_col);
                            break;
                        case w+Q:
                        case b+Q:
                            flag = queen_move(from_row, from_col, to_row, to_col);
                            break;
                        case w+K:
                        case b+K:
                            flag = king_move(from_row, from_col, to_row, to_col);
                            break;
                    }*/
                    if (flag) {
                        temp = board[from_row][from_col];
                        board[from_row][from_col] = 0;
                        board[to_row][to_col] = temp;
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
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 5 * tile_size / 2, 0, "Turn: %c", (turn % 2) ? 'B' : 'W');
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 6 * tile_size / 2, 0, "Pause: %c", (game_state % 2) ? 'N' : 'Y');

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
                    /*int piece_color = board[row][col] / 10;
                    int piece = board[row][col] % 10;
                    int bitmap_address = piece - 1 + 6 * (piece_color-1);
                    if (piece) al_draw_scaled_bitmap(bitmaps[bitmap_address], 0, 0, 200, 200, x, y, tile_size, tile_size, 0);*/
                    if (board[row][col]) {
                        piece p = code_to_piece[board[row][col] / 10 - 1][board[row][col] % 10 - 1]; // [piece color][piece type]
                        al_draw_scaled_bitmap(p.bitmap, 0, 0, 200, 200, x, y, tile_size, tile_size, 0);
                    }
                }
                x = x_offset;
                color_flag += 1;
            }

            //draw menu
            y = y_offset+3*tile_size;
            x = x_offset+3*tile_size;
            if (game_state % 2 == 0) {
                al_draw_filled_rectangle(x, y, x+4*tile_size, y+tile_size, al_map_rgb(0, 0, 0));
                al_draw_textf(font, al_map_rgb(255, 255, 255), x, y + tile_size / 8, 0, "RESUME");
                y += tile_size;
                al_draw_filled_rectangle(x, y, x+4*tile_size, y+tile_size, al_map_rgb(0, 0, 0));
                al_draw_textf(font, al_map_rgb(255, 255, 255), x, y + tile_size / 8, 0, "SAVE");
                y += tile_size;
                al_draw_filled_rectangle(x, y, x+4*tile_size, y+tile_size, al_map_rgb(0, 0, 0));
                al_draw_textf(font, al_map_rgb(255, 255, 255), x, y + tile_size / 8, 0, "LOAD");
                y += tile_size;
                al_draw_filled_rectangle(x, y, x+4*tile_size, y+tile_size, al_map_rgb(0, 0, 0));
                al_draw_textf(font, al_map_rgb(255, 255, 255), x, y + tile_size / 8, 0, "EXIT");
                y += tile_size;

            }

            //ALLEGRO_BITMAP* test = al_load_bitmap("graphics/king_black.png");
            //must_init(test, "test");
            //al_draw_scaled_bitmap(wP.bitmap, 0, 0, 200, 200, x_offset, y_offset, tile_size, tile_size, 0);
            //al_draw_scaled_bitmap(test, 0, 0, 200, 200, x_offset, y_offset, tile_size, tile_size, 0);


            al_flip_display();

            redraw = false;
        }
    }
    //for (int i = 0; i < 12; i++) al_destroy_bitmap(bitmaps[i]);
    for (int i = 0; i < 2; i++) for (int j = 0; j < 6; j++) al_destroy_bitmap(code_to_piece[i][j].bitmap);
    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}
