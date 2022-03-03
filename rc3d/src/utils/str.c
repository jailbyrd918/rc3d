#include <string.h>
#include <stdbool.h>

#include "str.h"


bool str_matched
(const char *str1, const char *str2)
{
        return (str1 != NULL) && (str2 != NULL) && (strcmp(str1, str2) == 0);
}

bool substr_exists
(const char *str, const char *substr)
{
        return (str != NULL) && (substr != NULL) && (strstr(str, substr) != NULL);
}

int search_substr
(const char *str, const char *substr)
{
        if ((!str) || (!substr) || (!substr_exists(str, substr)))
                return -1;

        int     foundidx = -1,
                curridx = 0;

        while (str[curridx] != '\0') {
                if (str[curridx] != substr[0])
                        ++curridx;

                // if character at current index matches the first letter of substring
                else {
                        // string to compare
                        char cmpstr[256];
                        size_t cmpstridx = 0;

                        size_t cmplen = 0;
                        while (
                                (cmplen++ < strlen(substr))
                                && (str[curridx] != ' ')
                                && (str[curridx] != '\t')
                                && (str[curridx] != '\r')
                                && (str[curridx] != '\n')
                                && (str[curridx] != '\0')
                                ) {
                                cmpstr[cmpstridx++] = str[curridx++];
                        }
                        cmpstr[cmpstridx] = '\0';

                        if (str_matched(cmpstr, substr)) {
                                foundidx = curridx - (int)strlen(substr);
                                break;
                        }
                }
        }

        return foundidx;
}


