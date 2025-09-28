#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* --- utilidades HEX --- */
static int hex_char_to_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1; /* inválido */
}

/* hex -> bytes; devuelve buffer malloc'd y pone out_len */
static unsigned char *hex_to_bytes(const char *hex, size_t *out_len) {
    size_t n = strlen(hex);
    if (n % 2 != 0) return NULL; /* longitud debe ser par */

    size_t len = n / 2;
    unsigned char *out = (unsigned char *)malloc(len);
    if (!out) return NULL;

    for (size_t i = 0; i < len; i++) {
        int hi = hex_char_to_val(hex[2*i]);
        int lo = hex_char_to_val(hex[2*i + 1]);
        if (hi < 0 || lo < 0) { free(out); return NULL; }
        out[i] = (unsigned char)((hi << 4) | lo);
    }
    *out_len = len;
    return out;
}

/* bytes -> hex en minúsculas; devuelve buffer malloc'd con '\0' */
static char *bytes_to_hex(const unsigned char *buf, size_t len) {
    static const char *digits = "0123456789abcdef";
    char *out = (char *)malloc(len * 2 + 1);
    if (!out) return NULL;

    for (size_t i = 0; i < len; i++) {
        out[2*i]     = digits[(buf[i] >> 4) & 0xF];
        out[2*i + 1] = digits[buf[i] & 0xF];
    }
    out[len*2] = '\0';
    return out;
}

/* --- XOR fijo: a y b deben tener la misma longitud --- */
static void xor_buffers(unsigned char *dst,
                        const unsigned char *a,
                        const unsigned char *b,
                        size_t len)
{
    for (size_t i = 0; i < len; i++) {
        dst[i] = (unsigned char)(a[i] ^ b[i]);
    }
}

int main(void) {
    const char *hex_a = "1c0111001f010100061a024b53535009181c";
    const char *hex_b = "686974207468652062756c6c277320657965";
    const char *expected_hex = "746865206b696420646f6e277420706c6179";

    size_t len_a = 0, len_b = 0;
    unsigned char *a = hex_to_bytes(hex_a, &len_a);
    unsigned char *b = hex_to_bytes(hex_b, &len_b);

    if (!a || !b) {
        fprintf(stderr, "Error: entrada hex inválida o memoria insuficiente.\n");
        free(a); free(b);
        return 1;
    }
    if (len_a != len_b) {
        fprintf(stderr, "Error: los buffers deben tener la misma longitud.\n");
        free(a); free(b);
        return 1;
    }

    unsigned char *out = (unsigned char *)malloc(len_a);
    if (!out) {
        fprintf(stderr, "Error: memoria insuficiente.\n");
        free(a); free(b);
        return 1;
    }

    xor_buffers(out, a, b, len_a);

    char *out_hex = bytes_to_hex(out, len_a);
    if (!out_hex) {
        fprintf(stderr, "Error: memoria insuficiente.\n");
        free(a); free(b); free(out);
        return 1;
    }

    /* Mostrar resultado y comprobar contra el esperado */
    printf("%s\n", out_hex);

    if (strcmp(out_hex, expected_hex) == 0) {
        printf("OK: coincide con la salida esperada.\n");
    } else {
        printf("FALLO: no coincide con la salida esperada.\n");
        printf("Esperado: %s\n", expected_hex);
    }

    /* Limpieza */
    free(a); free(b); free(out); free(out_hex);
    return 0;
}

