#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#define IMAGE_FILE  "bat.png"

struct sprite {
	int x;
	int y;
	int w;
	int h;
	int di; /* face direction */
	int st; /* status */
	ALLEGRO_BITMAP *bmp;
};

enum direction {
	LEFT = 1,
	RIGHT = 2,
	UP = 3,
	DOWN = 4,
};

#define BG_WIDTH		640
#define BG_HEIGHT		480
#define BG_COLOR	(al_map_rgb(0xff, 0xff, 0xff))

static void sprite_clear(struct sprite *s)
{
	al_set_clipping_rectangle(s->x, s->y, s->w, s->h);
	al_clear_to_color(BG_COLOR);
}

static void sprite_flip(struct sprite *s)
{
	int x, y;

	if (s->di == UP)
		y = 0;
	else if (s->di == LEFT)
		y = s->h;
	else if (s->di == DOWN)
		y = s->h*2;
	else if (s->di == RIGHT)
		y = s->h*3;
	x = s->st * s->w;

	al_set_clipping_rectangle(s->x, s->y, s->w, s->h);
	al_draw_bitmap_region(s->bmp, x, y, s->w, s->h,
					s->x, s->y, 0);
	al_flip_display();
}

static void move_left(struct sprite *s, int step)
{
	sprite_clear(s);

	if (s->x >= step)
		s->x -= step;
	else
		s->x = 0;

	sprite_flip(s);
}

static void move_right(struct sprite *s, int step)
{
	sprite_clear(s);

	if (s->x + s->w <= BG_WIDTH - step)
		s->x += step;
	else
		s->x = BG_WIDTH - s->w;

	sprite_flip(s);
}

static void move_up(struct sprite *s, int step)
{
	sprite_clear(s);

	if (s->y >= step)
		s->y -= step;
	else
		s->y = 0;

	sprite_flip(s);
}

static void move_down(struct sprite *s, int step)
{
	sprite_clear(s);

	if (s->y + s->h < BG_HEIGHT - step)
		s->y += step;
	else
		s->y = BG_HEIGHT - s->h;

	sprite_flip(s);
}

int main(int argc, char **argv)
{
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *eventq = NULL;
	ALLEGRO_EVENT event;
	ALLEGRO_TIMER *timer = NULL;
	int move_direction = RIGHT;
	int move_step = 3;
	struct sprite s = {
		.x = 0,
		.y = 0,
		.w = 32,
		.h = 32,
		.di = RIGHT,
		.st = 0,
		.bmp = NULL,
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

	/* Load image */
	s.bmp = al_load_bitmap(IMAGE_FILE);
	if (!s.bmp) {
		fprintf(stderr, "failed to load image "IMAGE_FILE"!\n");
		return -1;
	}

	/* Create timer, 0.04 second, 25fps */
	timer = al_create_timer(0.04);
	if (!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}

	printf("%s: START.\n", argv[0]);

	/* Set window title */
	al_set_window_title(display, "SPRITE MOVING!");
	/* Set background color */
	al_clear_to_color(BG_COLOR);
	/* Show the window */
	al_flip_display();

	/* Register event source */
	al_register_event_source(eventq, al_get_keyboard_event_source());
	al_register_event_source(eventq, al_get_display_event_source(display));
	al_register_event_source(eventq, al_get_timer_event_source(timer));

	/* Show the sprite */
	sprite_flip(&s);

	al_start_timer(timer);

	/* main loop */
	while (1) {
		if (!al_is_event_queue_empty(eventq)) {
			al_wait_for_event(eventq, &event);
			if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
				printf("Got DISPLAY CLOSE event!\n");
				break;
			}
			if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
				printf("Got KEYBOARD event, %u\n", event.keyboard.keycode);
				if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
					break;
				else if (event.keyboard.keycode == ALLEGRO_KEY_LEFT)
					move_direction = LEFT;
				else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT)
					move_direction = RIGHT;
				else if (event.keyboard.keycode == ALLEGRO_KEY_UP)
					move_direction = UP;
				else if (event.keyboard.keycode == ALLEGRO_KEY_DOWN)
					move_direction = DOWN;
			}
			if (event.type == ALLEGRO_EVENT_TIMER) {
				if (event.timer.count % 8 == 0)
					s.st++;
				if (s.st >= 3)
					s.st = 0;
				s.di = move_direction;
				switch (move_direction) {
					case LEFT:
						move_left(&s, move_step);
						if (s.x <= 0)
							move_direction = RIGHT;
						break;
					case RIGHT:
						move_right(&s, move_step);
						if (s.x + s.w >= BG_WIDTH)
							move_direction = LEFT;
						break;
					case UP:
						move_up(&s, move_step);
						if (s.y <= 0)
							move_direction = DOWN;
						break;
					case DOWN:
						move_down(&s, move_step);
						if (s.y + s.h >= BG_HEIGHT)
							move_direction = UP;
						break;
					default:
						break;
				}
			}
		}
	}

	al_stop_timer(timer);

	/* Release resource */
	al_destroy_display(display);
	al_destroy_timer(timer);
	al_destroy_bitmap(s.bmp);
	al_destroy_event_queue(eventq);

	printf("%s: QUIT!\n", argv[0]);
	return 0;
}
