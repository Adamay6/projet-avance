#ifndef EMAIL_SENDER_H
#define EMAIL_SENDER_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    const char *smtp_server;
    const char *smtp_port;
    const char *username;
    const char *password;
    const char *from;
    const char *to;
} email_config_t;

int send_email_curl_cmd(const email_config_t *config, const char *subject, const char *attachment_b64_path);

#endif
