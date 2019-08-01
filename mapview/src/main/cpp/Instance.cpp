//
// Created by jaakko on 22.6.2019.
//

#include "Instance.h"


Instance::Instance(unsigned mapsCount) {
    tileCache = new TileCache(mapsCount);
    textureDrawer = new TextureDrawer();
    tile = new Tile;
}


Instance::~Instance() {
    delete tileCache;
    delete textureDrawer;
    delete tile;
}


void Instance::addTileToDisplayList(unsigned layer, unsigned row, unsigned col, float x, float y, float tw, float c, float s, JNIEnv *env, jobject obj) {
    int tc = 1U << layer;
    if (row < 0 || col < 0 || row >= tc || col >= tc)
    {
        return;
    }

    float sLeft;
    float sTop;
    float sRight;
    float sBottom;

    auto textureId = tileCache->getTile(layer, row, col, &sLeft, &sTop, &sRight, &sBottom, env, obj);
    if (textureId != -1)
    {
        displayList.push_back(TileData(screenWidth, screenHeight, GLuint(textureId), sLeft, sTop, sRight, sBottom, x, y, tw, c, s));
    }
}