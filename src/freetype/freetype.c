#include <swilib.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "../include/sie/freetype/cache.h"
#include "../include/sie/freetype/freetype.h"

extern char SKIN_FONT_REGULAR_PATH[];

FT_Library *FT_LIBRARY;
FT_Face *FT_FACE_REGULAR;
unsigned int FT_CLIENTS_COUNT;

void Sie_FT_Init() {
    if (!FT_LIBRARY) {
        FT_LIBRARY = malloc(sizeof(FT_Library));
        FT_Init_FreeType(FT_LIBRARY);
        Sie_FT_LoadFaces();
    }
    FT_CLIENTS_COUNT++;
}

void Sie_FT_Destroy() {
    if (FT_CLIENTS_COUNT) {
        FT_CLIENTS_COUNT--;
    }
    if (!FT_CLIENTS_COUNT) {
        if (FT_LIBRARY) {
            Sie_FT_UnloadFaces();
            FT_Done_FreeType(*FT_LIBRARY);
            mfree(FT_LIBRARY);
            FT_LIBRARY = NULL;
            Sie_FT_DestroyCache();
        }
    }
}

void Sie_FT_UnloadFaces() {
    if (FT_FACE_REGULAR) {
        FT_Done_Face(*FT_FACE_REGULAR);
        mfree(FT_FACE_REGULAR);
        FT_FACE_REGULAR = NULL;
    }
}

void Sie_FT_LoadFaces() {
    FT_FACE_REGULAR = malloc(sizeof(FT_Face));
    FT_New_Face(*FT_LIBRARY, SKIN_FONT_REGULAR_PATH,
                SIE_FT_FACE_REGULAR_ID, FT_FACE_REGULAR);
}

void Sie_FT_DestroyCache() {
    extern SIE_FT_CACHE *FT_CACHE;
    extern unsigned int FT_CACHE_SIZE;
    for (int i = 0; i < FT_CACHE_SIZE; i++) {
        SIE_FT_CACHE ft_cache = FT_CACHE[i];
        for (unsigned int j = 0; j < ft_cache.size; j++) {
            SIE_FT_GLYPH_CACHE ft_wc_cache = ft_cache.cache[j];
            mfree(ft_wc_cache.img->bitmap);
            mfree(ft_wc_cache.img);
        }
        mfree(ft_cache.cache);
    }
    mfree(FT_CACHE);
    FT_CACHE = NULL;
    FT_CACHE_SIZE = 0;
}

int Sie_FT_GetMaxHeight(int font_size) {
    FT_Face *face = FT_FACE_REGULAR;
    FT_Set_Pixel_Sizes(*face, 0, font_size);
    const int max_ascender = ((*face)->size->metrics.ascender >> 6);
    const int max_descender = abs(((*face)->size->metrics.descender >> 6));
    return max_ascender + max_descender;
}

void Sie_FT_GetStringSize(WSHDR *ws, int font_size, unsigned int *w, unsigned int *h) {
    FT_Face *face = FT_FACE_REGULAR;
    SIE_FT_CACHE *cache = FT_GetOrCreateCache(face, font_size);
    SIE_FT_GLYPH_CACHE *glyph_cache = NULL;

    unsigned int width = 0, height = Sie_FT_GetMaxHeight(font_size);
    FT_ULong charcode;
    for (unsigned int i = 0; i < wstrlen(ws); i++) {
        charcode = ws->wsbody[1 + i];
        glyph_cache = FT_GlyphGetOrAddCache(face, cache, charcode);
        width += glyph_cache->h_advance;
    }
    *w = width;
    *h = height;
}

IMGHDR *BrushGlyphIMGHDR(IMGHDR *img, const char *color) {
    IMGHDR *res = malloc(sizeof(IMGHDR));
    memcpy(res, img, sizeof(IMGHDR));
    size_t size = CalcBitmapSize((short)res->w, (short)res->h, IMGHDR_TYPE_BGRA8888);
    res->bitmap = malloc(size);
    memcpy(res->bitmap, img->bitmap, size);
    for (int i = 0; i < size; i += 4) {
        res->bitmap[i + 0] = color[2];
        res->bitmap[i + 1] = color[1];
        res->bitmap[i + 2] = color[0];
    }
    return res;
}
