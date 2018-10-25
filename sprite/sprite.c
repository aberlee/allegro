#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <cJSON.h>
#include <cJSON_Utils.h>

#define ERROR_RETURN(x) \
	do{	\
		fprintf(stderr, "ALLEGRO_SPRITE ERROR [%s(), LINE %d in %s]\n", __func__, __LINE__, __FILE__); \
		return x; \
	} while(0)

typedef struct {
	int x;
	int y;
} ALLEGRO_SPRITE_TILE;

typedef struct {
	int tile_count;
	int tile_width;
	int tile_height;
	ALLEGRO_SPRITE_TILE *tiles;

	ALLEGRO_SPRITE_TILE *tiles_down;  /* pointer to tiles[0] */
	ALLEGRO_SPRITE_TILE *tiles_up;    /* pointer to tiles[tile_count * 1 / 4] */
	ALLEGRO_SPRITE_TILE *tiles_right; /* pointer to tiles[tile_count * 2 / 4] */
	ALLEGRO_SPRITE_TILE *tiles_left;  /*/pointer to tiles[tile_count * 3 / 4] */
} ALLEGRO_SPRITE_TILESET;

typedef struct {
	/* bitmap resource */
	char *image_file;
	int image_width;
	int image_height;
	ALLEGRO_BITMAP *bitmap;

	/* tilesets */
	int tileset_count;
	ALLEGRO_SPRITE_TILESET *tilesets;
} ALLEGRO_SPRITE_LAYER;

typedef enum {
	ALLEGRO_SPRITE_FACE_DOWN = 0,
	ALLEGRO_SPRITE_FACE_UP = 1,
	ALLEGRO_SPRITE_FACE_RIGHT = 2,
	ALLEGRO_SPRITE_FACE_LEFT = 3,
} ALLEGRO_SPRITE_FACE_DIRECTION;

struct _ALLEGRO_SPRITE {
	int x;
	int y;
	int w;
	int h;

	int map_width;
	int map_height;

	ALLEGRO_SPRITE_FACE_DIRECTION faceto;
	int moving;
	int attacking;
	int jumping;

	int layer_count;
	ALLEGRO_SPRITE_LAYER *layers;

	int selected_tileset_id;
	int animation_counter;
};

typedef struct _ALLEGRO_SPRITE ALLEGRO_SPRITE;

static void al_dump_sprite(ALLEGRO_SPRITE *s)
{
	int i, j, t;

	if (!s)
		return;

	printf("Sprite: pos (%d, %d), %d layers.\n", s->x, s->y, s->layer_count);

	for (i = 0; i < s->layer_count; i++) {
		printf("Layer %d: %d tilesets\n", i+1, s->layers[i].tileset_count);
		printf("\tImage:\n");
		printf("\t\tFile: %s\n", s->layers[i].image_file);
		printf("\t\tWidth:  %4d\n", s->layers[i].image_width);
		printf("\t\tHeight: %4d\n", s->layers[i].image_height);
		for (j = 0; j < s->layers[i].tileset_count; j++) {
			printf("\tTileset %d: %d tiles\n", j+1, s->layers[i].tilesets[i].tile_count);
			printf("\t\tTile width:  %4d\n", s->layers[i].tilesets[i].tile_width);
			printf("\t\tTile height: %4d\n", s->layers[i].tilesets[i].tile_height);
			for (t = 0; t < s->layers[i].tilesets[j].tile_count; t++) {
				printf("\t\t%02d: %4d, %4d\n", t+1,
						s->layers[i].tilesets[j].tiles[t].x,
						s->layers[i].tilesets[j].tiles[t].y);
			}
		}
	}
	printf("\n");
}

int al_destroy_sprite(ALLEGRO_SPRITE *s)
{
	int i, j;

	if (!s->layers)
		return 0;

	for (i = 0; i < s->layer_count; i++) {
		if (s->layers[i].image_file)
			free(s->layers[i].image_file);

		if (s->layers[i].bitmap)
			al_destroy_bitmap(s->layers[i].bitmap);

		if (s->layers[i].tilesets) {
			for (j = 0; j < s->layers[i].tileset_count; j++) {
				if (s->layers[i].tilesets[j].tiles)
					free(s->layers[i].tilesets[j].tiles);
			}
			free(s->layers[i].tilesets);
		}
	}

	free(s->layers);
	free(s);
	return 0;
}


/***************************************************************************************/
/********** Parse Sprite Data From JSON File *******************************************/
/***************************************************************************************/

static int al_parse_sprite_tiles(ALLEGRO_SPRITE_TILE *tiles, cJSON *obj)
{
	int i, size = 0;

	if (!obj || !cJSON_IsArray(obj))
		ERROR_RETURN(-1);

	size = cJSON_GetArraySize(obj);
	for (i = 0; i < size; i++) {
		cJSON *item, *item_x, *item_y;

		item = cJSON_GetArrayItem(obj, i);
		if (!item || !cJSON_IsObject(item))
			ERROR_RETURN(-1);

		item_x = cJSON_GetObjectItem(item, "x");
		if (!item_x || !cJSON_IsNumber(item_x))
			ERROR_RETURN(-1);

		item_y = cJSON_GetObjectItem(item, "y");
		if (!item_y || !cJSON_IsNumber(item_y))
			ERROR_RETURN(-1);

		tiles[i].x = (int)item_x->valuedouble;
		tiles[i].y = (int)item_y->valuedouble;
	}
	return 0;
}

static int al_parse_sprite_tileset(ALLEGRO_SPRITE_TILESET *tileset, cJSON *obj)
{
	cJSON *item_s, *item_t;
	int w, h, c, i;

	/* Parse tile count */
	item_s = cJSON_GetObjectItem(obj, "count");
	if (!item_s || !cJSON_IsNumber(item_s))
		ERROR_RETURN(-1);
	c = (int)item_s->valuedouble;

	/* Parse tile size */
	item_s = cJSON_GetObjectItem(obj, "size");
	if (!item_s || !cJSON_IsObject(item_s))
		ERROR_RETURN(-1);

	item_t = cJSON_GetObjectItem(item_s, "w");
	if (!item_t || !cJSON_IsNumber(item_t))
		ERROR_RETURN(-1);
	w = (int)item_t->valuedouble;

	item_t = cJSON_GetObjectItem(item_s, "h");
	if (!item_t || !cJSON_IsNumber(item_t))
		ERROR_RETURN(-1);
	h = (int)item_t->valuedouble;

	/* malloc tiles array */
	tileset->tiles = malloc(sizeof(ALLEGRO_SPRITE_TILE) * c);
	if (!tileset->tiles)
		ERROR_RETURN(-1);

	tileset->tile_count = c;
	tileset->tile_width = w;
	tileset->tile_height = h;
	tileset->tiles_down = &(tileset->tiles[0]);
	tileset->tiles_up = &(tileset->tiles[c/4]);
	tileset->tiles_right = &(tileset->tiles[c*2/4]);
	tileset->tiles_left = &(tileset->tiles[c*3/4]);

	/* Parse tiles */
	item_s = cJSON_GetObjectItem(obj, "face_down");
	if (!item_s || !cJSON_IsArray(item_s))
		ERROR_RETURN(-1);
	if (al_parse_sprite_tiles(tileset->tiles_down, item_s))
		ERROR_RETURN(-1);

	item_s = cJSON_GetObjectItem(obj, "face_up");
	if (!item_s || !cJSON_IsArray(item_s))
		ERROR_RETURN(-1);
	if (al_parse_sprite_tiles(tileset->tiles_up, item_s))
		ERROR_RETURN(-1);

	item_s = cJSON_GetObjectItem(obj, "face_right");
	if (!item_s || !cJSON_IsArray(item_s))
		ERROR_RETURN(-1);
	if (al_parse_sprite_tiles(tileset->tiles_right, item_s))
		ERROR_RETURN(-1);

	item_s = cJSON_GetObjectItem(obj, "face_left");
	if (!item_s || !cJSON_IsArray(item_s))
		ERROR_RETURN(-1);
	if (al_parse_sprite_tiles(tileset->tiles_left, item_s))
		ERROR_RETURN(-1);

	return 0;
}

static int al_parse_sprite_layer(ALLEGRO_SPRITE_LAYER *layer, cJSON *obj)
{
	int ret = 0;
	int count = 0, i;
	cJSON *item, *item_s, *item_t;

	memset(layer, 0, sizeof(ALLEGRO_SPRITE_LAYER));

	/* Parse image info */
	item = cJSON_GetObjectItem(obj, "image");
	if (!item || !cJSON_IsObject(item))
		ERROR_RETURN(-1);

	/* Image file path */
	item_s = cJSON_GetObjectItem(item, "file");
	if (!item_s || !cJSON_IsString(item_s))
		ERROR_RETURN(-1);
	layer->image_file = malloc(strlen(item_s->valuestring)+2);
	if (!layer->image_file)
		ERROR_RETURN(-1);
	strcpy(layer->image_file, item_s->valuestring);
		
	/* Image size, width X height */
	item_s = cJSON_GetObjectItem(item, "size");
	if (!item_s || !cJSON_IsObject(item_s))
		ERROR_RETURN(-1);

	item_t = cJSON_GetObjectItem(item_s, "w");
	if (!item_t || !cJSON_IsNumber(item_t))
		ERROR_RETURN(-1);
	layer->image_width = (int)item_t->valuedouble;

	item_t = cJSON_GetObjectItem(item_s, "h");
	if (!item_t || !cJSON_IsNumber(item_t))
		ERROR_RETURN(-1);
	layer->image_height = (int)item_t->valuedouble;

	/* Get tileset count */
	while (item = item->next)
		count++;
	if (!count) {
		fprintf(stderr, "Cannot find any tilesets.\n");
		ERROR_RETURN(-1);
	}

	/* Alloc tielsets array */
	layer->tilesets = malloc(sizeof(ALLEGRO_SPRITE_TILESET) * count);
	if (!layer->tilesets)
		ERROR_RETURN(-1);
	layer->tileset_count = count;

	/* Parse tilesets */
	item = cJSON_GetObjectItem(obj, "image"); /* return to head */
	for (i = 0; i < count; i++) {
		item = item->next;
		ret = al_parse_sprite_tileset(&layer->tilesets[i], item);
		if (ret)
			ERROR_RETURN(-1);
	}

	/* Load image file to bitmap */
	layer->bitmap = al_load_bitmap(layer->image_file);
	if (!layer->bitmap)
		ERROR_RETURN(-1);
	return 0;
}

static int al_parse_sprite(ALLEGRO_SPRITE *s, cJSON *obj)
{
	int i, ret = 0;
	cJSON *item;

	if (!s || !obj)
		ERROR_RETURN(-1);

	memset(s, 0, sizeof(ALLEGRO_SPRITE));

	item = cJSON_GetObjectItem(obj, "layers");
	if (!item || !cJSON_IsArray(item))
		ERROR_RETURN(-1);

	s->layer_count = cJSON_GetArraySize(item);
	s->layers = malloc(sizeof(ALLEGRO_SPRITE_LAYER) * s->layer_count);
	if (!s->layers)
		ERROR_RETURN(-1);

	for (i = 0; i < s->layer_count; i++) {
		ret = al_parse_sprite_layer(&s->layers[i], cJSON_GetArrayItem(item, i));
		if (ret)
			ERROR_RETURN(ret);
	}

	return 0;
}

static cJSON *al_json_parse(const char *file_name)
{
	FILE *fp = NULL;
	char *data = NULL;
	int size = 0;
	cJSON *json = NULL;

	fp = fopen(file_name, "r");
	if (!fp) {
		fprintf(stderr, "Open file [%s] error!\n", file_name);
		goto exit;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	data = malloc(size);
	if (!data) {
		fprintf(stderr, "Can't malloc memory, %d bytes\n", size);
		goto exit;
	}

	fread(data, 1, size, fp);

	json = cJSON_Parse(data);
	if (!json)
		fprintf(stderr, "Parse json data error!\n");

exit:
	if (fp)
		fclose(fp);
	if (data)
		free(data);
	return json;
}

ALLEGRO_SPRITE *al_load_sprite(const char *file_name)
{
	int ret;
	cJSON *json;
	ALLEGRO_SPRITE *s;
	
	s = malloc(sizeof(ALLEGRO_SPRITE));
	if (!s)
		return NULL;

	json = al_json_parse(file_name);

	ret = al_parse_sprite(s, json);
	if (ret) {
		al_destroy_sprite(s);
		return NULL;
	}
	return s;
}


/***************************************************************************************/
/********** Sprite Status Update *******************************************************/
/***************************************************************************************/

int al_sprite_select_tileset(ALLEGRO_SPRITE *s, int id)
{
	if (id < 0 || id > s->layers[0].tileset_count)
		return -1;

	s->selected_tileset_id = id;
	s->w = s->layers[0].tilesets[id].tile_width;
	s->h = s->layers[0].tilesets[id].tile_height;
	return 0;
}

void al_sprite_set_map_size(ALLEGRO_SPRITE *s, int map_w, int map_h)
{
	s->map_width = map_w;
	s->map_height = map_h;
}

void al_sprite_set_position(ALLEGRO_SPRITE *s, int x, int y)
{
	s->x = x;
	s->y = y;
}

int al_sprite_get_x(ALLEGRO_SPRITE *s)
{
	return s->x;
}

int al_sprite_get_y(ALLEGRO_SPRITE *s)
{
	return s->y;
}

void al_sprite_turn_down(ALLEGRO_SPRITE *s)
{
	s->faceto = ALLEGRO_SPRITE_FACE_DOWN;
}

void al_sprite_turn_up(ALLEGRO_SPRITE *s)
{
	s->faceto = ALLEGRO_SPRITE_FACE_UP;
}

void al_sprite_turn_right(ALLEGRO_SPRITE *s)
{
	s->faceto = ALLEGRO_SPRITE_FACE_RIGHT;
}

void al_sprite_turn_left(ALLEGRO_SPRITE *s)
{
	s->faceto = ALLEGRO_SPRITE_FACE_LEFT;
}

void al_sprite_move(ALLEGRO_SPRITE *s, int step)
{
	s->moving++;

	switch (s->faceto) {
		case ALLEGRO_SPRITE_FACE_DOWN:
			if (s->y + s->h < s->map_height - step)
				s->y += step;
			else
				s->y = s->map_height - s->h;
			break;
		case ALLEGRO_SPRITE_FACE_UP:
			if (s->y >= step)
				s->y -= step;
			else
				s->y = 0;
			break;
		case ALLEGRO_SPRITE_FACE_RIGHT:
			if (s->x + s->w < s->map_width - step)
				s->x += step;
			else
				s->x = s->map_width - s->w;
			break;
		case ALLEGRO_SPRITE_FACE_LEFT:
			if (s->x >= step)
				s->x -= step;
			else
				s->x = 0;
			break;
		default:
			break;
	}
}

void al_sprite_move_stop(ALLEGRO_SPRITE *s)
{
	s->moving--;
	if (s->moving < 0)
		s->moving = 0;
}

void al_sprite_jump(ALLEGRO_SPRITE *s)
{
	int step = s->h/2;
	s->jumping++;

	switch (s->faceto) {
		case ALLEGRO_SPRITE_FACE_DOWN:
			if (s->y + s->h < s->map_height - step)
				s->y += step;
			else
				s->y = s->map_height - s->h;
			break;
		case ALLEGRO_SPRITE_FACE_UP:
			if (s->y >= step)
				s->y -= step;
			else
				s->y = 0;
			break;
		case ALLEGRO_SPRITE_FACE_RIGHT:
		case ALLEGRO_SPRITE_FACE_LEFT:
			s->y -= step;
			break;
		default:
			break;
	}
}

void al_sprite_jump_stop(ALLEGRO_SPRITE *s)
{
	int step = s->h/2;

	s->jumping--;
	if (s->jumping < 0)
		s->jumping = 0;

	switch (s->faceto) {
		case ALLEGRO_SPRITE_FACE_RIGHT:
		case ALLEGRO_SPRITE_FACE_LEFT:
			s->y += step;
			break;
		default:
			break;
	}
}

void al_sprite_attack(ALLEGRO_SPRITE *s)
{
	s->attacking++;
}

void al_sprite_attack_stop(ALLEGRO_SPRITE *s)
{
	s->attacking--;
	if (s->attacking < 0)
		s->attacking = 0;
}

void al_sprite_update_animation(ALLEGRO_SPRITE *s)
{
	s->animation_counter++;
	if (s->animation_counter >= 2520)
		s->animation_counter = 0;
}

int al_sprite_is_jumping(ALLEGRO_SPRITE *s)
{
	return s->jumping;
}

int al_sprite_is_moving(ALLEGRO_SPRITE *s)
{
	return s->moving;
}

int al_sprite_is_attacking(ALLEGRO_SPRITE *s)
{
	return s->attacking;
}


/***************************************************************************************/
/********** Sprite Draw ****************************************************************/
/***************************************************************************************/

int al_draw_sprite(ALLEGRO_SPRITE *s)
{
	int i;
	int tileset_id = s->selected_tileset_id;
	int tile_id;

	for (i = 0; i < s->layer_count; i++) {
		tile_id = s->animation_counter % (s->layers[i].tilesets[tileset_id].tile_count / 4);
		tile_id += s->faceto * s->layers[i].tilesets[tileset_id].tile_count / 4;

		al_draw_bitmap_region(s->layers[i].bitmap,
				s->layers[i].tilesets[tileset_id].tiles[tile_id].x,
				s->layers[i].tilesets[tileset_id].tiles[tile_id].y,
				s->layers[i].tilesets[tileset_id].tile_width,
				s->layers[i].tilesets[tileset_id].tile_height,
				s->x, s->y, 0);
	}
	return 0;
}


