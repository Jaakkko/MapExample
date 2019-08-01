//
// Created by jaakko on 22.6.2019.
//

#ifndef MAPLIBRARY_TILEDATA_H
#define MAPLIBRARY_TILEDATA_H

#include <GLES2/gl2.h>

struct TileData {
    GLuint texture;

    /**
     * s alkuiset tarkoittaa kuvan pikseleitä
     * Esim:
     *  sLeft = 0
     *  sTop = 0
     *  sRight = 128
     *  sBottom = 128
     * Tarkoittaa vasenta ylänurkkaa kuvasta
     */
    float sLeft;
    float sTop;
    float sRight;
    float sBottom;

    float tx;   // x koordinaatti
    float ty;   // y koordinaatti
    float wi;   // sivun pituus

    float c;    // cos(tile_angle)
    float s;    // sin(tile_angle)

    /**
     * src matriisiin tarvittavat
     * arvot
     */
    float sm0;
    float sm4;
    float sm6;
    float sm7;

    /**
     * dst matriisiin tarvittavat
     * arvot
     */
    float dm0;
    float dm1;
    float dm3;
    float dm4;
    float dm6;
    float dm7;

    TileData(unsigned screenWidth, unsigned screenHeight, GLuint texture, float sLeft, float sTop, float sRight, float sBottom, float tx, float ty, float wi, float c, float s)
            : texture(texture), sLeft(sLeft), sTop(sTop), sRight(sRight), sBottom(sBottom), tx(tx), ty(ty), wi(wi), c(c), s(s) {
        sm0 = (sRight - sLeft) / 256.0f;
        sm4 = (sTop - sBottom) / 256.0f;
        sm6 = sLeft / 256.0f;
        sm7 = sBottom / 256.0f;

        float w = wi / screenWidth;
        float h = wi / screenHeight;

        dm0 = w * c;
        dm1 = h * s;
        dm3 = w * -s;
        dm4 = h * c;
        dm6 = 2.0f * tx / screenWidth - 1.0f;
        dm7 = -2.0f * ty / screenHeight + 1.0f;
    }
};

#endif //MAPLIBRARY_TILEDATA_H
