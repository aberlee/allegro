#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_tiled.h>

#define FPS			60
#define MAP_DIR		"."
#define MAP_FILE	"demo2.tmx"

#define BG_WIDTH		640
#define BG_HEIGHT		480
#define BG_COLOR	(al_map_rgb(0, 0, 0))

int main(int argc, char **argv)
{
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *eventq = NULL;
	ALLEGRO_TIMER *timer = NULL;
	ALLEGRO_MAP *map = NULL;

	bool running = true;
	bool redraw = false;
	bool reload = false;

	int map_x = 0, map_y = 0;
	int map_w = 0, map_h = 0;

	/* Init allegro engine */
	if (!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}

	/* Init keyboard & mouse support */
	al_install_keyboard();
	al_install_mouse();

	/* Init image addon */
	al_init_image_addon();

	/* Create a window */
	display = al_create_display(BG_WIDTH, BG_HEIGHT);
	if (!display) {
		fprintf(stderr, "failed to create display!\n");
		return -1;
	}

	/* Create event queue */
	eventq = al_create_event_queue();
	if (!eventq) {
		fprintf(stderr, "failed to create event queue!\n");
		return -1;
	}

	/* Create timer */
	timer = al_create_timer(1.0 / FPS);
	if (!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}

	/* Load map */
	map = al_open_map(MAP_DIR, MAP_FILE);
	if (!map) {
		fprintf(stderr, "failed to load map "MAP_FILE"!\n");
		return -1;
	}
	map_w = al_get_map_width(map) * al_get_tile_width(map);
	map_h = al_get_map_height(map) * al_get_tile_height(map);

	printf("%s: START.\n", argv[0]);

	/* Set window title */
	al_set_window_title(display, "TILED MAP!");
	/* Set background color */
	al_clear_to_color(BG_COLOR);
	/* Draw the map */
	al_draw_map_region(map, map_x, map_y, BG_WIDTH, BG_HEIGHT, 0, 0, 0);
	/* Show the window */
	al_flip_display();

	/* Register event source */
	al_register_event_source(eventq, al_get_keyboard_event_source());
	al_register_event_source(eventq, al_get_display_event_source(display));
	al_register_event_source(eventq, al_get_timer_event_source(timer));

	al_start_timer(timer);

	/* main loop */
	while (running) {
		ALLEGRO_EVENT event;
		ALLEGRO_KEYBOARD_STATE key_state;

		if (al_is_event_queue_empty(eventq)) {
			if (redraw) {
				al_clear_to_color(BG_COLOR);
				al_draw_map_region(map, map_x, map_y,
						BG_WIDTH, BG_HEIGHT, 0, 0, 0);
				al_flip_display();
			}
			continue;
		}

		al_wait_for_event(eventq, &event);
		switch (event.type) {
			case ALLEGRO_EVENT_TIMER:
				al_get_keyboard_state(&key_state);
				if (al_key_down(&key_state, ALLEGRO_KEY_RIGHT)) {
					map_x += 5;
					if (map_x > (map_w - BG_WIDTH))
						map_x = map_w - BG_WIDTH;
				} else if (al_key_down(&key_state, ALLEGRO_KEY_LEFT)) {
					map_x -= 5;
					if (map_x < 0)
						map_x = 0;
				} else if (al_key_down(&key_state, ALLEGRO_KEY_DOWN)) {
					map_y += 5;
					if (map_y > (map_h - BG_HEIGHT))
						map_y = map_h - BG_HEIGHT;
				} else if (al_key_down(&key_state, ALLEGRO_KEY_UP)) {
					map_y -= 5;
					if (map_y < 0)
						map_y = 0;
				}
				redraw = true;
				break;
			case ALLEGRO_EVENT_KEY_DOWN:
				break;
			case ALLEGRO_EVENT_KEY_UP:
				break;
			case ALLEGRO_EVENT_KEY_CHAR:
				if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
					running = false;
				else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
					reload = true;
				break;
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				printf("Got DISPLAY CLOSE event!\n");
				break;
			default:
				break;
		}
	}

	al_stop_timer(timer);

	/* Release resource */
	al_destroy_display(display);
	al_destroy_timer(timer);
	al_free_map(map);
	al_destroy_event_queue(eventq);

	printf("%s: QUIT!\n", argv[0]);
	return 0;
}
