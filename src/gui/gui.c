#include <swilib.h>
#include <stdlib.h>
#include <string.h>
#include <swilib/nucleus.h>
#include "../include/sie/gui/gui.h"

#define TMR_MS_FOCUS (5)

void Focus_GUI(GBSTMR *tmr) {
    SIE_GUI_FOCUS_DATA *data = (SIE_GUI_FOCUS_DATA*)tmr->param6;
    unsigned int gui_id = data->gui_id;
    if (IsGuiOnTop((int)gui_id)) {
        GBS_DelTimer(tmr);
        if (data->proc) {
            data->proc(data->data);
        }
    } else {
        GBS_StartTimerProc(tmr, TMR_MS_FOCUS, Focus_GUI);
    }
}

void Sie_GUI_FocusGUI_Async(GBSTMR *tmr, SIE_GUI_FOCUS_DATA *data) {
    tmr->param6 = (int)data;
    FocusGUI((int)data->gui_id);
    Focus_GUI(tmr);
}

void Sie_GUI_FocusGUI_Sync(unsigned int gui_id) {
    FocusGUI((int)gui_id);
    while (!IsGuiOnTop((int)gui_id)) {NU_Sleep(50);}
}

void Sie_GUI_InitCanvas(RECT *canvas) {
    canvas->x = 0;
    canvas->y = 0;
    canvas->x2 = (short)(ScreenW() - 1);
    canvas->y2 = (short)(ScreenH() - 1);
}

void Sie_GUI_InitHeader(HEADER_DESC *header_desc) {
    header_desc->rc.x = 0;
    header_desc->rc.y = YDISP;
    header_desc->rc.x2 = (short)(ScreenW() - 1);
    header_desc->rc.y2 = (short)(YDISP + HeaderH() - 1);
}

void Sie_GUI_InitHeaderSmall(HEADER_DESC *header_desc) {
    header_desc->rc.x = 0;
    header_desc->rc.y = (short)(YDISP + HeaderH());
    header_desc->rc.x2 = (short)(ScreenW() - 1);
#ifdef ELKA
    header_desc->rc.y += 4;
    header_desc->rc.y2 = (short)(header_desc->rc.y + HeaderH() - 1);
#else
    header_desc->rc.y2 = (short)(HeaderH() + 0x11);
#endif
}

void Sie_GUI_InitInput(INPUTDIA_DESC *input_desc) {
    input_desc->rc.x = 0;
    input_desc->rc.y = YDISP + HeaderH() + 1;
    input_desc->rc.x2 = (short)(ScreenW() - 1);
    input_desc->rc.y2 = (short)(ScreenH() - SoftkeyH() - 1);
}

void Sie_GUI_SetRGB(char *rgb, char r, char g, char b) {
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}

IMGHDR *Sie_GUI_TakeScrot() {
    LockSched();
    IMGHDR *scrot = malloc(sizeof(IMGHDR));
    size_t size = CalcBitmapSize((short) ScreenW(), (short) ScreenH(), IMGHDR_TYPE_BGR565);
    scrot->w = ScreenW();
    scrot->h = ScreenH();
    scrot->bpnum = IMGHDR_TYPE_BGR565;
    scrot->bitmap = malloc(size);
    memcpy(scrot->bitmap, RamScreenBuffer(), size);
    UnlockSched();
    return scrot;
}
