//
// Created by safa on 12/29/20.
//
#include <ulfius.h>
#include <yder.h>
#include <jansson.h>
#include <jwt.h>
#include <time.h>
#include <string.h>

#include "database.h"
const unsigned char* secretKey = "SecretKey";
int callback_register(const struct _u_request *request, struct _u_response * response , void * user_data) {
    mongoc_collection_t *collection;
    json_t * responsebody = json_object();
    char * username = u_map_get(request->map_post_body, "username");
    char * password = u_map_get(request->map_post_body, "password");
    y_log_message(Y_LOG_LEVEL_DEBUG, "username = %s, password = %s", username, password);

    /*** initialize the database and the collection ****/
    collection = mongoc_client_get_collection (client, "db_server", "users");
    y_log_message(Y_LOG_LEVEL_DEBUG, "collection users loaded");

    //step two: search for user using username* (return error if the user exist)
    if (check_username_exist(collection, username)) {
        //user exit make error
        y_log_message(Y_LOG_LEVEL_ERROR, "%s username already exist", username); // log message
        json_object_set(responsebody, "success", json_integer(-1)); // add success integer to the response ! {"success": -1}
        json_object_set(responsebody, "message", json_string("username already exist")); // add message to the response: {"success": -1, "message": "username already exist"}
        ulfius_set_json_body_response(response, 409, responsebody);// send response to the client with http status code 409
        json_decref(responsebody);
        return U_CALLBACK_CONTINUE;
    }
    //step three: register user in the database
    if (!create_user(collection, username, password)) {
        //user exit make error
        json_object_set(responsebody, "success", json_integer(-1)); // add success integer to the response ! {"success": -1}
        json_object_set(responsebody, "message", json_string("error inserting user")); // add message to the response: {"success": -1, "message": "username already exist"}
        ulfius_set_json_body_response(response, 500, responsebody);// send response to the client with http status code 409
        json_decref(responsebody);
        return U_CALLBACK_CONTINUE;
    }
    json_object_set(responsebody, "success", json_integer(1)); // add success integer to the response ! {"success": -1}
    json_object_set(responsebody, "message", json_string("user has been created succesfully")); // add message to the response: {"success": -1, "message": "username already exist"}
    ulfius_set_json_body_response(response, 200, responsebody);// send response to the client with http status code 409
    json_decref(responsebody);

    //step four: disconnect mongodb

    return U_CALLBACK_CONTINUE;
}


int callback_login(const struct  _u_request *request, struct _u_response *response ,void *user_data){
    jwt_t *jwt;
    int jwt_ret = 0;
    char * username = u_map_get(request->map_post_body, "username"); // get the username from the post request (data sent by the user)
    char * password = u_map_get(request->map_post_body, "password"); // get the password from the post request (data sent by the user)
    mongoc_collection_t *collection = mongoc_client_get_collection (client, "db_server", "users");



    json_t * responsebody = find_user(collection, username, password);
    int success = json_integer_value(json_object_get(responsebody, "success"));
    if (success != -1){
        jwt_ret = jwt_new(&jwt);
        time_t iat = time(NULL);
        jwt_add_grant_int(jwt, "iat", iat);
        jwt_add_grant(jwt, "id", json_string_value(json_object_get(json_object_get(responsebody, "_id"), "$oid")));
        jwt_set_alg(jwt, JWT_ALG_HS256, secretKey, strlen( "SecretKey"));
        char * out = jwt_encode_str(jwt);
        json_object_set(responsebody, "token", json_string(out));
    }
    y_log_message(Y_LOG_LEVEL_DEBUG, "success = %d", success);
    ulfius_set_json_body_response(response, success == -1 ? 401 : 200, responsebody);
    json_decref(responsebody);

    return U_CALLBACK_CONTINUE;
}


int callback_upload_file (const struct _u_request * request, struct _u_response * response, void * user_data) {
    FILE * file;
    char * final_name = (char*)malloc(1024);
    char * file_name = u_map_get(request->map_header, "file_name");
    char * username = u_map_get(request->map_header, "username");
    sprintf(final_name, "%s_%s", username, file_name);
    mongoc_collection_t *collection = mongoc_client_get_collection (client, "db_server", "files");

    if (create_file(collection, username, final_name)) {
        y_log_message(Y_LOG_LEVEL_CURRENT, "Error inserting the document into the database");
    }
    file=fopen(final_name,"a");
    fwrite(request->binary_body, request->binary_body_length, 1, file);
    fclose(file);
    ulfius_set_string_body_response(response, 200, "Done!");
    return U_CALLBACK_CONTINUE;
}

int callback_list_files (const struct _u_request * request, struct _u_response * response, void * user_data) {
    mongoc_collection_t *collection = mongoc_client_get_collection (client, "db_server", "files");
    //char * username = u_map_get(request->map_header, "username");
    json_t * responsebody = list_files(collection, "");//username);
    ulfius_set_json_body_response(response,  200, responsebody);
    return U_CALLBACK_CONTINUE;
}

int file_upload_callback (const struct _u_request * request,
                          const char * key,
                          const char * filename,
                          const char * content_type,
                          const char * transfer_encoding,
                          const char * data,
                          uint64_t off,
                          size_t size,
                          void * cls) {
    y_log_message(Y_LOG_LEVEL_DEBUG, "Got from file '%s' of the key '%s', offset %llu, size %zu, strlen(data) %u, cls is '%s'", filename, key, off, size,strlen(data), cls);
    FILE * file;
    char * file_name = u_map_get(request->map_post_body, "file_name");
    file=fopen(file_name,"w");
    fwrite(data, size, 1, file);
    fclose(file);
    return U_OK;
}

int callback_serve_file (const struct _u_request * request, struct _u_response * response, void * user_data) {
    char* fname = u_map_get(request->map_header, "file");
    y_log_message(Y_LOG_LEVEL_DEBUG, "Downloading %s", fname);

   unsigned char* buffer;
   char * final_name = (char*)malloc(1024);
   buffer = (char*) malloc(4096 * 1024);
   sprintf(final_name, "./%s", fname);
  FILE *ptr;
   ptr = fopen(final_name,"rb");
   size_t size = fread(buffer,sizeof(unsigned char), 4096 * 1024,ptr);
   fclose(ptr);
    y_log_message(Y_LOG_LEVEL_DEBUG, "Size %d", size);
    ulfius_set_binary_body_response(response, 200, buffer, size);

    return U_CALLBACK_CONTINUE;
}