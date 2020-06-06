/* #define STANDALONE_BIOS_VIDEO_TEST */

/*
 * BIOS video routines for GO32 version of TADS
 *
 * Modification History
 *
 * 22-Apr-95	dmb	Created.
 *
 */
#include <pc.h>
#include <dos.h>
#include <sys/farptr.h>

#include "biosvid.h"

static int Page, Cols = 80, Rows = 25, Color = 7, Bkgnd = 7, Mode;
static int Cstart, Cend;

#ifdef STANDALONE_BIOS_VIDEO_TEST

main(int argc, char **argv)
{
	int	i;
	char	s[80];
	
	bios_video_init();
	bios_video_clear_region(0, Rows - 1, 0, Cols - 1);
	sprintf(s, "cols: %d   rows: %d   cursor = [%d, %d]", Cols, Rows, Cstart, Cend);
	bios_video_write_string_at_pos(6, 12, s);
	bios_video_set_cursor_position(5, 13);
	bios_video_write_char('!');
	
	for (i = 0; i < 1000; i++) {
		bios_video_scroll_region(0, Rows - 1, 0, Cols - 1, 1);
		bios_video_scroll_region(0, Rows - 1, 0, Cols - 1, -1);
	}
}
#endif

void
bios_video_get_screen_dimensions(int *x, int *y)
{
	*x = Cols;
	*y = Rows;
}

void
bios_video_set_bkgnd(int bkgnd)
{
	Bkgnd = bkgnd;
}

void
bios_video_set_color(int color)
{
	Color = color;
}

void
bios_video_init()
{
	union REGS regs;
	
	regs.h.ah = 15;
	int86(0x10, &regs, &regs);
	Mode = regs.h.al;
	Page = regs.h.bh;

	/*
	 * If EGA or VGA, determine screen size.
	 * Otherwise, assume 80x25.
	 */
	if (bios_is_ega_or_vga()) {
		unsigned short s;
			
		Rows = (int) _farpeekb(_go32_conventional_mem_selector(), 0x40*16 + 0x84) + 1;
		Cols = (int) _farpeekw(_go32_conventional_mem_selector(), 0x40*16 + 0x4a);
		s = (int) _farpeekw(_go32_conventional_mem_selector(), 0x40*16 + 0x60);
		Cstart = s >> 8;
		Cend = s & 0xFF;
	}
	else {
		Cstart = 6;
		Cend = 7;
		Cols = 80;
		Rows = 25;
	}
}

int
bios_is_ega_or_vga()
{
	union REGS regs;
	
	regs.x.ax = 0x1a00;
	int86(0x10, &regs, &regs);
	if (regs.h.al == 0x1a) {
		switch (regs.h.bl) {
			case 4:	/* color EGA */
			case 5: /* mono EGA */
			case 7: /* mono VGA */
			case 8: /* color VGA */
				return 1;
			default:
				return 0;
		}
	}
	
	regs.h.ah = 0x12;
	regs.h.bl = 0x10;
	int86(0x10, &regs, &regs);
	if (regs.h.bl != 0x10)
		return 1;

	return 0;
}

int
bios_video_monochrome()
{
	if (Mode == 7)
		return 1;
	else
		return 0;
}

void
bios_video_set_cursor_shape(char start, char end)
{
	union REGS regs;
	regs.h.ah = 1;
	regs.h.ch = start;
	regs.h.cl = end;
	int86(0x10, &regs, &regs);
}

/* write char to screen at current cursor pos and with current attr */
void
bios_video_write_char(char c)
{
	union REGS regs;

	regs.h.ah = 9;
	regs.h.al = c;
	regs.h.bh = Page;
	regs.h.bl = Color;
	regs.x.cx = 1;
	int86(0x10, &regs, &regs);
}

void
bios_video_write_string(char *s)
{
	int	x, y;
	
	bios_video_get_cursor_position(&x, &y);
	while (*s) {
		bios_video_set_cursor_position(x++, y);
		bios_video_write_char(*s++);
	}
}

void
bios_video_write_string_at_pos(int x, int y, char *s)
{
	while (*s) {
		bios_video_set_cursor_position(x++, y);
		bios_video_write_char(*s++);
	}
}

void
bios_video_set_cursor_position(int x, int y)
{
	union REGS regs;
	
	if (x >= Cols || y >= Rows)
		return;
	
	regs.h.ah = 2;
	regs.h.bh = Page;
	regs.h.dh = y;
	regs.h.dl = x;
	regs.x.cx = 1;
	int86(0x10, &regs, &regs);
}

void
bios_video_get_cursor_position(int *x, int *y)
{
	union REGS regs;
	
	regs.h.ah = 3;
	regs.h.bh = Page;
	int86(0x10, &regs, &regs);
	*x = (int) regs.h.dl;
	*y = (int) regs.h.dh;
}

void
bios_video_scroll_region(int top, int bottom, int left, int right, int lines)
{
	union REGS regs;
	
	if (lines < 0) {
		/* scroll up --- delete lines */
		regs.h.ah = 6;
		regs.h.al = -lines;
		regs.h.bh = Bkgnd;
		regs.h.ch = top;
		regs.h.cl = left;
		regs.h.dh = bottom;
		regs.h.dl = right;
		int86(0x10, &regs, &regs);
	}
	else {
		/* scroll down --- insert lines */
		regs.h.ah = 7;
		regs.h.al = lines;
		regs.h.bh = Bkgnd;
		regs.h.ch = top;
		regs.h.cl = left;
		regs.h.dh = bottom;
		regs.h.dl = right;
		int86(0x10, &regs, &regs);
	}
}

void
bios_video_clear_region(int top, int bottom, int left, int right)
{
	union REGS regs;

	regs.h.ah = 6;
	regs.h.al = 0;
	regs.h.bh = Bkgnd;
	regs.h.ch = top;
	regs.h.cl = left;
	regs.h.dh = bottom;
	regs.h.dl = right;
	int86(0x10, &regs, &regs);
}

void
bios_video_cursor_hide()
{
	bios_video_set_cursor_shape(Cend, Cstart);
}

void
bios_video_cursor_show()
{
	bios_video_set_cursor_shape(Cstart, Cend);
}

int
bios_getchar()
{
	union REGS regs;

	regs.h.ah = 7;
	int86(0x21, &regs, &regs);
	return (int) regs.h.al;
}
