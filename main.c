#include <stdio.h>
#include "database.h"
#include "functions.h"


#define PORT 8080

mongoc_client_t *client = NULL;
mongoc_uri_t *uri = NULL;

void main(){
    struct _u_instance server;

    if (ulfius_init_instance(&server, PORT, NULL, NULL) != U_OK) {
        fprintf(stderr, "Error initiating server\n");
        return;
    }
    y_init_logs("Cryptos Cloud", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, "log.debug", "");
    database_init();
    server.max_post_param_size = 4096*1024;


    ulfius_add_endpoint_by_val(&server, "POST", "/api/register", NULL, 0, &callback_register, NULL);
    ulfius_add_endpoint_by_val(&server, "POST", "/api/login", NULL, 0, &callback_login, NULL);
    ulfius_add_endpoint_by_val(&server, "POST", "/api/upload", NULL, 1, &callback_upload_file, NULL);
    ulfius_add_endpoint_by_val(&server, "GET", "/api/files", NULL, 1, &callback_list_files, NULL);
    ulfius_add_endpoint_by_val(&server, "GET", "/api/file", NULL, 1, &callback_serve_file, NULL);



    if(ulfius_start_framework(&server) == U_OK){
        printf("Starting server on port ... %d\n", server.port);
        getchar();
    } else {
        fprintf(stderr, "Error starting server \n");
    }
    printf("Request processing ended\n");

    ulfius_stop_framework(&server);
    ulfius_clean_instance(&server);
    database_destory();
}