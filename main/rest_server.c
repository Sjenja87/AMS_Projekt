/* HTTP Restful API Server

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <fcntl.h>
#include "esp_http_server.h"
#include "esp_chip_info.h"
#include "esp_random.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "cJSON.h"
#include "sd_storage.h"
#include "sdkconfig.h" 
#include "sensors.h"

static const char *REST_TAG = "esp-rest";


#define chunked_buffer_size 2048

#define REST_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                 \
    {                                                                                  \
        if (!(a))                                                                      \
        {                                                                              \
            ESP_LOGE(REST_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                             \
        }                                                                              \
    } while (0)

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (chunked_buffer_size)

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
    } else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
        type = "application/javascript";
    } else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
        type = "text/css";
    } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
        type = "image/png";
    } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
        type = "image/x-icon";
    } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
        type = "text/xml";
    }
    return httpd_resp_set_type(req, type);
}

/* Send HTTP response with the contents of the requested file */
static esp_err_t index_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];

    rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));

    if(CHECK_FILE_EXTENSION(req->uri, ".html"))
    {
        strlcat(filepath, "/INDEX~1.HTM", sizeof(filepath));
        set_content_type_from_file(req, ".html");

    }
    else if(CHECK_FILE_EXTENSION(req->uri, ".css"))
    {
        strlcat(filepath, "/INDEX.CSS", sizeof(filepath));
        set_content_type_from_file(req, ".css");    

    }
    else if(CHECK_FILE_EXTENSION(req->uri, ".js"))
    {
        strlcat(filepath, "/INDEX.JS", sizeof(filepath));
        set_content_type_from_file(req, ".js");    
   
    }
    
    ESP_LOGI(REST_TAG,"%s",filepath); 

    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }

    char *chunk = rest_context->scratch;
    ssize_t read_bytes;
    do {
        /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1) {
            ESP_LOGE(REST_TAG, "Failed to read file : %s", filepath);
        } else if (read_bytes > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(REST_TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGI(REST_TAG, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t data_get_handler(httpd_req_t *req)
{
    rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;
    char *chunk = rest_context->scratch;

    cJSON curr_data_cJSON = get_data_JSON();

    chunk = cJSON_Print(&curr_data_cJSON);
   
    httpd_resp_sendstr(req, chunk);
    return ESP_OK;
}


esp_err_t start_rest_server(const char *base_path)
{
    REST_CHECK(base_path, "wrong base path", err);
    rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
    REST_CHECK(rest_context, "No memory for rest context", err);
    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed", err_start);

    /* URI handler for sensordata control */
    httpd_uri_t data_get_uri = {
        .uri = "/api/data",
        .method = HTTP_GET,
        .handler = data_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &data_get_uri);

     httpd_uri_t index_html = {
        .uri = "/index.html",
        .method = HTTP_GET,
        .handler = index_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &index_html);

     httpd_uri_t index_css = {
        .uri = "/index.css",
        .method = HTTP_GET,
        .handler = index_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &index_css);

         httpd_uri_t index_js = {
        .uri = "/index.js",
        .method = HTTP_GET,
        .handler = index_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &index_js);

    return ESP_OK;
err_start:
    free(rest_context);
err:
    return ESP_FAIL;
}
