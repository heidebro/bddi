#include <stddef.h>

//
struct icon_link{
	char icon[10];
    char class_name[64]; 
	struct icon_link *next;
};

// Function definitions
void load_icon_list();
char *get_icon(const char*);
int free_linked_list_icons(struct icon_link*);
