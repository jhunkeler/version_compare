#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define GT 1 << 1
#define LT 1 << 2
#define EQ 1 << 3
#define NOT 1 << 4

/**
 * Remove leading whitespace from string
 * @param s input string
 * @return pointer to string
 */
char *lstrip(char **s) {
    char *orig;
    char *end;

    orig = (*s);
    if (!isblank(*(*s)) || *(*s) == '\0') {
        return (*s);
    }

    while (isblank(*(*s)) && *(*s) != '\0') {
        end = (*s);
        (*s)++;
    }
    memmove(orig, (*s), strlen(end));

    (*s) = orig;
    return (*s);
}

/**
 * Remove tailing whitespace from string
 * @param s input string
 * @return pointer to string
 */
char *rstrip(char **s) {
    char *orig;
    char *end;

    orig = (*s);
    if (strlen((*s)) > 1) {
        (*s) = (*s) + strlen((*s)) - 1;
    } else {
        return (*s);
    }

    if (!isblank(*(*s)) || *(*s) == '\0') {
        (*s) = orig;
        return (*s);
    }

    while (isblank(*(*s)) && *(*s) != '\0') {
        end = (*s);
        (*s)--;
    }
    *end = '\0';

    (*s) = orig;
    return (*s);
}

/**
 * Reduces multiple whitespace between characters
 * @param s input string
 * @return pointer to string
 */
char *collapse_whitespace(char **s) {
    char *orig;
    char *space;

    lstrip(s);
    rstrip(s);

    space = NULL;
    orig = (*s);
    while (*(*s) != '\0') {
        char *space_next;

        if (space) {
            int numspace = 0;
            while (isblank(*space) && *space != '\0') {
                numspace++;
                space++;
            }

            //memmove((*s), space, numspace);
            memmove((*s), space, strlen(space) + numspace);
            (*s)[strlen((*s))] = '\0';
            space = NULL;
        }

        space_next = (*s) + 1;
        if (isblank(*(*s)) && (space_next != NULL && isblank(*space_next))) {
            space = (*s);
            (*s)++;
            continue;
        }

        (*s)++;
    }
    (*s) = orig;
    return (*s);
}

/**
 * Sum each part of a '.'-delimited version string
 * @param str version string
 * @return sum of each part
 */
int version_sum(const char *str) {
    int result;
    char *s, *ptr, *end;

    result = 0;
    s = strdup(str);
    ptr = s;
    end = ptr;

    // Parsing stops at the first non-alpha, non-'.' character
    // Digits are processed until the first invalid character
    // I'm torn whether this should be considered an error
    while (end != NULL) {
        result += (int) strtoul(ptr, &end, 10);
        ptr = end;
        if (*ptr == '.')
            ptr++;
        else if (isalpha(*ptr)) {
            result += *ptr - ('a' - 1);
            ptr++;
        }
        else
            end = NULL;
    }

    free(s);
    return result;
}

/**
 * Convert version operator(s) to flags
 * @param str input string
 * @return operator flags
 */
int version_parse_operator(char *str) {
    const char *valid = "><=!";
    char *pos;
    char *strp;
    int result;

    strp = str;
    pos = strp;
    result = 0;
    while ((pos = strpbrk(pos, valid)) != NULL) {
        switch (*pos) {
            case '>':
                result |= GT;
                break;
            case '<':
                result |= LT;
                break;
            case '=':
                result |= EQ;
                break;
            case '!':
                result |= NOT;
                break;
        }
        strp = pos++;
    }

    return result;
}

/**
 * Compare version strings based on flag(s)
 * @param flags verison operators
 * @param aa version1
 * @param bb version2
 * @return 1 flag operation is true
 * @return 0 flag operation is false
 */
int version_compare(int flags, const char *aa, const char *bb) {
    int result, result_a, result_b;

    result_a = version_sum(aa);
    result_b = version_sum(bb);
    result = 0;

    if (flags & GT && flags & EQ)
        result |= result_a >= result_b;
    else if (flags & LT && flags & EQ)
        result |= result_a <= result_b;
    else if (flags & NOT && flags & EQ)
        result |= result_a != result_b;
    else if (flags & GT)
        result |= result_a > result_b;
    else if (flags & LT)
        result |= result_a < result_b;
    else if (flags & EQ)
        result |= result_a == result_b;

    return result;
}

void usage(char *prog) {
    char *name;
    name = strrchr(prog, '/');
    if (!name)
        name = prog;
    else
        name++;

    char *examples[] = {
            "{v} execution example:\n",
            "    %s \"1.2.3 >  1.2.3\"\n",
            "    0\n",
            "    %s \"1.2.3 >= 1.2.3\"\n",
            "    1\n",
            "    %s \"1.2.3 <  1.2.3\"\n",
            "    0\n",
            "    %s \"1.2.3 <= 1.2.3\"\n",
            "    1\n",
            "    %s \"1.2.3 != 1.2.3\"\n",
            "    0\n",
            "    %s \"1.2.3 =  1.2.3\"\n",
            "    1\n",
            "\n",
            "{v1} {operator} {v2} execution example:\n",
            "    %s \"1.2.3\" \">\"  \"1.2.3\"\n",
            "    0\n",
            "    %s \"1.2.3\" \">=\" \"1.2.3\"\n",
            "    1\n",
            "    %s \"1.2.3\" \"<\"  \"1.2.3\"\n",
            "    0\n",
            "    %s \"1.2.3\" \"<=\" \"1.2.3\"\n",
            "    1\n",
            "    %s \"1.2.3\" \"!=\" \"1.2.3\"\n",
            "    0\n",
            "    %s \"1.2.3\" \"=\"  \"1.2.3\"\n",
            "    1\n",
            NULL,
    };

    printf("usage: %s {{v} | {v1} {operator} {v2}}\n", name);
    for (int i = 0; examples[i] != NULL; i++) {
        char *output;

        output = examples[i];
        if (strchr(examples[i], '%')) {
            printf(output, name);
        } else {
            printf("%s", output);
        }
    }
    puts("");
}

int main(int argc, char *argv[]) {
    int result, op, must_free;
    char *v1, *v2, *operator, *arg, *arg_orig, *token;
    char *tokens[4] = {NULL, NULL, NULL, NULL};

    if (argc < 2) {
        fprintf(stderr, "Not enough arguments.\n");
        usage(argv[0]);
        exit(1);
    }

    must_free = 0;
    if (argc < 3) {
        int i;
        i = 0;
        arg = strdup(argv[1]);
        arg_orig = arg;
        collapse_whitespace(&arg);

        for (; (token = strsep(&arg, " ")) != NULL; i++) {
            tokens[i] = strdup(token);
        }
        arg = arg_orig;

        if (i < 3) {
            fprintf(stderr, "Invalid version spec (missing whitespace or token?): '%s'\n", argv[1]);
            usage(argv[0]);
            exit(-2);
        }

        v1 = tokens[0];
        operator = tokens[1];
        v2 = tokens[2];
        must_free = 1;
    } else {
        collapse_whitespace(&argv[1]);
        collapse_whitespace(&argv[2]);
        collapse_whitespace(&argv[3]);
        v1 = argv[1];
        operator = argv[2];
        v2 = argv[3];
    }

    op = version_parse_operator(operator);
    if (op < 0) {
        fprintf(stderr, "Invalid operator sequence: %s\n", operator);
        exit(1);
    }

    result = version_compare(op, v1, v2);
    printf("%d\n", result);

    if (must_free) {
        for (int i = 0; tokens[i] != NULL; i++) {
            free(tokens[i]);
        }
        free(arg);
    }
    return 0;
}
