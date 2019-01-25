static char RCSid[] =
"$Header: d:/cvsroot/tads/TADS2/unix/os0td_un.c,v 1.2 1999/05/17 02:52:20 MJRoberts Exp $";

/* Copyright (c) 1992, 2002 Michael J. Roberts.  All Rights Reserved. */
/*
Name
  os0td.c - os mainline for tads2 debugger
Function
  invokes debugger from operating system command line
Notes
  none
Modified
  04/04/92 MJRoberts     - creation
*/

int main(argc, argv)
int   argc;
char *argv[];
{

    int tddmain(/*_ int argc, char *argv[] _*/);
    int err;
    extern int unix_tdb;

    unix_tdb = 1;
    os_init(&argc, argv, (char *)0, (char *)0, 0);
    err = os0main2(argc, argv, tddmain, "i", "config.tdb", 0);
    os_uninit();
    os_term(err);
}
