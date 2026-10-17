// Font handles for the browser port. CreateFontA records height and weight;
// SelectObject remembers which one is current so text metrics and the glyph
// atlas can follow bold and big the way GDI does.

#include <stdlib.h>
#include <string.h>

struct MuFont {
    int height;
    int weight;
    int italic;
    char face[32];
    MuFont* next;
};

static MuFont* g_fonts = nullptr;
static MuFont* g_selected = nullptr;

static MuFont* find_font(void* handle) {
    for (MuFont* font = g_fonts; font; font = font->next)
        if (font == handle) return font;
    return nullptr;
}

extern "C" void* mu_font_create(int height, int weight, int italic, const char* face) {
    MuFont* font = (MuFont*)calloc(1, sizeof(MuFont));
    if (!font) return nullptr;
    // CreateFontA's positive height is the GDI cell, not the em. Arial at
    // cell 16 has a 13px em (internal leading 3); a larger cell leads a
    // little more. Canvas font-size is the em, so storing the cell made
    // every line about a fifth wider than the desktop client.
    int cell = height < 0 ? -height : height;
    if (cell < 1) cell = 12;
    if (cell > 64) cell = 64;
    int lead = 0;
    if (height >= 0) lead = (cell <= 16) ? 3 : (cell * 5 + 16) / 32;
    int em = cell - lead;
    if (em < 8) em = 8;
    font->height = em;
    font->weight = weight > 0 ? weight : 400;
    font->italic = italic ? 1 : 0;
    const char* src = (face && face[0]) ? face : "Arial";
    size_t i = 0;
    for (; i < sizeof(font->face) - 1 && src[i]; ++i) font->face[i] = src[i];
    font->face[i] = 0;
    font->next = g_fonts;
    g_fonts = font;
    if (!g_selected) g_selected = font;
    return font;
}

extern "C" void* mu_gdi_select_font(void* obj) {
    MuFont* prev = g_selected;
    MuFont* font = find_font(obj);
    if (font) g_selected = font;
    return prev;
}

extern "C" void* mu_gdi_current_font(void) {
    return g_selected;
}

extern "C" int mu_font_query(void* obj, int* height, int* weight, int* italic, char* face, int faceCap) {
    MuFont* font = find_font(obj);
    if (!font) return 0;
    if (height) *height = font->height;
    if (weight) *weight = font->weight;
    if (italic) *italic = font->italic;
    if (face && faceCap > 0) {
        size_t i = 0;
        for (; i < (size_t)faceCap - 1 && font->face[i]; ++i) face[i] = font->face[i];
        face[i] = 0;
    }
    return 1;
}
