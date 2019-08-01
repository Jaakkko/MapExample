//
// Created by jaakko on 21.6.2019.
//

#ifndef MAPLIBRARY_INSTANCE_H
#define MAPLIBRARY_INSTANCE_H

#include <jni.h>
#include <vector>

#include "TileCache.h"
#include "Tile.h"
#include "TileData.h"

struct Instance {
    unsigned halfScreenWidth;
    unsigned halfScreenHeight;
    unsigned screenWidth;
    unsigned screenHeight;


    TileCache* tileCache;
    TextureDrawer* textureDrawer;
    Tile* tile;

    std::mutex displayListLock;
    std::vector<TileData> displayList;

    Instance(unsigned mapsCount);
    ~Instance();

    void addTileToDisplayList(unsigned layer, unsigned row, unsigned col, float x, float y, float tw, float c, float s, JNIEnv* env, jobject obj);
};

#endif //MAPLIBRARY_INSTANCE_H
