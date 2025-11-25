#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;

void main()
{
    float tiles = 8.0;

    vec2 uv = TexCoord * tiles;

    int x = int(floor(uv.x));
    int y = int(floor(uv.y));

    bool isWhite = ((x + y) % 2) == 0;

    if (isWhite)
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    else
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
