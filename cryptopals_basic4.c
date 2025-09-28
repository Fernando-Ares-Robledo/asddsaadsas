#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---------- Utilidades HEX ---------- */
int hex_char_to_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

unsigned char *hex_to_bytes(const char *hex, size_t *out_len) {
    size_t n = strlen(hex);
    if (n % 2 != 0) return NULL;              /* longitud debe ser par */
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

/* ---------- Scoring inglés (heurística) ---------- */
double score_english(const unsigned char *buf, size_t len) {
    const double letter_weight[26] = {
        /* a  b   c   d   e   f   g   h   i   j   k   l   m   n   o */
         0.65,0.12,0.22,0.34,1.00,0.20,0.16,0.49,0.56,0.02,0.06,0.33,0.20,0.57,0.60,
        /* p   q   r   s   t   u   v   w   x   y   z */
         0.20,0.01,0.48,0.51,0.71,0.33,0.10,0.19,0.01,0.16,0.01
    };

    double score = 0.0;
    size_t bad = 0;

    for (size_t i = 0; i < len; i++) {
        unsigned char c = buf[i];

        if (c == 0x00) { score -= 8.0; bad++; continue; }
        if (c == '\n' || c == '\r' || c == '\t') { score += 0.05; continue; }

        if (c >= 32 && c <= 126) {
            if (c == ' ') { score += 0.60; continue; }
            if (isalpha(c)) {
                int idx = tolower(c) - 'a';
                if (idx >= 0 && idx < 26) score += letter_weight[idx];
            } else if (isdigit(c)) {
                score += 0.08;
            } else if (strchr(".,'\"!?;:-()", c)) {
                score += 0.06;
            } else {
                score += 0.01;
            }
        } else {
            score -= 3.5;
            bad++;
        }
    }

    if (len > 0) {
        double bad_ratio = (double)bad / (double)len;
        if (bad_ratio > 0.05) score -= (bad_ratio * 20.0);
    }

    return score;
}

/* ---------- XOR con clave de 1 byte ---------- */
void xor_with_key(unsigned char *dst, const unsigned char *src, size_t len, unsigned char key) {
    for (size_t i = 0; i < len; i++) dst[i] = (unsigned char)(src[i] ^ key);
}

/* ---------- Helpers ---------- */
static void rstrip_newline(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[--n] = '\0';
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <file_with_hex_lines>\n", argv[0]);
        return 1;
    }

    const char *path = argv[1];
    FILE *f = fopen(path, "r");
    if (!f) {
        perror("No pude abrir el archivo");
        return 1;
    }

    char line[4096];
    size_t line_no = 0;

    double global_best_score = -1e9;
    unsigned char global_best_key = 0;
    char *global_best_plain = NULL;
    size_t global_best_line_no = 0;

    while (fgets(line, sizeof line, f)) {
        line_no++;
        rstrip_newline(line);

        /* saltar líneas vacías o con longitud rara */
        if (line[0] == '\0') continue;

        size_t bytes_len = 0;
        unsigned char *bytes = hex_to_bytes(line, &bytes_len);
        if (!bytes) continue;

        unsigned char *cand = malloc(bytes_len + 1);
        if (!cand) { free(bytes); fclose(f); return 1; }

        double best_score = -1e9;
        unsigned char best_key = 0;
        char *best_plain = NULL;

        for (int k = 0; k <= 255; k++) {
            xor_with_key(cand, bytes, bytes_len, (unsigned char)k);
            cand[bytes_len] = '\0';
            double sc = score_english(cand, bytes_len);
            if (sc > best_score) {
                best_score = sc;
                best_key = (unsigned char)k;
                free(best_plain);
                best_plain = malloc(bytes_len + 1);
                if (!best_plain) { free(cand); free(bytes); fclose(f); return 1; }
                memcpy(best_plain, cand, bytes_len + 1);
            }
        }

        if (best_score > global_best_score) {
            global_best_score = best_score;
            global_best_key = best_key;
            global_best_line_no = line_no;
            free(global_best_plain);
            global_best_plain = best_plain;   /* tomar propiedad */
            best_plain = NULL;                /* ya no liberar aquí */
        }

        free(best_plain);
        free(cand);
        free(bytes);
    }

    fclose(f);

    if (global_best_plain) {
        printf("Linea: %zu\n", global_best_line_no);
        printf("Key: 0x%02X ('%c')\n",
               global_best_key,
               (global_best_key >= 32 && global_best_key <= 126) ? global_best_key : '.');
        printf("Plaintext: %s\n", global_best_plain);
        printf("Score: %.3f\n", global_best_score);
        free(global_best_plain);
        return 0;
    } else {
        fprintf(stderr, "No se encontró candidato válido.\n");
        return 2;
    }
}

