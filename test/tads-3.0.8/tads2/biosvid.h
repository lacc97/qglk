#ifndef	_BIOSVID_
#define	_BIOSVID_

extern void bios_video_init();
extern void bios_video_get_screen_dimensions(int *x, int *y);
extern void bios_video_set_bkgnd(int bkgnd);
extern void bios_video_set_color(int color);
extern int bios_is_ega_or_vga();
extern int bios_video_monochrome();
extern void bios_video_write_char(char c);
extern void bios_video_write_string(char *s);
extern void bios_video_write_string_at_pos(int x, int y, char *s);
extern void bios_video_get_cursor_position(int *x, int *y);
extern void bios_video_set_cursor_position(int x, int y);
extern void bios_video_scroll_region(int top, int bottom, int left, int right, int lines);
extern void bios_video_clear_region(int top, int bottom, int left, int right);
extern void bios_video_set_cursor_shape(char start, char end);
extern void bios_video_cursor_hide();
extern void bios_video_cursor_show();
extern int bios_getchar();

#endif	/* _BIOSVID_ */
