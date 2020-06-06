#
# Makefile for Unix-based TADS 2 ports.
# Please add to this file, following the format, if you do new Unix ports.
#
# Compiling with GCC is strongly recommended.  It may be difficult to
# get stdargs (variable-length argument lists to functions) working
# with other compilers.
#

#
# Uncomment the definitions for your system and comment all the others.
# Check to make sure you've got the right build line at the bottom as well.
# If you are doing a new port, you will almost certainly have to edit
# some of the source files (at least osunixt.h).  In some cases, options
# can be selected by passing options through the UNIXFLAGS variable.
# For example, -DHAVE_STRCASECMP signifies that you have strcasecmp() in
# your system instead of stricmp().
#
# NOTE: If you run into weird display problems with the interpreter (in
# particular, if each line starts with 20 or so spaces), try using these
# definitions:
#   LIBS= -lncurses
#   CFLAGS= -DHAVE_TPARM (plus whatever other CFLAGS you were already using)
#
# Or, if ncurses is the default, try switching to LIBS=-ltermcap and removing
# -DHAVE_TPARM from CFLAGS.
#
# NOTE: Do NOT put optimization flags in the CFLAGS; put them in OPTIMIZE
# instead.  Some files can't be compiled with optimization on all systems
# due to missing volatiles, compiler optimization bugs, and who knows
# what else.
#
# Warning: The Tgetstr library routine does not seem to work quite right
# under some version of SunOS.  In particular, it can get confused by
# a TERMCAP environment variable that contains more than one termcap
# entry.  Perhaps this is actually not incorrect behavior, but emacs
# and vi both work in this case since they have their own (better)
# Tgetstr equivalents.
#
# Build the targets tadsc.static and tadsr.static to get statically
# compiled binaries on systems that default to dynamic linking.  You
# will only want to do this to support people on your Unix system with
# outdated dynamic libraries.  Generally it is not necessary to distribute
# statically linked binaries.
#
# The debugger (tdb) port is currently very half-assed.  To simulate
# the page flipping of two 80x25 screens as in the DOS version, it
# assumes an 80x50 screen and splits it into two halves.  If the window
# you run the debugger in is not exactly 80x50 it will not work properly.
#

# Install points for TADS 2.
#
# BINDIR in the installation directory for the tadsc (compiler), tadsr
# (interpreter) and tdb (debugger) executables.
#
# DATADIR is where the TADS 2 development libraries (adv.t, std.t and
# gameinfo.t) will get installed.
PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
DATADIR=$(PREFIX)/share/tads2

#
# Places to look for include files.
#
INCLUDES=-I.

#
# Current patchlevel of Unix sources.  This applies to all
# machines.  Use -DSYSPL=\"<string>\" for single-machine
# patches.
#
# Note that this is a *string*.
#
# Only change this if you've made a patch that applies to all
# Unix ports.  If you've fixed something in a specific port,
# change SYSPL in the port's SYSFLAGS line.
#
UNIXPATCHLEVEL= -DUNIXPATCHLEVEL=\"1\"

#
# Names of various maintainers
# Add your name here if you do a new Unix port.
#
DAVE=Dave Baggett <dmb@ai.mit.edu>
ADAM=Adam Thornton <adam@phoenix.princeton.edu>
STEPHEN=Stephen Granade <sgranade@phy.duke.edu>
JAY=Jay Glascoe <jglascoe@jay.giss.nasa.gov>
MICHAEL=Michael Vokits <vokitsmi@pilot.msu.edu>
DOUGLAS=Douglas Brebner <kirtai@users.sourceforge.net>
MARKW=Mark Wickham <macwickham@attbi.com>
MATT=Matt Herberg <meherberg@earthlink.net>

#
# IBM AIX with GCC 2.8.1
#
#CC=gcc
#OPTIMIZE=-O2
#DEBUGGING=-v -g
#UNIXFLAGS=-DOSANSI -DUNIX -DUNIXPATCHLEVEL=\"1\" -DSTD_OS_HILITE -DSTD_OSCLS
#SYSFLAGS=-DOS_UCHAR_DEFINED -DHAVE_STRCASECMP -DOS_SYSTEM_NAME=\""IBM_AIX"\" -DSYSPL=\"1\" -DPORTER=\""$(JAY)"\" -DSYSMAINTAINER=\""IBM AIX port maintained by $(JAY)\n"\" -DSYSNAME=\""IBM RS/6000 running AIX"\"
#CFLAGS= $(DEBUGGING) $(UNIXFLAGS) $(SYSFLAGS) -DIBM_AIX
#MACHDEP=cmap.o
#LIBS= -ltermcap
#MAKEXEC=touch

#
# IBM RT with GCC 2.1
# Has no stdlib.h, so you need to make one in the current directory
# that includes stdio.h and sys/types.h
#
# This hasn't been tested since version 2.1, so you will almost
# certainly have to upgrade it.
#
#CC=gcc
#OPTIMIZE=-O2
#UNIXFLAGS=-v -g -DOSANSI -DUNIX
#SYSFLAGS= -DOS_SYSTEM_NAME=\""IBM_RT"\" -DSYSNAME=\""IBM RT"\" -DSYSPL=\"1\" -DPORTER=\""$(DAVE)"\" -DSYSMAINTAINER=\""IBM RT port maintained by $(DAVE)\n"\"
#CFLAGS= $(UNIXFLAGS) $(SYSFLAGS) -DIBM_RT -I.
#AFLAGS=
#OVLFLG=
#MACHDEP=
#LIBS= -ltermcap
#MAKEXEC=touch

#
# SGI running Irix with GCC
#
#CC=gcc
#OPTIMIZE=-O2
#UNIXFLAGS=-v -g -DOSANSI -DUNIX -DHAVE_STRCASECMP
#SYSFLAGS= -DOS_SYSTEM_NAME=\""SGI_IRIX"\" -DSYSNAME=\""SGI running Irix"\" -DSYSPL=\"1\" -DPORTER=\""$(DAVE)"\" -DSYSMAINTAINER=\""SGI Irix port maintained by $(DAVE)\n"\"
#CFLAGS= $(UNIXFLAGS) $(SYSFLAGS) -DSGI_IRIX
#AFLAGS=
#OVLFLG=
#MACHDEP=
#LIBS= -ltermcap
#MAKEXEC=touch

#
# SGI running Irix with CC:
#
#CC=cc -32
#OPTIMIZE=-O2
#UNIXFLAGS=-DOSANSI -DUNIX -DHAVE_STRCASECMP -DOS_MCM_NO_MACRO
#SYSFLAGS= -DOS_SYSTEM_NAME=\""SGI_IRIX"\" -DSYSNAME=\""SGI running Irix"\" -DSYSPL=\"1\" -DPORTER=\""$(DAVE)"\" -DSYSMAINTAINER=\""SGI Irix port maintained by $(DAVE)\n"\"
#CFLAGS= $(UNIXFLAGS) $(SYSFLAGS) -DSGI_IRIX
#AFLAGS=
#OVLFLG=
#MACHDEP=
#LIBS= -ltermcap
#MAKEXEC=touch

#
# SPARCstation (Solaris) with GCC 2.4.5
#
#CC=gcc
#OPTIMIZE=-O2
#UNIXFLAGS=-v -g -DOSANSI -DUNIX -DHAVE_STRCASECMP
#SYSFLAGS= -DOS_SYSTEM_NAME=\""SUN_SPARC_SOLARIS"\" -DSYSNAME=\""Sun Sparc running Solaris"\" -DSYSPL=\"1\" -DPORTER=\""$(STEPHEN)"\" -DSYSMAINTAINER=\""Solaris port maintained by $(STEPHEN)\n"\"
#CFLAGS= $(UNIXFLAGS) $(SYSFLAGS) -DSUN_SPARC_SOLARIS
#AFLAGS=
#OVLFLG=
#MACHDEP=
#LIBS= -ltermcap
#MAKEXEC=touch

#
# SPARCstation running SunOS with GCC 2.4.5
#
#CC=gcc
#OPTIMIZE=-O2
#UNIXFLAGS=-g -DOSANSI -DUNIX
#SYSFLAGS= -DOS_SYSTEM_NAME=\""SUN_SPARC_SUNOS"\" -DSYSNAME=\""Sun Sparc running SunOS"\" -DSYSPL=\"1\" -DPORTER=\""$(DAVE)"\" -DSYSMAINTAINER=\""SunOS port maintained by $(DAVE)\n"\"
#CFLAGS= $(UNIXFLAGS) $(SYSFLAGS) -DSUN_SPARC_SUNOS
#AFLAGS=
#OVLFLG=
#MACHDEP=
#LIBS= -ltermcap
#MAKEXEC=touch

#
# Sun 3 with GCC 2.3.2
#
#CC=gcc
#OPTIMIZE=-O2
#UNIXFLAGS=-v -g -DOSANSI -DUNIX
#SYSFLAGS= -DOS_SYSTEM_NAME=\""SUN_3"\" -DSYSNAME=\""Sun 3 running SunOS"\" -DSYSPL=\"1\" -DPORTER=\""$(DAVE)"\" -DSYSMAINTAINER=\""Sun 3 port maintained by $(DAVE)\n"\"
#CFLAGS= $(UNIXFLAGS) $(SYSFLAGS) -DSUN3
#AFLAGS=
#OVLFLG=
#MACHDEP=
#LIBS= -ltermcap
#MAKEXEC=touch

#
# NeXT (Cube or Station) with NeXTstep cc (which is a modified gcc)
# (may work as well on a PC running NeXTstep)
#
#CC=cc
#OPTIMIZE=-O
#UNIXFLAGS=-v -g -DOSANSI -DUNIX -DHAVE_STRCASECMP
#SYSFLAGS= -DOS_SYSTEM_NAME=\""NeXT"\" -DSYSNAME=\""NeXT"\" -DSYSPL=\"1\" -DPORTER=\""$(ADAM)"\" -DSYSMAINTAINER=\""NeXT port maintained by $(ADAM)\n"\"
#CFLAGS= $(UNIXFLAGS) $(SYSFLAGS) -DNEXT
#AFLAGS=
#OVFLG=
#MACHDEP=
#LIBS= -ltermcap
#MAKEXEC=touch

#
# DECstation (MIPS) running Ultrix with GCC 2.3.3
#
#CC=gcc
#OPTIMIZE=-O2
#UNIXFLAGS=-v -g -DOSANSI -DUNIX -DHAVE_STRCASECMP
#SYSFLAGS= -DOS_SYSTEM_NAME=\""DEC_MIPS_ULTRIX"\" -DSYSNAME=\""DECstation MIPS running Ultrix"\" -DSYSPL=\"1\" -DPORTER=\""$(DAVE)"\" -DSYSMAINTAINER=\""DEC MIPS Ultrix port maintained by $(DAVE)\n"\"
#CFLAGS= $(UNIXFLAGS) $(SYSFLAGS) -DULTRIX_MIPS
#AFLAGS=
#OVLFLG=
#MACHDEP=
#LIBS= -ltermcap
#MAKEXEC=touch

#
# Linux 386 with GCC
#
# -fomit-frame-pointer causes problems with some recent versions (3.0+) of GCC.
# In 2.95.3 it works fine. Uncomment it if you think it's safe and are hoping
# for a performance boost.
#
#CC=gcc
#OPTIMIZE=-O2 -funroll-loops -mcpu=pentiumpro -fno-strict-aliasing # -fomit-frame-pointer
#UNIXFLAGS=-DOSANSI -DUNIX -DHAVE_STRCASECMP
#SYSFLAGS= -DOS_SYSTEM_NAME=\""LINUX_386"\" -DSYSNAME=\""i386+ running Linux"\" -DSYSPL=\"1\" -DPORTER=\""$(STEPHEN)"\" -DSYSMAINTAINER=\""Linux port maintained by $(STEPHEN)\n"\"
#CFLAGS= $(UNIXFLAGS) $(SYSFLAGS) -DHAVE_TPARM -DLINUX_386
#AFLAGS=
#OVLFLG=
#MACHDEP=
#LIBS=-lncurses
#MAKEXEC=touch

#
# FreeBSD 2.1.x
# This has compiled on a system running 2.1.7.1-RELEASE with the Linux
# compatibility libraries installed.  It has not been tested with a bare
# FreeBSD system without the Linux libraries.
#
#CC=gcc
#OPTIMIZE=
#UNIXFLAGS=-g -DOSANSI -DUNIX -DHAVE_STRCASECMP
#SYSFLAGS= -DOS_SYSTEM_NAME=\""FREEBSD_386"\" -DSYSNAME=\""i386+ running FreeBSD"\" -DSYSPL=\"1\" -DPORTER=\""$(DAVE)"\" -DSYSMAINTAINER=\""FreeBSD port maintained by $(DAVE)\n"\"
#CFLAGS= $(UNIXFLAGS) $(SYSFLAGS) -DFREEBSD_386
#AFLAGS=
#OVLFLG=
#MACHDEP=
#LIBS=-ltermcap
#MAKEXEC=touch

# OpenBSD 3.0 with gcc 2.95.3
# This has been compiled on an i386 system running OPENBSD 3.0
# HAVE_TPARM is set in osunixt.h
#
#CC=gcc
#OPTIMIZE=-O2
#UNIXFLAGS=-g -DOSANSI -DUNIX -DHAVE_STRCASECMP
#SYSFLAGS= -DOS_SYSTEM_NAME=\""OPENBSD"\" -DSYSNAME=\""OpenBSD"\" -DSYSPL=\"1\" -DPORTER=\""$(MARKW)"\" -DSYSMAINTAINER=\""OpenBSD port maintained by $(MARKW)\n"\"
#CFLAGS= $(UNIXFLAGS) $(SYSFLAGS) -DOPENBSD
#AFLAGS=
#OVLFLG=
#MACHDEP=
#LIBS=-L/usr/pkg/lib -Wl,-R/usr/pkg/lib -lcurses
#MAKEXEC=touch

#
# NetBSD 1.5
# This has compiled on an i386 system running 1.5_ALPHA2
# No testing has been done on pre 1.5 or a.out systems
# HAVE_TPARM is set in osunixt.h
#
#CC=gcc
#OPTIMIZE=
#UNIXFLAGS=-g -DOSANSI -DUNIX -DHAVE_STRCASECMP
#SYSFLAGS= -DOS_SYSTEM_NAME=\""NETBSD"\" -DSYSNAME=\""NetBSD"\" -DSYSPL=\"1\" -DPORTER=\""$(DOUGLAS)"\" -DSYSMAINTAINER=\""NetBSD port maintained by $(DOUGLAS)\n"\"
#CFLAGS= $(UNIXFLAGS) $(SYSFLAGS) -DNETBSD
#AFLAGS=
#OVLFLG=
#MACHDEP=
#LIBS=-L/usr/pkg/lib -Wl,-R/usr/pkg/lib -lncurses
#MAKEXEC=touch

#
# i386+ running DOS/Windows, using DJGPP 2.X
#
# Be sure to use a recent port of GNU Make (>= 3.69) to get
# around command line length limitations.
#
#
# NOTE: You will need the GNUish versions of cp and rm to
# use this makefile.  You can get these from
#
#   <http://oak.oakland.edu/pub/simtelnet/gnu/gnuish/gnuish.htm>
#
# or
#
#   <ftp://wuarchive.wustl.edu/systems/msdos/gnuish>
#
# Both cp and rm are in the fut312bx.zip archive.
#
#CC=gcc
#OPTIMIZE=-g -O2 -funroll-loops -fomit-frame-pointer -m486
#UNIXFLAGS=-DOSANSI -DUNIX -UMSDOS
#SYSFLAGS= -DOS_SYSTEM_NAME=\"GO32\" -DSYSNAME=\"GO32\" -DSYSPL=\"1\" -DPORTER=\""$(MICHAEL)"\" -DSYSMAINTAINER=\""GO32 port maintained by $(MICHAEL)\n"\"
#CFLAGS= $(UNIXFLAGS) $(SYSFLAGS) -DDJGCC_386
#AFLAGS=
#OVLFLG=
#MACHDEP=biosvid.o
#LIBS=
#MAKEXEC=coff2exe

#
# Mac OS X / Darwin
#
#CC=cc
#OPTIMIZE=-O2
#UNIXFLAGS= -g -DOSANSI -DUNIX -DHAVE_STRCASECMP
#SYSFLAGS= -DOS_SYSTEM_NAME=\""Darwin"\" -DSYSNAME=\""Darwin"\" -DSYSPL=\"1\" -DPORTER=\""$(MATT)"\" -DSYSMAINTAINER=\""Darwin port maintained by $(MATT)\n"\"
#CFLAGS= $(UNIXFLAGS) $(SYSFLAGS) -DDARWIN
#AFLAGS=
#OVFLG=
#MACHDEP=
#LIBS=-lncurses
#MAKEXEC=touch

#
# Don't change stuff below this line unless you really know what you're doing.
#

# library object files
LIBOBJS= ler.o

# objects in common to all executables
UNIXOBJS=

# removed runstat.o from common objects, but added osifc.o
COMMONOBJS= $(LIBOBJS) $(UNIXOBJS) mcm.o mcs.o mch.o obj.o cmd.o \
            errmsg.o fioxor.o os0.o argize.o oserr.o \
	    fio.o getstr.o cmap.o oemunix.o regex.o askf_tx.o indlg_tx.o \
            osifc.o

CMNRUNOBJS= $(COMMONOBJS) dat.o lst.o run.o out.o voc.o bif.o output.o \
	    suprun.o osnoui.o tparm.o osrestad.o

# objects for run-time
# added runstat.o
RUNOBJS= vocab.o execmd.o ply.o qas.o runstat.o

# extra objects for character-mode run-time
CHAROBJS= os0tr_un.o bifgdum.o osgen3.o

# objects for compiler (includes run-time objects)
COMPOBJS= linf.o prs.o prscomp.o emt.o sup.o fiowrt.o tok.o objcomp.o \
	  tokth.o dbg.o voccomp.o bifgdum.o

TCOBJS= $(CMNRUNOBJS) $(COMPOBJS) tcgdum.o osgen3_stdio.o osunixt_stdio.o os0tc_un.o tcd.o

TPOBJS= $(CMNRUNOBJS) \
        linf.o linm.o prs.o emt.o sup.o tok.o fio.o \
        objcomp.o getstr.o $(MACHDEP) osgen3.o osunixt.o tokth.o \
        dbg.o voccomp.o bifgdum.o qas.o tpd.o vocab.o \
        execmd.o dbgrun.o runstat.o ply.o

TCGOBJS= $(COMPOBJS) tcg.o

TROBJS= $(CMNRUNOBJS) $(RUNOBJS) linfdum.o $(CHAROBJS) osunixt.o tputs.o $(MACHDEP) trd.o dbgtr.o

# objects for debugger
DBGOBJS= $(CMNRUNOBJS) $(COMPOBJS) $(RUNOBJS) tcgdum.o tdd.o dbgu.o dbgrun.o

TDBOBJS= $(DBGOBJS) osgen.o osunixt.o tputs.o $(MACHDEP) os0td_un.o

all: tadsr tadsc tdb

install: all
	install -d $(BINDIR)
	install -d $(DATADIR)
	install -s -m755 tadsc tadsr tdb $(BINDIR)
	install -m644 adv.t std.t gameinfo.t $(DATADIR)

tadsr: $(TROBJS)
	$(CC) $(CFLAGS) $(TROBJS) -o tadsr $(LIBS)
	$(MAKEXEC) tadsr

tadsc: $(TCOBJS)
	$(CC) $(CFLAGS) $(TCOBJS) -o tadsc $(LIBS)
	$(MAKEXEC) tadsc

tdb: $(TDBOBJS)
	$(CC) $(CFLAGS) $(TDBOBJS) -o tdb $(LIBS)
	$(MAKEXEC) tdb

tadsr.static: $(TROBJS)
	$(CC) $(CFLAGS) $(TROBJS) -static -o tadsr.static $(LIBS)
	strip tadsr.static

tadsc.static: $(TCOBJS)
	$(CC) $(CFLAGS) $(TCOBJS) -static -o tadsc.static $(LIBS)
	strip tadsc.static

tdb.static: $(TDBOBJS)
	$(CC) $(CFLAGS) $(TDBOBJS) -static -o tdb.static $(LIBS)
	strip tdb.static

# Object files compiled with USE_STDIO. These are used only for the TADS 2 and
# TADS 3 compilers, which do not require a curses-type interface.
osunixt_stdio.o: osunixt.c
	$(CC) -c $(CFLAGS) $(UNIXPATCHLEVEL) $(INCLUDES) -DUSE_STDIO -o $@ osunixt.c
osgen3_stdio.o: osgen3.c
	$(CC) -c $(CFLAGS) $(UNIXPATCHLEVEL) $(INCLUDES) -DUSE_STDIO -o $@ osgen3.c

#
# prscomp.c gets hosed by the optimizer on some machines, so we
# build it without optimization.
#
# This GCC bug may have been fixed by now, but I wouldn't count on it,
# since I don't remember how to test for it.  :)
#
prscomp.o: prscomp.c
	$(CC) -c $(CFLAGS) $(UNIXPATCHLEVEL) $(INCLUDES) prscomp.c

.c.o:
	$(CC) -c $(OPTIMIZE) $(UNIXPATCHLEVEL) $(CFLAGS) $(INCLUDES) $*.c


clean:
	rm -f *.o
	rm -f tadsc tadsr tdb tadsc.static tadsr.static tdb.static
