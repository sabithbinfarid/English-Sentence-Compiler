%{
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

typedef struct {
    char *stmt_type;
    char *description;
    char *code;
    char *scope;
    int line_num;
    int depth;
} ASTNode;

static Symbol *symbols = NULL;
static int sym_count = 0;
static int sym_cap = 0;

static Function *functions = NULL;
static int fn_count = 0;
static int fn_cap = 0;

static ASTNode *ast_nodes = NULL;
static int ast_count = 0;
static int ast_cap = 0;

static StrBuf ir_code_buf;
static int ir_instr_no = 0;

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
static void add_ir_operation(const char *op, const char *operands, const char *result);

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
    const char *ir_op;

    if (lhs->type == TY_TEXT || rhs->type == TY_TEXT) {
        semantic_error("Arithmetic operators are only valid for numeric values");
    }

    rt = coerce_numeric(lhs->type, rhs->type);
    if (rt == TY_UNKNOWN) {
        semantic_error("Incompatible numeric expression");
    }

    if (op == OP_ADD) {
        op_text = "+";
        ir_op = "ADD";
    } else if (op == OP_SUB) {
        op_text = "-";
        ir_op = "SUB";
    } else if (op == OP_MUL) {
        op_text = "*";
        ir_op = "MUL";
    } else {
        op_text = "/";
        ir_op = "DIV";
    }

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
        char *code = xprintf("%s %s %s", left_code, op_text, right_code);
        char *operands = xprintf("%s, %s", left_code, right_code);
        Expr *out = expr_new(rt, code, 0, 0.0);
        add_ir_operation(ir_op, operands, code);
        free(operands);
        free(code);
        free(left_code);
        free(right_code);
        return out;
    }
}

static Expr *build_comparison(Expr *lhs, int rel, Expr *rhs) {
    if (lhs->type == TY_TEXT || rhs->type == TY_TEXT) {
        char *code;
        char *operands;
        Expr *out;
        if (lhs->type != TY_TEXT || rhs->type != TY_TEXT) {
            semantic_error("Text comparison requires both operands to be text");
        }
        if (rel != REL_EQ) {
            semantic_error("Text supports only 'is equals' comparison");
        }
        code = xprintf("strcmp(%s, %s) == 0", lhs->code, rhs->code);
        operands = xprintf("%s, %s", lhs->code, rhs->code);
        add_ir_operation("CMP_EQ", operands, code);
        out = expr_new(TY_INT, code, 0, 0.0);
        free(code);
        free(operands);
        return out;
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
            char *operands = xprintf("%s, %s", left_code, right_code);
            Expr *out = expr_new(TY_INT, code, 0, 0.0);
            add_ir_operation(rel == REL_GT ? "CMP_GT" : (rel == REL_LT ? "CMP_LT" : "CMP_EQ"), operands, code);
            free(operands);
            free(code);
            free(left_code);
            free(right_code);
            return out;
        }
    }
}

static Expr *build_logic(Expr *lhs, const char *op, Expr *rhs) {
    char *code = xprintf("%s %s %s", lhs->code, op, rhs->code);
    char *operands = xprintf("%s, %s", lhs->code, rhs->code);
    Expr *out = expr_new(TY_INT, code, 0, 0.0);
    add_ir_operation(strcmp(op, "&&") == 0 ? "AND" : "OR", operands, code);
    free(operands);
    free(code);
    return out;
}

static Expr *build_not(Expr *inner) {
    char *code = xprintf("!(%s)", inner->code);
    Expr *out = expr_new(TY_INT, code, 0, 0.0);
    add_ir_operation("NOT", inner->code, code);
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
    add_ir_operation("CALL", sb.buf, name);
    sb_free(&sb);
    return out;
}

static void compiler_init(void) {
    ast_count = 0;
    ir_instr_no = 0;
    if (ir_code_buf.buf) {
        sb_free(&ir_code_buf);
    }
    sb_init(&ir_code_buf);

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

static void print_symbol_table_to(FILE *out) {
    int i;
    fprintf(out, "=== Symbol Table (IR) ===\n");
    fprintf(out, "%-20s %-12s %-20s\n", "Name", "Type", "Scope");
    fprintf(out, "------------------------------------------------------------\n");
    for (i = 0; i < sym_count; ++i) {
        fprintf(out, "%-20s %-12s %-20s\n", symbols[i].name, type_name(symbols[i].type), symbols[i].scope);
    }
    fprintf(out, "\n");
}

static void print_symbol_table(void) {
    print_symbol_table_to(stdout);
}

static void add_ast_node(const char *type, const char *desc, const char *code) {
    ASTNode *temp;
    if (ast_count == ast_cap) {
        ast_cap = ast_cap ? ast_cap * 2 : 32;
        temp = (ASTNode *)realloc(ast_nodes, (size_t)ast_cap * sizeof(ASTNode));
        if (!temp) {
            fprintf(stderr, "Out of memory for AST\n");
            exit(1);
        }
        ast_nodes = temp;
    }
    ast_nodes[ast_count].stmt_type = xstrdup(type);
    ast_nodes[ast_count].description = xstrdup(desc);
    ast_nodes[ast_count].code = xstrdup(code ? code : "");
    ast_nodes[ast_count].scope = xstrdup(current_scope);
    ast_nodes[ast_count].line_num = line_num;
    ast_nodes[ast_count].depth = indent_level > 0 ? indent_level - 1 : 0;
    ast_count++;
}

static void add_ir_operation(const char *op, const char *operands, const char *result) {
    ir_instr_no++;
    sb_appendf(&ir_code_buf,
               "%03d: %-10s %-30s -> %s\n",
               ir_instr_no,
               op ? op : "",
               operands ? operands : "",
               result ? result : "");
}

static void print_ast_to(FILE *out) {
    int i;
    int j;
    const char *last_scope = NULL;

    fprintf(out, "=== Abstract Syntax Tree (AST) ===\n");
    fprintf(out, "Program\n");
    for (i = 0; i < ast_count; ++i) {
        if (!last_scope || strcmp(last_scope, ast_nodes[i].scope) != 0) {
            fprintf(out, "  Scope: %s\n", ast_nodes[i].scope);
            last_scope = ast_nodes[i].scope;
        }

        for (j = 0; j < ast_nodes[i].depth + 2; ++j) {
            fprintf(out, "  ");
        }
        fprintf(out, "|- [L%d] %s: %s", ast_nodes[i].line_num, ast_nodes[i].stmt_type, ast_nodes[i].description);
        if (ast_nodes[i].code[0] != '\0') {
            fprintf(out, " => %s", ast_nodes[i].code);
        }
        fprintf(out, "\n");
    }
    fprintf(out, "\n");
}

static void print_ast(void) {
    print_ast_to(stdout);
}

static void print_intermediate_code_to(FILE *out) {
    fprintf(out, "=== Intermediate Code Generation (3AC) ===\n");
    if (ir_instr_no > 0) {
        fprintf(out, "%s", ir_code_buf.buf);
    } else {
        fprintf(out, "(No intermediate operations recorded)\n");
    }
    fprintf(out, "\n");
}

static void print_intermediate_code(void) {
    print_intermediate_code_to(stdout);
}

static void write_analysis_report_file(void) {
    FILE *f = fopen(output_text_path, "wb");
    if (!f) {
        fprintf(stderr, "Could not create analysis output file: %s\n", output_text_path);
        exit(1);
    }

    print_symbol_table_to(f);
    print_ast_to(f);
    print_intermediate_code_to(f);
    fclose(f);
    printf("Analysis output saved to: %s\n", output_text_path);
}

static int run_generated_program(void) {
    char exe_path[1024];
    char cmd[4096];
    const char *redir = show_ir_flag ? ">>" : ">";
    int rc;

#ifdef _WIN32
    replace_extension(output_path, ".out.exe", exe_path, sizeof(exe_path));
    snprintf(cmd,
             sizeof(cmd),
             "gcc -w -o \"%s\" \"%s\" && \"%s\" %s \"%s\" && type \"%s\"",
             exe_path,
             output_path,
             exe_path,
             redir,
             output_text_path,
             output_text_path);
#else
    replace_extension(output_path, ".out", exe_path, sizeof(exe_path));
    snprintf(cmd,
             sizeof(cmd),
             "gcc -w -o \"%s\" \"%s\" && \"%s\" %s \"%s\" && cat \"%s\"",
             exe_path,
             output_path,
             exe_path,
             redir,
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
%}

%union {
    int ival;
    double fval;
    char *sval;
    void *ptr;
}

%token PROGRAM WHEN OTHERWISE REPEAT TIMES WHILE DEFINE GIVE SAY SET TO
%token NUM TEXT DECIMAL CALL AND OR NOT IS GREATER LESS THAN EQUALS
%token END_WHEN END_REPEAT END_WHILE END_DEFINE END_PROGRAM
%token <sval> IDENTIFIER STRING_LITERAL
%token <ival> INT_LITERAL
%token <fval> FLOAT_LITERAL
%token NEWLINE
%token PLUS MINUS STAR SLASH LPAREN RPAREN COMMA

%left PLUS MINUS
%left STAR SLASH
%right UMINUS

%type <ival> type_spec
%type <ptr> expr term factor primary
%type <ptr> cond_or cond_and cond_not cond_atom comparison
%type <ptr> call_expr arg_list arg_list_opt

%start source

%%

source
    : PROGRAM IDENTIFIER NEWLINE define_list stmt_list END_PROGRAM
      {
                    (void)$2;
      }
        | PROGRAM IDENTIFIER NEWLINE define_list stmt_list END_PROGRAM NEWLINE
            {
                    (void)$2;
            }
    ;

define_list
    :
    | define_list define_stmt
    ;

define_stmt
    : DEFINE IDENTIFIER LPAREN
      {
          add_ast_node("DEFINE", "Function definition begins", $2);
          add_ir_operation("FUNC_BEGIN", $2, "");
          begin_function_header($2);
      }
      param_list_opt RPAREN NEWLINE
      {
          start_function_body();
      }
            stmt_list END_DEFINE NEWLINE
      {
          add_ast_node("END_DEFINE", "Function definition ends", current_scope);
          add_ir_operation("FUNC_END", current_scope, "");
          finish_function_body();
      }
    ;

param_list_opt
    :
    | param_list
    ;

param_list
    : param
    | param_list COMMA param
    ;

param
    : type_spec IDENTIFIER
      {
          add_pending_param($1, $2);
      }
    ;

stmt_list
    :
    | stmt_list statement
    ;

statement
    : NEWLINE
    | set_stmt NEWLINE
    | say_stmt NEWLINE
    | give_stmt NEWLINE
    | call_stmt NEWLINE
    | when_stmt
    | repeat_stmt
    | while_stmt
    ;

set_stmt
    : SET type_spec IDENTIFIER TO expr
      {
          Symbol *local = sym_lookup_local($3, current_scope);
          Expr *rhs = EXPR($5);
          char *rhs_code;
          char desc[256];

          if (!local) {
              sym_add($3, $2, current_scope);
              rhs_code = assignment_rhs_code($2, rhs, $3);
              emit_line("%s %s = %s;", ctype_name($2), $3, rhs_code);
              snprintf(desc, sizeof(desc), "Declare and init %s as %s", $3, type_name($2));
              add_ast_node("SET", desc, rhs->code);
              add_ir_operation("ASSIGN", rhs->code, $3);
              free(rhs_code);
          } else {
              fprintf(stderr,
                      "[Semantic Warning] Line %d: Variable '%s' already declared in scope '%s'; treated as assignment.\n",
                      line_num,
                      $3,
                      current_scope);
              rhs_code = assignment_rhs_code(local->type, rhs, $3);
              emit_line("%s = %s;", $3, rhs_code);
              snprintf(desc, sizeof(desc), "Assign to %s", $3);
              add_ast_node("SET", desc, rhs->code);
              add_ir_operation("ASSIGN", rhs->code, $3);
              free(rhs_code);
          }
      }
    | SET IDENTIFIER TO expr
      {
          Symbol *s = sym_lookup_visible($2, current_scope);
          Expr *rhs = EXPR($4);
          char *rhs_code;
          int inferred;
          char desc[256];

          if (!s) {
              inferred = (rhs->type == TY_UNKNOWN) ? TY_INT : rhs->type;
              sym_add($2, inferred, current_scope);
              rhs_code = assignment_rhs_code(inferred, rhs, $2);
              emit_line("%s %s = %s;", ctype_name(inferred), $2, rhs_code);
              snprintf(desc, sizeof(desc), "Declare and init %s (inferred %s)", $2, type_name(inferred));
              add_ast_node("SET", desc, rhs->code);
              add_ir_operation("ASSIGN", rhs->code, $2);
              free(rhs_code);
          } else {
              rhs_code = assignment_rhs_code(s->type, rhs, $2);
              emit_line("%s = %s;", $2, rhs_code);
              snprintf(desc, sizeof(desc), "Assign to %s", $2);
              add_ast_node("SET", desc, rhs->code);
              add_ir_operation("ASSIGN", rhs->code, $2);
              free(rhs_code);
          }
      }
    ;

say_stmt
    : SAY expr
      {
          Expr *e = EXPR($2);
          char desc[256];
          if (e->type == TY_TEXT) {
              emit_line("printf(\"%%s\\n\", %s);", e->code);
              snprintf(desc, sizeof(desc), "Print text expression");
          } else if (e->type == TY_FLOAT) {
              emit_line("printf(\"%%.4g\\n\", %s);", e->code);
              snprintf(desc, sizeof(desc), "Print float expression");
          } else {
              emit_line("printf(\"%%d\\n\", %s);", e->code);
              snprintf(desc, sizeof(desc), "Print int expression");
          }
          add_ast_node("SAY", desc, e->code);
          add_ir_operation("PRINT", e->code, "stdout");
      }
    ;

when_stmt
    : WHEN cond_or NEWLINE
      {
          Expr *cond = EXPR($2);
          emit_line("if (%s) {", cond->code);
          add_ast_node("WHEN", "Conditional branch", cond->code);
          add_ir_operation("BRANCH", cond->code, "then_block");
          indent_level++;
      }
      stmt_list else_opt END_WHEN
      {
          indent_level--;
          emit_line("}");
      }
    ;

else_opt
    :
    | OTHERWISE NEWLINE
      {
          indent_level--;
          emit_line("} else {");
          indent_level++;
      }
      stmt_list
    ;

repeat_stmt
    : REPEAT expr TIMES NEWLINE
      {
          Expr *n = EXPR($2);
          int id = ++repeat_counter;
          char idx[32];
          char *n_code = cast_code(n->type, TY_INT, n->code);
          char desc[256];

          snprintf(idx, sizeof(idx), "__r%d", id);
          push_repeat_idx(idx);
          emit_line("for (int %s = 0; %s < %s; ++%s) {", idx, idx, n_code, idx);
          snprintf(desc, sizeof(desc), "Loop %s times", n->code);
          add_ast_node("REPEAT", desc, n_code);
          add_ir_operation("LOOP", n_code, idx);
          free(n_code);
          indent_level++;
      }
      stmt_list END_REPEAT
      {
          indent_level--;
          emit_line("}");
          pop_repeat_idx();
      }
    ;

while_stmt
    : WHILE cond_or NEWLINE
      {
          Expr *cond = EXPR($2);
          emit_line("while (%s) {", cond->code);
          add_ast_node("WHILE", "Loop while condition", cond->code);
          add_ir_operation("WHILE", cond->code, "loop_body");
          indent_level++;
      }
      stmt_list END_WHILE
      {
          indent_level--;
          emit_line("}");
      }
    ;

give_stmt
    : GIVE expr
      {
          Expr *e = EXPR($2);
          char desc[256];
          if (!current_fn) {
              semantic_error("'give' is only valid inside function definitions");
          }
          merge_function_return(current_fn, e->type);
          emit_line("return %s;", e->code);
          snprintf(desc, sizeof(desc), "Return from %s", current_fn ? current_fn->name : "?");
          add_ast_node("GIVE", desc, e->code);
          add_ir_operation("RETURN", e->code, current_fn ? current_fn->name : "");
      }
    ;

call_stmt
    : call_expr
      {
          Expr *call = EXPR($1);
          emit_line("%s;", call->code);
          add_ast_node("CALL", "Function call", call->code);
      }
    ;

call_expr
    : CALL IDENTIFIER LPAREN arg_list_opt RPAREN
      {
          $$ = PTR(build_call_expr($2, ARGS($4)));
      }
    ;

arg_list_opt
    :
      {
          $$ = PTR(arglist_new());
      }
    | arg_list
      {
          $$ = $1;
      }
    ;

arg_list
    : expr
      {
          ArgList *a = arglist_new();
          arglist_push(a, EXPR($1));
          $$ = PTR(a);
      }
    | arg_list COMMA expr
      {
          ArgList *a = ARGS($1);
          arglist_push(a, EXPR($3));
          $$ = PTR(a);
      }
    ;

type_spec
    : NUM     { $$ = TY_INT; }
    | DECIMAL { $$ = TY_FLOAT; }
    | TEXT    { $$ = TY_TEXT; }
    ;

cond_or
        : cond_or OR cond_and
            {
                    $$ = PTR(build_logic(EXPR($1), "||", EXPR($3)));
            }
        | cond_and
            {
                    $$ = $1;
            }
        ;

cond_and
        : cond_and AND cond_not
            {
                    $$ = PTR(build_logic(EXPR($1), "&&", EXPR($3)));
            }
        | cond_not
            {
                    $$ = $1;
            }
        ;

cond_not
        : NOT cond_not
            {
                    $$ = PTR(build_not(EXPR($2)));
            }
        | cond_atom
            {
                    $$ = $1;
            }
        ;

cond_atom
        : comparison
            {
                    $$ = $1;
            }
    | LPAREN cond_or RPAREN
            {
                    $$ = $2;
            }
        ;

comparison
    : expr IS GREATER THAN expr
      {
                    $$ = PTR(build_comparison(EXPR($1), REL_GT, EXPR($5)));
      }
    | expr IS LESS THAN expr
      {
                    $$ = PTR(build_comparison(EXPR($1), REL_LT, EXPR($5)));
      }
    | expr IS EQUALS expr
      {
                    $$ = PTR(build_comparison(EXPR($1), REL_EQ, EXPR($4)));
      }
    ;

expr
    : expr PLUS term
      {
                    $$ = PTR(build_numeric_bin(EXPR($1), OP_ADD, EXPR($3)));
      }
    | expr MINUS term
      {
                    $$ = PTR(build_numeric_bin(EXPR($1), OP_SUB, EXPR($3)));
      }
    | term
      {
          $$ = $1;
      }
    ;

term
    : term STAR factor
      {
                    $$ = PTR(build_numeric_bin(EXPR($1), OP_MUL, EXPR($3)));
      }
    | term SLASH factor
      {
                    $$ = PTR(build_numeric_bin(EXPR($1), OP_DIV, EXPR($3)));
      }
    | factor
      {
          $$ = $1;
      }
    ;

factor
    : MINUS factor %prec UMINUS
      {
          Expr *inner = EXPR($2);
          if (inner->type != TY_INT && inner->type != TY_FLOAT) {
              semantic_error("Unary '-' requires a numeric operand");
          }
          if (inner->is_const_numeric) {
              double v = -inner->const_num;
              if (inner->type == TY_FLOAT) {
                  $$ = PTR(expr_new(TY_FLOAT, xprintf("%.12g", v), 1, v));
              } else {
                  $$ = PTR(expr_new(TY_INT, xprintf("%d", (int)v), 1, (double)(int)v));
              }
          } else {
              $$ = PTR(expr_new(inner->type, xprintf("(-(%s))", inner->code), 0, 0.0));
          }
      }
    | primary
      {
          $$ = $1;
      }
    ;

primary
    : INT_LITERAL
      {
          $$ = PTR(expr_new(TY_INT, xprintf("%d", $1), 1, (double)$1));
      }
    | FLOAT_LITERAL
      {
          $$ = PTR(expr_new(TY_FLOAT, xprintf("%.12g", $1), 1, $1));
      }
    | STRING_LITERAL
      {
          $$ = PTR(expr_new(TY_TEXT, $1, 0, 0.0));
      }
    | IDENTIFIER
      {
          const char *loop_idx = NULL;
          if (strcmp($1, "i") == 0) {
              loop_idx = current_repeat_idx();
          }

          if (loop_idx) {
              $$ = PTR(expr_new(TY_INT, loop_idx, 0, 0.0));
          } else {
              Symbol *s = sym_lookup_visible($1, current_scope);
              if (!s) {
                  semantic_error("Variable '%s' used before declaration", $1);
              }
              $$ = PTR(expr_new(s->type, $1, 0, 0.0));
          }
      }
    | call_expr
      {
          $$ = $1;
      }
    | LPAREN expr RPAREN
      {
          $$ = $2;
      }
    ;

%%

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
        print_ast();
        print_intermediate_code();
        write_analysis_report_file();
    }

    if (run_flag) {
        return run_generated_program();
    }

    return 0;
}
