# Fontmaker

Small program for creating .srft fonts for use in [sofren](https://github.com/cyprus327/sofren),
made using [raylib](https://github.com/raysan5/raylib).

As of right now, this is pretty barebones and I'm sure has bugs, but it is useable.

---

[*Example of using a font created with this tool in sofren*](https://github.com/cyprus327/sofren/blob/main/examples/font-starter-sdl2.c)

![sfrfontdemo1](https://github.com/user-attachments/assets/87f62598-b39e-4d04-b19d-0f97ddba1622)

---

Being a sofrware renderer, sofren has efficient triangle rasterization functions.
As a result of this and of me not wanting to make "real" font rendering,
text rendered with sofren is made of triangles, whose vertices are defined in a .srft file.

The format of .srft fonts is very simple, and it could be compressed / improved
in general in multiple areas. However, I don't think there's currently a need for that.

```
.srft format

[0..4] header:
- [0..4] = ['s']['r']['f']['t'] = 1936877172

[5...]:
- {
    vert count, u16, 2 bytes, max defined by SFR_FONT_VERT_MAX
    vert data,  f32, 4 bytes, [x0][y0][x1][y1][x2][...]
}
```

## Todos
- Better "drawing"
- Overlay translucent glyphs to trace
- Miscelanious bugfixes
