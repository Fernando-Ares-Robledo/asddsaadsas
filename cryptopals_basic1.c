#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char base64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// convierte un hex char a su valor (0–15)
int hex_char_to_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1; // error
}

// convierte string hex -> bytes
unsigned char *hex_to_bytes(const char *hex, size_t *out_len) {
    size_t len = strlen(hex);
    if (len % 2 != 0) return NULL; // debe ser par
    *out_len = len / 2;

    unsigned char *out = malloc(*out_len);
    for (size_t i = 0; i < *out_len; i++) {
        int high = hex_char_to_val(hex[2 * i]);
        int low  = hex_char_to_val(hex[2 * i + 1]);
        out[i] = (high << 4) | low;
    }
    return out;
}

// convierte bytes -> base64 string
char *bytes_to_base64(const unsigned char *data, size_t len) {
    size_t out_len = 4 * ((len + 2) / 3);
    char *out = malloc(out_len + 1);
    size_t i, j;
    for (i = 0, j = 0; i < len;) {
        unsigned int octet_a = i < len ? data[i++] : 0;
        unsigned int octet_b = i < len ? data[i++] : 0;
        unsigned int octet_c = i < len ? data[i++] : 0;

        unsigned int triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        out[j++] = base64_table[(triple >> 18) & 0x3F];
        out[j++] = base64_table[(triple >> 12) & 0x3F];
        out[j++] = (i > len + 1) ? '=' : base64_table[(triple >> 6) & 0x3F];
        out[j++] = (i > len)     ? '=' : base64_table[triple & 0x3F];
    }
    out[j] = '\0';
    return out;
}

int main() {
    const char *hex_string =
        "49276d206b696c6c696e6720796f757220627261696e206c696b6520"
        "6120706f69736f6e6f7573206d757368726f6f6d";

    size_t byte_len;
    unsigned char *bytes = hex_to_bytes(hex_string, &byte_len);
    if (!bytes) {
        fprintf(stderr, "Error: hex inválido\n");
        return 1;
    }

    char *b64 = bytes_to_base64(bytes, byte_len);
    printf("%s\n", b64);

    free(bytes);
    free(b64);
    return 0;
}

