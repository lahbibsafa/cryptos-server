//
// Created by safa on 12/29/20.
//
#include <mongoc.h>
#include <yder.h>
#include "database.h"
#include<jansson.h>
int database_init(){
    const char *uri_string = "mongodb://localhost:27017";
    bson_error_t error;
    mongoc_init ();
    uri = mongoc_uri_new_with_error (uri_string, &error);
    y_log_message(Y_LOG_LEVEL_DEBUG, "intiating connection to mongodb");

    if (!uri) {
        fprintf (stderr,
                 "failed to parse URI: %s\n"
                 "error message:       %s\n",
                 uri_string,
                 error.message);
        return EXIT_FAILURE;
    }
    /*
       * Create a new client instance
       */
    client = mongoc_client_new_from_uri (uri);
    y_log_message(Y_LOG_LEVEL_DEBUG, "mongo client created");

    if (!client) {
        return EXIT_FAILURE;
    }
}
int database_destory(){
    mongoc_uri_destroy (uri);
    mongoc_client_destroy (client);
    mongoc_cleanup ();
}
int check_username_exist(mongoc_collection_t *collection, char *username)
{
    //Objectif : search in the messages collection if there is any record with the same value as the user input.
    // Step 1: construct query (create the query {"message": buffer})
    bson_t *query;
    bson_t *doc;
    query = bson_new();
    bson_append_utf8(query, "username", strlen("username"), username, strlen(username)); // query = {"message" : buffer}
    // Step 2: execute Query
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
    // Step 3: check if there is any record in the query
    while(mongoc_cursor_next (cursor, &doc)){
        return 1;
    }
    // Step 4: return result
    bson_destroy (query);
    mongoc_cursor_destroy (cursor);
    return 0;
}

int create_user(mongoc_collection_t *collection, char *username, char *password) {
    bson_error_t error;
    bson_t *document = bson_new();
    bson_append_utf8(document, "username", strlen("username"), username, strlen(username));
    bson_append_utf8(document, "password", strlen("password"), password, strlen(password));
    if (!mongoc_collection_insert_one (collection, document, NULL, NULL, &error)) {
        y_log_message(Y_LOG_LEVEL_ERROR, "error while inserting user: %s", error.message);
        return 0;
    }
    free(document);
    return 1;
}
int create_file(mongoc_collection_t *collection, char *username, char *filePath) {
    bson_error_t error;
    bson_t *document = bson_new();
    bson_append_utf8(document, "username", strlen("username"), username, strlen(username));
    bson_append_utf8(document, "filePath", strlen("filePath"), filePath, strlen(filePath));
    if (!mongoc_collection_insert_one (collection, document, NULL, NULL, &error)) {
        y_log_message(Y_LOG_LEVEL_ERROR, "error while inserting file: %s", error.message);
        return 0;
    }
    free(document);
    return 1;
}
/***
 * function that return a user json object if the user was found otherwise it return a response (json format {success: -1, message: 'user  not found'})
 * @param collection
 * @param username
 * @param password
 * @return
 */
json_t* find_user(mongoc_collection_t *collection, char *username , char *password ) {
    json_t * response; // our response object
    bson_error_t error;
    bson_t *query = bson_new(); // var query = {};
    bson_t *user;
    char *user_string;
    size_t len;
    json_error_t errorJSon;
    bson_append_utf8(query, "username", strlen("username"), username, strlen(username)); // query["username"] = username;
    bson_append_utf8(query, "password", strlen("password"), password, strlen(password));
    // Step 2: execute Query
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
    // Step 3: check if there is any record in the query
    while(mongoc_cursor_next (cursor, &user)){
        //user exist
        user_string = bson_as_relaxed_extended_json(user, &len);
        response = json_loads(user_string, 0, &errorJSon);
        return response;
    }
    //user does not exist
    response = json_object();
    json_object_set(response, "success", json_integer(-1));
    json_object_set(response, "message", json_string("There is not user using the specified username/password"));
    return response;
}
json_t * list_files(mongoc_collection_t *collection, char *username){
    json_t * response; // our response object
    bson_error_t error;
    bson_t *query = bson_new(); // var query = {};
    bson_t *doc;
    char *user_string;
    size_t len;
    json_error_t errorJSon;
   // bson_append_utf8(query, "username", strlen("username"), username, strlen(username)); // query["username"] = username;
    // Step 2: execute Query
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);
    response = json_array();
    // Step 3: check if there is any record in the query
    while(mongoc_cursor_next (cursor, &doc)){
        //user exist
       json_array_append(response, json_loads(bson_as_relaxed_extended_json(doc, &len), 0, &errorJSon));
    }
    return response;
}
