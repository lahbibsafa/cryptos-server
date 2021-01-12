//
// Created by safa on 12/29/20.
//
#include <mongoc.h>
#include<jansson.h>
#ifndef SERVER_CRYPTOS_CLOUD_DATABASE_H
#define SERVER_CRYPTOS_CLOUD_DATABASE_H
extern mongoc_client_t *client;
extern mongoc_uri_t *uri;
int database_init();
int database_destory();
int check_username_exist(mongoc_collection_t *collection, char *username);
int create_user(mongoc_collection_t *collection, char *username, char *password);
int create_file(mongoc_collection_t *collection, char *username, char *file);
json_t * list_files(mongoc_collection_t *collection, char *username);
json_t * find_user(mongoc_collection_t *collection, char *username, char *password);
#endif //SERVER_CRYPTOS_CLOUD_DATABASE_H
