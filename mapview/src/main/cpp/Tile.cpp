//
// Created by jaakko on 21.11.2018.
//

#include "Tile.h"

#include "Helper.h"

#include <GLES2/gl2ext.h>
#include <android/log.h>
#include <cmath>

static const char* vertexShaderSource =
        "attribute vec2 aPosition;"
        "uniform mat3 uDstMatrix;"
        "uniform mat3 uSrcMatrix;"
        "varying vec2 vTexCoord;"
        "void main() {"
        "   gl_Position = vec4(uDstMatrix * vec3(aPosition, 1.0), 1.0);"
        "   vTexCoord = (uSrcMatrix * vec3(0.5 * (aPosition + 1.0), 1.0)).xy;"
        "}";

static const char* fragmentShaderSource =
        "precision lowp float;"
        "uniform sampler2D uTexture;"
        "varying vec2 vTexCoord;"
        "void main() {"
        "   gl_FragColor = texture2D(uTexture, vTexCoord);"
        "}";

static const GLfloat positions[] = {
        -1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
         1.0f, -1.0f,
};


int Tile::screenWidth;
int Tile::screenHeight;


Tile::Tile() {
    program = createProgram(vertexShaderSource, fragmentShaderSource);

    dstMatrixHandle = glGetUniformLocation(program, "uDstMatrix");
    srcMatrixHandle = glGetUniformLocation(program, "uSrcMatrix");

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

    //for (const auto&& s : srcMatrix) { s = 0; }

    //for (int i = 0; i < 9; i++) srcMatrix[i] = 0;
    //for (int i = 0; i < 9; i++) dstMatrix[i] = 0;

    srcMatrix[8] = 1.0f;
}


void Tile::prepareToDraw() {
    glUseProgram(program);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(0);
}


void Tile::draw(const TileData &tileData) {
    srcMatrix[0] = tileData.sm0;
    srcMatrix[4] = tileData.sm4;
    srcMatrix[6] = tileData.sm6;
    srcMatrix[7] = tileData.sm7;

    dstMatrix[0] = tileData.dm0;
    dstMatrix[1] = tileData.dm1;
    dstMatrix[3] = tileData.dm3;
    dstMatrix[4] = tileData.dm4;
    dstMatrix[6] = tileData.dm6;
    dstMatrix[7] = tileData.dm7;

    glBindTexture(GL_TEXTURE_2D, tileData.texture);

    glUniformMatrix3fv(srcMatrixHandle, 1, GL_FALSE, srcMatrix);
    glUniformMatrix3fv(dstMatrixHandle, 1, GL_FALSE, dstMatrix);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}


void Tile::finishDrawing() {
    glDisableVertexAttribArray(0);
}
