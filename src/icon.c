#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "icon.h"

// Global handles for icon LL
struct icon_link *icon_list_start = NULL, *icon_list_ptr;

void cleanup(){
    free_linked_list_icons(icon_list_start);
}

/*
 *  Free the linked list for icons
 */
int free_linked_list_icons(struct icon_link* list){
    struct icon_link *loc_link = list;
    struct icon_link *tmp_link;

    while(loc_link != NULL) {
        tmp_link = loc_link->next;
        free(loc_link);
        loc_link = tmp_link;
    }

    icon_list_start = NULL;
    icon_list_ptr = NULL;
}

/*
 *  Construct config path and try to open config file
 */
FILE *open_config_file(){
    char *home_dir = getenv("HOME");
    int home_dir_len = strlen(home_dir);
    char *conf_dir = calloc(home_dir_len + 25, 1);

    strncpy(conf_dir, home_dir, home_dir_len);
    strncat(conf_dir, "/.config/bddi/icons.list", 25);

    FILE *file = fopen(conf_dir, "r");

    if (file == NULL) {
        printf("No icon file found");
        exit(-1);
    }

    free(conf_dir);
    return file;
}

/*
 *  Load icons defined in config file
 */
void load_icon_list(){
    int status;
    FILE *file = open_config_file();

    printf("Loading icons\n");

    while (true) {

        struct icon_link *link = calloc(sizeof(struct icon_link), 1);
        link->next = NULL;

        status = fscanf(file, "%s %s", link->class_name, link->icon);

        // If no two elements were found, quit.
        if(status != 2){ 
            free(link);
            break;
        }

        printf("Loading icon for %s\n", link->class_name);

        if (icon_list_start == NULL){
            icon_list_ptr = icon_list_start = link;
        }else{
            icon_list_ptr->next = link;
            icon_list_ptr = icon_list_ptr->next;
        }
    }

    fclose(file);
}

/*
 *  Return icon corresponding to passed class_name if present in icon LL
 */
char *get_icon(const char *class_name){
    struct icon_link *link = icon_list_start;
    int class_name_len = strlen(class_name);

    while (link != NULL){
        if (strncmp(class_name, link->class_name, class_name_len) == 0){
            return link->icon;
        } else {
            link = link->next;
        }
    }

    printf("No suitable icon found for %s", class_name);
    return NULL;
} 
