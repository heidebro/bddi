#include <json.h>

#define MAX_NAME_LEN 30
#define QUERY_SIZE 150
#define CHUNK_SIZE 512
#define PLACEHOLDER_ICON "?"

// Struct for leaf node linked list
struct link{
	struct json_object *node;
	struct link *next;
};

// Function definitions
struct json_object *get_desktop_info(char*);
void get_nodes(struct json_object*);
int update_desktop(char*);
char *get_name(struct link*);
int free_linked_list_nodes(struct link*);
int set_desktop_name(char*, char*);
