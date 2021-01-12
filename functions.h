//
// Created by safa on 12/29/20.
//
#include <ulfius.h>

#ifndef SERVER_CRYPTOS_CLOUD_FUNCTIONS_H
#define SERVER_CRYPTOS_CLOUD_FUNCTIONS_H
int callback_register(const struct _u_request *request, struct _u_response * response , void * user_data);
int callback_login(const struct  _u_request *request, struct _u_response *response ,void *user_data);
int callback_upload_file (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_list_files (const struct _u_request * request, struct _u_response * response, void * user_data);
int callback_serve_file (const struct _u_request * request, struct _u_response * response, void * user_data);
int file_upload_callback (const struct _u_request * request,
                          const char * key,
                          const char * filename,
                          const char * content_type,
                          const char * transfer_encoding,
                          const char * data,
                          uint64_t off,
                          size_t size,
                          void * cls);
#endif //SERVER_CRYPTOS_CLOUD_FUNCTIONS_H
