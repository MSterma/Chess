#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
typedef struct piece{
    char name;
    char color;
    char row;
    char col;
    int* moveset;
    int captured;
} piece;
char name[6] = { 'P', 'N', 'B', 'R', 'Q', 'K' };
char color[2] = { 'W','B' };
char cols[8] = { 'a','b','c','d','e','f','g','h' };
char rows[8] = { '1','2','3','4','5','6','7','8' };
int king_moveset[8][2] = {
    {1,-1},
    {1,0},
    {1,1},
    {0,1},
    {-1,1},
    {-1,0},
    {-1,-1},
    {0,-1}
};

void must_init(bool test, const char* description) {
    if (test) return;
    printf("couldn't initialize %s\n", description);
    exit(1);
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

    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);

    ALLEGRO_DISPLAY* disp = al_create_display(desktop_width, desktop_height);
    must_init(disp, "display");

    int board_size = (desktop_height < desktop_width) ? desktop_height : desktop_width;
    int tile_size = board_size / 10;
    int x_offset = 0, y_offset = 0;
    if (desktop_height < desktop_width) x_offset = (desktop_width - board_size) / 2;
    else y_offset = (desktop_height - board_size) / 2;

    al_init_font_addon();
    al_init_ttf_addon();
    ALLEGRO_FONT* font = al_load_ttf_font("ariblk.ttf", tile_size / 2, 0);
    must_init(font, "font");

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_mouse_event_source());

    bool done = false;
    bool redraw = true;
    ALLEGRO_EVENT event;

    int pos_x = 0, pos_y = 0;
    al_grab_mouse(disp);
    al_start_timer(timer);


    while (1) {
        al_wait_for_event(queue, &event);

        switch (event.type) {
            case ALLEGRO_EVENT_TIMER:
                // game logic goes here.
                redraw = true;
                break;
            case ALLEGRO_EVENT_MOUSE_AXES:
                pos_x = event.mouse.x;
                pos_y = event.mouse.y;
                break;
            case ALLEGRO_EVENT_KEY_DOWN:
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                done = true;
                break;
        }

        if (done) break;

        if (redraw && al_is_event_queue_empty(queue)) {
            int y = y_offset;
            int x = x_offset;
            int color_flag = 0;
            ALLEGRO_COLOR tile_color = al_map_rgb(222, 184, 135);

            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_filled_rectangle(0, 0, desktop_width, desktop_height, al_map_rgb(128, 128, 128));
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 0, 0, "X: %d", pos_x);
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, tile_size / 2, 0, "Y: %d", pos_y);

            char pos[4] = "Out";
            //char rows[9] = "abcdefgh";
            //char cols[9] = "12345678";
            int row = 0, col = 0;
            if (pos_x >= x + tile_size && pos_x <= x + 9 * tile_size && pos_y >= y + tile_size && pos_y <= y + 9 * tile_size) {
                row = (pos_y - y - tile_size) / tile_size;
                col = (pos_x - x - tile_size) / tile_size;
                pos[0] = rows[row];
                pos[1] = cols[col];
                pos[2] = ' ';
            }
            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 3 * tile_size / 2, 0, "Position: %s", pos);
            
            //draw corners
            al_draw_filled_rectangle(x, y, x + tile_size, y + tile_size, tile_color);
            al_draw_filled_rectangle(x + 9 * tile_size, y, x + board_size, y + tile_size, tile_color);
            al_draw_filled_rectangle(x, y + 9 * tile_size, x + tile_size, y + board_size, tile_color);
            al_draw_filled_rectangle(x + 9 * tile_size, y + 9 * tile_size, x + board_size, y + board_size, tile_color);
            
            //draw sides
            char letter[2] = "@";
            char digit[2] = "0";
            for (y += tile_size; y < y_offset + 9 * tile_size; y += tile_size) {
                digit[0] += 1;
                al_draw_filled_rectangle(x, y, x + tile_size, y + tile_size, tile_color);
                al_draw_text(font, al_map_rgb(128, 0, 0), x + tile_size / 3, y + tile_size / 6, 0, digit);
                al_draw_filled_rectangle(x + 9 * tile_size, y, x + board_size, y + tile_size, tile_color);
                al_draw_text(font, al_map_rgb(128, 0, 0), x + tile_size / 3 + tile_size * 9, y + tile_size / 6, 0, digit);
            }
            y = y_offset;
            x = x_offset;
            for (x += tile_size; x < x_offset + 9 * tile_size; x += tile_size) {
                letter[0]+=1;
                al_draw_filled_rectangle(x, y, x + tile_size, y + tile_size, tile_color);
                al_draw_text(font, al_map_rgb(128, 0, 0), x + tile_size / 3, y + tile_size / 6, 0, letter);
                al_draw_filled_rectangle(x , y + 9 * tile_size, x + tile_size, y + board_size, tile_color);
                al_draw_text(font, al_map_rgb(128, 0, 0), x + tile_size / 3, y + tile_size / 6 + tile_size * 9, 0, letter);
            }

            //draw center
            y = y_offset;
            x = x_offset;
            for (y += tile_size; y < y_offset + 9 * tile_size; y += tile_size) {
                for (x += tile_size; x < x_offset + 9 * tile_size; x += tile_size) {
                    tile_color = (color_flag++ % 2) ? al_map_rgb(255, 228, 196) : al_map_rgb(139, 69, 19);
                    al_draw_filled_rectangle(x, y, x + tile_size, y + tile_size, tile_color);
                }
                x = x_offset;
                color_flag += 1;
            }

            al_flip_display();

            redraw = false;
        }
    }

    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}
