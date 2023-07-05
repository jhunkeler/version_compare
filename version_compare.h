#ifndef VERSION_COMPARE_VERSION_COMPARE_H
#define VERSION_COMPARE_VERSION_COMPARE_H

#define GT 1 << 1
#define LT 1 << 2
#define EQ 1 << 3
#define NOT 1 << 4
#define EPOCH_MOD 100

int isempty(char *str);
char *lstrip(char **s);
char *rstrip(char **s);
char *collapse_whitespace(char **s);
int version_sum(const char *str);
int version_parse_operator(char *str);
int version_compare(int flags, const char *aa, const char *bb);
int entry(int argc, char *argv[]);

#endif //VERSION_COMPARE_VERSION_COMPARE_H