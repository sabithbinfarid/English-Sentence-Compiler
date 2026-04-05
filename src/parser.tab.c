
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "src/parser.y"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    TY_UNKNOWN = 0,
    TY_INT,
    TY_FLOAT,
    TY_TEXT
};

enum {
    OP_ADD = 1,
    OP_SUB,
    OP_MUL,
    OP_DIV
};

enum {
    REL_GT = 1,
    REL_LT,
    REL_EQ
};

typedef struct {
    int type;
    char *code;
    int is_const_numeric;
    double const_num;
} Expr;

typedef struct {
    Expr **items;
    int count;
    int cap;
} ArgList;

typedef struct {
    char *name;
    int type;
    char *scope;
} Symbol;

typedef struct {
    char *name;
    int return_type;
    int *param_types;
    char **param_names;
    int param_count;
} Function;

typedef struct {
    char *buf;
    size_t len;
    size_t cap;
} StrBuf;

static Symbol *symbols = NULL;
static int sym_count = 0;
static int sym_cap = 0;

static Function *functions = NULL;
static int fn_count = 0;
static int fn_cap = 0;

static char current_scope[256] = "global";
static int repeat_counter = 0;

static StrBuf fn_code;
static StrBuf main_code;
static StrBuf temp_fn_body;
static StrBuf *active_buf = NULL;
static int indent_level = 1;
static int fn_body_open = 0;

static Function *current_fn = NULL;
static char *pending_fn_name = NULL;
static int *pending_param_types = NULL;
static char **pending_param_names = NULL;
static int pending_param_count = 0;
static int pending_param_cap = 0;

static int show_ir_flag = 0;
static int run_flag = 0;
static char input_path[1024];
static char output_path[1024];
static char output_text_path[1024];

#define MAX_REPEAT_DEPTH 128
static char repeat_idx_stack[MAX_REPEAT_DEPTH][32];
static int repeat_depth = 0;

extern int yylex(void);
extern int line_num;
extern FILE *yyin;
extern char *yytext;

void yyerror(const char *msg);

#define EXPR(v) ((Expr *)(v))
#define ARGS(v) ((ArgList *)(v))
#define PTR(v) ((void *)(v))

static void *xmalloc(size_t n) {
    void *p = malloc(n);
    if (!p) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return p;
}

static char *xstrdup(const char *s) {
    size_t n = s ? strlen(s) : 0;
    char *p = (char *)xmalloc(n + 1);
    if (n > 0) {
        memcpy(p, s, n);
    }
    p[n] = '\0';
    return p;
}

static char *xprintf(const char *fmt, ...) {
    va_list ap;
    va_list ap2;
    int need;
    char *buf;

    va_start(ap, fmt);
    va_copy(ap2, ap);
    need = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    if (need < 0) {
        va_end(ap);
        fprintf(stderr, "Formatting failure\n");
        exit(1);
    }

    buf = (char *)xmalloc((size_t)need + 1);
    vsnprintf(buf, (size_t)need + 1, fmt, ap);
    va_end(ap);
    return buf;
}

static void semantic_error(const char *fmt, ...) {
    va_list ap;
    fprintf(stderr, "[Syntax Error] Line %d: ", line_num);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(1);
}

static void sb_init(StrBuf *sb) {
    sb->cap = 1024;
    sb->len = 0;
    sb->buf = (char *)xmalloc(sb->cap);
    sb->buf[0] = '\0';
}

static void sb_reserve(StrBuf *sb, size_t extra) {
    size_t need = sb->len + extra + 1;
    size_t ncap = sb->cap;
    char *nbuf;

    if (need <= sb->cap) {
        return;
    }

    while (ncap < need) {
        ncap *= 2;
    }

    nbuf = (char *)realloc(sb->buf, ncap);
    if (!nbuf) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    sb->buf = nbuf;
    sb->cap = ncap;
}

static void sb_append(StrBuf *sb, const char *s) {
    size_t n = strlen(s);
    sb_reserve(sb, n);
    memcpy(sb->buf + sb->len, s, n + 1);
    sb->len += n;
}

static void sb_appendf(StrBuf *sb, const char *fmt, ...) {
    va_list ap;
    va_list ap2;
    int need;

    va_start(ap, fmt);
    va_copy(ap2, ap);
    need = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    if (need < 0) {
        va_end(ap);
        fprintf(stderr, "Formatting failure\n");
        exit(1);
    }

    sb_reserve(sb, (size_t)need);
    vsnprintf(sb->buf + sb->len, sb->cap - sb->len, fmt, ap);
    va_end(ap);
    sb->len += (size_t)need;
}

static void sb_free(StrBuf *sb) {
    if (sb->buf) {
        free(sb->buf);
    }
    sb->buf = NULL;
    sb->len = 0;
    sb->cap = 0;
}

static void emit_line(const char *fmt, ...) {
    int i;
    va_list ap;
    va_list ap2;
    int need;

    if (!active_buf) {
        fprintf(stderr, "Internal error: emit target not set\n");
        exit(1);
    }

    for (i = 0; i < indent_level; ++i) {
        sb_append(active_buf, "    ");
    }

    va_start(ap, fmt);
    va_copy(ap2, ap);
    need = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    if (need < 0) {
        va_end(ap);
        fprintf(stderr, "Formatting failure\n");
        exit(1);
    }

    sb_reserve(active_buf, (size_t)need + 1);
    vsnprintf(active_buf->buf + active_buf->len, active_buf->cap - active_buf->len, fmt, ap);
    va_end(ap);
    active_buf->len += (size_t)need;

    sb_append(active_buf, "\n");
}

static const char *type_name(int t) {
    switch (t) {
        case TY_INT:
            return "TY_INT";
        case TY_FLOAT:
            return "TY_FLOAT";
        case TY_TEXT:
            return "TY_TEXT";
        default:
            return "TY_UNKNOWN";
    }
}

static const char *ctype_name(int t) {
    switch (t) {
        case TY_INT:
            return "int";
        case TY_FLOAT:
            return "double";
        case TY_TEXT:
            return "char*";
        default:
            return "int";
    }
}

static int coerce_numeric(int a, int b) {
    if ((a != TY_INT && a != TY_FLOAT) || (b != TY_INT && b != TY_FLOAT)) {
        return TY_UNKNOWN;
    }
    if (a == TY_FLOAT || b == TY_FLOAT) {
        return TY_FLOAT;
    }
    return TY_INT;
}

static Symbol *sym_lookup_local(const char *name, const char *scope) {
    int i;
    for (i = sym_count - 1; i >= 0; --i) {
        if (strcmp(symbols[i].name, name) == 0 && strcmp(symbols[i].scope, scope) == 0) {
            return &symbols[i];
        }
    }
    return NULL;
}

static Symbol *sym_lookup_visible(const char *name, const char *scope) {
    Symbol *s = sym_lookup_local(name, scope);
    int i;
    if (s) {
        return s;
    }
    for (i = sym_count - 1; i >= 0; --i) {
        if (strcmp(symbols[i].name, name) == 0 && strcmp(symbols[i].scope, "global") == 0) {
            return &symbols[i];
        }
    }
    return NULL;
}

static Symbol *sym_add(const char *name, int type, const char *scope) {
    Symbol *nitems;
    Symbol *s;

    if (sym_count == sym_cap) {
        sym_cap = sym_cap ? sym_cap * 2 : 128;
        nitems = (Symbol *)realloc(symbols, (size_t)sym_cap * sizeof(Symbol));
        if (!nitems) {
            fprintf(stderr, "Out of memory\n");
            exit(1);
        }
        symbols = nitems;
    }

    s = &symbols[sym_count++];
    s->name = xstrdup(name);
    s->type = type;
    s->scope = xstrdup(scope);
    return s;
}

static Function *fn_lookup(const char *name) {
    int i;
    for (i = 0; i < fn_count; ++i) {
        if (strcmp(functions[i].name, name) == 0) {
            return &functions[i];
        }
    }
    return NULL;
}

static Function *fn_add(const char *name) {
    Function *nitems;
    Function *f;

    if (fn_count == fn_cap) {
        fn_cap = fn_cap ? fn_cap * 2 : 32;
        nitems = (Function *)realloc(functions, (size_t)fn_cap * sizeof(Function));
        if (!nitems) {
            fprintf(stderr, "Out of memory\n");
            exit(1);
        }
        functions = nitems;
    }

    f = &functions[fn_count++];
    f->name = xstrdup(name);
    f->return_type = TY_UNKNOWN;
    f->param_types = NULL;
    f->param_names = NULL;
    f->param_count = 0;
    return f;
}

static Expr *expr_new(int type, const char *code, int is_const, double const_num) {
    Expr *e = (Expr *)xmalloc(sizeof(Expr));
    e->type = type;
    e->code = xstrdup(code);
    e->is_const_numeric = is_const;
    e->const_num = const_num;
    return e;
}

static char *cast_code(int from, int to, const char *code) {
    if (from == to || from == TY_UNKNOWN || to == TY_UNKNOWN) {
        return xstrdup(code);
    }
    if (from == TY_INT && to == TY_FLOAT) {
        return xprintf("(double)(%s)", code);
    }
    if (from == TY_FLOAT && to == TY_INT) {
        return xprintf("(int)(%s)", code);
    }
    return xstrdup(code);
}

static char *assignment_rhs_code(int target_type, Expr *rhs, const char *var_name) {
    if (target_type == TY_TEXT) {
        if (rhs->type != TY_TEXT) {
            semantic_error("Cannot assign non-text expression to text variable '%s'", var_name);
        }
        return xstrdup(rhs->code);
    }
    if (rhs->type == TY_TEXT) {
        semantic_error("Cannot assign text expression to numeric variable '%s'", var_name);
    }
    return cast_code(rhs->type, target_type, rhs->code);
}

static Expr *build_numeric_bin(Expr *lhs, int op, Expr *rhs) {
    int rt;
    char *left_code;
    char *right_code;
    const char *op_text;

    if (lhs->type == TY_TEXT || rhs->type == TY_TEXT) {
        semantic_error("Arithmetic operators are only valid for numeric values");
    }

    rt = coerce_numeric(lhs->type, rhs->type);
    if (rt == TY_UNKNOWN) {
        semantic_error("Incompatible numeric expression");
    }

    if (op == OP_ADD) op_text = "+";
    else if (op == OP_SUB) op_text = "-";
    else if (op == OP_MUL) op_text = "*";
    else op_text = "/";

    if (lhs->is_const_numeric && rhs->is_const_numeric) {
        double v = 0.0;
        if (op == OP_ADD) v = lhs->const_num + rhs->const_num;
        else if (op == OP_SUB) v = lhs->const_num - rhs->const_num;
        else if (op == OP_MUL) v = lhs->const_num * rhs->const_num;
        else {
            if (rhs->const_num == 0.0) {
                semantic_error("Division by zero in constant expression");
            }
            if (rt == TY_FLOAT) v = lhs->const_num / rhs->const_num;
            else v = (double)((int)lhs->const_num / (int)rhs->const_num);
        }

        if (rt == TY_FLOAT) {
            return expr_new(TY_FLOAT, xprintf("%.12g", v), 1, v);
        }
        return expr_new(TY_INT, xprintf("%d", (int)v), 1, (double)(int)v);
    }

    left_code = cast_code(lhs->type, rt, lhs->code);
    right_code = cast_code(rhs->type, rt, rhs->code);

    {
        char *code = xprintf("(%s %s %s)", left_code, op_text, right_code);
        Expr *out = expr_new(rt, code, 0, 0.0);
        free(code);
        free(left_code);
        free(right_code);
        return out;
    }
}

static Expr *build_comparison(Expr *lhs, int rel, Expr *rhs) {
    if (lhs->type == TY_TEXT || rhs->type == TY_TEXT) {
        if (lhs->type != TY_TEXT || rhs->type != TY_TEXT) {
            semantic_error("Text comparison requires both operands to be text");
        }
        if (rel != REL_EQ) {
            semantic_error("Text supports only 'is equals' comparison");
        }
        return expr_new(TY_INT, xprintf("strcmp(%s, %s) == 0", lhs->code, rhs->code), 0, 0.0);
    }

    {
        int rt = coerce_numeric(lhs->type, rhs->type);
        char *left_code;
        char *right_code;
        char *code;

        if (rt == TY_UNKNOWN) {
            semantic_error("Comparison operands must be numeric or text");
        }

        left_code = cast_code(lhs->type, rt, lhs->code);
        right_code = cast_code(rhs->type, rt, rhs->code);

        if (rel == REL_GT) code = xprintf("%s > %s", left_code, right_code);
        else if (rel == REL_LT) code = xprintf("%s < %s", left_code, right_code);
        else code = xprintf("%s == %s", left_code, right_code);

        {
            Expr *out = expr_new(TY_INT, code, 0, 0.0);
            free(code);
            free(left_code);
            free(right_code);
            return out;
        }
    }
}

static Expr *build_logic(Expr *lhs, const char *op, Expr *rhs) {
    char *code = xprintf("(%s %s %s)", lhs->code, op, rhs->code);
    Expr *out = expr_new(TY_INT, code, 0, 0.0);
    free(code);
    return out;
}

static Expr *build_not(Expr *inner) {
    char *code = xprintf("!(%s)", inner->code);
    Expr *out = expr_new(TY_INT, code, 0, 0.0);
    free(code);
    return out;
}

static ArgList *arglist_new(void) {
    ArgList *a = (ArgList *)xmalloc(sizeof(ArgList));
    a->items = NULL;
    a->count = 0;
    a->cap = 0;
    return a;
}

static void arglist_push(ArgList *a, Expr *e) {
    Expr **nitems;
    if (a->count == a->cap) {
        a->cap = a->cap ? a->cap * 2 : 8;
        nitems = (Expr **)realloc(a->items, (size_t)a->cap * sizeof(Expr *));
        if (!nitems) {
            fprintf(stderr, "Out of memory\n");
            exit(1);
        }
        a->items = nitems;
    }
    a->items[a->count++] = e;
}

static void reset_pending_params(void) {
    int i;
    if (pending_fn_name) {
        free(pending_fn_name);
        pending_fn_name = NULL;
    }
    if (pending_param_names) {
        for (i = 0; i < pending_param_count; ++i) {
            free(pending_param_names[i]);
        }
        free(pending_param_names);
        pending_param_names = NULL;
    }
    if (pending_param_types) {
        free(pending_param_types);
        pending_param_types = NULL;
    }
    pending_param_count = 0;
    pending_param_cap = 0;
}

static void begin_function_header(const char *name) {
    if (fn_lookup(name)) {
        semantic_error("Function '%s' is already defined", name);
    }
    reset_pending_params();
    pending_fn_name = xstrdup(name);

    strncpy(current_scope, name, sizeof(current_scope) - 1);
    current_scope[sizeof(current_scope) - 1] = '\0';
}

static void add_pending_param(int type, const char *name) {
    int i;
    int *ntypes;
    char **nnames;

    for (i = 0; i < pending_param_count; ++i) {
        if (strcmp(pending_param_names[i], name) == 0) {
            semantic_error("Duplicate parameter '%s' in function '%s'", name, pending_fn_name ? pending_fn_name : "<unknown>");
        }
    }

    if (pending_param_count == pending_param_cap) {
        pending_param_cap = pending_param_cap ? pending_param_cap * 2 : 8;
        ntypes = (int *)realloc(pending_param_types, (size_t)pending_param_cap * sizeof(int));
        nnames = (char **)realloc(pending_param_names, (size_t)pending_param_cap * sizeof(char *));
        if (!ntypes || !nnames) {
            fprintf(stderr, "Out of memory\n");
            exit(1);
        }
        pending_param_types = ntypes;
        pending_param_names = nnames;
    }

    pending_param_types[pending_param_count] = type;
    pending_param_names[pending_param_count] = xstrdup(name);
    pending_param_count++;
}

static void start_function_body(void) {
    int i;
    if (!pending_fn_name) {
        semantic_error("Internal function-state error");
    }

    current_fn = fn_add(pending_fn_name);
    current_fn->param_count = pending_param_count;
    current_fn->param_types = (int *)xmalloc((size_t)pending_param_count * sizeof(int));
    current_fn->param_names = (char **)xmalloc((size_t)pending_param_count * sizeof(char *));

    for (i = 0; i < pending_param_count; ++i) {
        current_fn->param_types[i] = pending_param_types[i];
        current_fn->param_names[i] = xstrdup(pending_param_names[i]);

        if (sym_lookup_local(pending_param_names[i], current_scope)) {
            semantic_error("Duplicate symbol '%s' in scope '%s'", pending_param_names[i], current_scope);
        }
        sym_add(pending_param_names[i], pending_param_types[i], current_scope);
    }

    sb_init(&temp_fn_body);
    active_buf = &temp_fn_body;
    indent_level = 1;
    fn_body_open = 1;
}

static void finish_function_body(void) {
    int i;

    if (!fn_body_open || !current_fn) {
        semantic_error("Internal function close-state error");
    }

    if (current_fn->return_type == TY_UNKNOWN) {
        current_fn->return_type = TY_INT;
        emit_line("return 0;");
    }

    sb_appendf(&fn_code, "%s %s(", ctype_name(current_fn->return_type), current_fn->name);
    for (i = 0; i < current_fn->param_count; ++i) {
        if (i > 0) {
            sb_append(&fn_code, ", ");
        }
        sb_appendf(&fn_code, "%s %s", ctype_name(current_fn->param_types[i]), current_fn->param_names[i]);
    }
    sb_append(&fn_code, ") {\n");
    sb_append(&fn_code, temp_fn_body.buf);
    sb_append(&fn_code, "}\n\n");

    sb_free(&temp_fn_body);

    active_buf = &main_code;
    indent_level = 1;
    fn_body_open = 0;
    current_fn = NULL;

    strncpy(current_scope, "global", sizeof(current_scope) - 1);
    current_scope[sizeof(current_scope) - 1] = '\0';

    reset_pending_params();
}

static void merge_function_return(Function *fn, int expr_type) {
    if (fn->return_type == TY_UNKNOWN) {
        fn->return_type = expr_type;
        return;
    }

    if (fn->return_type == TY_FLOAT && expr_type == TY_INT) {
        return;
    }
    if (fn->return_type == TY_INT && expr_type == TY_FLOAT) {
        fn->return_type = TY_FLOAT;
        return;
    }
    if (fn->return_type == expr_type) {
        return;
    }

    semantic_error("Inconsistent return types in function '%s'", fn->name);
}

static Expr *build_call_expr(const char *name, ArgList *args) {
    Function *fn = fn_lookup(name);
    int i;
    StrBuf sb;
    Expr *out;

    if (!fn) {
        semantic_error("Function '%s' is not defined", name);
    }

    if (args->count != fn->param_count) {
        semantic_error("Function '%s' expects %d argument(s), got %d", name, fn->param_count, args->count);
    }

    sb_init(&sb);
    sb_appendf(&sb, "%s(", name);

    for (i = 0; i < args->count; ++i) {
        int pt = fn->param_types[i];
        Expr *a = args->items[i];
        char *arg_code;

        if (pt == TY_TEXT) {
            if (a->type != TY_TEXT) {
                semantic_error("Argument %d for '%s' must be text", i + 1, name);
            }
            arg_code = xstrdup(a->code);
        } else {
            if (a->type == TY_TEXT) {
                semantic_error("Argument %d for '%s' must be numeric", i + 1, name);
            }
            arg_code = cast_code(a->type, pt, a->code);
        }

        if (i > 0) {
            sb_append(&sb, ", ");
        }
        sb_append(&sb, arg_code);
        free(arg_code);
    }

    sb_append(&sb, ")");

    out = expr_new(fn->return_type == TY_UNKNOWN ? TY_INT : fn->return_type, sb.buf, 0, 0.0);
    sb_free(&sb);
    return out;
}

static void compiler_init(void) {
    sb_init(&fn_code);
    sb_init(&main_code);
    active_buf = &main_code;
    indent_level = 1;
    fn_body_open = 0;

    strncpy(current_scope, "global", sizeof(current_scope) - 1);
    current_scope[sizeof(current_scope) - 1] = '\0';

    repeat_counter = 0;
}

static void replace_extension(const char *src, const char *ext, char *out, size_t out_sz) {
    const char *last_dot = NULL;
    const char *p;
    size_t prefix_len;

    for (p = src; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            last_dot = NULL;
        } else if (*p == '.') {
            last_dot = p;
        }
    }

    prefix_len = last_dot ? (size_t)(last_dot - src) : strlen(src);

    if (prefix_len + strlen(ext) + 1 > out_sz) {
        fprintf(stderr, "Path too long\n");
        exit(1);
    }

    memcpy(out, src, prefix_len);
    out[prefix_len] = '\0';
    strcat(out, ext);
}

static void write_output_c(void) {
    FILE *f = fopen(output_path, "wb");
    if (!f) {
        fprintf(stderr, "Could not create output C file: %s\n", output_path);
        exit(1);
    }

    fprintf(f, "#include <stdio.h>\n");
    fprintf(f, "#include <stdlib.h>\n");
    fprintf(f, "#include <string.h>\n\n");

    fprintf(f, "%s", fn_code.buf);
    fprintf(f, "int main(void) {\n");
    fprintf(f, "%s", main_code.buf);
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");

    fclose(f);
}

static void print_symbol_table(void) {
    int i;
    printf("=== Symbol Table (IR) ===\n");
    printf("%-20s %-12s %-20s\n", "Name", "Type", "Scope");
    printf("------------------------------------------------------------\n");
    for (i = 0; i < sym_count; ++i) {
        printf("%-20s %-12s %-20s\n", symbols[i].name, type_name(symbols[i].type), symbols[i].scope);
    }
}

static int run_generated_program(void) {
    char exe_path[1024];
    char cmd[4096];
    int rc;

#ifdef _WIN32
    replace_extension(output_path, ".out.exe", exe_path, sizeof(exe_path));
    snprintf(cmd,
             sizeof(cmd),
             "gcc -w -o \"%s\" \"%s\" && \"%s\" > \"%s\" && type \"%s\"",
             exe_path,
             output_path,
             exe_path,
             output_text_path,
             output_text_path);
#else
    replace_extension(output_path, ".out", exe_path, sizeof(exe_path));
    snprintf(cmd,
             sizeof(cmd),
             "gcc -w -o \"%s\" \"%s\" && \"%s\" > \"%s\" && cat \"%s\"",
             exe_path,
             output_path,
             exe_path,
             output_text_path,
             output_text_path);
#endif

    rc = system(cmd);
    if (rc != 0) {
        fprintf(stderr, "Failed to compile/run generated C file\n");
        return 1;
    }
    printf("Program output saved to: %s\n", output_text_path);
    return 0;
}

static void parse_cli(int argc, char **argv) {
    const char *input = NULL;
    int i;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.blk> [--show-ir] [--run]\n", argv[0]);
        exit(1);
    }

    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--show-ir") == 0) {
            show_ir_flag = 1;
        } else if (strcmp(argv[i], "--run") == 0) {
            run_flag = 1;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            exit(1);
        } else if (!input) {
            input = argv[i];
        } else {
            fprintf(stderr, "Unexpected argument: %s\n", argv[i]);
            exit(1);
        }
    }

    if (!input) {
        fprintf(stderr, "No input .blk file provided\n");
        exit(1);
    }

    strncpy(input_path, input, sizeof(input_path) - 1);
    input_path[sizeof(input_path) - 1] = '\0';

    replace_extension(input_path, ".c", output_path, sizeof(output_path));
    replace_extension(input_path, ".txt", output_text_path, sizeof(output_text_path));
}

static void push_repeat_idx(const char *idx_name) {
    if (repeat_depth >= MAX_REPEAT_DEPTH) {
        semantic_error("Repeat nesting is too deep");
    }
    strncpy(repeat_idx_stack[repeat_depth], idx_name, sizeof(repeat_idx_stack[repeat_depth]) - 1);
    repeat_idx_stack[repeat_depth][sizeof(repeat_idx_stack[repeat_depth]) - 1] = '\0';
    repeat_depth++;
}

static void pop_repeat_idx(void) {
    if (repeat_depth <= 0) {
        semantic_error("Internal repeat-state error");
    }
    repeat_depth--;
}

static const char *current_repeat_idx(void) {
    if (repeat_depth <= 0) {
        return NULL;
    }
    return repeat_idx_stack[repeat_depth - 1];
}


/* Line 189 of yacc.c  */
#line 964 "src/parser.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     PROGRAM = 258,
     WHEN = 259,
     OTHERWISE = 260,
     REPEAT = 261,
     TIMES = 262,
     WHILE = 263,
     DEFINE = 264,
     GIVE = 265,
     SAY = 266,
     SET = 267,
     TO = 268,
     NUM = 269,
     TEXT = 270,
     DECIMAL = 271,
     CALL = 272,
     AND = 273,
     OR = 274,
     NOT = 275,
     IS = 276,
     GREATER = 277,
     LESS = 278,
     THAN = 279,
     EQUALS = 280,
     END_WHEN = 281,
     END_REPEAT = 282,
     END_WHILE = 283,
     END_DEFINE = 284,
     END_PROGRAM = 285,
     IDENTIFIER = 286,
     STRING_LITERAL = 287,
     INT_LITERAL = 288,
     FLOAT_LITERAL = 289,
     NEWLINE = 290,
     PLUS = 291,
     MINUS = 292,
     STAR = 293,
     SLASH = 294,
     LPAREN = 295,
     RPAREN = 296,
     COMMA = 297,
     UMINUS = 298
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 891 "src/parser.y"

    int ival;
    double fval;
    char *sval;
    void *ptr;



/* Line 214 of yacc.c  */
#line 1052 "src/parser.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 1064 "src/parser.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   180

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  44
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  37
/* YYNRULES -- Number of rules.  */
#define YYNRULES  71
/* YYNRULES -- Number of states.  */
#define YYNSTATES  136

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   298

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,    10,    18,    19,    22,    23,    24,    36,
      37,    39,    41,    45,    48,    49,    52,    54,    57,    60,
      63,    66,    68,    70,    72,    78,    83,    86,    87,    95,
      96,    97,   102,   103,   111,   112,   119,   122,   124,   130,
     131,   133,   135,   139,   141,   143,   145,   147,   151,   153,
     157,   159,   162,   164,   166,   170,   176,   182,   187,   191,
     195,   197,   201,   205,   207,   210,   212,   214,   216,   218,
     220,   222
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      45,     0,    -1,     3,    31,    35,    46,    53,    30,    -1,
       3,    31,    35,    46,    53,    30,    35,    -1,    -1,    46,
      47,    -1,    -1,    -1,     9,    31,    40,    48,    50,    41,
      35,    49,    53,    29,    35,    -1,    -1,    51,    -1,    52,
      -1,    51,    42,    52,    -1,    70,    31,    -1,    -1,    53,
      54,    -1,    35,    -1,    55,    35,    -1,    56,    35,    -1,
      65,    35,    -1,    66,    35,    -1,    57,    -1,    61,    -1,
      63,    -1,    12,    70,    31,    13,    77,    -1,    12,    31,
      13,    77,    -1,    11,    77,    -1,    -1,     4,    71,    35,
      58,    53,    59,    26,    -1,    -1,    -1,     5,    35,    60,
      53,    -1,    -1,     6,    77,     7,    35,    62,    53,    27,
      -1,    -1,     8,    71,    35,    64,    53,    28,    -1,    10,
      77,    -1,    67,    -1,    17,    31,    40,    68,    41,    -1,
      -1,    69,    -1,    77,    -1,    69,    42,    77,    -1,    14,
      -1,    16,    -1,    15,    -1,    72,    -1,    72,    19,    73,
      -1,    73,    -1,    73,    18,    74,    -1,    74,    -1,    20,
      74,    -1,    75,    -1,    76,    -1,    40,    71,    41,    -1,
      77,    21,    22,    24,    77,    -1,    77,    21,    23,    24,
      77,    -1,    77,    21,    25,    77,    -1,    77,    36,    78,
      -1,    77,    37,    78,    -1,    78,    -1,    78,    38,    79,
      -1,    78,    39,    79,    -1,    79,    -1,    37,    79,    -1,
      80,    -1,    33,    -1,    34,    -1,    32,    -1,    31,    -1,
      67,    -1,    40,    77,    41,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   921,   921,   925,   931,   933,   938,   942,   937,   951,
     953,   957,   958,   962,   968,   970,   974,   975,   976,   977,
     978,   979,   980,   981,   985,  1007,  1029,  1044,  1043,  1055,
    1058,  1057,  1068,  1067,  1090,  1089,  1102,  1114,  1121,  1129,
    1132,  1139,  1145,  1154,  1155,  1156,  1160,  1167,  1171,  1178,
    1182,  1189,  1193,  1200,  1204,  1211,  1215,  1219,  1226,  1230,
    1234,  1241,  1245,  1249,  1256,  1273,  1280,  1284,  1288,  1292,
    1309,  1313
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "PROGRAM", "WHEN", "OTHERWISE", "REPEAT",
  "TIMES", "WHILE", "DEFINE", "GIVE", "SAY", "SET", "TO", "NUM", "TEXT",
  "DECIMAL", "CALL", "AND", "OR", "NOT", "IS", "GREATER", "LESS", "THAN",
  "EQUALS", "END_WHEN", "END_REPEAT", "END_WHILE", "END_DEFINE",
  "END_PROGRAM", "IDENTIFIER", "STRING_LITERAL", "INT_LITERAL",
  "FLOAT_LITERAL", "NEWLINE", "PLUS", "MINUS", "STAR", "SLASH", "LPAREN",
  "RPAREN", "COMMA", "UMINUS", "$accept", "source", "define_list",
  "define_stmt", "$@1", "$@2", "param_list_opt", "param_list", "param",
  "stmt_list", "statement", "set_stmt", "say_stmt", "when_stmt", "$@3",
  "else_opt", "$@4", "repeat_stmt", "$@5", "while_stmt", "$@6",
  "give_stmt", "call_stmt", "call_expr", "arg_list_opt", "arg_list",
  "type_spec", "condition", "cond_or", "cond_and", "cond_not", "cond_atom",
  "comparison", "expr", "term", "factor", "primary", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    44,    45,    45,    46,    46,    48,    49,    47,    50,
      50,    51,    51,    52,    53,    53,    54,    54,    54,    54,
      54,    54,    54,    54,    55,    55,    56,    58,    57,    59,
      60,    59,    62,    61,    64,    63,    65,    66,    67,    68,
      68,    69,    69,    70,    70,    70,    71,    72,    72,    73,
      73,    74,    74,    75,    75,    76,    76,    76,    77,    77,
      77,    78,    78,    78,    79,    79,    80,    80,    80,    80,
      80,    80
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     6,     7,     0,     2,     0,     0,    11,     0,
       1,     1,     3,     2,     0,     2,     1,     2,     2,     2,
       2,     1,     1,     1,     5,     4,     2,     0,     7,     0,
       0,     4,     0,     7,     0,     6,     2,     1,     5,     0,
       1,     1,     3,     1,     1,     1,     1,     3,     1,     3,
       1,     2,     1,     1,     3,     5,     5,     4,     3,     3,
       1,     3,     3,     1,     2,     1,     1,     1,     1,     1,
       1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     1,     4,    14,     0,     5,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     2,    16,
      15,     0,     0,    21,    22,    23,     0,     0,    37,     6,
       0,    69,    68,    66,    67,     0,     0,    70,     0,    46,
      48,    50,    52,    53,     0,    60,    63,    65,     0,     0,
       0,    36,    26,    43,    45,    44,     0,     0,     0,     3,
      17,    18,    19,    20,     9,    51,    64,     0,     0,    27,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    34,
       0,     0,    39,     0,    10,    11,     0,    54,    71,    14,
      47,    49,     0,     0,     0,    58,    59,    61,    62,    32,
      14,    25,     0,     0,    40,    41,     0,     0,    13,    29,
       0,     0,    57,    14,     0,    24,    38,     0,     7,    12,
       0,     0,    55,    56,     0,    35,    42,    14,    30,    28,
      33,     0,    14,     0,    31,     8
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     6,     8,    64,   127,    83,    84,    85,     9,
      20,    21,    22,    23,    89,   121,   132,    24,   113,    25,
     100,    26,    27,    37,   103,   104,    86,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -89
static const yytype_int16 yypact[] =
{
      10,   -16,    38,     6,   -89,   -89,    39,    19,   -89,    41,
      16,   130,   140,   130,   140,   140,     4,    28,    30,   -89,
     -89,    34,    35,   -89,   -89,   -89,    43,    45,   -89,   -89,
     130,   -89,   -89,   -89,   -89,   140,   130,   -89,    52,    44,
      72,   -89,   -89,   -89,   -10,   -29,   -89,   -89,   140,    -4,
      54,     0,     0,   -89,   -89,   -89,    79,    62,    55,   -89,
     -89,   -89,   -89,   -89,     8,   -89,   -89,    57,    -7,   -89,
     130,   130,    32,   140,   140,   140,   140,   -20,    61,   -89,
     140,    84,   140,    60,    65,   -89,    73,   -89,   -89,   -89,
      72,   -89,    87,    89,   140,   -29,   -29,   -89,   -89,   -89,
     -89,     0,   140,    67,    77,     0,    85,     8,   -89,    56,
     140,   140,     0,   -89,    71,     0,   -89,   140,   -89,   -89,
      86,   101,     0,     0,   106,   -89,     0,   -89,   -89,   -89,
     -89,   120,   -89,    94,   134,   -89
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -89,   -89,   -89,   -89,   -89,   -89,   -89,   -89,    27,   -88,
     -89,   -89,   -89,   -89,   -89,   -89,   -89,   -89,   -89,   -89,
     -89,   -89,   -89,    -9,   -89,   -89,   119,    -5,   -89,    66,
     -25,   -89,   -89,    -8,    11,   -33,   -89
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      28,   109,    66,    78,    49,    65,    51,    52,    50,    75,
      76,    72,   114,     1,    72,     3,    73,    74,    53,    54,
      55,    88,    53,    54,    55,   124,    73,    74,    68,    73,
      74,    67,    73,    74,    88,    56,    73,    74,     4,   131,
      77,     5,    97,    98,   134,    11,    91,    12,     7,    13,
      10,    14,    15,    16,    92,    93,    29,    94,    17,    58,
      11,   120,    12,    70,    13,    59,    14,    15,    16,    60,
      61,    18,   101,    17,   105,    11,    19,    12,    62,    13,
      63,    14,    15,    16,    95,    96,   112,    69,    17,    79,
      71,    19,    80,    81,   115,    82,    99,   102,    87,   125,
      28,   106,   122,   123,   108,    28,    19,   107,   116,   126,
      11,   110,    12,   111,    13,    28,    14,    15,    16,   117,
     118,   128,    28,    17,    11,    28,    12,   129,    13,   135,
      14,    15,    16,   130,   119,    57,    90,    17,    11,     0,
      12,    19,    13,     0,    14,    15,    16,    17,     0,   133,
      30,    17,     0,     0,     0,    19,     0,    17,     0,     0,
       0,    31,    32,    33,    34,     0,     0,    35,     0,    19,
      36,    31,    32,    33,    34,     0,     0,    35,     0,     0,
      48
};

static const yytype_int16 yycheck[] =
{
       9,    89,    35,     7,    12,    30,    14,    15,    13,    38,
      39,    21,   100,     3,    21,    31,    36,    37,    14,    15,
      16,    41,    14,    15,    16,   113,    36,    37,    36,    36,
      37,    36,    36,    37,    41,    31,    36,    37,     0,   127,
      48,    35,    75,    76,   132,     4,    71,     6,     9,     8,
      31,    10,    11,    12,    22,    23,    40,    25,    17,    31,
       4,     5,     6,    19,     8,    35,    10,    11,    12,    35,
      35,    30,    80,    17,    82,     4,    35,     6,    35,     8,
      35,    10,    11,    12,    73,    74,    94,    35,    17,    35,
      18,    35,    13,    31,   102,    40,    35,    13,    41,    28,
     109,    41,   110,   111,    31,   114,    35,    42,    41,   117,
       4,    24,     6,    24,     8,   124,    10,    11,    12,    42,
      35,    35,   131,    17,     4,   134,     6,    26,     8,    35,
      10,    11,    12,    27,   107,    16,    70,    17,     4,    -1,
       6,    35,     8,    -1,    10,    11,    12,    17,    -1,    29,
      20,    17,    -1,    -1,    -1,    35,    -1,    17,    -1,    -1,
      -1,    31,    32,    33,    34,    -1,    -1,    37,    -1,    35,
      40,    31,    32,    33,    34,    -1,    -1,    37,    -1,    -1,
      40
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,    45,    31,     0,    35,    46,     9,    47,    53,
      31,     4,     6,     8,    10,    11,    12,    17,    30,    35,
      54,    55,    56,    57,    61,    63,    65,    66,    67,    40,
      20,    31,    32,    33,    34,    37,    40,    67,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    40,    77,
      71,    77,    77,    14,    15,    16,    31,    70,    31,    35,
      35,    35,    35,    35,    48,    74,    79,    71,    77,    35,
      19,    18,    21,    36,    37,    38,    39,    77,     7,    35,
      13,    31,    40,    50,    51,    52,    70,    41,    41,    58,
      73,    74,    22,    23,    25,    78,    78,    79,    79,    35,
      64,    77,    13,    68,    69,    77,    41,    42,    31,    53,
      24,    24,    77,    62,    53,    77,    41,    42,    35,    52,
       5,    59,    77,    77,    53,    28,    77,    49,    35,    26,
      27,    53,    60,    29,    53,    35
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 922 "src/parser.y"
    {
                    (void)(yyvsp[(2) - (6)].sval);
      ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 926 "src/parser.y"
    {
                    (void)(yyvsp[(2) - (7)].sval);
            ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 938 "src/parser.y"
    {
          begin_function_header((yyvsp[(2) - (3)].sval));
      ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 942 "src/parser.y"
    {
          start_function_body();
      ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 946 "src/parser.y"
    {
          finish_function_body();
      ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 963 "src/parser.y"
    {
          add_pending_param((yyvsp[(1) - (2)].ival), (yyvsp[(2) - (2)].sval));
      ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 986 "src/parser.y"
    {
          Symbol *local = sym_lookup_local((yyvsp[(3) - (5)].sval), current_scope);
          Expr *rhs = EXPR((yyvsp[(5) - (5)].ptr));
          char *rhs_code;

          if (!local) {
              sym_add((yyvsp[(3) - (5)].sval), (yyvsp[(2) - (5)].ival), current_scope);
              rhs_code = assignment_rhs_code((yyvsp[(2) - (5)].ival), rhs, (yyvsp[(3) - (5)].sval));
              emit_line("%s %s = %s;", ctype_name((yyvsp[(2) - (5)].ival)), (yyvsp[(3) - (5)].sval), rhs_code);
              free(rhs_code);
          } else {
              fprintf(stderr,
                      "[Semantic Warning] Line %d: Variable '%s' already declared in scope '%s'; treated as assignment.\n",
                      line_num,
                      (yyvsp[(3) - (5)].sval),
                      current_scope);
              rhs_code = assignment_rhs_code(local->type, rhs, (yyvsp[(3) - (5)].sval));
              emit_line("%s = %s;", (yyvsp[(3) - (5)].sval), rhs_code);
              free(rhs_code);
          }
      ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 1008 "src/parser.y"
    {
          Symbol *s = sym_lookup_visible((yyvsp[(2) - (4)].sval), current_scope);
          Expr *rhs = EXPR((yyvsp[(4) - (4)].ptr));
          char *rhs_code;
          int inferred;

          if (!s) {
              inferred = (rhs->type == TY_UNKNOWN) ? TY_INT : rhs->type;
              sym_add((yyvsp[(2) - (4)].sval), inferred, current_scope);
              rhs_code = assignment_rhs_code(inferred, rhs, (yyvsp[(2) - (4)].sval));
              emit_line("%s %s = %s;", ctype_name(inferred), (yyvsp[(2) - (4)].sval), rhs_code);
              free(rhs_code);
          } else {
              rhs_code = assignment_rhs_code(s->type, rhs, (yyvsp[(2) - (4)].sval));
              emit_line("%s = %s;", (yyvsp[(2) - (4)].sval), rhs_code);
              free(rhs_code);
          }
      ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 1030 "src/parser.y"
    {
          Expr *e = EXPR((yyvsp[(2) - (2)].ptr));
          if (e->type == TY_TEXT) {
              emit_line("printf(\"%%s\\n\", %s);", e->code);
          } else if (e->type == TY_FLOAT) {
              emit_line("printf(\"%%.4g\\n\", %s);", e->code);
          } else {
              emit_line("printf(\"%%d\\n\", %s);", e->code);
          }
      ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 1044 "src/parser.y"
    {
          emit_line("if (%s) {", EXPR((yyvsp[(2) - (3)].ptr))->code);
          indent_level++;
      ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 1049 "src/parser.y"
    {
          indent_level--;
          emit_line("}");
      ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 1058 "src/parser.y"
    {
          indent_level--;
          emit_line("} else {");
          indent_level++;
      ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 1068 "src/parser.y"
    {
          Expr *n = EXPR((yyvsp[(2) - (4)].ptr));
          int id = ++repeat_counter;
          char idx[32];
          char *n_code = cast_code(n->type, TY_INT, n->code);

          snprintf(idx, sizeof(idx), "__r%d", id);
          push_repeat_idx(idx);
          emit_line("for (int %s = 0; %s < %s; ++%s) {", idx, idx, n_code, idx);
          free(n_code);
          indent_level++;
      ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 1081 "src/parser.y"
    {
          indent_level--;
          emit_line("}");
          pop_repeat_idx();
      ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 1090 "src/parser.y"
    {
          emit_line("while (%s) {", EXPR((yyvsp[(2) - (3)].ptr))->code);
          indent_level++;
      ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 1095 "src/parser.y"
    {
          indent_level--;
          emit_line("}");
      ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 1103 "src/parser.y"
    {
          Expr *e = EXPR((yyvsp[(2) - (2)].ptr));
          if (!current_fn) {
              semantic_error("'give' is only valid inside function definitions");
          }
          merge_function_return(current_fn, e->type);
          emit_line("return %s;", e->code);
      ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 1115 "src/parser.y"
    {
          emit_line("%s;", EXPR((yyvsp[(1) - (1)].ptr))->code);
      ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 1122 "src/parser.y"
    {
          (yyval.ptr) = PTR(build_call_expr((yyvsp[(2) - (5)].sval), ARGS((yyvsp[(4) - (5)].ptr))));
      ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 1129 "src/parser.y"
    {
          (yyval.ptr) = PTR(arglist_new());
      ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 1133 "src/parser.y"
    {
          (yyval.ptr) = (yyvsp[(1) - (1)].ptr);
      ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 1140 "src/parser.y"
    {
          ArgList *a = arglist_new();
          arglist_push(a, EXPR((yyvsp[(1) - (1)].ptr)));
          (yyval.ptr) = PTR(a);
      ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 1146 "src/parser.y"
    {
          ArgList *a = ARGS((yyvsp[(1) - (3)].ptr));
          arglist_push(a, EXPR((yyvsp[(3) - (3)].ptr)));
          (yyval.ptr) = PTR(a);
      ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 1154 "src/parser.y"
    { (yyval.ival) = TY_INT; ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 1155 "src/parser.y"
    { (yyval.ival) = TY_FLOAT; ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 1156 "src/parser.y"
    { (yyval.ival) = TY_TEXT; ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 1161 "src/parser.y"
    {
                    (yyval.ptr) = (yyvsp[(1) - (1)].ptr);
            ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 1168 "src/parser.y"
    {
                    (yyval.ptr) = PTR(build_logic(EXPR((yyvsp[(1) - (3)].ptr)), "||", EXPR((yyvsp[(3) - (3)].ptr))));
            ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 1172 "src/parser.y"
    {
                    (yyval.ptr) = (yyvsp[(1) - (1)].ptr);
            ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 1179 "src/parser.y"
    {
                    (yyval.ptr) = PTR(build_logic(EXPR((yyvsp[(1) - (3)].ptr)), "&&", EXPR((yyvsp[(3) - (3)].ptr))));
            ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 1183 "src/parser.y"
    {
                    (yyval.ptr) = (yyvsp[(1) - (1)].ptr);
            ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 1190 "src/parser.y"
    {
                    (yyval.ptr) = PTR(build_not(EXPR((yyvsp[(2) - (2)].ptr))));
            ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 1194 "src/parser.y"
    {
                    (yyval.ptr) = (yyvsp[(1) - (1)].ptr);
            ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 1201 "src/parser.y"
    {
                    (yyval.ptr) = (yyvsp[(1) - (1)].ptr);
            ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 1205 "src/parser.y"
    {
                    (yyval.ptr) = (yyvsp[(2) - (3)].ptr);
            ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 1212 "src/parser.y"
    {
                    (yyval.ptr) = PTR(build_comparison(EXPR((yyvsp[(1) - (5)].ptr)), REL_GT, EXPR((yyvsp[(5) - (5)].ptr))));
      ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 1216 "src/parser.y"
    {
                    (yyval.ptr) = PTR(build_comparison(EXPR((yyvsp[(1) - (5)].ptr)), REL_LT, EXPR((yyvsp[(5) - (5)].ptr))));
      ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 1220 "src/parser.y"
    {
                    (yyval.ptr) = PTR(build_comparison(EXPR((yyvsp[(1) - (4)].ptr)), REL_EQ, EXPR((yyvsp[(4) - (4)].ptr))));
      ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 1227 "src/parser.y"
    {
                    (yyval.ptr) = PTR(build_numeric_bin(EXPR((yyvsp[(1) - (3)].ptr)), OP_ADD, EXPR((yyvsp[(3) - (3)].ptr))));
      ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 1231 "src/parser.y"
    {
                    (yyval.ptr) = PTR(build_numeric_bin(EXPR((yyvsp[(1) - (3)].ptr)), OP_SUB, EXPR((yyvsp[(3) - (3)].ptr))));
      ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 1235 "src/parser.y"
    {
          (yyval.ptr) = (yyvsp[(1) - (1)].ptr);
      ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 1242 "src/parser.y"
    {
                    (yyval.ptr) = PTR(build_numeric_bin(EXPR((yyvsp[(1) - (3)].ptr)), OP_MUL, EXPR((yyvsp[(3) - (3)].ptr))));
      ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 1246 "src/parser.y"
    {
                    (yyval.ptr) = PTR(build_numeric_bin(EXPR((yyvsp[(1) - (3)].ptr)), OP_DIV, EXPR((yyvsp[(3) - (3)].ptr))));
      ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 1250 "src/parser.y"
    {
          (yyval.ptr) = (yyvsp[(1) - (1)].ptr);
      ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 1257 "src/parser.y"
    {
          Expr *inner = EXPR((yyvsp[(2) - (2)].ptr));
          if (inner->type != TY_INT && inner->type != TY_FLOAT) {
              semantic_error("Unary '-' requires a numeric operand");
          }
          if (inner->is_const_numeric) {
              double v = -inner->const_num;
              if (inner->type == TY_FLOAT) {
                  (yyval.ptr) = PTR(expr_new(TY_FLOAT, xprintf("%.12g", v), 1, v));
              } else {
                  (yyval.ptr) = PTR(expr_new(TY_INT, xprintf("%d", (int)v), 1, (double)(int)v));
              }
          } else {
              (yyval.ptr) = PTR(expr_new(inner->type, xprintf("(-(%s))", inner->code), 0, 0.0));
          }
      ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 1274 "src/parser.y"
    {
          (yyval.ptr) = (yyvsp[(1) - (1)].ptr);
      ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 1281 "src/parser.y"
    {
          (yyval.ptr) = PTR(expr_new(TY_INT, xprintf("%d", (yyvsp[(1) - (1)].ival)), 1, (double)(yyvsp[(1) - (1)].ival)));
      ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 1285 "src/parser.y"
    {
          (yyval.ptr) = PTR(expr_new(TY_FLOAT, xprintf("%.12g", (yyvsp[(1) - (1)].fval)), 1, (yyvsp[(1) - (1)].fval)));
      ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 1289 "src/parser.y"
    {
          (yyval.ptr) = PTR(expr_new(TY_TEXT, (yyvsp[(1) - (1)].sval), 0, 0.0));
      ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 1293 "src/parser.y"
    {
          const char *loop_idx = NULL;
          if (strcmp((yyvsp[(1) - (1)].sval), "i") == 0) {
              loop_idx = current_repeat_idx();
          }

          if (loop_idx) {
              (yyval.ptr) = PTR(expr_new(TY_INT, loop_idx, 0, 0.0));
          } else {
              Symbol *s = sym_lookup_visible((yyvsp[(1) - (1)].sval), current_scope);
              if (!s) {
                  semantic_error("Variable '%s' used before declaration", (yyvsp[(1) - (1)].sval));
              }
              (yyval.ptr) = PTR(expr_new(s->type, (yyvsp[(1) - (1)].sval), 0, 0.0));
          }
      ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 1310 "src/parser.y"
    {
          (yyval.ptr) = (yyvsp[(1) - (1)].ptr);
      ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 1314 "src/parser.y"
    {
          (yyval.ptr) = (yyvsp[(2) - (3)].ptr);
      ;}
    break;



/* Line 1455 of yacc.c  */
#line 2945 "src/parser.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 1319 "src/parser.y"


void yyerror(const char *msg) {
    fprintf(stderr, "[Syntax Error] Line %d: %s near '%s'\n", line_num, msg, yytext ? yytext : "<eof>");
}

int main(int argc, char **argv) {
    int rc;

    parse_cli(argc, argv);
    compiler_init();

    yyin = fopen(input_path, "rb");
    if (!yyin) {
        fprintf(stderr, "Could not open input file: %s\n", input_path);
        return 1;
    }

    rc = yyparse();
    fclose(yyin);

    if (rc != 0) {
        return 1;
    }

    write_output_c();
    printf("Generated C file: %s\n", output_path);

    if (show_ir_flag) {
        print_symbol_table();
    }

    if (run_flag) {
        return run_generated_program();
    }

    return 0;
}

