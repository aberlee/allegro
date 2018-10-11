#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>

static void draw_circle(int x, int y,
				int r, ALLEGRO_COLOR color)
{
	int w = r * 2;
	int h = r * 2;
	int i, j;

	for (i = 0; i <= w; i++) {
		for (j = 0; j <= h; j++) {
			int x1 = x - r + i;
			int y1 = y - r + j;
			int a = sqrt(abs(x1-x)*abs(x1-x)+abs(y1-y)*abs(y1-y));
			if (a <= r)
				al_draw_pixel(x1, y1, color);
		}
	}
}

static void draw_rect(int x, int y,
				int w, int h, ALLEGRO_COLOR color)
{
	int i, j;

	for (i = 0; i <= w; i++) {
		for (j = 0; j <= h; j++)
			al_draw_pixel(x+i, y+j, color);
	}
}

static void draw_rect_xy(int x1, int y1,
				int x2, int y2, ALLEGRO_COLOR color)
{
	int x = (x1 < x2) ? x1 : x2;
	int y = (y1 < y2) ? y1 : y2;
	int w = abs(x2 - x1);
	int h = abs(y2 - y1);
	draw_rect(x, y, w, h, color);
}

static void draw_line(int x1, int y1,
				int x2, int y2, ALLEGRO_COLOR color)
{
	int x, y;
	int x_step, y_step;

	x_step = (x1 < x2) ? 1 : -1;
	y_step = (y1 < y2) ? 1 : -1;

	if (x1 == x2) {
		for (y = y1; (y_step > 0) ? (y <= y2) : (y >= y2); y += y_step)
			al_draw_pixel(x1, y, color);
		return;
	}

	if (y1 == y2) {
		for (x = x1; (x_step > 0) ? (x <= x2) : (x >= x2); x += x_step)
			al_draw_pixel(x, y1, color);
		return;
	}

	for (x = x1; (x_step > 0) ? (x <= x2) : (x >= x2); x += x_step) {
		y = (x - x1) * (y2 - y1) / (x2 - x1) + y1;
		al_draw_pixel(x, y, color);
	}

	for (y = y1; (y_step > 0) ? (y <= y2) : (y >= y2); y += y_step) {
		x = (y - y1) * (x2 - x1) / (y2 - y1) + x1;
		al_draw_pixel(x, y, color);
	}
}


struct block {
	int x;
	int y;
	int w;
	int h;
	ALLEGRO_COLOR c;
};

enum direction {
	LEFT = 1,
	RIGHT = 2,
	UP = 3,
	DOWN = 4,
};

#define BG_WIDTH		640
#define BG_HEIGHT		480
#define BG_COLOR	(al_map_rgb(0, 0, 0))

static void block_flip(struct block *b)
{
	draw_rect(b->x, b->y, b->w, b->h, b->c);
	al_flip_display();
}

static void move_left(struct block *b, int step)
{
	draw_rect(b->x, b->y, b->w, b->h, BG_COLOR);

	if (b->x >= step)
		b->x -= step;
	else
		b->x = 0;

	block_flip(b);
}

static void move_right(struct block *b, int step)
{
	draw_rect(b->x, b->y, b->w, b->h, BG_COLOR);

	if (b->x + b->w <= BG_WIDTH - step)
		b->x += step;
	else
		b->x = BG_WIDTH - b->w;

	block_flip(b);
}

static void move_up(struct block *b, int step)
{
	draw_rect(b->x, b->y, b->w, b->h, BG_COLOR);

	if (b->y >= step)
		b->y -= step;
	else
		b->y = 0;

	block_flip(b);
}

static void move_down(struct block *b, int step)
{
	draw_rect(b->x, b->y, b->w, b->h, BG_COLOR);

	if (b->y + b->h < BG_HEIGHT - step)
		b->y += step;
	else
		b->y = BG_HEIGHT - b->h;

	block_flip(b);
}

int main(int argc, char **argv)
{
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *eventq = NULL;
	ALLEGRO_EVENT event;
	ALLEGRO_TIMER *timer = NULL;
	int move_direction = RIGHT;
	int move_step = 5;
	struct block b = {
		.x = 10,
		.y = 10,
		.w = 30,
		.h = 30,
		.c = {
			.r = 0xff,
			.g = 0xff,
			.b = 0xff,
			.a = 0xff,
		}
	};

	/* Init allegro engine */
	if (!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}

	/* Init keyboard & mouse support */
	al_install_keyboard();
	al_install_mouse();

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

	/* Create timer, 0.04 second, 25fps */
	timer = al_create_timer(0.04);
	if (!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}

	printf("%s: START.\n", argv[0]);

	/* Set window title */
	al_set_window_title(display, "BLOCK MOVING!");
	/* Set background color */
	al_clear_to_color(BG_COLOR);
	/* Show the window */
	al_flip_display();

	/* Register event source */
	al_register_event_source(eventq, al_get_keyboard_event_source());
	al_register_event_source(eventq, al_get_display_event_source(display));
	al_register_event_source(eventq, al_get_timer_event_source(timer));

	/* Draw a block */
	block_flip(&b);

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
				switch (move_direction) {
					case LEFT:
						move_left(&b, move_step);
						if (b.x <= 0)
							move_direction = RIGHT;
						break;
					case RIGHT:
						move_right(&b, move_step);
						if (b.x + b.w >= BG_WIDTH)
							move_direction = LEFT;
						break;
					case UP:
						move_up(&b, move_step);
						if (b.y <= 0)
							move_direction = DOWN;
						break;
					case DOWN:
						move_down(&b, move_step);
						if (b.y + b.h >= BG_HEIGHT)
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
	al_destroy_event_queue(eventq);

	printf("%s: QUIT!\n", argv[0]);
	return 0;
}
