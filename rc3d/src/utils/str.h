/*
        DESCRIPTION:
                Handles string utility functions
*/

#ifndef STR_H
#define STR_H


// >> checks if two given string are matched
bool str_matched
(const char *str1, const char *str2);

// >> checks if the subtring exists inside the given string
bool substr_exists
(const char *str, const char *substr);

// >> searches through the given string
// >> returns the index location after the substring, if found
int search_substr
(const char *str, const char *substr);


#endif
