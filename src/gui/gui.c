#include <swilib.h>
#include <stdlib.h>
#include "../include/sie/resources.h"
#include "../include/sie/gui/gui.h"
#include "../include/sie/freetype/freetype.h"

#define FONT_SIZE_ICONBAR 14
#define FONT_SIZE_HEADER 20
#define COLOR_HEADER_BG {0x00, 0x00, 0x00, 0x35}

extern IMGHDR *SIE_RES_IMG_WALLPAPER;

GBSTMR TIMER_DRAW_ICONBAR;

SIE_GUI_SURFACE *Sie_GUI_Surface_Init(int type, const SIE_GUI_SURFACE_HANDLERS *handlers) {
    SIE_GUI_SURFACE *surface = malloc(sizeof(SIE_GUI_SURFACE));
    zeromem(surface, sizeof(SIE_GUI_SURFACE));
    surface->type = type;
    if (handlers) {
        memcpy(&(surface->handlers), handlers, sizeof(SIE_GUI_SURFACE_HANDLERS));
    }
    surface->hdr_ws = AllocWS(256);
    return surface;
};

void Sie_GUI_Surface_Destroy(SIE_GUI_SURFACE *surface) {
    FreeWS(surface->hdr_ws);
    if (surface->scrot.bitmap) {
        mfree(surface->scrot.bitmap);
    }
    mfree(surface);
}

void Sie_GUI_Surface_DoScrot(SIE_GUI_SURFACE *surface) {
    LockSched();
    size_t size = ScreenW() * ScreenH() * 2;
    surface->scrot.w = ScreenW();
    surface->scrot.h = ScreenH();
    surface->scrot.bpnum = 8;
    surface->scrot.bitmap = malloc(size);
    memcpy(surface->scrot.bitmap, RamScreenBuffer(), size);
    UnlockSched();
}

void Sie_GUI_Surface_Draw(const SIE_GUI_SURFACE *surface) {
    Sie_GUI_DrawIconBar();
    if (wstrlen(surface->hdr_ws)) {
        Sie_GUI_DrawHeader(surface->hdr_ws);
    }
}

void Sie_GUI_Surface_OnFocus(SIE_GUI_SURFACE *surface) {
#ifdef ELKA
    DisableIconBar(1);
#endif
    TIMER_DRAW_ICONBAR.param6 = (unsigned int)surface;
    GBS_StartTimerProc(&TIMER_DRAW_ICONBAR, 216, Sie_GUI_DrawIconBar);
}

void Sie_GUI_Surface_OnUnFocus(SIE_GUI_SURFACE *surface) {
#ifdef ELKA
    DisableIconBar(0);
#endif
    TIMER_DRAW_ICONBAR.param6 = 0;
    GBS_StopTimer(&TIMER_DRAW_ICONBAR);
}

int Sie_GUI_Surface_OnKey(SIE_GUI_SURFACE *surface, void *data, GUI_MSG *msg) {
    if (surface->handlers.OnKey) {
        return surface->handlers.OnKey(data, msg);
    }
    return 0;
}

/**********************************************************************************************************************/

void Sie_GUI_InitCanvas(RECT *canvas) {
    canvas->x = 0;
    canvas->y = 0;
    canvas->x2 = SCREEN_X2;
    canvas->y2 = SCREEN_Y2;
}

void Sie_GUI_SetRGB(char *rgb, char r, char g, char b) {
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}

void Sie_GUI_DrawIMGHDR(IMGHDR *img, int x, int y, int w, int h) {
    RECT rc;
    DRWOBJ drwobj;
    StoreXYWHtoRECT(&rc, x, y, w, h);
    SetPropTo_Obj5(&drwobj, &rc, 0, img);
    SetColor(&drwobj, NULL, NULL);
    DrawObject(&drwobj);
}

void Sie_GUI_DrawBleedIMGHDR(IMGHDR *img, int x, int y, int x2, int y2, int bleed_x, int bleed_y) {
    RECT rc;
    DRWOBJ drwobj;
    StoreXYXYtoRECT(&rc, x, y, x2, y2);
    SetPropTo_Obj5(&drwobj, &rc, 0, img);
    SetProp2ImageOrCanvas(&drwobj, &rc, 0, img, bleed_x, bleed_y);
    SetColor(&drwobj, NULL, NULL);
    DrawObject(&drwobj);
}

void Sie_GUI_DrawIconBar() {
    const SIE_GUI_SURFACE *surface = NULL;
    if (TIMER_DRAW_ICONBAR.param6) {
        surface = (SIE_GUI_SURFACE*)TIMER_DRAW_ICONBAR.param6;
    }
    if (IsTimerProc(&TIMER_DRAW_ICONBAR)) {
        GBS_StopTimer(&TIMER_DRAW_ICONBAR);
    }
//    if (surface) {
//        TIMER_DRAW_ICONBAR.param6 = (unsigned int)surface;
//    }

    Sie_GUI_DrawIMGHDR(SIE_RES_IMG_WALLPAPER, 0, 0, ScreenW(), YDISP);

    int x = 0, y = 0;
    char img_name[64];
    unsigned int w = 0, h = 0;
    WSHDR *ws = AllocWS(64);

    // free ram
    wsprintf(ws, "%d %t", GetFreeRamAvail(), "Б");
    Sie_FT_DrawString(ws,
                      0 + PADDING_ICONBAR,
                      0 + (YDISP - Sie_FT_GetMaxHeight(FONT_SIZE_ICONBAR)) / 2,
                      FONT_SIZE_ICONBAR, NULL);
    // battery cap
    int percent = 0;
    const int cap = *RamCap();
    const unsigned short ls = *RamLS();
    if (cap <= 5) {
        percent = 0;
    } else if (cap <= 20) {
        percent = 20;
    } else if (cap <= 40) {
        percent = 40;
    } else if (cap <= 60) {
        percent = 60;
    } else if (cap <= 90) {
        percent = 80;
    } else if (cap <= 100) {
        percent = 100;
    }
    sprintf(img_name, "%s-%02d", "battery", percent);
    if (ls) {
        strcat(img_name, "-charging");
    }
    SIE_RESOURCES_IMG *res = Sie_Resources_LoadImage(SIE_RESOURCES_TYPE_STATUS, 24, img_name);
    if (res) {
        IMGHDR *img = res->icon;
        x = SCREEN_X2 - PADDING_ICONBAR - img->w;
        y = 0 + (YDISP - img->h) / 2;
        Sie_GUI_DrawIMGHDR(img, x, y, img->w, img->h);
    } else {
        wsprintf(ws, "%d%%", *RamCap());
        Sie_FT_GetStringSize(ws, FONT_SIZE_ICONBAR, &w, &h);
        x = SCREEN_X2 - PADDING_ICONBAR - w;
        y = 0 + (YDISP - (int)h) / 2;
        Sie_FT_DrawString(ws, x, y, FONT_SIZE_ICONBAR, NULL);
    }
    // clock
    TTime time;
    GetDateTime(NULL, &time);
    wsprintf(ws, "%02d:%02d", time.hour, time.min);
    Sie_FT_GetStringSize(ws, FONT_SIZE_ICONBAR, &w, &h);
    x = x - PADDING_ICONBAR - (int)w;
    y = 0 + (YDISP - (int)h) / 2;
    Sie_FT_DrawString(ws,  x, y, FONT_SIZE_ICONBAR, NULL);
    
    FreeWS(ws);
    if (surface) {
        if (surface->handlers.OnAfterDrawIconBar) {
            surface->handlers.OnAfterDrawIconBar();
        }
    }
    GBS_StartTimerProc(&TIMER_DRAW_ICONBAR, 216, Sie_GUI_DrawIconBar);
}

void Sie_GUI_DrawHeader(WSHDR *ws) {
    const char color_bg[] = COLOR_HEADER_BG;
    const int header_x = 0;
    const int header_y = YDISP;
    const int header_x2 = SCREEN_X2;
    const int header_y2 = header_y + HeaderH();
    Sie_GUI_DrawBleedIMGHDR(SIE_RES_IMG_WALLPAPER,
                            header_x, header_y, header_x2, header_y2,
                            0, YDISP);
    DrawRectangle(header_x, header_y, header_x2, header_y2 - 1, 0, color_bg, color_bg);

    int text_x = 0;
    int text_y = 0;

    char str[64];
    WSHDR *ws_left = AllocWS(64);
    WSHDR *ws_right = AllocWS(64);
    ws_2str(ws, str, wstrlen(ws));
    char *p = strchr(str, '\t');
    if (p) {
        const int len_left = p - str;
        if (len_left > 0) {
            str_2ws(ws_left, str, len_left);
        }
        str_2ws(ws_right, p + 1, strlen(str) - (p - str - 1));
    } else {
        wstrcpy(ws_left, ws);
    }
    text_x = 0 + PADDING_HEADER;
    text_y = YDISP + (HeaderH() - Sie_FT_GetMaxHeight(FONT_SIZE_HEADER)) / 2;
    if (wstrlen(ws_left)) {
        Sie_FT_DrawString(ws_left, text_x, text_y, FONT_SIZE_HEADER, NULL);
    }
    if (wstrlen(ws_right)) {
        unsigned int w = 0, h = 0;
        Sie_FT_GetStringSize(ws_right,  FONT_SIZE_HEADER, &w, &h);
        text_x = SCREEN_X2 - 1 - PADDING_HEADER - w;
        Sie_FT_DrawString(ws_right, text_x, text_y, FONT_SIZE_HEADER, NULL);
    }
    FreeWS(ws_left);
    FreeWS(ws_right);
}
