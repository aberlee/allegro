#include <stdio.h>
#include <stdlib.h>
#include <json-c/json.h>

static void json_travel(struct json_object *obj, int level)
{
	int i;
	struct json_object_iterator item;
	struct json_object_iterator item_end;
	struct json_object *child = NULL;

	item = json_object_iter_begin(obj);
	item_end = json_object_iter_end(obj);

	while (!json_object_iter_equal(&item, &item_end)) {
		for (i = 0; i < level; i++)
			printf("    ");

		printf("%s: ", json_object_iter_peek_name(&item));

		child = json_object_iter_peek_value(&item);
		if (child) {
			if (json_object_is_type(child, json_type_string))
				printf("%s", json_object_get_string(child));
			else if (json_object_is_type(child, json_type_int))
				printf("%d", json_object_get_int(child));
			else if (json_object_is_type(child, json_type_double))
				printf("%f", json_object_get_double(child));
			else if (json_object_is_type(child, json_type_boolean))
				printf("%s", json_object_get_boolean(child) ? "true" : "false");
		}
		printf("\n");

		if (child && json_object_is_type(child, json_type_object))
			json_travel(child, level+1);

		json_object_iter_next(&item);
	}
}

int main(int argc, char **argv)
{
	char *fn = argv[1];
	struct json_object *json = NULL, *obj = NULL;

	if (!fn) {
		printf("usage: %s fn\n", argv[0]);
		return -1;
	}

	json = json_object_from_file(fn);
	if (!json) {
		printf("Parse json file [%s] error!\n", fn);
		return -1;
	}

	json_travel(json, 0);
	json_object_put(json);
	return 0;
}
