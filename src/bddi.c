#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <string.h>
#include <json.h>

#include <bddi.h>
#include <icon.h>

struct link *start = NULL, *ptr;

/*
 *  Opens a bspwm socket and returns its filedescriptor
 */ 
int get_socket(){
    int sock_fd;
    struct sockaddr_un sock_address;

    sock_address.sun_family = AF_UNIX;

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    snprintf(sock_address.sun_path, sizeof(sock_address.sun_path), "%s", "/tmp/bspwm_0_0-socket");

    connect(sock_fd, (struct sockaddr *) &sock_address, sizeof(sock_address));

    return sock_fd;
}

/*
 * Configures the passed socket to listen for node events.
 * When either a node_add, node_transfer or a node_remove event is recieved, 
 * the corresponding desktop names are updated.
 */
int subscribe(int sock_fd){
    char resp[512];
    int res;
    char msg[] = "subscribe\x00node";
    int msglen = sizeof("subscribe\x00node");

    res = send(sock_fd, msg, msglen, 0);
    if (res <= 0) printf("ERROR");

	printf("Listening for node events...\n");

    while(res > 0){
        res = recv(sock_fd, resp, sizeof(resp), 0);
        resp[res] = '\0';

        if (res <= 0) {
			printf("ERROR");
			break;
		}

        char *sep = " ";
        char *token, *dest_desktop_id, *src_desktop_id;

        token = strtok(resp, sep);

        if (strncmp(token, "node_add", 8) == 0 || strncmp(token, "node_remove", 11) == 0) {
            strtok(NULL, sep);
            dest_desktop_id = strtok(NULL, sep);

            update_desktop(dest_desktop_id);
        }else if (strncmp(token, "node_transfer", 13) == 0) {
            strtok(NULL, sep);
            src_desktop_id = strtok(NULL, sep);
            strtok(NULL, sep);
            strtok(NULL, sep);
            dest_desktop_id = strtok(NULL, sep);

            update_desktop(src_desktop_id);
            update_desktop(dest_desktop_id);
        }
    }
}

/*
 *  Updates the name of the desktop associated with the passed desktop_id. 
 */
int update_desktop(char *desktop_id){
    struct json_object *desktop_info = get_desktop_info(desktop_id);
    char *name;

    // Get desktop info JSON tree 
	desktop_info = json_object_object_get(desktop_info, "root");

    // Populate linked list with leaf nodes
	get_nodes(desktop_info);

    // Generate name based on leaf nodes
    name = get_name(start);

    // setname
    set_desktop_name(desktop_id, name);

    // Clean linked list
    free_linked_list_nodes(start);


    // Free JSON tree
	json_object_put(desktop_info);
    free(name);
}


/*
 *  Set name of desktop associated with passed desktop_id to passed name.
 */
int set_desktop_name(char *desktop_id, char *name){;
    int sock_fd = get_socket();
    char query_format[] = "desktop\x00%s\x00-n\x00%s";
    char *query = calloc(QUERY_SIZE,1);

    strncpy(query, "desktop", 8); 
    strncpy((query+8), desktop_id, 11); 
    strncpy((query+19), "-n", 3); 
    strncpy((query+22), name, MAX_NAME_LEN); 

    send(sock_fd, query, QUERY_SIZE, 0);

    char *resp[512];

    close(sock_fd); 
}

/*
 *  Free the linked list for desktop nodes
 */
int free_linked_list_nodes(struct link* list){
    struct link *loc_link = list;
    struct link *tmp_link;

    while(loc_link != NULL) {
        tmp_link = loc_link->next;
        free(loc_link);
        loc_link = tmp_link;
    }

    start = NULL;
    ptr = NULL;
}

/*
 *  Generate and return desktop name based on passed list of nodes 
 */
char *get_name(struct link* list){
    struct link* loc_link = list;
    struct json_object *obj;
    const char *class_name;
    char *icon, *name = calloc(MAX_NAME_LEN, 1);
    int offset = 0;
    
    while(loc_link != NULL){
        obj = json_object_object_get(loc_link->node, "className");
        class_name = json_object_get_string(obj);
        icon = get_icon(class_name);

        // Add Space between icons if it's not the first one
        if (offset != 0) {
            strncat(name, " ", MAX_NAME_LEN-offset);
            offset += 1;
        }

        // If no icon was found, append placeholder
        // else, append icon.
        if (icon == NULL) {
            strncat(name, PLACEHOLDER_ICON, MAX_NAME_LEN-offset);

            offset += 1;
        }else{
            strncat(name, icon, MAX_NAME_LEN-offset);

            offset += strlen(icon);
        }

        loc_link = loc_link->next;
    }

    return name;
}

/*
 *  Recursively traverse passed BSPWM tree and add all leaf nodes to global linked list
 */
void get_nodes(struct json_object *tree){

	if (tree == NULL ) {
		printf("ERROR tree: %p", tree);
	   	return;
	}

    struct link *link = calloc(sizeof(struct link), 1);

	// recursively get leaf nodes
	json_object *first = json_object_object_get(tree, "firstChild");
	json_object *second = json_object_object_get(tree, "secondChild");
	json_object *node = json_object_object_get(tree, "client");

    // Populate linked list
	if (node != NULL) {
        link->node = node;
        link->next = NULL;

        if (start == NULL){
            ptr = start = link;
        } else {
            ptr->next = link;
            ptr = ptr->next;
        }
	} else {
		get_nodes(first);
		get_nodes(second);
	}
}

/*
 *  Request desktop tree of desktop with passed id and return it as parsed json.
 */
struct json_object *get_desktop_info(char *desktop_id){
    // Defines
    struct json_object *parsed_json;
    int res, bytes_recieved = 0;
    char *json = calloc(CHUNK_SIZE, 16);
	char resp[512];
    char msg[23];
    int sock_fd = get_socket();

    // Prepare request
	strncpy(msg, "query", 6);
	strncpy((msg+6), "-T", 3);
	strncpy((msg+9), "-d", 3);
	strncpy((msg+12), desktop_id, 11);

    // Send request
    res = send(sock_fd, msg, 23, 0);


    // Receive answer
    do{
        // Resize array that holds json
        if (bytes_recieved >= CHUNK_SIZE*16) {
            void *b = reallocarray(json, CHUNK_SIZE, 1);
            if (b == NULL) break;
        }

        // Recieve next chunk of answer
        res = recv(sock_fd, (json+bytes_recieved), CHUNK_SIZE, 0);
        bytes_recieved += CHUNK_SIZE;
    } while(res > 0);

    // Terminate connection
    close(sock_fd);

    // Parse and return json
    parsed_json = json_tokener_parse(json);
    return parsed_json;
}

/*
 * Main function
 */
int main(int argc, char *argv[]){
    int sock_fd = get_socket();
    load_icon_list();
    subscribe(sock_fd);
    return 0;
}
