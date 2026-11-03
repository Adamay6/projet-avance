#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"


static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'}; // 64 caractères
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

void build_decoding_table(void) {

    if (decoding_table != NULL) return;
    decoding_table = malloc(256);
    if (decoding_table == NULL) return;
    memset(decoding_table, -1, 256);

    for (int i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
}


void base64_cleanup(void) {
    free(decoding_table);
    decoding_table = NULL;
}

char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length) {

    if (output_length == NULL) return NULL;
    *output_length = 0;
    if (data == NULL && input_length != 0) return NULL;
    if (input_length > SIZE_MAX - 2 || (input_length + 2) / 3 > (SIZE_MAX - 1) / 4)
        return NULL;
    *output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = malloc(*output_length+1);
    if (encoded_data == NULL) { *output_length = 0; return NULL; }
//printf("out lth=%d\n",*output_length);
    for (size_t i = 0, j = 0; i < input_length;) {

        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
        //printf(">> i=%d j=%d\n",i,j);
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    encoded_data[*output_length] = '\0';

    return encoded_data;
}


unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *output_length) {

    if (output_length == NULL) return NULL;
    *output_length = 0;
    if (data == NULL && input_length != 0) return NULL;

    if (input_length % 4 != 0) return NULL;
    if (input_length == 0) return calloc(1, 1);

    if (decoding_table == NULL) build_decoding_table();
    if (decoding_table == NULL) return NULL;

    size_t padding = 0;
    if (data[input_length - 1] == '=') padding++;
    if (data[input_length - 2] == '=') padding++;
    for (size_t i = 0; i < input_length - padding; i++) {
        if ((unsigned char)decoding_table[(unsigned char)data[i]] == 255)
            return NULL;
    }
    for (size_t i = input_length - padding; i < input_length; i++) {
        if (data[i] != '=') return NULL;
    }
    if (padding == 2 && (decoding_table[(unsigned char)data[input_length - 3]] & 15))
        return NULL;
    if (padding == 1 && (decoding_table[(unsigned char)data[input_length - 2]] & 3))
        return NULL;

    size_t decoded_length = input_length / 4 * 3 - padding;

    unsigned char *decoded_data = malloc(decoded_length ? decoded_length : 1);
    if (decoded_data == NULL) return NULL;
    *output_length = decoded_length;

    for (size_t i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : (uint32_t)decoding_table[(unsigned char)data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : (uint32_t)decoding_table[(unsigned char)data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : (uint32_t)decoding_table[(unsigned char)data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : (uint32_t)decoding_table[(unsigned char)data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
        + (sextet_b << 2 * 6)
        + (sextet_c << 1 * 6)
        + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}
