static char RCSid[] =
"$Header: d:/cvsroot/tads/TADS2/unix/os0tc_un.c,v 1.2 1999/05/17 02:52:20 MJRoberts Exp $";

/* Copyright (c) 1992, 2002 Michael J. Roberts.  All Rights Reserved. */
/*
Name
  os0tc.c - os mainline for tads2 compiler
Function
  invokes compiler from operating system command line
Notes
  none
Modified
  04/02/92 MJRoberts     - creation
*/

int main(argc, argv)
int   argc;
char *argv[];
{
    int tcdmain(/*_ int argc, char *argv[] _*/);
    int err;
    extern int unix_tc;

    unix_tc = 1;
    os_init(&argc, argv, (char *)0, (char *)0, 0);
    err = os0main(argc, argv, tcdmain, "i", "config.tc");
    os_uninit();
    os_term(err);
}
