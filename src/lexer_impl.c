#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

/*
 * Hand-written lexer reference implementation.
 * This is useful in environments where Flex is unavailable.
 */

typedef struct {
    const char *src;
    size_t pos;
    int line;
} Lexer;

static int is_ident_start(char c) {
    return isalpha((unsigned char)c) || c == '_';
}

static int is_ident_char(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

static int keyword_token(const char *id) {
    if (strcmp(id, "program") == 0) return PROGRAM;
    if (strcmp(id, "when") == 0) return WHEN;
    if (strcmp(id, "otherwise") == 0) return OTHERWISE;
    if (strcmp(id, "repeat") == 0) return REPEAT;
    if (strcmp(id, "times") == 0) return TIMES;
    if (strcmp(id, "while") == 0) return WHILE;
    if (strcmp(id, "define") == 0) return DEFINE;
    if (strcmp(id, "give") == 0) return GIVE;
    if (strcmp(id, "say") == 0) return SAY;
    if (strcmp(id, "set") == 0) return SET;
    if (strcmp(id, "to") == 0) return TO;
    if (strcmp(id, "num") == 0) return NUM;
    if (strcmp(id, "text") == 0) return TEXT;
    if (strcmp(id, "decimal") == 0) return DECIMAL;
    if (strcmp(id, "call") == 0) return CALL;
    if (strcmp(id, "and") == 0) return AND;
    if (strcmp(id, "or") == 0) return OR;
    if (strcmp(id, "not") == 0) return NOT;
    if (strcmp(id, "is") == 0) return IS;
    if (strcmp(id, "greater") == 0) return GREATER;
    if (strcmp(id, "less") == 0) return LESS;
    if (strcmp(id, "than") == 0) return THAN;
    if (strcmp(id, "equals") == 0) return EQUALS;
    return IDENTIFIER;
}

void lexer_init(Lexer *lx, const char *src) {
    lx->src = src;
    lx->pos = 0;
    lx->line = 1;
}

static int starts_with(const char *s, const char *kw) {
    return strncmp(s, kw, strlen(kw)) == 0;
}

int lexer_next_token(Lexer *lx, YYSTYPE *yylval_out) {
    const char *s = lx->src;

    while (s[lx->pos]) {
        char c = s[lx->pos];

        if (c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v') {
            lx->pos++;
            continue;
        }
        if (c == '#' || (c == '/' && s[lx->pos + 1] == '/')) {
            while (s[lx->pos] && s[lx->pos] != '\n') lx->pos++;
            continue;
        }
        if (c == '\n' || c == ';') {
            if (c == '\n') lx->line++;
            lx->pos++;
            return NEWLINE;
        }

        if (starts_with(s + lx->pos, "end when") && !is_ident_char(s[lx->pos + 8])) {
            lx->pos += 8;
            return END_WHEN;
        }
        if (starts_with(s + lx->pos, "end repeat") && !is_ident_char(s[lx->pos + 10])) {
            lx->pos += 10;
            return END_REPEAT;
        }
        if (starts_with(s + lx->pos, "end while") && !is_ident_char(s[lx->pos + 9])) {
            lx->pos += 9;
            return END_WHILE;
        }
        if (starts_with(s + lx->pos, "end define") && !is_ident_char(s[lx->pos + 10])) {
            lx->pos += 10;
            return END_DEFINE;
        }
        if (starts_with(s + lx->pos, "end program") && !is_ident_char(s[lx->pos + 11])) {
            lx->pos += 11;
            return END_PROGRAM;
        }

        if (is_ident_start(c)) {
            size_t start = lx->pos;
            while (is_ident_char(s[lx->pos])) lx->pos++;
            {
                size_t n = lx->pos - start;
                char *id = (char *)malloc(n + 1);
                int tok;
                memcpy(id, s + start, n);
                id[n] = '\0';
                tok = keyword_token(id);
                if (tok == IDENTIFIER) {
                    yylval_out->sval = id;
                } else {
                    free(id);
                }
                return tok;
            }
        }

        if (isdigit((unsigned char)c)) {
            size_t start = lx->pos;
            int has_dot = 0;
            while (isdigit((unsigned char)s[lx->pos])) lx->pos++;
            if (s[lx->pos] == '.' && isdigit((unsigned char)s[lx->pos + 1])) {
                has_dot = 1;
                lx->pos++;
                while (isdigit((unsigned char)s[lx->pos])) lx->pos++;
            }
            {
                size_t n = lx->pos - start;
                char *num = (char *)malloc(n + 1);
                memcpy(num, s + start, n);
                num[n] = '\0';
                if (has_dot) {
                    yylval_out->fval = atof(num);
                    free(num);
                    return FLOAT_LITERAL;
                }
                yylval_out->ival = atoi(num);
                free(num);
                return INT_LITERAL;
            }
        }

        if (c == '"') {
            size_t start = lx->pos;
            lx->pos++;
            while (s[lx->pos] && s[lx->pos] != '"') {
                if (s[lx->pos] == '\\' && s[lx->pos + 1]) lx->pos += 2;
                else lx->pos++;
            }
            if (!s[lx->pos]) {
                fprintf(stderr, "[Lexer Error] Line %d: Unterminated string literal\n", lx->line);
                return 0;
            }
            lx->pos++;
            {
                size_t n = lx->pos - start;
                char *str = (char *)malloc(n + 1);
                memcpy(str, s + start, n);
                str[n] = '\0';
                yylval_out->sval = str;
                return STRING_LITERAL;
            }
        }

        lx->pos++;
        if (c == '+') return PLUS;
        if (c == '-') return MINUS;
        if (c == '*') return STAR;
        if (c == '/') return SLASH;
        if (c == '(') return LPAREN;
        if (c == ')') return RPAREN;
        if (c == ',') return COMMA;

        fprintf(stderr, "[Lexer Error] Line %d: Unknown token '%c'\n", lx->line, c);
        return 0;
    }

    return 0;
}
