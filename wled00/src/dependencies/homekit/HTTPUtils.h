#pragma once

#include "HKDebug.h"

#include <stdint.h>
#include <string.h>

static const char* header_204_fmt = 
    "HTTP/1.1 204 No Content\r\n"
    "\r\n";

static const char* header_207_fmt = 
    "HTTP/1.1 207 Multi-Status\r\n"
    "Content-Type: application/hap+json\r\n"
    "Content-Length: %d\r\n"
    "\r\n";

static const char * header_200_app_tlv8 = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: application/pairing+tlv8\r\n"
    "Content-Length: %d\r\n"
    "\r\n";

static const char* header_200_app_json = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: application/hap+json\r\n"
    "Content-Length: %d\r\n"
    "\r\n";

static const char * header_404_fmt =
    "HTTP/1.1 404 Not Found\r\n"
    "\r\n";

static const char * header_400_fmt =
    "HTTP/1.1 400 Bad Request\r\n"
    "\r\n";

static const char * header_470_fmt =
    "HTTP/1.1 470 Connection Authorization Required\r\n"
    "\r\n";

enum class HTTPMethod {
    UNKNOWN = 0,
    POST = 1,
    GET = 2,
    PUT = 3
};

enum class HKUri {
    UNKNOWN = 0,
    ACCESSORIES = 1,
    CHARACTERISTICS = 2,
    IDENTIFY = 3,
    PAIR_SETUP = 4,
    PAIR_VERIFY = 5,
    PAIRINGS = 6
    // PREPARE,
    // SECURE_MESSAGE,
    // RESOURCE
};

const char * HKUri_to_str(HKUri uri) {
    switch (uri)
    {
    case HKUri::UNKNOWN:
        return "Unknown";
    case HKUri::ACCESSORIES:
        return "/accessories";
    case HKUri::CHARACTERISTICS:
        return "/characteristics";
    case HKUri::IDENTIFY:
        return "/identify";
    case HKUri::PAIR_SETUP:
        return "/pair-setup";
    case HKUri::PAIR_VERIFY:
        return "/pair-verify";
    case HKUri::PAIRINGS:
        return "/pairings";
    default:
        return "Unknown";
    }

    return "Unknown";
}

const char * HTTPMethod_to_str(HTTPMethod method) {
    switch (method)
    {
    case HTTPMethod::GET:
        return "GET";
    case HTTPMethod::POST:
        return "POST";
    case HTTPMethod::PUT:
        return "PUT";
    case HTTPMethod::UNKNOWN:
        return "Unknown";
    }

    return "Unknown";
}

HKUri parse_path_request(const char * http_buf) {
    static const char * pair_setup_uri = "/pair-setup";
    static const char * pair_verify_uri = "/pair-verify";
    static const char * characteristics_uri = "/characteristics";
    static const char * accessories_uri = "/accessories";
    static const char * pairings_uri = "/pairings";
    static const char * identify_uri = "/identify";

    char * start_of_path = strchr(http_buf, ' ') + 1;
    if (!start_of_path) {
        return HKUri::UNKNOWN;
    }

    char * end_of_path = strchr(start_of_path, ' ');

    if (!end_of_path) {
        return HKUri::UNKNOWN;
    }

    uint8_t len = end_of_path - start_of_path;
    char path[len + 1];
    memcpy(path, start_of_path, len);
    path[len] = '\0';

    if (!strcmp(path, accessories_uri)) {
        return HKUri::ACCESSORIES;
    } else if (!strncmp(path, characteristics_uri, strlen(characteristics_uri))) {
        return HKUri::CHARACTERISTICS;
    } else if (!strcmp(path, identify_uri)) {
        return HKUri::IDENTIFY;
    } else if (!strcmp(path, pair_setup_uri)) {
        return HKUri::PAIR_SETUP;
    } else if (!strcmp(path, pair_verify_uri)) {
        return HKUri::PAIR_VERIFY;
    } else if (!strcmp(path, pairings_uri)) {
        return HKUri::PAIRINGS;
    }

    return HKUri::UNKNOWN;
};

HTTPMethod parse_method_request(const char * http_buf) {
    static const char * get_method = "GET";
    static const char * put_method = "PUT";
    static const char * post_method = "POST";

    const char * start_of_method = http_buf;
    if (!start_of_method) {
        return HTTPMethod::UNKNOWN;
    }

    char *end_of_method = strchr(start_of_method, ' ');

    if (!end_of_method) {
        return HTTPMethod::UNKNOWN;
    }

    size_t len = end_of_method - start_of_method;
    char method[len + 1];
    memcpy(method, start_of_method, len);
    method[len] = '\0';

    if (!strcmp(method, get_method)) {
        return HTTPMethod::GET;
    } else if (!strcmp(method, put_method)) {
        return HTTPMethod::PUT;
    } else if (!strcmp(method, post_method)) {
        return HTTPMethod::POST;
    }

    return HTTPMethod::UNKNOWN;
}

size_t parse_characteristics_query_count(const char * http_buf) {
    char * start_of_query = strchr(http_buf, '=') + 1;
    if (!start_of_query) {
        return -1;
    }
    char * end_of_query = strchr(start_of_query, ' ');
    if (!end_of_query) {
        return -1;
    }

    int len = end_of_query - start_of_query;
    int ids_count = 1;

    for (int i = 0; i < len; i++) {
        if (start_of_query[i] == ',') {
            ids_count++;
        }
    }

    return ids_count;
}

size_t parse_characteristics_query(int * out, int out_len, const char * http_buf) {
    char * start_of_query = strchr(http_buf, '=');
    if (!start_of_query) {
        return 1;
    } else {
        start_of_query += 1;
    }

    char * end_of_query = strchr(start_of_query, ' ');

    if (!end_of_query) {
        return 1;
    }

    int len = end_of_query - start_of_query;

    char ids[len + 1];
    memcpy(ids, start_of_query, len);
    ids[len] = '\0';

    char * id_str = ids;        // Start of pointer

    int ids_inx = 0;

    while(id_str) {
        char * id_start = strchr(id_str, '.') + 1;
        char * id_end = strchr(id_str, ',');
        if (!id_end) {
            id_end = id_start + strlen(id_start);
            id_str = NULL;
        } else {
            id_str = id_end + 1;
        }

        int id_len = id_end - id_start;
        char parse_int[id_len + 1];
        for (int i = 0; i < id_len; i++) {
            parse_int[i] = id_start[i];
        }

        if (ids_inx < out_len) {
            out[ids_inx] = atoi(parse_int);
        }

        ids_inx++;
    }

    return 0;
}

size_t parse_content_len(const char * http_buf, size_t http_buf_len) {
    size_t content_len = -1;

    const char * http_content_len_str = "Content-Length: ";
    char * http_content_len_ptr = strstr(http_buf, http_content_len_str);

    if (!http_content_len_ptr) {
        return content_len;
    }

    http_content_len_ptr += strlen(http_content_len_str);
    content_len = atoi(http_content_len_ptr);

    return content_len;
}

int parse_body_request(char * output_buf, const char * http_buf, size_t http_content_len) {
    char * http_body_content_ptr = strstr(http_buf, "\r\n\r\n");
    if (!http_body_content_ptr) {
        return 1;
    }

    http_body_content_ptr += sizeof(char) * 4;
    if (!http_body_content_ptr) {
        return 1;
    }

    memcpy(output_buf, http_body_content_ptr, http_content_len);
    output_buf[http_content_len] = '\0';
    return 0;
}