# Tads3 Makefile for Linux and other Unix variants.
# Created by Suzanne Skinner (tril@igs.net), 2001/Nov/5
# Updated 2002/Apr/16, tril@igs.net

###########################################################################
#
#			        Instructions
#
###########################################################################

# Note: You must compile the TADS2 source (under ../tads2) before compiling
# TADS 3. See ../INSTALL for further details.

# Installation points for TADS3. Tailor these to your liking. If you install
# manually, DATADIR must match where you actually put the libraries and
# resources, or the TADS3 compiler will not be able to find them.
PREFIX  = /usr/local
BINDIR  = $(PREFIX)/bin
DOCDIR  = $(PREFIX)/doc/tads3
DATADIR = $(PREFIX)/share/tads3
LIBDIR  = $(DATADIR)/libraries
INCDIR  = $(DATADIR)/includes
RESDIR  = $(DATADIR)/resources

# Uncomment the CFLAGS_DEBUG line below if you want to create debuggable
# executables (necessary for "make test"):
#
#CFLAGS_DEBUG = -g -DT3_DEBUG

# Now search below for your flavor of Unix, uncomment the lines in that
# section, and configure them as needed. The main defines are:
#
# CXX         - Your C++ compiler. I've found that g++ works well.
# CC          - Compiler to be used for C source files. This should not
#               generally be the same as CXX.
# IDENT       - This simply identifies your chosen system. You shouldn't
#               have to change it.
# OPTIMIZE    - The optimization flags, if any, to pass to the compiler.
# OS_FLAGS    - Miscellaneous OS-specific flags to pass to the compiler.
# DISPLAY_LIB - The display library to link against. Choices are
#               -ltermcap or -lncurses. I have better luck with -ltermcap.
#               This should match whatever you compiled TADS 2 with.
#
# Compile by typing "make". Binaries t3make and t3run (dynamically linked with
# DISPLAY_LIB) will be created. To install the binaries and libraries into
# systemwide directories (see above), type "make install". You must be root to
# install (except when installing into your home directory).
#
# So far, only Linux, FreeBSD, and Solaris versions have been ported. If you
# want to port another flavor, you're welcome to give it a shot. Other values
# for IDENT include NEXT, IBM_AIX, IBM_RT, SGI_IRIX, SUN3, SUN_SPARC_SUNOS,
# SUN_SPARC_SOLARIS, and ULTRIX_MIPS (the same as in TADS 2).
#
# Michael Roberts has created a test suite to help porters, consisting of
# a combination of executables and scripts. To compile and run the test
# executables, make sure CFLAGS_DEBUG is uncommented and type "make test".
# If you've previously done a "make" without debugging turned on, type "make
# clean" first.
#
# NOTE: If you run into weird display problems with the interpreter (in
# particular, if each line starts with 20 or so spaces), try using ncurses
# (or just curses as appropriate) as your display lib and adding -DHAVE_TPARM
# to CFLAGS (and you should have compiled tads2 using the same options):
#
#     DISPLAY_LIB = -lncurses
#     CFLAGS      = -DHAVE_TPARM (in addition to any other flags)
#
# Alternately, if ncurses is the default, you can try switching to
# DISPLAY_LIB = -ltermcap and remove -DHAVE_TPARM from CFLAGS.

###########################################################################
# Linux
# Compiled and tested on RedHat Linux 6.2 and Slackware 8.0
# Maintainer: Suzanne Skinner (tril@igs.net)
#
# Switch pentiumpro (in OPTIMIZE) for i386, i486, pentium, or whatever
# kind of processor you have (-mcpu=athlon requires a quite recent
# version of gcc). To optimize even more, use -march=pentiumpro (or
# similar for other processors) instead to generate processor-specific
# machine code.
#
# -fomit-frame-pointer causes problems with some recent versions (3.0+)
# of GCC. In 2.95.3 it works fine. Uncomment it if you think it's safe
# and are hoping for a performance boost.
#
# Uncomment -fno-exceptions from CXX_OPTIMIZE if you have a reasonably
# current (> 2.7) version of g++.
#
# Warning: Don't remove the -fno-strict-aliasing switch.  Due to a bug
# in the optimizer of 3.3.x (and maybe 3.4.x) versions of GCC, TADS 3
# will crash at startup.
###########################################################################
#
#CXX          = g++
#CC           = gcc
#IDENT        = -DLINUX_386 -DOS_SYSTEM_NAME=\"Linux\"
#OPTIMIZE     = -O2 -mcpu=pentiumpro -fno-strict-aliasing # -fomit-frame-pointer
#CXX_OPTIMIZE = -fno-strict-aliasing # -fno-exceptions
#OS_FLAGS     = -DOS_ANSI -DHAVE_TPARM -DHAVE_STRCASECMP -Dmemicmp=strncasecmp
#DISPLAY_LIB  = -lncurses

###########################################################################
# FreeBSD
# Compiled and tested on FreeBSD 3.4
# Maintainer: Suzanne Skinner (tril@igs.net)
#
# Uncomment -fno-exceptions from CXX_OPTIMIZE if you have a reasonably
# current (> 2.7) version of g++.
###########################################################################
#
#CXX          = g++
#CC           = gcc
#IDENT        = -DFREEBSD_386 -DOS_SYSTEM_NAME=\"FreeBSD\"
#OPTIMIZE     = -O2
#CXX_OPTIMIZE = # -fno-exceptions
#OS_FLAGS     = -DOS_ANSI -DHAVE_STRCASECMP -Dmemicmp=strncasecmp -I./BSD
#DISPLAY_LIB  = -ltermcap

###########################################################################
# OpenBSD
# Compiled and tested on OpenBSD 3.0
# Maintainer: Mark Wickham (macwickham@attbi.com)
#
# Uncomment -fno-exceptions from CXX_OPTIMIZE if you have a reasonably
# current (> 2.7) version of g++.
###########################################################################
#
#CXX          = g++
#CC           = gcc
#IDENT        = -DOPENBSD -DOS_SYSTEM_NAME=\"OpenBSD\"
#OPTIMIZE     = -O2
#CXX_OPTIMIZE = # -fno-exceptions
#OS_FLAGS     = -DOS_ANSI -DHAVE_STRCASECMP -Dmemicmp=strncasecmp -I./BSD
#DISPLAY_LIB  = -ltermcap

###########################################################################
# Solaris 2
# Compiled and tested on Solaris 2.6
# Maintainer: Suzanne Skinner (tril@igs.net)
#
# Uncomment -fno-exceptions from CXX_OPTIMIZE if you have a reasonably
# current (> 2.7) version of g++.
###########################################################################
#
#CXX          = g++
#CC           = gcc
#IDENT        = -DSUN_SPARC_SOLARIS -DOS_SYSTEM_NAME=\"Solaris\"
#OPTIMIZE     = -O2
#CXX_OPTIMIZE = # -fno-exceptions
#OS_FLAGS     = -DOS_ANSI -DHAVE_STRCASECMP -Dmemicmp=strncasecmp
#DISPLAY_LIB  = -ltermcap

###########################################################################
# SPARCstation running SunOS
# Compiled and tested on SunOS 5.7 with GCC 2.95.2
# Maintainer: John McCall (tenthstone@hotmail.com)
#
# Uncomment -fno-exceptions from CXX_OPTIMIZE if you have a reasonably
# current (> 2.7) version of g++.
###########################################################################
#
#CXX          = g++
#CC           = gcc
#OPTIMIZE     = -O2
#CXX_OPTIMIZE = # -fno-exceptions
#IDENT        = -DSUN_SPARC_SUNOS -DOS_SYSTEM_NAME=\""Sun Sparc running SunOS"\"
#OS_FLAGS     = -DOS_ANSI -DHAVE_STRCASECMP -Dmemicmp=strncasecmp
#DISPLAY_LIB  = -ltermcap

###########################################################################
# Mac OS X / Darwin
# Compiled and tested on Mac OS X 10.2 (Darwin xnu-344)
# Maintainer: Matt Herberg (meherberg@earthlink.net)
###########################################################################
#
#CXX          = c++
#CC           = cc
#IDENT        = -DDARWIN -DOS_SYSTEM_NAME=\"Darwin\"
#OPTIMIZE     = -O2
#CXX_OPTIMIZE =
#OS_FLAGS     = -DOS_ANSI -DHAVE_STRCASECMP -Dmemicmp=strncasecmp -I./BSD
#DISPLAY_LIB  = -lncurses

###########################################################################
# You shouldn't need to change anything beyond this line.
###########################################################################

############################## Misc. Defines ##############################

REG_BUILTIN      = vmbifreg
REG_METACLASS    = vmmcreg
T2_DIR           = ../tads2
T3_GLOBAL_CONFIG = VMGLOB_STRUCT

INCLUDE_DIRS = -I. -I$(T2_DIR) -I$(T2_DIR)/unix

CFLAGS = $(CFLAGS_DEBUG) -c $(IDENT) $(OPTIMIZE) $(OS_FLAGS) $(INCLUDE_DIRS) -D$(T3_GLOBAL_CONFIG) -DTC_TARGET_T3 -DUNIX -DBINDIR=\"$(BINDIR)\" -DLIBDIR=\"$(LIBDIR)\" -DINCDIR=\"$(INCDIR)\" -DRESDIR=\"$(RESDIR)\"
CXXFLAGS = $(CFLAGS) $(CXX_OPTIMIZE)

############################# Object File Lists ###########################

T3MAKE_OBJS = vmcrc.o rcmain.o tcmakecl.o tccmdutl.o tcmake.o tclibprs.o std.o std_dbg.o tcglob.o vmerr.o vmerrmsg.o utf8.o charmap.o resload.o resldexe.o tcmain.o tcerr.o tcerrmsg.o tchostsi.o tcsrc.o tctok.o tcprs.o tcprsstm.o tcprsimg.o tcgen.o tct3.o tct3stm.o tct3img.o tct3unas.o vmhash.o vmwrtimg.o vmtypedh.o vmfile.o vminit.o vmini_nd.o vminitim.o vmcfgmem.o vmobj.o vmundo.o vmtobj.o vmpat.o vmstrcmp.o vmdict.o vmgram.o vmstr.o vmcoll.o vmiter.o vmlst.o vmsort.o vmsortv.o vmbignum.o vmvec.o vmintcls.o vmanonfn.o vmlookup.o vmbytarr.o vmcset.o vmfilobj.o vmstack.o vmpool.o vmpoolim.o vmtype.o vmglob.o vmrun.o vmfunc.o vmmeta.o vmpreini.o vmimgrb.o vmbif.o vmbifc.o vmimage.o vmrunsym.o vmimg_nd.o vmsrcf.o vmbiftad.o vmbiftio.o askf_tx.o indlg_tx.o vmsave.o vmbift3.o vmbt3_nd.o vmregex.o vmconsol.o vmconmor.o vmconhmp.o os_stdio.o vmhosttx.o vmhostsi.o $(REG_METACLASS).o $(REG_BUILTIN).o vmuni_cs.o osunix.o osunix3.o wcs.o $(T2_DIR)/osifc.o $(T2_DIR)/osnoui.o $(T2_DIR)/osunixt_stdio.o $(T2_DIR)/osrestad.o

T3RUN_OBJS = vmcrc.o vmmaincl.o vmmain.o std.o std_dbg.o charmap.o resload.o resldexe.o vminit.o vmini_nd.o vmconsol.o vmconmor.o vmconhmp.o vminitim.o vmcfgmem.o vmobj.o vmundo.o vmtobj.o vmpat.o vmstrcmp.o vmdict.o vmgram.o vmstr.o vmcoll.o vmiter.o vmlst.o vmsort.o vmsortv.o vmbignum.o vmvec.o vmintcls.o vmanonfn.o vmlookup.o vmbytarr.o vmcset.o vmfilobj.o vmstack.o vmerr.o vmerrmsg.o vmpool.o vmpoolim.o vmtype.o vmtypedh.o utf8.o vmglob.o vmrun.o vmfunc.o vmmeta.o vmsa.o vmbif.o vmbifl.o vmimage.o vmimg_nd.o vmrunsym.o vmsrcf.o vmfile.o vmbiftad.o vmbiftio.o askf_tx.o indlg_tx.o vmsave.o vmbift3.o vmbt3_nd.o vmregex.o vmhosttx.o vmhostsi.o vmhash.o $(REG_METACLASS).o $(REG_BUILTIN).o vmuni_cs.o osunix.o osunix3.o wcs.o $(T2_DIR)/osifc.o $(T2_DIR)/osgen3.o $(T2_DIR)/osnoui.o $(T2_DIR)/osunixt.o $(T2_DIR)/osrestad.o $(T2_DIR)/tparm.o $(T2_DIR)/tputs.o

T23RUN_OBJS = vmcrc.o vmcl23.o vmmain.o std.o std_dbg.o charmap.o resload.o resldexe.o vminit.o vmini_nd.o vmconsol.o vmconmor.o vmconhmp.o vminitim.o vmcfgmem.o vmobj.o vmundo.o vmtobj.o vmpat.o vmstrcmp.o vmdict.o vmgram.o vmstr.o vmcoll.o vmiter.o vmlst.o vmsort.o vmsortv.o vmbignum.o vmvec.o vmintcls.o vmanonfn.o vmlookup.o vmbytarr.o vmcset.o vmfilobj.o vmstack.o vmerr.o vmerrmsg.o vmpool.o vmpoolim.o vmtype.o vmtypedh.o utf8.o vmglob.o vmrun.o vmfunc.o vmmeta.o vmsa.o vmbif.o vmbifl.o vmimage.o vmimg_nd.o vmrunsym.o vmsrcf.o vmfile.o vmbiftad.o vmbiftio.o askf_tx.o indlg_tx.o vmsave.o vmbift3.o vmbt3_nd.o vmregex.o vmhosttx.o vmhostsi.o vmhash.o $(REG_METACLASS).o $(REG_BUILTIN).o vmuni_cs.o osunix.o wcs.o $(T2_DIR)/osifc.o $(T2_DIR)/osgen3.o $(T2_DIR)/osnoui.o $(T2_DIR)/osunixt.o $(T2_DIR)/osrestad.o $(T2_DIR)/tparm.o $(T2_DIR)/tputs.o $(T2_DIR)/mcm.o $(T2_DIR)/mcs.o $(T2_DIR)/mch.o $(T2_DIR)/obj.o $(T2_DIR)/cmd.o $(T2_DIR)/errmsg.o $(T2_DIR)/fioxor.o $(T2_DIR)/os0.o $(T2_DIR)/argize.o $(T2_DIR)/oserr.o $(T2_DIR)/cmap.o $(T2_DIR)/dat.o $(T2_DIR)/lst.o $(T2_DIR)/out.o $(T2_DIR)/voc.o $(T2_DIR)/bif.o $(T2_DIR)/output.o $(T2_DIR)/suprun.o $(T2_DIR)/regex.o $(T2_DIR)/vocab.o $(T2_DIR)/execmd.o $(T2_DIR)/runstat.o $(T2_DIR)/ply.o $(T2_DIR)/trd.o $(T2_DIR)/linfdum.o $(T2_DIR)/fio.o $(T2_DIR)/getstr.o $(T2_DIR)/qas.o $(T2_DIR)/dbgtr.o $(T2_DIR)/ler.o $(T2_DIR)/bifgdum.o $(T2_DIR)/indlg_tx.o $(T2_DIR)/oemunix.o $(T2_DIR)/run.o $(T2_DIR)/askf_tx.o

T3PRE_OBJS = vmcrc.o test_pre.o std.o std_dbg.o charmap.o resload.o resldexe.o vmwrtimg.o vminit.o vmini_nd.o vminitim.o vmcfgmem.o vmobj.o vmundo.o vmtobj.o vmpat.o vmstrcmp.o vmdict.o vmgram.o vmstr.o vmcoll.o vmiter.o vmlst.o vmsort.o vmsortv.o vmbignum.o vmvec.o vmintcls.o vmanonfn.o vmlookup.o vmbytarr.o vmcset.o vmfilobj.o vmstack.o vmerr.o vmerrmsg.o vmpool.o vmpoolim.o vmtype.o vmtypedh.o utf8.o vmglob.o vmrun.o vmfunc.o vmmeta.o vmpreini.o vmimgrb.o vmbif.o vmbifc.o vmimage.o vmimg_nd.o vmrunsym.o vmsrcf.o vmfile.o vmbiftad.o vmbiftio.o askf_tx.o indlg_tx.o vmsave.o vmbift3.o vmbt3_nd.o vmregex.o vmconsol.o vmconmor.o vmconhmp.o os_stdio.o vmhosttx.o vmhostsi.o vmhash.o $(REG_METACLASS).o $(REG_BUILTIN).o vmuni_cs.o osunix.o osunix3.o wcs.o $(T2_DIR)/osifc.o $(T2_DIR)/osnoui.o $(T2_DIR)/osunixt_stdio.o $(T2_DIR)/osrestad.o

T3RES_OBJS = rcmaincl.o rcmain.o std.o osunix.o $(T2_DIR)/osnoui.o $(T2_DIR)/osunixt_stdio.o

MKCHRTAB_OBJS = mkchrtab.o $(T2_DIR)/osunixt_stdio.o

################################# Targets #################################

all: t3make t3run t23run t3pre t3res mkchrtab

install: all install-common
	install -s -m755 t3make t3run t23run t3res mkchrtab $(BINDIR)

install-common: all
	install -d $(BINDIR)
	install -d $(DOCDIR)
	install -d $(DATADIR)
	rm -rf $(INCDIR) $(LIBDIR) $(RESDIR) $(DOCDIR)
	cp -rp t3include $(INCDIR)
	cp -rp t3library $(LIBDIR)
	cp -rp t3resource $(RESDIR)
	cp -rp doc $(DOCDIR)
	-chown -fR root.root $(INCDIR) $(LIBDIR) $(RESDIR) $(DOCDIR)

clean:
	rm -f *.o t3make t3run t23run t3pre t3res mkchrtab test_utf8 test_chr test_pool test_err test_obj test_write test_exec test_gets test_tok test_prs test_sym test_prs_top test_comp_obj test_link test_regex test_sort iter.t3s test.t3v
	rm -f test/out/*

t3make: $(T3MAKE_OBJS)
	$(CXX) -o $@ $(T3MAKE_OBJS)

t3run: $(T3RUN_OBJS)
	$(CXX) -o $@ $(T3RUN_OBJS) $(DISPLAY_LIB)

t23run: $(T23RUN_OBJS)
	$(CXX) -o $@ $(T23RUN_OBJS) $(DISPLAY_LIB)

t3pre: $(T3PRE_OBJS)
	$(CXX) -o $@ $(T3PRE_OBJS)

t3res: $(T3RES_OBJS)
	$(CXX) -o $@ $(T3RES_OBJS)

mkchrtab: $(MKCHRTAB_OBJS)
	$(CXX) -o $@ $(MKCHRTAB_OBJS)

############################## Test Targets ###############################

test: all test_utf8 test_chr test_pool test_err test_obj test_write test_exec test_gets test_tok test_prs test_sym test_prs_top test_comp_obj test_link test_regex test_sort
	cd test/scripts; ./run_all_tests

TEST_UTF8_OBJS = test_utf8.o utf8.o
test_utf8: $(TEST_UTF8_OBJS)
	$(CXX) -o $@ $(TEST_UTF8_OBJS)

TEST_CHR_OBJS = std.o std_dbg.o test_chr.o utf8.o charmap.o resload.o resnoexe.o vmerr.o vmerrmsg.o osunix.o osunix3.o $(T2_DIR)/osnoui.o $(T2_DIR)/osunixt_stdio.o
test_chr: $(TEST_CHR_OBJS)
	$(CXX) -o $@ $(TEST_CHR_OBJS)

TEST_GETS_OBJS = std.o std_dbg.o test_gets.o utf8.o charmap.o resload.o resnoexe.o tcsrc.o vmerr.o vmerrmsg.o osunix.o osunix3.o $(T2_DIR)/osnoui.o $(T2_DIR)/osunixt_stdio.o
test_gets: $(TEST_GETS_OBJS)
	$(CXX) -o $@ $(TEST_GETS_OBJS)

TEST_POOL_OBJS = std.o std_dbg.o test_pool.o vmpool.o vmpoolim.o vmerr.o vmerrmsg.o vmglob.o $(T2_DIR)/osunixt_stdio.o
test_pool: $(TEST_POOL_OBJS)
	$(CXX) -o $@ $(TEST_POOL_OBJS)

TEST_ERR_OBJS = std.o std_dbg.o test_err.o vmerr.o vmerrmsg.o $(T2_DIR)/osunixt_stdio.o
test_err: $(TEST_ERR_OBJS)
	$(CXX) -o $@ $(TEST_ERR_OBJS)

TEST_REGEX_OBJS = test_regex.o vmregex.o vmerr.o vmerrmsg.o vmuni_cs.o std.o std_dbg.o $(T2_DIR)/osunixt_stdio.o
test_regex: $(TEST_REGEX_OBJS)
	$(CXX) -o $@ $(TEST_REGEX_OBJS)

TEST_OBJ_OBJS = std.o std_dbg.o test_obj.o charmap.o resload.o resnoexe.o vmfile.o vmundo.o vmobj.o vmtobj.o vmpat.o vmstrcmp.o vmstr.o vmcoll.o vmiter.o vmlst.o vmsort.o vmsortv.o vmbignum.o vmsave.o vmvec.o vmintcls.o vmanonfn.o vmlookup.o vmbytarr.o vmcset.o vmfilobj.o vmstack.o vmdict.o vmgram.o vmerr.o vmerrmsg.o vmpool.o vmpoolim.o vmtype.o vmtypedh.o utf8.o vmglob.o vmrun.o vmsrcf.o vmfunc.o vmmeta.o vmbif.o vmsa.o vmbifl.o vmbiftad.o vmbiftio.o askf_tx.o indlg_tx.o vmbift3.o vmbt3_nd.o vminit.o vmini_nd.o vmconsol.o vmconmor.o vmconhmp.o os_stdio.o vminitim.o vmcfgmem.o vmregex.o vmhosttx.o vmhostsi.o vmhash.o $(REG_METACLASS).o $(REG_BUILTIN).o vmuni_cs.o osunix.o osunix3.o wcs.o vmcrc.o $(T2_DIR)/osifc.o $(T2_DIR)/osnoui.o $(T2_DIR)/osunixt_stdio.o $(T2_DIR)/osrestad.o
test_obj: $(TEST_OBJ_OBJS)
	$(CXX) -o $@ $(TEST_OBJ_OBJS)

TEST_SORT_OBJS = test_sort.o vmsort.o std.o $(T2_DIR)/osunixt_stdio.o
test_sort: $(TEST_SORT_OBJS)
	$(CXX) -o $@ $(TEST_SORT_OBJS)

TEST_WRITE_OBJS = std.o std_dbg.o test_write.o vmwrtimg.o vmfile.o vmerr.o vmerrmsg.o vmtypedh.o vmglob.o $(T2_DIR)/osunixt_stdio.o
test_write: $(TEST_WRITE_OBJS)
	$(CXX) -o $@ $(TEST_WRITE_OBJS)

TEST_EXEC_OBJS = vmmain.o std.o std_dbg.o charmap.o resload.o resldexe.o test_exec.o vminit.o vmini_nd.o vmconsol.o vmconmor.o vmconhmp.o os_stdio.o vminitim.o vmcfgmem.o vmobj.o vmundo.o vmtobj.o vmpat.o vmstrcmp.o vmstr.o vmcoll.o vmiter.o vmlst.o vmsort.o vmsortv.o vmbignum.o vmvec.o vmintcls.o vmanonfn.o vmdict.o vmgram.o vmlookup.o vmbytarr.o vmcset.o vmfilobj.o vmstack.o vmerr.o vmerrmsg.o vmpool.o vmpoolim.o vmtype.o vmtypedh.o utf8.o vmglob.o vmrun.o vmfunc.o vmmeta.o vmsa.o vmbif.o vmbifl.o vmimage.o vmimg_nd.o vmrunsym.o vmsrcf.o vmfile.o vmbiftad.o vmbiftio.o askf_tx.o indlg_tx.o vmsave.o vmbift3.o vmbt3_nd.o vmregex.o vmhosttx.o vmhostsi.o vmhash.o $(REG_METACLASS).o $(REG_BUILTIN).o vmuni_cs.o osunix.o osunix3.o wcs.o vmcrc.o $(T2_DIR)/osifc.o $(T2_DIR)/osnoui.o $(T2_DIR)/osunixt_stdio.o $(T2_DIR)/osrestad.o
test_exec: $(TEST_EXEC_OBJS)
	$(CXX) -o $@ $(TEST_EXEC_OBJS)

TEST_TOK_OBJS = std.o std_dbg.o test_tok.o tcglob.o vmerr.o vmerrmsg.o utf8.o charmap.o resload.o resnoexe.o tcmain.o tcerr.o tcerrmsg.o tchostsi.o tcsrc.o tctok.o tcprs.o tcprs_d.o tcprsnl.o tct3nl.o tcgen.o tct3.o tct3_d.o tct3unas.o vmhash.o vmwrtimg.o vmtypedh.o vmglob.o osunix.o osunix3.o $(T2_DIR)/osunixt_stdio.o $(T2_DIR)/osnoui.o
test_tok: $(TEST_TOK_OBJS)
	$(CXX) -o $@ $(TEST_TOK_OBJS)

TEST_PRS_OBJS = test_prs.o tcglob.o std.o std_dbg.o vmerr.o vmerrmsg.o utf8.o charmap.o resload.o resnoexe.o tcmain.o tcerr.o tcerrmsg.o tchostsi.o tcsrc.o tctok.o tcprs.o tcprsstm.o tcprsimg.o tcgen.o tct3.o tct3stm.o tct3img.o tct3unas.o vmhash.o vmwrtimg.o vmtypedh.o vmfile.o osunix.o osunix3.o $(T2_DIR)/osnoui.o $(T2_DIR)/osunixt_stdio.o vmglob.o
test_prs: $(TEST_PRS_OBJS)
	$(CXX) -o $@ $(TEST_PRS_OBJS)

TEST_SYM_OBJS = test_sym.o tcglob.o std.o std_dbg.o vmerr.o vmerrmsg.o utf8.o charmap.o resload.o resnoexe.o tcmain.o tcerr.o tcerrmsg.o tchostsi.o tcsrc.o tctok.o tcprs.o tcprsnl.o tct3nl.o tcprsstm.o tcgen.o tct3.o tct3stm.o tct3unas.o vmhash.o vmtypedh.o vmfile.o osunix.o osunix3.o $(T2_DIR)/osnoui.o $(T2_DIR)/osunixt_stdio.o vmglob.o
test_sym: $(TEST_SYM_OBJS)
	$(CXX) -o $@ $(TEST_SYM_OBJS)

TEST_PRS_TOP_OBJS = test_prs_top.o tcglob.o std.o std_dbg.o vmerr.o vmerrmsg.o utf8.o charmap.o resload.o resnoexe.o tcmain.o tcerr.o tcerrmsg.o tchostsi.o tcsrc.o tctok.o tcprs.o tcprsstm.o tcprsimg.o tcgen.o tct3.o tct3stm.o tct3img.o tct3unas.o vmhash.o vmwrtimg.o vmtypedh.o vmfile.o osunix.o osunix3.o $(T2_DIR)/osnoui.o $(T2_DIR)/osunixt_stdio.o vmglob.o
test_prs_top: $(TEST_PRS_TOP_OBJS)
	$(CXX) -o $@ $(TEST_PRS_TOP_OBJS)

TEST_COMP_OBJ_OBJS = test_comp_obj.o tcglob.o std.o std_dbg.o vmerr.o vmerrmsg.o utf8.o charmap.o resload.o resnoexe.o tcmain.o tcerr.o tcerrmsg.o tchostsi.o tcsrc.o tctok.o tcprs.o tcprsstm.o tcprsimg.o tcgen.o tct3.o tct3stm.o tct3img.o tct3unas.o vmhash.o vmwrtimg.o vmtypedh.o vmfile.o osunix.o osunix3.o $(T2_DIR)/osnoui.o $(T2_DIR)/osunixt_stdio.o vmglob.o
test_comp_obj: $(TEST_COMP_OBJ_OBJS)
	$(CXX) -o $@ $(TEST_COMP_OBJ_OBJS)

TEST_LINK_OBJS = test_link.o tcglob.o std.o std_dbg.o vmerr.o vmerrmsg.o utf8.o charmap.o resload.o resnoexe.o tcmain.o tcerr.o tcerrmsg.o tchostsi.o tcsrc.o tctok.o tcprs.o tcprsstm.o tcprsimg.o tcgen.o tct3.o tct3stm.o tct3img.o tct3unas.o vmhash.o vmwrtimg.o vmtypedh.o vmfile.o osunix.o osunix3.o $(T2_DIR)/osnoui.o $(T2_DIR)/osunixt_stdio.o vmglob.o
test_link: $(TEST_LINK_OBJS)
	$(CXX) -o $@ $(TEST_LINK_OBJS)

########################### Source Dependencies ###########################

mkchrtab.o: mkchrtab.cpp t3std.h
charmap.o: charmap.cpp charmap.h resload.h utf8.h
resload.o: resload.cpp resload.h t3std.h
resldexe.o: resload.cpp resload.h vmimage.h t3std.h
resnoexe.o: resload.cpp resload.h t3std.h
std.o: std.cpp t3std.h utf8.h
std_dbg.o: std_dbg.cpp t3std.h utf8.h
utf8.o: utf8.cpp utf8.h
vmcrc.o: vmcrc.cpp vmcrc.h
vmhash.o: vmhash.cpp vmhash.h
vmerr.o: vmerr.cpp vmerr.h vmerrnum.h
vmerrmsg.o: vmerrmsg.cpp vmerr.h vmerrnum.h
vmpool.o: vmpool.cpp vmpool.h vmtype.h
vmpoolim.o: vmpoolim.cpp vmpool.h vmtype.h
vmpoolsw.o: vmpoolsw.cpp vmpool.h vmtype.h
vmobj.o: vmobj.cpp vmobj.h vmrun.h vmtype.h vmstack.h vmundo.h vmfile.h vmmeta.h vmlst.h vmstr.h vmintcls.h vmpool.h vmfunc.h vmimport.h vmpredef.h vmhash.h
vmstack.o: vmstack.cpp vmstack.h vmfile.h vmtype.h t3std.h
vmfile.o: vmfile.cpp vmfile.h
vmstr.o: vmstr.cpp vmstr.h vmobj.h vmtype.h vmfile.h vmerrnum.h vmerr.h vmpool.h vmstack.h vmmeta.h vmrun.h vmbif.h vmpredef.h vmlst.h vmuni.h vmcset.h vmbytarr.h charmap.h
vmlst.o: vmlst.cpp vmlst.h vmobj.h vmtype.h vmerr.h vmerrnum.h vmfile.h vmpool.h vmstack.h vmmeta.h vmrun.h vmbif.h vmpredef.h vmcoll.h vmiter.h vmsort.h
vmsort.o: vmsort.cpp vmsort.h vmglob.h vmpoolsl.h t3std.h
vmsortv.o: vmsortv.cpp vmsort.h vmglob.h vmpoolsl.h t3std.h vmrun.h vmstack.h vmerr.h vmerrnum.h
vmbignum.o: vmbignum.cpp vmbignum.h vmobj.h vmtype.h vmerr.h vmerrnum.h vmfile.h vmstack.h utf8.h vmstr.h vmbif.h vmmeta.h vmlst.h
vmvec.o: vmvec.cpp vmvec.h vmobj.h vmtype.h vmerr.h vmerrnum.h vmfile.h vmstack.h vmbif.h vmmeta.h vmlst.h vmglob.h vmundo.h vmcoll.h vmiter.h vmsort.h vmpoolsl.h
vmlookup.o: vmlookup.cpp vmlookup.h vmobj.h vmtype.h vmerr.h vmerrnum.h vmfile.h vmstack.h vmbif.h vmmeta.h vmlst.h vmglob.h vmundo.h vmcoll.h vmiter.h vmrun.h vmlst.h vmpoolsl.h
vmbytarr.o: vmbytarr.cpp vmbytarr.h vmobj.h vmtype.h vmerr.h vmerrnum.h vmfile.h vmstack.h vmbif.h vmmeta.h vmglob.h vmundo.h vmrun.h charmap.h vmstr.h vmcset.h vmpoolsl.h
vmcset.o: vmcset.cpp vmcset.h vmobj.h vmtype.h vmerr.h vmerrnum.h vmfile.h vmstack.h vmbif.h vmmeta.h vmglob.h vmundo.h vmrun.h charmap.h vmstr.h vmimport.h vmpredef.h vmhost.h vmpoolsl.h
vmfilobj.o: vmfilobj.cpp vmfilobj.h vmobj.h vmtype.h vmerr.h vmerrnum.h vmfile.h vmstack.h vmbif.h vmmeta.h vmglob.h vmundo.h vmrun.h charmap.h vmstr.h vmimport.h vmpredef.h vmbytarr.h vmbignum.h vmhost.h vmpoolsl.h
vmcoll.o: vmcoll.cpp vmcoll.h vmobj.h vmtype.h vmerr.h vmerrnum.h vmglob.h vmiter.h vmmeta.h vmstack.h vmpoolsl.h
vmiter.o: vmiter.cpp vmiter.h vmobj.h vmtype.h vmerr.h vmerrnum.h vmmeta.h vmstack.h vmundo.h vmlst.h vmfile.h
vmintcls.o: vmintcls.cpp vmintcls.h vmobj.h vmtype.h vmerr.h vmerrnum.h vmfile.h vmmeta.h vmglob.h vmrun.h vmlst.h vmpoolsl.h
vmanonfn.o: vmanonfn.cpp vmanonfn.h vmobj.h vmtype.h vmerr.h vmerrnum.h vmmeta.h vmglob.h vmrun.h vmstack.h vmpredef.h vmvec.h vmpoolsl.h
vmundo.o: vmundo.cpp vmundo.h vmobj.h
vmtobj.o: vmtobj.cpp vmtobj.h vmobj.h vmerr.h vmerrnum.h vmglob.h vmundo.h vmtype.h vmfile.h vmstack.h vmrun.h vmpredef.h vmmeta.h vmlst.h vmpoolsl.h
vmdict.o: vmdict.cpp vmdict.h vmobj.h t3std.h vmtype.h vmglob.h vmundo.h vmfile.h vmhash.h vmerr.h vmerrnum.h vmbif.h vmpredef.h vmmeta.h vmstack.h vmstr.h vmlst.h vmrun.h vmpoolsl.h vmstrcmp.h
vmgram.o: vmgram.cpp vmgram.h vmobj.h t3std.h vmtype.h vmglob.h vmerr.h vmerrnum.h vmmeta.h vmstack.h vmdict.h vmtobj.h vmlst.h vmpredef.h vmimport.h vmpoolsl.h
vmpat.o: vmpat.cpp vmpat.h vmregex.h vmtype.h t3std.h vmstack.h vmbif.h vmbiftad.h vmglob.h vmmeta.h vmfile.h
vmstrcmp.o: vmstrcmp.cpp vmstrcmp.h vmtype.h t3std.h vmstack.h vmbif.h vmbiftad.h vmglob.h vmmeta.h vmfile.h vmuni.h utf8.h
vmrun.o: vmrun.cpp vmrun.h vmstack.h vmop.h vmerr.h vmerrnum.h vmobj.h vmlst.h vmstr.h vmtobj.h vmfunc.h vmmeta.h vmbif.h vmpredef.h vmfile.h vmdbg.h vmhash.h vmsave.h vmprof.h vmprofty.h
vmfunc.o: vmfunc.cpp vmfunc.h vmtype.h t3std.h
vmdbg.o: vmdbg.cpp vmdbg.h vmhash.h vmrun.h vmglob.h vmtype.h vmfunc.h t3std.h vmsrcf.h tcprstyp.h tcpnbase.h tcpnint.h tcpndrv.h tcglob.h tcprs.h tcmain.h tctok.h resload.h tcgen.h vmop.h tct3.h tct3base.h tct3int.h tct3drv.h vmpool.h vmlst.h vmimage.h charmap.h vmhost.h vmbignum.h vmvec.h vmpoolsl.h
vmimgrb.o: vmimgrb.cpp t3std.h vmfile.h vmimage.h vmpool.h vmglob.h vmwrtimg.h vmobj.h vmtobj.h vmmeta.h vmimgrb.h vmlst.h vmstr.h vmdict.h vmhash.h vmgram.h vmbignum.h vmintcls.h vmiter.h vmvec.h vmlookup.h vmstack.h vmbytarr.h vmcset.h vmfilobj.h vmpoolsl.h vmpat.h vmstrcmp.h
vmpreini.o: vmpreini.cpp t3std.h vminit.h vmerr.h vmfile.h vmimage.h vmrun.h vmimgrb.h vmpreini.h vmconsol.h
vmsa.o: vmsa.cpp t3std.h vmtobj.h vmobj.h vmlst.h vmstr.h vmdict.h vmmeta.h vmgram.h vmbignum.h vmintcls.h vmiter.h vmvec.h vmlookup.h vmbytarr.h vmcset.h vmfilobj.h vmpat.h vmstrcmp.h
vmtype.o: vmtype.cpp vmtype.h vmobj.h vmstr.h vmlst.h vmpool.h
vmtypedh.o: vmtypedh.cpp vmtype.h vmglob.h vmstack.h
vmglob.o: vmglob.cpp vmglob.h vmimport.h vmpoolsl.h
vmmeta.o: vmmeta.cpp vmmeta.h vmfile.h vmtype.h vmerr.h vmerrnum.h vmglob.h vmobj.h vmintcls.h vmimport.h vmpredef.h vmpoolsl.h
vmmcreg.o: vmmcreg.cpp vmmcreg.h vmmccore.h vmdict.h vmstr.h vmlst.h vmtobj.h vmgram.h vmbignum.h vmintcls.h vmanonfn.h vmiter.h vmcoll.h vmvec.h vmlookup.h vmbytarr.h vmcset.h vmfilobj.h vmpat.h vmstrcmp.h
vmbif.o: vmbif.cpp vmbif.h vmtype.h vmerr.h vmerrnum.h vmglob.h vmbifreg.h vmstr.h vmobj.h vmrun.h utf8.h charmap.h vmpoolsl.h
vminit.o: vminit.cpp vminit.h vmerr.h vmfile.h vmimage.h vmpool.h vmobj.h vmstack.h vmundo.h vmmeta.h vmbif.h vmrun.h vmpredef.h vmmcreg.h vmbiftad.h resload.h charmap.h vmhost.h vmconsol.h vmbignum.h vmimport.h vmsrcf.h vmparam.h vmmain.h vmtobj.h
vmini_d.o: vmini_d.cpp vminit.h vmdbg.h tcprs.h tcprstyp.h tcglob.h t3std.h
vmini_nd.o: vmini_nd.cpp vminit.h t3std.h
vminitim.o: vminitim.cpp vminit.h vmpool.h
vminitsw.o: vminitsw.cpp vminit.h vmpool.h
vmcfgmem.o: vmcfgmem.cpp vminit.h
vmcfgsw.o: vmcfgsw.cpp vminit.h
vmbifl.o: vmbifl.cpp vmbif.h vmtype.h vmerr.h vmerrnum.h vmglob.h vmbifreg.h vmstr.h vmobj.h vmrun.h vmpoolsl.h
vmbifc.o: vmbifc.cpp vmbif.h vmtype.h vmerr.h vmerrnum.h vmglob.h vmbifreg.h vmstr.h vmobj.h vmrun.h vmpoolsl.h
vmimage.o: vmimage.cpp vmimage.h vmtype.h vmfile.h vmerr.h vmerrnum.h vmmeta.h vmbif.h vmpredef.h vmrun.h vmtobj.h vminit.h vmhost.h vmstr.h vmlst.h vmobj.h vmsave.h vmrunsym.h vmlookup.h vmcoll.h vmstack.h tcprstyp.h vmhash.h vmimport.h vmbytarr.h vmcset.h vmfilobj.h vmsrcf.h vmvec.h
vmimg_d.o: vmimg_d.cpp vmimage.h vmtype.h vmfile.h vmerr.h vmerrnum.h tcprs.h tctarg.h vmdbg.h vmglob.h vmfunc.h vmhash.h tcglob.h tcpnbase.h tcpnint.h tcpndrv.h tct3base.h tct3int.h tct3drv.h tcprstyp.h tctok.h vmpoolsl.h
vmimg_nd.o: vmimg_nd.cpp vmimage.h vmrunsym.h vmtype.h
vmrunsym.o: vmrunsym.cpp vmrunsym.h vmtype.h t3std.h
vmsave.o: vmsave.cpp vmsave.h vmglob.h vmfile.h vmimage.h vmobj.h vmrun.h vmstack.h vmundo.h vmmeta.h vmpoolsl.h
vmwrtimg.o: vmwrtimg.cpp vmwrtimg.h
vmres.o: vmres.cpp vmres.h
vmbifreg.o: vmbifreg.cpp vmbifreg.h vmbiftad.h vmbift3.h vmbiftio.h
vmbifreg_core.o: vmbifreg_core.cpp vmbifreg.h vmbiftad.h vmbift3.h vmcore.h
vmbiftad.o: vmbiftad.cpp vmbiftad.h vmstack.h vmerr.h vmerrnum.h vmglob.h vmpool.h vmobj.h vmrun.h utf8.h vmlst.h vmstr.h vmregex.h vmundo.h vmsave.h vmuni.h vmbignum.h vmfunc.h vmpoolsl.h vmpat.h
vmbiftio.o: vmbiftio.cpp vmbiftio.h vmstack.h vmerr.h vmerrnum.h vmglob.h vmpool.h vmobj.h vmrun.h utf8.h vmlst.h vmstr.h charmap.h vmfile.h vmconsol.h vmstrres.h vmvsn.h vmhost.h vmpoolsl.h
vmbift3.o: vmbift3.cpp vmbift3.h vmstack.h vmvsn.h vmerr.h vmerrnum.h vmglob.h vmpool.h vmobj.h vmrun.h utf8.h vmimage.h vmlst.h vmtobj.h vmfunc.h vmimport.h vmpredef.h vmsrcf.h charmap.h vmpoolsl.h
vmbt3_d.o: vmbt3_d.cpp vmbift3.h vmstack.h vmerr.h vmerrnum.h vmglob.h vmdbg.h vmpoolsl.h
vmbt3_nd.o: vmbt3_nd.cpp vmbift3.h vmstack.h vmerr.h vmerrnum.h vmglob.h vmpoolsl.h
vmhostsi.o: vmhostsi.cpp vmhostsi.h vmhost.h t3std.h resload.h vmhosttx.h
vmhosttx.o: vmhosttx.cpp vmhosttx.h vmhost.h t3std.h vmhash.h
vmconsol.o: vmconsol.cpp vmconsol.h vmhash.h t3std.h utf8.h charmap.h vmuni.h vmglob.h vmpoolsl.h
vmconmor.o: vmconmor.cpp vmconsol.h
vmconnom.o: vmconnom.cpp vmconsol.h
vmconhmp.o: vmconhmp.cpp vmconsol.h t3std.h utf8.h vmglob.h vmuni.h charmap.h vmhash.h vmpoolsl.h
vmconhtm.o: vmconhtm.cpp vmconsol.h
vmmain.o: vmmain.cpp vmmain.h t3std.h vmerr.h vmfile.h vmimage.h vmrun.h vmimgrb.h vmglob.h vmhost.h vminit.h vmvsn.h charmap.h vmsave.h vmtype.h vmrunsym.h vmimport.h vmpoolsl.h
vmmaincl.o: vmmaincl.cpp vmmain.h t3std.h vmmaincn.h vmconsol.h vmhostsi.h vmhosttx.h
vmcl23.o: vmcl23.cpp vmmain.h t3std.h vmmaincn.h vmconsol.h vmhostsi.h vmhosttx.h
vmcore.o: vmcore.cpp vmcore.h vmbif.h vmhost.h vmhostsi.h vmglob.h vmtype.h charmap.h vmstr.h t3std.h vmhosttx.h vmpoolsl.h
vmsrcf.o: vmsrcf.cpp t3std.h vmsrcf.h

askf_tx.o: askf_tx.cpp t3std.h vmglob.h vmconsol.h vmpoolsl.h
askf_os.o: askf_os.cpp t3std.h vmglob.h vmconsol.h charmap.h vmpoolsl.h
indlg_tx.o: indlg_tx.cpp t3std.h vmglob.h vmconsol.h vmpoolsl.h
indlg_os.o: indlg_os.cpp t3std.h vmglob.h vmconsol.h charmap.h vmpoolsl.h

t3egex.o: vmregex.cpp vmregex.h t3std.h utf8.h vmerr.h vmerrnum.h vmuni.h vmfile.h

tcglob.o: tcglob.cpp tcglob.h
tcerr.o: tcerr.cpp tcerr.h tcerrnum.h vmerr.h
tcerrmsg.o: tcerrmsg.cpp tcerr.h tcerrnum.h
tcmain.o: tcmain.cpp tcmain.h tchost.h tcerr.h tctok.h tcglob.h tcprs.h tct3.h tcprstyp.h charmap.h resload.h tcunas.h
tchostsi.o: tchostsi.cpp tchostsi.h tchost.h charmap.h tcmain.h tcglob.h
tcsrc.o: tcsrc.cpp tcsrc.h charmap.h t3std.h tcglob.h
tctok.o: tctok.cpp tctok.h vmhash.h t3std.h utf8.h tcerrnum.h vmerr.h tcmain.h tcglob.h tcprs.h charmap.h tcprstyp.h vmfile.h
tcprs.o: tcprs.cpp tcprs.h tctok.h vmhash.h t3std.h utf8.h tcerrnum.h vmerr.h tcmain.h tcglob.h tcpnbase.h tcpnint.h tcpndrv.h tct3.h tct3base.h tct3int.h tct3drv.h tcgen.h vmfile.h tctok.h tcprstyp.h
tcprs_d.o: tcprs_d.cpp tcprs.h tcprstyp.h t3std.h tcerrnum.h vmerr.h tcmain.h tcglob.h tcpnbase.h tcpnint.h tcpndrv.h tct3.h tct3base.h tct3int.h tct3drv.h tcgen.h vmfile.h tctok.h
tcprsnl.o: tcprsnl.cpp tcprs.h tctok.h vmhash.h t3std.h utf8.h tcerrnum.h vmerr.h tcmain.h tcglob.h tcpnbase.h tcpnint.h tcpndrv.h tct3.h tct3base.h tct3int.h tct3drv.h tcgen.h vmfile.h tctok.h tcprstyp.h
tcprsstm.o: tcprsstm.cpp tcprs.h tctok.h vmhash.h t3std.h utf8.h tcerrnum.h vmerr.h tcmain.h tcglob.h tcpnbase.h tcpnint.h tcpndrv.h tct3.h tct3base.h tct3int.h tct3drv.h tcgen.h vmfile.h tcprstyp.h utf8.h tcmake.h
tcprsimg.o: tcprsimg.cpp tcprs.h tctok.h vmhash.h t3std.h utf8.h tcerrnum.h vmerr.h tcmain.h tcglob.h tcpnbase.h tcpnint.h tcpndrv.h tct3.h tct3base.h tct3int.h tct3drv.h tcgen.h vmfile.h tcprstyp.h
tcgen.o: tcgen.cpp tcgen.h tcerrnum.h vmerr.h tcglob.h tctok.h tcprs.h tcpnbase.h tcpnint.h tcpndrv.h tct3.h tct3base.h tct3int.h tct3drv.h tcmain.h vmfile.h tcprstyp.h
tct3unas.o: tct3unas.cpp tct3unas.h tcunas.h vmtype.h vmop.h
tct3.o: tct3.cpp tct3.h tcgen.h tcprs.h tcpnbase.h tcpnint.h tcpndrv.h tcmain.h tcerr.h tct3base.h tct3int.h tct3drv.h vmfile.h vmwrtimg.h vmbignum.h tcprstyp.h vmrunsym.h tct3unas.h tcunas.h
tct3_d.o: tct3_d.cpp tct3.h tcpnbase.h tcpnint.h tcpndrv.h tct3base.h tct3int.h tct3drv.h tcprs.h tcprstyp.h
tct3stm.o: tct3stm.cpp tct3.h tcgen.h tcprs.h tcprstyp.h tcpnbase.h tcpnint.h tcpndrv.h tcmain.h tcerr.h tct3base.h tct3int.h tct3drv.h vmfile.h vmwrtimg.h
tct3img.o: tct3img.cpp tct3.h tcgen.h tcprs.h tcprstyp.h tcpnbase.h tcpnint.h tcpndrv.h tcmain.h tcerr.h tctok.h tct3base.h tct3int.h tct3drv.h vmfile.h vmwrtimg.h vmgram.h tcmake.h
tct3nl.o: tct3nl.cpp tct3.h tcprs.h tcprstyp.h tcpnbase.h tcpnint.h tcpndrv.h tct3base.h tct3int.h tct3drv.h
tcmake.o: tcmake.cpp tcmake.h tcsrc.h tctok.h resload.h t3std.h tcmain.h tchost.h tcprs.h tcprstyp.h tcpnbase.h tcpnint.h tcpndrv.h tct3.h tct3base.h tct3int.h tct3drv.h vmpreini.h vmhostsi.h vmhost.h tcvsn.h vmrunsym.h vmmaincn.h vmhosttx.h tcunas.h
tcmakecl.o: tcmakecl.cpp tcmake.h t3std.h tchost.h tchostsi.h vmerr.h tcvsn.h resload.h tcmain.h tclibprs.h tccmdutl.h
tccmdutl.o: tccmdutl.cpp tccmdutl.h t3std.h
tclibprs.o: tclibprs.cpp tclibprs.h t3std.h
rcmain.o: rcmain.cpp rcmain.h t3std.h vmimage.h
rcmaincl.o: rcmaincl.cpp rcmain.h t3std.h vmimage.h
msgcomp.o: msgcomp.cpp t3std.h vmerr.h charmap.h resload.h vmregex.h vmhash.h vmimage.h

os_stdio.o: os_stdio.cpp
vmuni_cs.o: vmuni_cs.cpp
osunix.o: osunix.c
osunix3.o: osunix3.c
wcs.o: wcs.cpp

test_utf8.o: test_utf8.cpp utf8.h
test_chr.o: test_chr.cpp utf8.h charmap.h resload.h
test_gets.o: test_gets.cpp tcsrc.h resload.h
test_pool.o: test_pool.cpp vmpool.h
test_err.o: test_err.cpp vmerr.h
test_obj.o: test_obj.cpp vmobj.h vmtobj.h vmundo.h vmstack.h vmtype.h vmlst.h vmstr.h vmrun.h vmhost.h vmhostsi.h vminit.h vmmaincn.h vmhosttx.h
test_write.o: test_write.cpp vmfile.h vmwrtimg.h vmerr.h
test_exec.o: test_exec.cpp vmmain.h vmconsol.h vmhostsi.h vmhosttx.h
test_tok.o: test_tok.cpp tcsrc.h tctok.h resload.h tcmain.h tchostsi.h tchost.h vmrunsym.h
test_prs.o: test_prs.cpp tcsrc.h tctok.h resload.h tcmain.h tchostsi.h tchost.h tcprs.h tcpnbase.h tcpnint.h tcpndrv.h tct3.h tct3base.h tct3int.h tct3drv.h tct3unas.h tcunas.h tcprstyp.h vmrunsym.h
test_pre.o: test_pre.cpp t3std.h vmpreini.h vmfile.h vmerr.h vmhost.h vmhostsi.h vmmaincn.h vmhosttx.h
test_prs_top.o: test_prs_top.cpp tcsrc.h tctok.h resload.h tcmain.h tchostsi.h tchost.h tcprs.h tcpnbase.h tcpnint.h tcpndrv.h tct3.h tct3base.h tct3int.h tct3drv.h tct3unas.h tcunas.h tcprstyp.h vmrunsym.h
test_sym.o: test_prs.cpp tcsrc.h tctok.h resload.h tcmain.h tchostsi.h tchost.h tcprs.h tcpnbase.h tcpnint.h tcpndrv.h tct3base.h tct3int.h tct3drv.h tct3.h tcprstyp.h vmrunsym.h
test_comp_obj.o: test_comp_obj.cpp tcsrc.h tctok.h resload.h tcmain.h tchostsi.h tchost.h tcprs.h tcpnbase.h tcpnint.h tcpndrv.h tct3.h tct3base.h tct3int.h tct3drv.h tcprstyp.h vmrunsym.h
test_link.o: test_link.cpp tcsrc.h tctok.h resload.h tcmain.h tchostsi.h tchost.h tcprs.h tcpnbase.h tcpnint.h tcpndrv.h tct3.h tct3base.h tct3int.h tct3drv.h tcprstyp.h vmrunsym.h
test_regex.o: test_regex.cpp vmregex.h

############################## Implicit Rules #############################

.SUFFIXES: .c .cpp .o

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
