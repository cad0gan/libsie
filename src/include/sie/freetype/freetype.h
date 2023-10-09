#ifndef __SIE_FREETYPE_H__
#define __SIE_FREETYPE_H__

#include <wchar.h>
#include <swilib.h>

#define SIE_FT_TEXT_ALIGN_LEFT TEXT_ALIGNLEFT

extern void Sie_FT_Init();
extern void Sie_FT_Destroy();


int Sie_FT_GetMaxHeight(int font_size);
void Sie_FT_GetStringSize(WSHDR *ws, int font_size, unsigned int *w, unsigned int *h);
void Sie_FT_DrawString(WSHDR *ws, int x, int y, int font_size, const char *rgb);
void Sie_FT_DrawBoundingString(WSHDR *ws, int x, int y, int x2, int y2, int font_size, int attr, const char *rgb);

#endif
