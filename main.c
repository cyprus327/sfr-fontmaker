#define SFR_IMPL
#define SFR_PREFIXED_TYPES
#include "../sofren/sofren.c"

#include <stdio.h>
#include <stdint.h>

#include <raylib.h> // raylib for rendering not sofren

typedef int8_t   i8;
typedef uint8_t  u8;
typedef int16_t  i16;
typedef uint16_t u16;
typedef int32_t  i32;
typedef uint32_t u32;
typedef int64_t  i64;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;

#define UI_COL_BG     ((Color){200, 200, 200, 255})
#define UI_COL_HOVER  ((Color){170, 170, 170, 255})
#define UI_COL_CLICK  ((Color){100, 100, 100, 255})
#define UI_COL_ACTIVE ((Color){200, 200, 255, 255})

i32 windowWidth = 1024, windowHeight = 768;

void verify_winding(f32* ax, f32* ay, f32* bx, f32* by, f32* cx, f32* cy);

u8 ui_button(i32 x, i32 y, i32 w, i32 h, const char* text);
u8 ui_inputbox(i32 x, i32 y, i32 w, i32 h, char* buffer, i32 bufferSize); // max 1 call per frame
u8 ui_save_menu(char* saveBuf, i32 saveBufSize, f32 verts[SFR_FONT_GLYPH_MAX][SFR_FONT_VERT_MAX]);

void font_save(const char* name, f32 verts[SFR_FONT_GLYPH_MAX][SFR_FONT_VERT_MAX]);

u8 is_symbol(char c);
u8 is_num(char c);

i32 main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(windowWidth, windowHeight, "Window");

    i32 glyphInd = 'a';
    i32 vertInd = 0;
    f32 verts[SFR_FONT_GLYPH_MAX][SFR_FONT_VERT_MAX];
    for (i32 i = 0; i < SFR_FONT_GLYPH_MAX; i += 1) {
        for (i32 j = 0; j < SFR_FONT_VERT_MAX; j += 1) {
            verts[i][j] = SFR_FONT_VERT_EMPTY;
        }
    }    

    while (!WindowShouldClose()) {
        windowWidth = GetScreenWidth(), windowHeight = GetScreenHeight();
        const f32 time = GetTime(), frameTime = GetFrameTime();
        const Vector2 mp = GetMousePosition();
        const u8 mousePressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        const u8 mouseDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        const u8 mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    
        static Vector2 points[3];
        static i32 pointInd = 0;

        static i32 draggingVertInd = -1;

        // grid variables
        const i32 size = 5;
        const i32 spacing = 40;
        const i32 gridWidth = 10;
        const i32 gridHeight = 12;
        const i32 totalWidth = gridWidth * spacing;
        const i32 totalHeight = gridHeight * spacing;
        const i32 offsetX = (windowWidth - totalWidth) / 2;
        const i32 offsetY = (windowHeight - totalHeight) / 2;

        { // dragging vertices
            if (mousePressed && 0 == pointInd) {
                i32 gridX = (i32)roundf((mp.x - offsetX) / spacing);
                i32 gridY = (i32)roundf((mp.y - offsetY) / spacing);
                gridX = SFR_CLAMP(gridX, 0, gridWidth - 1);
                gridY = SFR_CLAMP(gridY, 0, gridHeight - 1);
                const i32 savedY = -gridY;
    
                for (i32 i = 0; i < vertInd; i += 2) {
                    if (gridX == verts[glyphInd][i] && savedY == verts[glyphInd][i + 1]) {
                        draggingVertInd = i;
                        break;
                    }
                }
            }

            if (mouseDown && -1 != draggingVertInd) {
                i32 gridX = (i32)roundf((mp.x - offsetX) / spacing);
                i32 gridY = (i32)roundf((mp.y - offsetY) / spacing);
                gridX = SFR_CLAMP(gridX, 0, gridWidth - 1);
                gridY = SFR_CLAMP(gridY, 0, gridHeight - 1);
                const i32 savedY = -gridY;
    
                verts[glyphInd][draggingVertInd + 0] = gridX;
                verts[glyphInd][draggingVertInd + 1] = savedY;
    
                const i32 triangleInd = draggingVertInd / 6;
                const i32 baseInd = triangleInd * 6;
                if (baseInd + 5 < SFR_FONT_VERT_MAX) {
                    verify_winding(&verts[glyphInd][baseInd + 0], &verts[glyphInd][baseInd + 1],
                                   &verts[glyphInd][baseInd + 2], &verts[glyphInd][baseInd + 3],
                                   &verts[glyphInd][baseInd + 4], &verts[glyphInd][baseInd + 5]);
                }
            }
        }

        if (mouseReleased) {
            draggingVertInd = -1;
        }

        BeginDrawing(); {
            ClearBackground((Color){40, 40, 40, 255});

            static u8 saving = 0;
            static char saveBuf[64] = "";

            if (saving) {
                saving = ui_save_menu(saveBuf, sizeof(saveBuf), verts);
                EndDrawing();
                continue;
            }

            { // ui
                if (ui_button(20, 20, 80, 30, "Clear")) {
                    vertInd = 0;
                    pointInd = 0;
                    for (i32 i = 0; i < SFR_FONT_VERT_MAX; i += 1) {
                        verts[glyphInd][i] = SFR_FONT_VERT_EMPTY;
                    }
                }

                if (ui_button(110, 20, 80, 30, "Save")) {
                    saveBuf[0] = '\0';
                    saving = 1;
                }

                if (ui_button(200, 20, 140, 30, "Delete Prev")) {
                    if (vertInd >= 6) {
                        vertInd -= 6;
                        for (i32 i = vertInd; i < vertInd + 6; i += 1) {
                            verts[glyphInd][i] = SFR_FONT_VERT_EMPTY;
                        }
                        pointInd = 0;
                    }
                }

                // input for glyph ind
                static char glyphBuf[4] = "a";
                ui_inputbox(20, 60, 50, 30, glyphBuf, sizeof(glyphBuf));

                // parse glyphBuf
                i32 len = 0;
                while ('\0' != glyphBuf[len]) {
                    len += 1;
                }

                i32 newGlyphInd = -1;
                if (1 == len && is_symbol(glyphBuf[0])) {
                    newGlyphInd = glyphBuf[0];
                } else if (2 == len && is_num(glyphBuf[0]) && is_num(glyphBuf[1])) {
                    newGlyphInd = (glyphBuf[0] - '0') * 10 + glyphBuf[1] - '0';
                } else if (3 == len && is_num(glyphBuf[0]) && is_num(glyphBuf[1]) && is_num(glyphBuf[2])) {
                    newGlyphInd = (glyphBuf[0] - '0') * 100 + (glyphBuf[1] - '0') * 10 + glyphBuf[2] - '0';
                }

                if (-1 != newGlyphInd && newGlyphInd != glyphInd) {
                    glyphInd = newGlyphInd;
                    vertInd = pointInd = 0;
                }

                DrawText(TextFormat("GLYPH IND: %d, '%c'", glyphInd, glyphInd), 80, 60, 30, WHITE);
            }

            { // draw grid
                for (i32 y = 0, py = offsetY; y < gridHeight; y += 1, py += spacing) {
                    for (i32 x = 0, px = offsetX; x < gridWidth; x += 1, px += spacing) {
                        Color col = BLACK;

                        const i32 dy = (py - mp.y) * (py - mp.y);
                        const i32 dx = (px - mp.x) * (px - mp.x);
                        if (dy <= (size * 2) * (size * 2) && dx <= (size * 2) * (size * 2)) {
                            col = DARKGRAY;
                            if (mouseReleased) {
                                points[pointInd] = (Vector2){x, -y};
                                pointInd += 1;
                                if (pointInd >= 3) {
                                    pointInd = 0;
                                    verify_winding(
                                        &points[0].x, &points[0].y,
                                        &points[1].x, &points[1].y,
                                        &points[2].x, &points[2].y);
                                    verts[glyphInd][vertInd + 0] = points[0].x;
                                    verts[glyphInd][vertInd + 1] = points[0].y;
                                    verts[glyphInd][vertInd + 2] = points[1].x;
                                    verts[glyphInd][vertInd + 3] = points[1].y;
                                    verts[glyphInd][vertInd + 4] = points[2].x;
                                    verts[glyphInd][vertInd + 5] = points[2].y;
                                    vertInd += 6;
                                }
                            }
                        }

                        DrawCircle(px, py, size, col);
                    }
                }
            }

            { // draw triangles
                for (i32 i = 0; i < SFR_FONT_VERT_MAX; i += 6) {
                    if (SFR_FONT_VERT_EMPTY == verts[glyphInd][i]) {
                        break;
                    }

                    const Vector2 a = {verts[glyphInd][i + 0] * spacing + offsetX, -verts[glyphInd][i + 1] * spacing + offsetY};
                    const Vector2 b = {verts[glyphInd][i + 2] * spacing + offsetX, -verts[glyphInd][i + 3] * spacing + offsetY};
                    const Vector2 c = {verts[glyphInd][i + 4] * spacing + offsetX, -verts[glyphInd][i + 5] * spacing + offsetY};
                    DrawTriangle(a, c, b, (Color){255, 255, 255, 50});
                    DrawTriangleLines(a, b, c, WHITE);
                }
            }

            { // draw current triangle
                if (1 == pointInd) {
                    DrawLine(
                        mp.x, mp.y,
                        points[0].x * spacing + offsetX, -points[0].y * spacing + offsetY,
                        (Color){180, 240, 180, 180});
                } else if (2 == pointInd) {
                    DrawTriangleLines(
                        (Vector2){points[0].x * spacing + offsetX, -points[0].y * spacing + offsetY},
                        (Vector2){points[1].x * spacing + offsetX, -points[1].y * spacing + offsetY},
                        mp,
                        (Color){180, 240, 180, 180});
                }
            }

            // DrawText(TextFormat("PI: %d, VI: %d", pointInd, vertInd), 20, 100, 30, WHITE);
            DrawText("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", 20, windowHeight - 40, 30, WHITE);
        } EndDrawing();
    }

    return 0;
}

f32 calc_normal(f32 ax, f32 ay, f32 bx, f32 by, f32 cx, f32 cy) {
    return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

void verify_winding(f32* ax, f32* ay, f32* bx, f32* by, f32* cx, f32* cy) {
    const f32 n = calc_normal(*ax, *ay, *bx, *by, *cx, *cy);
    if (n > 0.f) { // if it equals -1.f
        SFR_SWAPF(*bx, *cx);
        SFR_SWAPF(*by, *cy);
    }
}

u8 ui_button(i32 x, i32 y, i32 w, i32 h, const char* text) {
    const Rectangle bounds = { (f32)x, (f32)y, (f32)w, (f32)h };
    const Vector2 mouse = GetMousePosition();
    const u8 hovering = CheckCollisionPointRec(mouse, bounds);
    const u8 clicking = hovering && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    const u8 clicked  = hovering && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    const Color col = clicking ? UI_COL_CLICK : hovering ? UI_COL_HOVER : UI_COL_BG; 
    
    DrawRectangleRec(bounds, col);
    DrawText(
        text,
        x + (w - MeasureText(text, 20)) / 2,
        y + (h - 20) / 2,
        20,
        BLACK
    );

    return clicked;
}

// max 1 call per frame
u8 ui_inputbox(i32 x, i32 y, i32 w, i32 h, char* buffer, i32 bufferSize) {
    static u8 mouseOver = 0;
    static u8 active = 0;
    const Rectangle bounds = { (f32)x, (f32)y, (f32)w, (float)h };

    const Vector2 mouse = GetMousePosition();
    mouseOver = CheckCollisionPointRec(mouse, bounds);

    if (mouseOver && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        active = 1;
    } else if (!mouseOver && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        active = 0;
    }

    if (active) {
        for (i32 key = GetCharPressed(); key > 0; key = GetCharPressed()) {
            const i32 len = strlen(buffer);
            if (len < bufferSize - 1 && key >= 32 && key <= 125) {
                buffer[len] = (char)key;
                buffer[len + 1] = '\0';
            }
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            const i32 len = strlen(buffer);
            if (len > 0) {
                buffer[len - 1] = '\0';
            }
        }
    }

    const Color color = active ? UI_COL_ACTIVE : mouseOver ? UI_COL_HOVER : UI_COL_BG;

    DrawRectangleRec(bounds, color);
    DrawRectangleLines(x, y, w, h, DARKGRAY);
    DrawText(buffer, x + 5, y + (h - 20) / 2, 20, BLACK);

    return active;
}

u8 ui_save_menu(char* saveBuf, i32 saveBufSize, f32 verts[SFR_FONT_GLYPH_MAX][SFR_FONT_VERT_MAX]) {
    const i32 w = 400;
    const i32 h = 50;

    DrawText("Enter your font's name in the text box\nPress Cancel to go back", 20, 20, 30, WHITE);
    ui_inputbox(windowWidth / 2 - w / 2, windowHeight / 2 - h / 2 - 10, w, h, saveBuf, sizeof(saveBuf));
    
    if (ui_button(windowWidth / 2 - w / 2, windowHeight / 2 - h / 2 + 60, w / 2 - 5, 50, "Cancel")) {
        return 0;
    }

    if (ui_button(windowWidth / 2 + 5, windowHeight / 2 - h / 2 + 60, w / 2 - 5, 50, "Confirm")) {
        font_save(saveBuf, verts);
        return 0;
    }

    return 1;
}

void font_save(const char* name, f32 verts[SFR_FONT_GLYPH_MAX][SFR_FONT_VERT_MAX]) {
    char fullname[128];
    snprintf(fullname, sizeof(fullname), "%s.srft", name);
    FILE* output = fopen(fullname, "wb");
    if (!output) {
        SFR_ERR_RET(, "font_save: failed to open output file (%s)\n", fullname);
        return;
    }

    const u32 code = ('s' << 24) | ('r' << 16) | ('f' << 8) | 't';
    fwrite(&code, 4, 1, output);

    for (u16 glyph = 0; glyph < SFR_FONT_GLYPH_MAX; glyph += 1) {
        u8 vertCount = 0;
        while (SFR_FONT_VERT_EMPTY != verts[glyph][vertCount] && vertCount < SFR_FONT_VERT_MAX) {
            vertCount += 1;
        }

        fwrite(&vertCount, 1, 1, output);
        if (vertCount) {
            fwrite(verts[glyph], 4, vertCount, output);
        }
    }

    fclose(output);
}

u8 is_symbol(char c) {
    return c >= '!' && c <= '~';
}

u8 is_num(char c) {
    return c >= '0' && c <= '9';
}
