#include "email_sender.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int send_email_curl_cmd(const email_config_t *config, const char *subject, const char *attachment_b64_path) {
    if (!config || !subject || !attachment_b64_path) return -1;
    if (!config->smtp_server || !config->to || !config->from) return -1;

    const char *temp_mail_file = "temp_mail_body.txt";
    FILE *f = fopen(temp_mail_file, "w");
    if (!f) return -1;

    int write_ok = (fprintf(f, "From: %s\nTo: %s\nSubject: %s\n\n", config->from, config->to, subject) >= 0);
    
    FILE *f_att = fopen(attachment_b64_path, "r");
    if (f_att) {
        char buffer[512];
        while (fgets(buffer, sizeof(buffer), f_att)) {
            fputs(buffer, f);
        }
        fclose(f_att);
    } else {
        write_ok = 0;
    }

    if (fclose(f) != 0) write_ok = 0;
    if (!write_ok) {
        remove(temp_mail_file);
        return -1;
    }

    char command[1024];
    int cmd_len = 0;

    if (config->username && config->password) {
        cmd_len = snprintf(command, sizeof(command),
            "curl --url \"smtp://%s:%s\" --mail-from \"%s\" --mail-rcpt \"%s\" --user \"%s:%s\" -T %s --ssl-reqd",
            config->smtp_server,
            config->smtp_port ? config->smtp_port : "587",
            config->from,
            config->to,
            config->username,
            config->password,
            temp_mail_file);
    } else {
        cmd_len = snprintf(command, sizeof(command),
            "curl --url \"smtp://%s:%s\" --mail-from \"%s\" --mail-rcpt \"%s\" -T %s",
            config->smtp_server,
            config->smtp_port ? config->smtp_port : "25",
            config->from,
            config->to,
            temp_mail_file);
    }

    if (cmd_len < 0 || cmd_len >= (int)sizeof(command)) {
        remove(temp_mail_file);
        return -1;
    }

    int ret = system(command);
    remove(temp_mail_file);

    return (ret == 0) ? 0 : -1;
}