#charset "us-ascii"

/* 
 *   Copyright (c) 1999, 2002 Michael J. Roberts
 *   
 *   This file is part of TADS 3 
 */

/*
 *   TADS Input/Output intrinsic function set 
 */

#ifndef TADSIO_H
#define TADSIO_H

/*
 *   the TADS input/output function set 
 */
intrinsic 'tads-io/030007'
{
    /* display values on the console */
    tadsSay(val, ...);

    /* set the output/command log file */
    setLogFile(fname, logType?);

    /* clear the display */
    clearScreen();

    /* show the "more" prompt, if supported on the platform */
    morePrompt();

    /* read a line of text from the keyboard */
    inputLine();

    /* read a single keystroke from the keyboard */
    inputKey();

    /* read a single input event */
    inputEvent(timeout?);

    /* display a simple dialog */
    inputDialog(icon, prompt, buttons, defaultButton, cancelButton);

    /* display a file selector dialog */
    inputFile(prompt, dialogType, fileType, flags);

    /* pause */
    timeDelay(delayMilliseconds);

    /* retrieve local system information */
    systemInfo(infoType, ...);

    /* set the status-line display mode, for text-only interpreters */
    statusMode(mode);

    /* write text on the right half of the status, for text-only terps */
    statusRight(txt);

    /* determine if a multimedia resource exists */
    resExists(fname);

    /* set the script input file */
    setScriptFile(filename, flags?);

    /* get the local default character set */
    getLocalCharSet(which);

    /* flush text output and update the display */
    flushOutput();

    /* read a line of text from the keyboard with an optional timeout */
    inputLineTimeout(timeout?);

    /* cancel an input line that was interrupted by timeout */
    inputLineCancel(reset);

    /* create a banner window */
    bannerCreate(parent, where, other, windowType, align,
                 size, sizeUnits, styleFlags);

    /* delete a banner window */
    bannerDelete(handle);

    /* 
     *   Clear the contents of a banner window.  'color' is the color to use
     *   for the screen color after clearing the window, given as a ColorXxx
     *   value (see below).  
     */
    bannerClear(handle);

    /* write text to a banner window */
    bannerSay(handle, ...);

    /* flush all buffers for a banner window */
    bannerFlush(handle);

    /* size a banner to fit its contents */
    bannerSizeToContents(handle);

    /* go to an output position (meaningful in a text grid banner only) */
    bannerGoTo(handle, row, col);

    /* 
     *   Set text foreground and background colors.  This affects the color
     *   of subsequently displayed text; text displayed previously is not
     *   affected.  The colors are given as ColorXxx values (see below).  If
     *   'bg' is ColorTransparent, then text is shown with the current screen
     *   color in the window.  
     */
    bannerSetTextColor(handle, fg, bg);

    /*
     *   Set the "screen color" in the banner window.  This is the color used
     *   to fill parts of the window that aren't displaying any text, and as
     *   the background color for all text displayed when the text background
     *   color is ColorTransparent.  Setting the screen color immediately
     *   sets the color for the entire window - even text previously
     *   displayed in the window is affected by this change.  
     */
    bannerSetScreenColor(handle, color);

    /* get information on the banner */
    bannerGetInfo(handle);

    /* set the size */
    bannerSetSize(handle, size, sizeUnits, isAdvisory);

    /* 
     *   Create a log file console.  This creates a console that has no
     *   display, but simply captures its output to the given log file.
     *   Writing to a log console is different from writing to a regular text
     *   file in that we apply all of the normal formatting (including
     *   text-only-mode HTML interpretation) to the output sent to this
     *   console.  
     */
    logConsoleCreate(filename, charset, width);

    /* close a log console; the console is no longer valid after this */
    logConsoleClose(handle);

    /* write text to a log console */
    logConsoleSay(handle, ...);
}

/* log file types */
#define LogTypeTranscript 1     /* log all input and output to a transcript */
#define LogTypeCommand    2                       /* log only command input */

/* 
 *   constants for the event codes returned by the inputEvent() and
 *   inputLineTimeout() intrinsic functions 
 */
#define InEvtKey        1
#define InEvtTimeout    2
#define InEvtHref       3
#define InEvtNotimeout  4
#define InEvtEof        5
#define InEvtLine       6
#define InEvtEndQuietScript  10000

/*
 *   constants for inputDialog() 
 */
#define InDlgOk               1
#define InDlgOkCancel         2
#define InDlgYesNo            3
#define InDlgYesNoCancel      4

#define InDlgIconNone        0
#define InDlgIconWarning     1
#define InDlgIconInfo        2
#define InDlgIconQuestion    3
#define InDlgIconError       4

#define InDlgLblOk           1
#define InDlgLblCancel       2
#define InDlgLblYes          3
#define InDlgLblNo           4

/*
 *   inputFile() dialog types 
 */
#define InFileOpen    1                /* open an existing file for reading */
#define InFileSave    2                                 /* save to the file */

/*
 *   inputFile() return codes - these are returned in the first element of
 *   the list returned from inputFile() 
 */
#define InFileSuccess        0    /* success - 2nd list element is filename */
#define InFileFailure        1       /* an error occurred asking for a file */
#define InFileCancel         2         /* player canceled the file selector */

/*
 *   constants for inputFile() file type codes 
 */
#define FileTypeLog     2                        /* a transcript (log) file */
#define FileTypeData    4                            /* arbitrary data file */
#define FileTypeCmd     5                             /* command input file */
#define FileTypeText    7                                      /* text file */
#define FileTypeBin     8                               /* binary data file */
#define FileTypeUnknown 11                             /* unknown file type */
#define FileTypeT3Image 12               /* T3 executable image (game) file */
#define FileTypeT3Save  15                           /* T3 saved state file */

/*
 *   constants for systemInfo information type codes 
 */
#define SysInfoVersion       2
#define SysInfoOsName        3
#define SysInfoJpeg          5
#define SysInfoPng           6
#define SysInfoWav           7
#define SysInfoMidi          8
#define SysInfoWavMidiOvl    9
#define SysInfoWavOvl        10
#define SysInfoPrefImages    11
#define SysInfoPrefSounds    12
#define SysInfoPrefMusic     13
#define SysInfoPrefLinks     14
#define SysInfoMpeg          15
#define SysInfoMpeg1         16
#define SysInfoMpeg2         17
#define SysInfoMpeg3         18
#define SysInfoLinksHttp     20
#define SysInfoLinksFtp      21
#define SysInfoLinksNews     22
#define SysInfoLinksMailto   23
#define SysInfoLinksTelnet   24
#define SysInfoPngTrans      25
#define SysInfoPngAlpha      26
#define SysInfoOgg           27
#define SysInfoMng           28
#define SysInfoMngTrans      29
#define SysInfoMngAlpha      30
#define SysInfoTextHilite    31
#define SysInfoTextColors    32
#define SysInfoBanners       33
#define SysInfoInterpClass   34

/* SysInfoTextColors support level codes */
#define SysInfoTxcNone       0
#define SysInfoTxcParam      1
#define SysInfoTxcAnsiFg     2
#define SysInfoTxcAnsiFgBg   3
#define SysInfoTxcRGB        4

/* SysInfoInterpClass codes */
#define SysInfoIClassText    1
#define SysInfoIClassTextGUI 2
#define SysInfoIClassHTML    3

/*
 *   constants for statusMode 
 */
#define StatModeNormal    0                       /* displaying normal text */
#define StatModeStatus    1                     /* display status line text */

/*
 *   flags for setScriptFile 
 */
#define ScriptFileQuiet    1  /* do not display output while reading script */
#define ScriptFileNonstop  2   /* turn off MORE prompt while reading script */

/*
 *   selectors for getLocalCharSet
 */
#define CharsetDisplay  1             /* the display/keyboard character set */
#define CharsetFileName 2                  /* the file system character set */
#define CharsetFileCont 3            /* default file contents character set */

/*
 *   banner insertion point specifies (for 'where' in bannerCreate)
 */
#define BannerFirst   1
#define BannerLast    2
#define BannerBefore  3
#define BannerAfter   4

/*
 *   banner types 
 */
#define BannerTypeText     1                 /* ordinary text stream window */
#define BannerTypeTextGrid 2                            /* text grid window */

/* 
 *   banner alignment types 
 */
#define BannerAlignTop     0
#define BannerAlignBottom  1
#define BannerAlignLeft    2
#define BannerAlignRight   3

/*
 *   banner size unit types 
 */
#define BannerSizePercent   1    /* size is a percentage of available space */
#define BannerSizeAbsolute  2    /* size is in natural units of window type */

/*
 *   banner style flags 
 */
#define BannerStyleBorder       0x0001       /* banner has a visible border */
#define BannerStyleVScroll      0x0002                /* vertical scrollbar */
#define BannerStyleHScroll      0x0004              /* horizontal scrollbar */
#define BannerStyleAutoVScroll  0x0008      /* automatic vertical scrolling */
#define BannerStyleAutoHScroll  0x0010    /* automatic horizontal scrolling */
#define BannerStyleTabAlign     0x0020           /* <TAB> alignment support */
#define BannerStyleMoreMode     0x0040                     /* use MORE mode */

/*
 *   Color codes.  A color can be specified with explicit RGB
 *   (red-green-blue) component values, or can be "parameterized," which
 *   means that the color uses a pre-defined color for a particular purpose.
 *   
 *   RGB colors are specified with each component given in the range 0 to
 *   255; the color (0,0,0) is pure black, and (255,255,255) is pure white.
 *   
 *   The special value "transparent" is not a color at all, but rather
 *   specifies that the current screen color should be used.
 *   
 *   The "Text" and "TextBg" colors are the current default text and text
 *   background colors.  The actual colors displayed for these values depend
 *   on the system, and on some systems these colors might be configurable by
 *   the user through a preferences selection.  These are the same colors
 *   selected by the HTML parameterized color names 'text' and 'bgcolor'.
 *   
 *   The "StatusText" and "StatusBg" colors are the current default
 *   statusline text and background colors, which depend on the system and
 *   may be user-configurable on some systems.  These are the same colors
 *   selected by the HTML parameterized color names 'statustext' and
 *   'statusbg'.
 *   
 *   The "input" color is the current default input text color.  
 */
#define ColorRGB(r, g, b) \
    ((((r) & 0xff) << 16) + (((g) & 0xff) << 8) + ((b) & 0xff))
#define ColorTransparent     0x01000000
#define ColorText            0x02000000
#define ColorTextBg          0x03000000
#define ColorStatusText      0x04000000
#define ColorStatusBg        0x05000000
#define ColorInput           0x06000000

/* some specific colors by name, for convenience */
#define ColorBlack    ColorRGB(0x00, 0x00, 0x00)
#define ColorWhite    ColorRGB(0xff, 0xff, 0xff)
#define ColorRed      ColorRGB(0xff, 0x00, 0x00)
#define ColorBlue     ColorRGB(0x00, 0x00, 0xFF)
#define ColorGreen    ColorRGB(0x00, 0x80, 0x00)
#define ColorYellow   ColorRGB(0xFF, 0xFF, 0x00)
#define ColorCyan     ColorRGB(0x00, 0xFF, 0xFF)
#define ColorAqua     ColorRGB(0x00, 0xFF, 0xFF)
#define ColorMagenta  ColorRGB(0xFF, 0x00, 0xFF)
#define ColorSilver   ColorRGB(0xC0, 0xC0, 0xC0)
#define ColorGray     ColorRGB(0x80, 0x80, 0x80)
#define ColorMaroon   ColorRGB(0x80, 0x00, 0x00)
#define ColorPurple   ColorRGB(0x80, 0x00, 0x80)
#define ColorFuchsia  ColorRGB(0xFF, 0x00, 0xFF)
#define ColorLime     ColorRGB(0x00, 0xFF, 0x00)
#define ColorOlive    ColorRGB(0x80, 0x80, 0x00)
#define ColorNavy     ColorRGB(0x00, 0x00, 0x80)
#define ColorTeal     ColorRGB(0x00, 0x80, 0x80)


#endif /* TADSIO_H */
