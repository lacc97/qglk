/* 
 *   Copyright (c) 2001, 2002 Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  test_regex.cpp - regular expression parser tester
Function
  Simple interactive regular expression tester.  Asks for patterns and
  strings to test from the keyboard and displays the results.
Notes
  
Modified
  11/11/01 MJRoberts  - Creation
*/

#include <stdlib.h>
#include "vmregex.h"
#include "t3test.h"

int main()
{
    CRegexParser regex;
    CRegexSearcherSimple searcher(&regex);

    /* initialize for testing */
    test_init();

    /* read patterns and match strings interactively */
    for (;;)
    {
        char pat[128];
        char str[128];

        printf("Enter pattern: ");
        if (gets(pat) == 0)
            break;
        for (;;)
        {
            int idx;
            int reslen;
            
            printf("Enter string:  ");
            if (gets(str) == 0
                || str[0] == '\0')
                break;

            /* match it */
            idx = searcher.compile_and_search(pat, strlen(pat),
                                              str, strlen(str), &reslen);
            if (idx == -1)
                printf("[Not found]\n");
            else
                printf("Found: index=%d, %.*s\n", idx, reslen, str + idx);
        }
    }

    return 0;
}
