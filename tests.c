#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "version_compare.h"

struct TestCase_strings {
    char *s;
    const char *result;
};

static struct TestCase_strings test_cases_collapse_whitespace[] = {
    {"", ""},
    {" ", ""},
    {"  ", ""},
    {" leading", "leading"},
    {"trailing ", "trailing"},
    {"         leading", "leading"},
    {"         leading and trailing         ", "leading and trailing"},
    {"This  line   will    be     collapsed", "This line will be collapsed"},
};

static struct TestCase_strings test_cases_lstrip[] = {
    {"", ""},
    {" ", ""},
    {"  ", ""},
    {" leading", "leading"},
    {"trailing ", "trailing "},
    {"         leading", "leading"},
    {"         leading and trailing         ", "leading and trailing         "},
    {"This  line   will    be     collapsed", "This  line   will    be     collapsed"},
};

static struct TestCase_strings test_cases_rstrip[] = {
    {"", ""},
    {" ", ""},
    {"  ", ""},
    {" leading", " leading"},
    {"trailing ", "trailing"},
    {"         leading", "         leading"},
    {"         leading and trailing         ", "         leading and trailing"},
    {"This  line   will    be     collapsed", "This  line   will    be     collapsed"},
};

struct TestCase_version_compare {
    char *a, *op, *b;
    int result;
};

static struct TestCase_version_compare test_cases_version_compare[] = {
    {"0", "=", "0", 1},
    {"0", "<",  "1",1},
    {"0", "<=",  "1",1},
    {"0", ">",  "1",0},
    {"0", ">=",  "1",0},
    {"0", "!=",  "1",1},

    {"1a", "=", "1b", 0},
    {"1a", "<", "1b", 1},
    {"1a", "<=", "1b", 1},
    {"1a", ">", "1b", 0},
    {"1a", ">=", "1b", 0},
    {"1a", "!=", "1b", 1},

    {"1.0", "=", "1.0.0", 1},
    {"1.0", "<", "1.0.0", 0},
    {"1.0", "<=", "1.0.0", 1},
    {"1.0", ">", "1.0.0", 0},
    {"1.0", ">=", "1.0.0", 1},
    {"1.0", "!=", "1.0.0", 0},

    {"1.0a", "=", "1.0.0", 0},
    {"1.0a", "<", "1.0.0", 0},
    {"1.0a", "<=", "1.0.0", 0},
    {"1.0a", ">", "1.0.0", 1},
    {"1.0a", ">=", "1.0.0", 1},
    {"1.0a", "!=", "1.0.0", 1},

    {"2022.1", "=", "2022.4", 0},
    {"2022.1", "<", "2022.4", 1},
    {"2022.1", "<=", "2022.4", 1},
    {"2022.1", ">", "2022.4", 0},
    {"2022.1", ">=", "2022.4", 0},
    {"2022.1", "!=", "2022.4", 1},

    {"2022.4", "=", "2022.1", 0},
    {"2022.4", "<", "2022.1", 0},
    {"2022.4", "<=", "2022.1", 0},
    {"2022.4", ">", "2022.1", 1},
    {"2022.4", ">=", "2022.1", 1},
    {"2022.4", "!=", "2022.1", 1},
};

static struct TestCase_version_compare error_cases_version_compare[] = {
        // nonsense++
        {"", "=", "", -1},
        {" ", "=", "     ", -1},
        {"a", "", "a", -1},
        {"a", "", "b", -1},
        {"a", "@", "b", -1},
};

static int run_cases_version_compare(struct TestCase_version_compare tests[], size_t size) {
    int failed = 0;
    for (size_t i = 0; i < size; i++) {
        int result = 0;
        struct TestCase_version_compare *test = &tests[i];
        int op = version_parse_operator(test->op);
        result = version_compare(op, test->a, test->b);

        printf("%s %s %s is %s", test->a, test->op, test->b, result ? "TRUE" : "FALSE" );
        if (test->result != result) {
            printf("    [FAILED: got %d, expected %d]\n", result, test->result);
            failed++;
        } else {
            puts("");
        }
    }
    return failed;
}

typedef char *(*strfn) (char **s);

static int run_cases_string(struct TestCase_strings tests[], size_t size, strfn fn) {
    int failed = 0;
    for (size_t i = 0; i < size; i++) {
        int result = 0;
        struct TestCase_strings *test = &tests[i];
        char *s = strdup(test->s);

        fn(&s);
        result = strcmp(test->result, s);

        printf("'%s' is %s", s, !result ? "CORRECT" : "INCORRECT" );
        if (result) {
            printf("    [FAILED: got '%s', expected '%s']\n", s, test->result);
            failed++;
        } else {
            puts("");
        }
        free(s);
    }
    return failed;
}


char **make_argv(int *argc, char *argv[]) {
    char **args;
    *argc = 0;

    for (size_t i = 0; argv[i] != NULL; i++) {
        *argc = *argc + 1;
    }

    //char **args = calloc(*argc + 1, sizeof(*argv));
    args = calloc(*argc + 1, sizeof(*argv));
    if (!args) {
        perror("unable to allocate args array");
        return NULL;
    }

    args[0] = strdup("version_compare");
    if (!args[0]) {
        perror("unable to allocate mock program name");
        return NULL;
    }

    for (int i = 0; i < *argc; i++) {
        args[i + 1] = strdup(argv[i]);
        if (!args[i + 1]) {
            perror("unable to allocate argument in array");
            return NULL;
        }
    }
    *argc = *argc + 1;
    return args;
}

void free_argv(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);
}

int run_program(char *argv[]) {
    int result = 0;
    int argc = 0;
    char **args = NULL;

    args = make_argv(&argc, argv);
    if (!args) {
        perror("make_argv failed");
        return -1;
    }
    const char *filename = "stdout.log";
    char data[255] = {0};
    char *end = NULL;
    int o_stdout;
    int save_stdout;


    o_stdout = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (o_stdout == -1) {
        perror("unable to open stdout log");
        goto run_program_failed;
    }

    save_stdout = dup(fileno(stdout));
    if (save_stdout == -1) {
        perror("unable to duplicate stdout");
        goto run_program_failed;
    }

    fflush(stdout);
    if (dup2(o_stdout, fileno(stdout)) == -1) {
        perror("unable to redirect stdout");
        goto run_program_failed;
    }

    result = entry(argc, args);

    fflush(stdout);
    close(o_stdout);

    if (dup2(save_stdout, fileno(stdout)) == -1) {
        free_argv(argc, args);
        perror("unable to restore stdout");
        goto run_program_failed;
    }
    close(save_stdout);

    if (result < 0) {
        free_argv(argc, args);
        return result;
    }

    if (access(filename, F_OK) < 0) {
        perror(filename);
        goto run_program_failed;
    }

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("unable to open output log file");
        goto run_program_failed;
    }

    if ((fgets(data, sizeof(data) - 1, fp)) == NULL) {
        fprintf(stderr, "no output recorded in main program execution\n");
        remove(filename);
        fclose(fp);
        goto run_program_failed;
    }

    result = (int)strtol(data, &end, 10);
    if (!end) {
        fprintf(stderr, "unexpected error in main program execution\n");
        remove(filename);
        fclose(fp);
        goto run_program_failed;
    }

    fclose(fp);
    remove(filename);
    free_argv(argc, args);
    return result;

run_program_failed:
    free_argv(argc, args);
    return -1;
}

static int run_cases_program_split(struct TestCase_version_compare tests[], size_t size) {
    int failed = 0;
    for (size_t i = 0; i < size; i++) {
        int result = 0;
        struct TestCase_version_compare *test = &tests[i];
        result = run_program((char *[]){test->a, test->op, test->b, NULL});

        printf("%s %s %s is %s", test->a, test->op, test->b, result ? "TRUE" : "FALSE" );
        if (test->result != result) {
            printf("    [FAILED: got %d, expected %d]\n", result, test->result);
            failed++;
        } else {
            puts("");
        }
    }
    return failed;
}

static int run_cases_program_standalone(struct TestCase_version_compare tests[], size_t size) {
    int failed = 0;
    for (size_t i = 0; i < size; i++) {
        int result = 0;
        struct TestCase_version_compare *test = &tests[i];
        char data[255] = {0};

        snprintf(data, sizeof(data) - 1, "%s %s %s", test->a, test->op, test->b);
        result = run_program((char *[]){data, NULL});

        printf("%s %s %s is %s", test->a, test->op, test->b, result ? "TRUE" : "FALSE" );
        if (test->result != result) {
            printf("    [FAILED: got %d, expected %d]\n", result, test->result);
            failed++;
        } else {
            puts("");
        }
    }
    return failed;
}

int main() {
    int failed = 0;

    printf("\nTEST version_compare()\n");
    failed += run_cases_version_compare(test_cases_version_compare,
                                        sizeof(test_cases_version_compare) / sizeof(test_cases_version_compare[0]));
    printf("\nTEST version_compare errors()\n");
    failed += run_cases_version_compare(error_cases_version_compare,
                                        sizeof(error_cases_version_compare) / sizeof(error_cases_version_compare[0]));
    printf("\nTEST collapse_whitespace()\n");
    failed += run_cases_string(test_cases_collapse_whitespace,
                               sizeof(test_cases_collapse_whitespace) / sizeof(test_cases_collapse_whitespace[0]),
                               &collapse_whitespace);
    printf("\nTEST lstrip()\n");
    failed += run_cases_string(test_cases_lstrip,
                               sizeof(test_cases_lstrip) / sizeof(test_cases_lstrip[0]),
                               &lstrip);
    printf("\nTEST rstrip()\n");
    failed += run_cases_string(test_cases_rstrip,
                               sizeof(test_cases_rstrip) / sizeof(test_cases_rstrip[0]),
                               &rstrip);

    printf("\nTEST main program entry point (split string)\n");
    failed += run_cases_program_split(test_cases_version_compare,
                                      sizeof(test_cases_version_compare) / sizeof(test_cases_version_compare[0]));

    printf("\nTEST main program entry point errors (split string)\n");
    failed += run_cases_program_split(error_cases_version_compare,
                                      sizeof(error_cases_version_compare) / sizeof(error_cases_version_compare[0]));

    printf("\nTEST main program entry point (standalone string)\n");
    failed += run_cases_program_standalone(test_cases_version_compare,
                                           sizeof(test_cases_version_compare) / sizeof(test_cases_version_compare[0]));

    printf("\nTEST main program entry point errors (standalone string)\n");
    failed += run_cases_program_standalone(error_cases_version_compare,
                                        sizeof(error_cases_version_compare) / sizeof(error_cases_version_compare[0]));

    return failed != 0;
}
