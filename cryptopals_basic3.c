#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* --- HEX utils --- */
int hex_char_to_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

unsigned char *hex_to_bytes(const char *hex, size_t *out_len) {
    size_t n = strlen(hex);
    if (n % 2 != 0) return NULL;
    *out_len = n / 2;

    unsigned char *out = malloc(*out_len);
    if (!out) return NULL;

    for (size_t i = 0; i < *out_len; i++) {
        int hi = hex_char_to_val(hex[2*i]);
        int lo = hex_char_to_val(hex[2*i + 1]);
        if (hi < 0 || lo < 0) { free(out); return NULL; }
        out[i] = (unsigned char)((hi << 4) | lo);
    }
    return out;
}

/* --- Scoring: heurística simple de "parece inglés" --- */
double score_english(const unsigned char *buf, size_t len) {
    /* Pesos aproximados por frecuencia (más alto = más común) */
    const double letter_weight[26] = {
        /* a  b   c   d   e   f   g   h   i   j   k   l   m   n   o */
         0.65,0.12,0.22,0.34,1.00,0.20,0.16,0.49,0.56,0.02,0.06,0.33,0.20,0.57,0.60,
        /* p   q   r   s   t   u   v   w   x   y   z */
         0.20,0.01,0.48,0.51,0.71,0.33,0.10,0.19,0.01,0.16,0.01
    };

    double score = 0.0;
    size_t printable = 0, bad = 0;

    for (size_t i = 0; i < len; i++) {
        unsigned char c = buf[i];

        if (c == 0x00) { score -= 8.0; bad++; continue; }
        if (c == '\n' || c == '\r' || c == '\t') { score += 0.05; printable++; continue; }

        if (c >= 32 && c <= 126) {
            printable++;
            if (c == ' ') { score += 0.6; continue; }
            if (isalpha(c)) {
                int idx = tolower(c) - 'a';
                if (idx >= 0 && idx < 26) score += letter_weight[idx];
            } else if (isdigit(c)) {
                score += 0.08;
            } else if (strchr(".,'\"!?;:-()", c)) {
                score += 0.06;
            } else {
                score += 0.01; /* otros imprimibles, poco peso */
            }
        } else {
            /* No imprimible → penaliza fuerte */
            score -= 3.5;
            bad++;
        }
    }

    /* Penaliza si hay demasiados no-imprimibles */
    if (len > 0) {
        double bad_ratio = (double)bad / (double)len;
        if (bad_ratio > 0.05) score -= (bad_ratio * 20.0);
    }

    return score;
}

/* XOR con una clave de un solo byte */
void xor_with_key(unsigned char *dst, const unsigned char *src, size_t len, unsigned char key) {
    for (size_t i = 0; i < len; i++) dst[i] = (unsigned char)(src[i] ^ key);
}

int main(void) {
    const char *hex_in =
        "1b37373331363f78151b7f2b783431333d78397828372d363c78"
        "373e783a393b3736";

    size_t in_len = 0;
    unsigned char *cipher = hex_to_bytes(hex_in, &in_len);
    if (!cipher) {
        fprintf(stderr, "Hex inválido o sin memoria.\n");
        return 1;
    }

    unsigned char *candidate = malloc(in_len + 1);
    if (!candidate) { free(cipher); return 1; }

    double best_score = -1e9;
    unsigned char best_key = 0;
    char *best_text = NULL;

    for (int k = 0; k <= 255; k++) {
        xor_with_key(candidate, cipher, in_len, (unsigned char)k);
        candidate[in_len] = '\0'; /* para imprimir como string */

        double sc = score_english(candidate, in_len);
        if (sc > best_score) {
            best_score = sc;
            best_key = (unsigned char)k;
            /* guarda el mejor texto */
            free(best_text);
            best_text = malloc(in_len + 1);
            if (!best_text) { free(candidate); free(cipher); return 1; }
            memcpy(best_text, candidate, in_len + 1);
        }
    }

    printf("Best key: 0x%02X ('%c')\n", best_key,
           (best_key >= 32 && best_key <= 126) ? best_key : '.');
    printf("Plaintext: %s\n", best_text);
    printf("Score: %.3f\n", best_score);

    free(best_text);
    free(candidate);
    free(cipher);
    return 0;
}

