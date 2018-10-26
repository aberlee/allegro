#ifndef __SPRITE_H__
#define __SPRITE_H__

typedef enum {
	ALLEGRO_SPRITE_DOWN = 0,
	ALLEGRO_SPRITE_UP = 1,
	ALLEGRO_SPRITE_RIGHT = 2,
	ALLEGRO_SPRITE_LEFT = 3,
} ALLEGRO_SPRITE_DIRECTION;

typedef struct _ALLEGRO_SPRITE ALLEGRO_SPRITE;

void al_dump_sprite(ALLEGRO_SPRITE *s);
int al_destroy_sprite(ALLEGRO_SPRITE *s);

ALLEGRO_SPRITE *al_load_sprite(const char *file_name);

int al_draw_sprite(ALLEGRO_SPRITE *s);

void al_sprite_set_map_size(ALLEGRO_SPRITE *s, int map_w, int map_h);
void al_sprite_move_to(ALLEGRO_SPRITE *s, int x, int y);
void al_sprite_move_step(ALLEGRO_SPRITE *s, int step_x, int step_y);
int al_sprite_get_x(ALLEGRO_SPRITE *s);
int al_sprite_get_y(ALLEGRO_SPRITE *s);
int al_sprite_get_width(ALLEGRO_SPRITE *s);
int al_sprite_get_height(ALLEGRO_SPRITE *s);
void al_sprite_set_direction(ALLEGRO_SPRITE *s, int d);
int al_sprite_get_direction(ALLEGRO_SPRITE *s);

int al_sprite_add_action(ALLEGRO_SPRITE *s, int id, int tileset_id,
					int counter_max, int fps_interval, bool stopable);
int al_sprite_start_action(ALLEGRO_SPRITE *s, int id);
void al_sprite_update_action(ALLEGRO_SPRITE *s);
void al_sprite_stop_action(ALLEGRO_SPRITE *s);
bool al_sprite_action_running(ALLEGRO_SPRITE *s);
int al_sprite_action_id(ALLEGRO_SPRITE *s);
void al_sprite_action_set_counter(ALLEGRO_SPRITE *s, int counter);
int al_sprite_action_counter(ALLEGRO_SPRITE *s);
int al_sprite_action_counter_max(ALLEGRO_SPRITE *s);
int al_sprite_action_fps_interval(ALLEGRO_SPRITE *s);
bool al_sprite_action_stopable(ALLEGRO_SPRITE *s);

#endif
