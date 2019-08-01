//
// Created by jaakko on 21.11.2018.
//

#ifndef NATIVEMAP_TILE_H
#define NATIVEMAP_TILE_H


#include <GLES2/gl2.h>
#include "TileData.h"

class Tile {
private:
    GLuint program;

    GLuint VBO;
    // GLuint texCoordVboId;

    float srcMatrix[9] = {
            1, 0, 0,
            0, 1, 0,
            0, 0, 1,
    };
    float dstMatrix[9] = {
            1, 0, 0,
            0, 1, 0,
            0, 0, 1,
    };

    // GLuint texture;

    // Uniforms
    GLint dstMatrixHandle;
    GLint srcMatrixHandle;

public:
    static int screenWidth;
    static int screenHeight;

    Tile();

    void prepareToDraw();
    void draw(const TileData& tileData);
    void finishDrawing();
};


#endif //NATIVEMAP_TILE_H
