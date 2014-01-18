#include <sys/string.h>
#include <defs.h>

int strcmp(const char *s1,const char *s2)
{
       while (*s1 == *s2++) {
          if (*s1++ == 0){ 
               return (0);
                 }
            }
        return (*(unsigned char *) s1 - *(unsigned char *) --s2);
}

char * strcpy(char *destination, const char *source) {
{
    char *ret = destination;
    while ((*destination++ = *source++))
        {;}
    return ret;
}
}

char *strncpy(char *dest, const char *src, int n)
{
    char *ret = dest;
    do {
        if (!n--)
            return ret;
    } while ((*dest++ = *src++));
    while (n--)
        *dest++ = 0;
    return ret;
}
//Return substring starting from character c
char *strchr(const char *s, int c)
{
    while (*s != (char)c)
        if (!*s++)
            return 0;
    return (char *)s;
}

int strlen(const char *str)
{
        const char *s;

        for (s = str; *s; ++s)
                ;
        return (s - str);
}


int strncmp(const char *s1, const char *s2, int n)
 {
     unsigned char uc1, uc2;
     /* Nothing to compare?  Return zero.  */
     if (n == 0)
         return 0;
     /* Loop, comparing bytes.  */
     while (n-- > 0 && *s1 == *s2) {
         /* If we've run out of bytes or hit a null, return zero
            since we already know *s1 == *s2.  */
         if (n == 0 || *s1 == '\0')
             return 0;
         s1++;
         s2++;
     }
     uc1 = (*(unsigned char *) s1);
     uc2 = (*(unsigned char *) s2);
     return ((uc1 < uc2) ? -1 : (uc1 > uc2));
 }
 
 
char *strstr(const char *haystack, const char *needle)
 {
     int needlelen;
     /* Check for the null needle case.  */
     if (*needle == '\0')
         return (char *) haystack;
     needlelen = strlen(needle);
     for (; (haystack = strchr(haystack, *needle)) != NULL; haystack++)
         if (strncmp(haystack, needle, needlelen) == 0)
             return (char *) haystack;
     return NULL;
 }

int starts_with(const char * base, const char *prefix) //returns 1 if match is found
{
    while(*prefix)
    {
        if(*prefix++ != *base++)
            return 0;
    }
    return 1;
}

int indexOf (const char* base, const char* str) {
        return indexOf_shift(base, str, 0);
}

int indexOf_shift(const char* base, const char* str, int startIndex) {
        int result;
        int baselen = strlen(base);
        // str should not longer than base
        if (strlen(str) > baselen || startIndex > baselen) {
                result = -1;
        } else {
                if (startIndex < 0 ) {
                        startIndex = 0;
                }
                char* pos = strstr(base+startIndex, str);
                if (pos == NULL) {
                        result = -1;
                } else {
                        result = pos - base;
                }
        }
        return result;
}

int lastIndexOf (const char* base, const char* str) {
        int result;
        // str should not longer than base
        if (strlen(str) > strlen(base)) {
                result = -1;
        } else {
                int start = 0;
                int endinit = strlen(base) - strlen(str);
                int end = endinit;
                int endtmp = endinit;
                while(start != end) {
                        start = indexOf_shift(base, str, start);
                        end = indexOf_shift(base, str, end);

                        // not found from start
                        if (start == -1) {
                                end = -1; // then break;
                        } else if (end == -1) {
                                // found from start
                                // but not found from end
                                // move end to middle
                                if (endtmp == (start+1)) {
                                        end = start; // then break;
                                } else {
                                        end = endtmp - (endtmp - start) / 2;
                                        if (end <= start) {
                                                end = start+1;
                                        }
                                        endtmp = end;
                                }
                        } else {
                                // found from both start and end
                                // move start to end and
                                // move end to base - strlen(str)
                                start = end;
                                end = endinit;
                        }
                }
                result = start;
        }
        return result;
}	

char *substring(char* dest , const char *s, int startIndex, int endIndex)
{
  //char* result = dest;
  /* check for null s */
  if (NULL == s)
    return NULL;

  if (startIndex > endIndex)
    return NULL;
	
 
  /* n < 0 or m < 0 is invalid */
  if (startIndex < 0 || endIndex < 0)
    return NULL;
	
  int i = 0;
  for ( ; endIndex-startIndex > 0; startIndex++, i++)
    if (*(s+startIndex) != '\0'){
	  *(dest+i) = *(s+startIndex);	
	}else{
	  break;
	}
  *(dest+i) = 0;
  return dest;
}