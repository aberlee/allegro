#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_tiled.h>

#define FPS			60
#define MAP_DIR		"."
#define MAP_FILE	"demo1.tmx"
#define SPRITE_TILE_FILE "sprites/character.png"

#define BG_WIDTH		640
#define BG_HEIGHT		480
#define BG_COLOR	(al_map_rgb(0, 0, 0))

struct sprite {
	int x;
	int y;
	int w;
	int h;
	int di; /* face direction */
	int st; /* status */
	ALLEGRO_BITMAP *bmp;
	ALLEGRO_MAP	*map;
};

enum DIRECTION {
	LEFT = 1,
	RIGHT = 2,
	UP = 3,
	DOWN = 4,
};

static void sprite_draw(struct sprite *s)
{
	int x, y;

	if (s->di == DOWN)
		y = 0;
	else if (s->di == RIGHT)
		y = s->h;
	else if (s->di == UP)
		y = s->h*2;
	else if (s->di == LEFT)
		y = s->h*3;
	x = s->st * s->w;

	//al_set_clipping_rectangle(s->x, s->y, s->w, s->h);
	al_draw_bitmap_region(s->bmp, x, y, s->w, s->h,
					s->x, s->y, 0);
}

static bool collision_check(int x1, int y1, int w1, int h1,
			int x2, int y2, int w2, int h2)
{
	if (x1 > x2 && x1 < x2 + w2 &&
		y1 > y2 && y1 < y2 + h2)
		return true;
	else if (x1 > x2 && x1 < x2 + w2 &&
			y1 + h1 > y2 && y1 + h1 < y2 + h2)
		return true;
	else if (x1 + w1 > x2 && x1 + w1 < x2 + w2 &&
			y1 > y2 && y1 < y2 + h2)
		return true;
	else if (x1 + w1 > x2 && x1 + w1 < x2 + w2 &&
			y1 + h1 > y2 && y1 + h1 < y2 + h2)
		return true;

	if (w1 > w2) {
		if (x1 < x2 && x1 + w1 > x2 + w2 &&
			y1 + h1 > y2 && y1 + h1 < y2 + h2)
			return true;
		else if (x1 < x2 && x1 + w1 > x2 + w2 &&
			y1 > y2 && y1 < y2 + h2)
			return true;
	}

	if (h1 > h2) {
		if (y1 < y2 && y1 + h1 > y2 + h2 &&
			x1 + w1 > x2 && x1 + w1 < x2 + w2)
			return true;
		if (y1 < y2 && y1 + h1 > y2 + h2 &&
			x1 > x2 && x1 < x2 + w2)
			return true;
	}

	return false;
}

static void sprite_move(struct sprite *s, int step)
{
	int old_x, old_y;
	ALLEGRO_MAP_LAYER *layer = NULL;
	ALLEGRO_MAP_OBJECT **objs = NULL;

	old_x = s->x;
	old_y = s->y;

	switch (s->di) {
		case LEFT:
			if (s->x >= step)
				s->x -= step;
			else
				s->x = 0;
			break;
		case RIGHT:
			if (s->x + s->w <= BG_WIDTH - step)
				s->x += step;
			else
				s->x = BG_WIDTH - s->w;
			break;
		case UP:
			if (s->y >= step)
				s->y -= step;
			else
				s->y = 0;
			break;
		case DOWN:
			if (s->y + s->h < BG_HEIGHT - step)
				s->y += step;
			else
				s->y = BG_HEIGHT - s->h;
			break;
		default:
			break;
	}


	/* check map collision */
	layer = al_get_map_layer(s->map, "Objects Layer");
	if (layer) {
		int i, len = 0;
		objs = al_get_objects(layer, &len);
		for (i = 0; i < len; i++) {
			int x, y, w, h;
			x = al_get_object_x(objs[i]);
			y = al_get_object_y(objs[i]);
			w = al_get_object_width(objs[i]);
			h = al_get_object_height(objs[i]);

			if (collision_check(s->x, s->y+s->h*2/3, s->w, s->h/3,
						x, y, w, h)) {
				s->x = old_x;
				s->y = old_y;
				return;
			}
		}
	}

	sprite_draw(s);
}


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

	int move_direction = DOWN;
	int move_step = 1;
	struct sprite s = {
		.x = 520,
		.y = 300,
		.w = 16,
		.h = 32,
		.di = DOWN,
		.st = 0,
		.bmp = NULL,
		.map = NULL,
	};


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
	s.map = map;

	/* Load image */
	s.bmp = al_load_bitmap(SPRITE_TILE_FILE);
	if (!s.bmp) {
		fprintf(stderr, "failed to load image "SPRITE_TILE_FILE"!\n");
		return -1;
	}

	printf("%s: START.\n", argv[0]);

	/* Set window title */
	al_set_window_title(display, "TILED MAP!");
	/* Set background color */
	al_clear_to_color(BG_COLOR);
	/* Draw the map */
	al_draw_map_region(map, map_x, map_y, BG_WIDTH, BG_HEIGHT, 0, 0, 0);
	sprite_draw(&s);
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
				sprite_draw(&s);
				al_flip_display();
			}
			continue;
		}

		al_wait_for_event(eventq, &event);
		switch (event.type) {
			case ALLEGRO_EVENT_TIMER:
				if (event.timer.count % 12 == 0) {
					s.st++;
					if (s.st >= 3)
						s.st = 0;
					redraw = true;
				}
				al_get_keyboard_state(&key_state);
				if (al_key_down(&key_state, ALLEGRO_KEY_RIGHT)) {
					s.di = RIGHT;
					sprite_move(&s, move_step);
					redraw = true;
				} else if (al_key_down(&key_state, ALLEGRO_KEY_LEFT)) {
					s.di = LEFT;
					sprite_move(&s, move_step);
					redraw = true;
				} else if (al_key_down(&key_state, ALLEGRO_KEY_DOWN)) {
					s.di = DOWN;
					sprite_move(&s, move_step);
					redraw = true;
				} else if (al_key_down(&key_state, ALLEGRO_KEY_UP)) {
					s.di = UP;
					sprite_move(&s, move_step);
					redraw = true;
				}
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
