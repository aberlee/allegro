#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include "sprite.c"

#define SPRITE_FILE  "character.json"

#define BG_WIDTH		640
#define BG_HEIGHT		480
#define BG_COLOR	(al_map_rgb(0, 0, 0))

#define STAY	0
#define WALK	1
#define JUMP	2
#define ATTACK	3

int main(int argc, char **argv)
{
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *eventq = NULL;
	ALLEGRO_TIMER *timer = NULL;
	ALLEGRO_SPRITE *sprite = NULL;
	bool running = true;
	bool redraw = true;

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
	sprite = al_load_sprite(SPRITE_FILE);
	if (!sprite) {
		fprintf(stderr, "failed to load sprite "SPRITE_FILE"!\n");
		return -1;
	}

	al_sprite_set_map_size(sprite, BG_WIDTH, BG_HEIGHT);
	al_sprite_move_to(sprite, BG_WIDTH/2, BG_HEIGHT/2);

	al_sprite_add_action(sprite, STAY, 0, 2, 20, true);
	al_sprite_add_action(sprite, WALK, 1, 4, 10, true);
	al_sprite_add_action(sprite, JUMP, 2, 4, 10, false);
	al_sprite_add_action(sprite, ATTACK, 4, 4, 10, false);

	al_sprite_start_action(sprite, STAY);
	al_sprite_set_direction(sprite, ALLEGRO_SPRITE_LEFT);

	/* Create timer, 60fps */
	timer = al_create_timer(1.0/60.0);
	if (!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
	}

	printf("%s: START.\n", argv[0]);

	/* Set window title */
	al_set_window_title(display, "SPRITE!");
	/* Set background color */
	al_clear_to_color(BG_COLOR);
	/* Draw sprite */
	al_draw_sprite(sprite);
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

		if (al_is_event_queue_empty(eventq)) {
			if (redraw) {
				al_clear_to_color(BG_COLOR);
				al_draw_sprite(sprite);
				al_flip_display();
				redraw = false;
			}
			continue;
		}

		al_wait_for_event(eventq, &event);
		switch (event.type) {
			case ALLEGRO_EVENT_TIMER:
				if (event.timer.count % 10 == 0) {
					if (al_sprite_action_running(sprite)) {
						al_sprite_update_action(sprite);
						redraw = true;

						if (al_sprite_action_id(sprite) == WALK) {
							switch (al_sprite_get_direction(sprite)) {
								case ALLEGRO_SPRITE_DOWN:
									al_sprite_move_step(sprite, 0, 4);
									break;
								case ALLEGRO_SPRITE_UP:
									al_sprite_move_step(sprite, 0, -4);
									break;
								case ALLEGRO_SPRITE_RIGHT:
									al_sprite_move_step(sprite, 4, 0);
									break;
								case ALLEGRO_SPRITE_LEFT:
									al_sprite_move_step(sprite, -4, 0);
									break;
								default:
									break;
							}
						} else if (al_sprite_action_id(sprite) == JUMP) {
							int c = al_sprite_action_counter(sprite);
							switch (c) {
								case 1:
									al_sprite_move_step(sprite, 0, -8);
									break;
								case 2:
									al_sprite_move_step(sprite, 0, -8);
									break;
								case 3:
									al_sprite_move_step(sprite, 0, 8);
									break;
								case 4:
									al_sprite_move_step(sprite, 0, 8);
									break;
								default:
									al_sprite_stop_action(sprite);
									al_sprite_start_action(sprite, STAY);
									break;
							}
						} else if (al_sprite_action_id(sprite) == ATTACK) {
							int c = al_sprite_action_counter(sprite);
							if (c > 4) {
								al_sprite_stop_action(sprite);
								al_sprite_start_action(sprite, STAY);
							}
						}
					}
				}
				break;
			case ALLEGRO_EVENT_KEY_DOWN:
				if (event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
					al_sprite_start_action(sprite, JUMP);
					redraw = true;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_A) {
					al_sprite_start_action(sprite, ATTACK);
					redraw = true;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_DOWN ||
						event.keyboard.keycode == ALLEGRO_KEY_UP ||
						event.keyboard.keycode == ALLEGRO_KEY_RIGHT ||
						event.keyboard.keycode == ALLEGRO_KEY_LEFT) {

					al_sprite_start_action(sprite, WALK);
					redraw = true;

					switch (event.keyboard.keycode) {
						case ALLEGRO_KEY_DOWN:
							al_sprite_set_direction(sprite, ALLEGRO_SPRITE_DOWN);
							break;
						case ALLEGRO_KEY_UP:
							al_sprite_set_direction(sprite, ALLEGRO_SPRITE_UP);
							break;
						case ALLEGRO_KEY_RIGHT:
							al_sprite_set_direction(sprite, ALLEGRO_SPRITE_RIGHT);
							break;
						case ALLEGRO_KEY_LEFT:
							al_sprite_set_direction(sprite, ALLEGRO_SPRITE_LEFT);
							break;
						default:
							break;
					}
				}
				break;
			case ALLEGRO_EVENT_KEY_UP:
				if (event.keyboard.keycode == ALLEGRO_KEY_DOWN ||
						event.keyboard.keycode == ALLEGRO_KEY_UP ||
						event.keyboard.keycode == ALLEGRO_KEY_RIGHT ||
						event.keyboard.keycode == ALLEGRO_KEY_LEFT) {

					al_sprite_stop_action(sprite);
					al_sprite_start_action(sprite, STAY);
					redraw = true;
				}
				break;
			case ALLEGRO_EVENT_KEY_CHAR:
				if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
					running = false;
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
	al_destroy_sprite(sprite);
	al_destroy_event_queue(eventq);

	printf("%s: QUIT!\n", argv[0]);
	return 0;
}
