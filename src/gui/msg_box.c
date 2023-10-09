#include <swilib.h>
#include <stdlib.h>
#include "../include/sie/gui/gui.h"
#include "../include/sie/freetype/freetype.h"

#define FONT_SIZE_MSG 20
#define FONT_SIZE_SOFT_KEYS 18
#define COLOR_BG {0x00, 0x00, 0x00, 0x64}
#define COLOR_BORDER {0x18, 0x18, 0x18, 0x64}
#define COLOR_SURFACE_BG {0x00, 0x00, 0x00, 0x20}

typedef struct {
    GUI gui;
    SIE_GUI_SURFACE *surface;
    WSHDR *msg_ws;
    void (*CallBackProc)(int);
} MAIN_GUI;

RECT canvas;

static void OnRedraw(MAIN_GUI *data) {
    const char color_bg[] = COLOR_BG;
    const char color_border[] = COLOR_BORDER;
    const char color_surface_bg[] = COLOR_SURFACE_BG;
    IMGHDR *scrot = &(data->surface->scrot);

    Sie_GUI_DrawIMGHDR(scrot, 0, 0, scrot->w, scrot->h);
    Sie_GUI_DrawIconBar();
    DrawRectangle(0, YDISP, SCREEN_X2, SCREEN_Y2, 0,
                  GetPaletteAdrByColorIndex(23), color_surface_bg);

    const int x = 20, x2 = SCREEN_X2 - 20;
    const int y = (YDISP + (ScreenH() - YDISP) / 2) - 80, y2 = y + 80 * 2;
    DrawRectangle(20, y, x2, y2, 0,color_border, color_bg);

    unsigned int w = 0, h = 0;
    // msg
    Sie_FT_GetStringSize(data->msg_ws, FONT_SIZE_MSG, &w, &h);
    const int x_msg = (ScreenW() / 2) - (int)w / 2;
    const int y_msg = (YDISP + (ScreenH() - YDISP) / 2) - (int)(h / 1.5);
    Sie_FT_DrawString(data->msg_ws, x_msg, y_msg, FONT_SIZE_MSG, NULL);
    // softkeys
    WSHDR *ws = AllocWS(128);
    wsprintf(ws, "%t", "Да");
    Sie_FT_GetStringSize(ws, FONT_SIZE_SOFT_KEYS, &w, &h);
    Sie_FT_DrawString(ws, x + 15, y2 - 5 - (int)h, FONT_SIZE_SOFT_KEYS, NULL);
    wsprintf(ws, "%t", "Нет");
    Sie_FT_GetStringSize(ws, FONT_SIZE_SOFT_KEYS, &w, &h);
    Sie_FT_DrawString(ws, x2 - 15 - (int)w, y2 - 5 - (int)h, FONT_SIZE_SOFT_KEYS, NULL);
    FreeWS(ws);
}

static void OnAfterDrawIconBar() {
    const char color_surface_bg[] = COLOR_SURFACE_BG;
    DrawRectangle(0, 0, SCREEN_X2, YDISP, 0,
                  GetPaletteAdrByColorIndex(23), color_surface_bg);
}

static void OnCreate(MAIN_GUI *data, void *(*malloc_adr)(int)) {
    data->gui.state = 1;
}

static void OnClose(MAIN_GUI *data, void (*mfree_adr)(void *)) {
    data->gui.state = 0;
    FreeWS(data->msg_ws);
    Sie_GUI_Surface_Destroy(data->surface);
}

static void OnFocus(MAIN_GUI *data, void *(*malloc_adr)(int), void (*mfree_adr)(void *)) {
    data->gui.state = 2;
    Sie_GUI_Surface_OnFocus(data->surface);
}

static void OnUnfocus(MAIN_GUI *data, void (*mfree_adr)(void *)) {
    if (data->gui.state != 2) return;
    data->gui.state = 1;
    Sie_GUI_Surface_OnUnfocus(data->surface);
}

static int OnKey(MAIN_GUI *data, GUI_MSG *msg) {
    if (data->CallBackProc) {
        if (msg->gbsmsg->msg == KEY_DOWN || msg->gbsmsg->msg == LONG_PRESS) {
            switch (msg->gbsmsg->submess) {
                case LEFT_SOFT:
                    data->CallBackProc(SIE_GUI_MSG_BOX_CALLBACK_YES);
                    return 1;
                case RIGHT_SOFT:
                    data->CallBackProc(SIE_GUI_MSG_BOX_CALLBACK_NO);
                    return 1;
            }
        }
    }
    return 0;
}

extern void kill_data(void *p, void (*func_p)(void *));

static int method8(void) { return 0; }

static int method9(void) { return 0; }

static const void *const gui_methods[11] = {
        (void *)OnRedraw,
        (void *)OnCreate,
        (void *)OnClose,
        (void *)OnFocus,
        (void *)OnUnfocus,
        (void *)OnKey,
        0,
        (void *)kill_data,
        (void *)method8,
        (void *)method9,
        0
};

void MsgBox(WSHDR *ws, void(*CallBackProc)(int)) {
    MAIN_GUI *main_gui = malloc(sizeof(MAIN_GUI));
    const SIE_GUI_SURFACE_HANDLERS surface_handlers = {
            OnAfterDrawIconBar,
            NULL
    };
    zeromem(main_gui, sizeof(MAIN_GUI));
    main_gui->msg_ws = AllocWS(ws->maxlen);
    wstrcpy(main_gui->msg_ws, ws);
    main_gui->CallBackProc = CallBackProc;
    main_gui->surface = Sie_GUI_Surface_Init(SIE_GUI_SURFACE_TYPE_DEFAULT, &surface_handlers);

    Sie_GUI_Surface_DoScrot(main_gui->surface);
    LockSched();
    Sie_GUI_InitCanvas(&canvas);
    main_gui->gui.canvas = (RECT*)(&canvas);
    main_gui->gui.methods = (void*)gui_methods;
    main_gui->gui.item_ll.data_mfree = (void (*)(void *))mfree_adr();
    CreateGUI(main_gui);
    UnlockSched();
}

void Sie_GUI_MsgBoxYesNo(WSHDR *ws, void(*CallBackProc)(int)) {
    MsgBox(ws, CallBackProc);
}
