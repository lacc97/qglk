static char RCSid[] =
"$Header: d:/cvsroot/tads/TADS2/unix/os0tr_un.c,v 1.2 1999/05/17 02:52:20 MJRoberts Exp $";

/* Copyright (c) 1992, 2002 Michael J. Roberts.  All Rights Reserved. */
/*
Name
  os0tc.c - os mainline for tads2 runtime
Function
  invokes runtime from operating system command line
Notes
  none
Modified
  04/02/92 MJRoberts     - creation
*/

int main(argc, argv)
int   argc;
char *argv[];
{
    int trdmain(/*_ int argc, char *argv[] _*/);
    int err;

    os_init(&argc, argv, (char *)0, (char *)0, 0);
    err = os0main2(argc, argv, trdmain, "", "config.tr", 0);
    os_uninit();
    os_term(err);
}
