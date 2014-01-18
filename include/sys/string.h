#ifndef STRING_H
#define STRING_H

int strcmp(const char *s1, const char *s2);
char* strcpy(char *destination, const char *source);
char* strncpy(char *destination, const char *source, int n);
char *strchr(const char *s, int c);
int lastIndexOf (const char* base, const char* str);
int strncmp(const char *s1, const char *s2, int n);
int indexOf_shift(const char* base, const char* str, int startIndex);
int strlen(const char *str);
int starts_with(const char * base, const char *prefix);
char *substring(char* dest , const char *s, int startIndex, int endIndex);

#endif