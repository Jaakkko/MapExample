//
// Created by jaakko on 13.12.2018.
//
/*
 * Lataa ja varastoi tiiliä
 */

#ifndef MAPV15_TILELOADER_H
#define MAPV15_TILELOADER_H

#include <GLES2/gl2.h>
#include <mutex>
#include <vector>
#include <list>
#include <unordered_map>
#include "Tile.h"
#include "TextureDrawer.h"


class TileCache {
private:
    struct Cache {
        struct Tile {
            GLuint textureID = 0;
            unsigned char loadIndex = 0;
        };

        unsigned size = 0;
        std::list<int64_t> keys;
        std::unordered_map<int64_t, Tile> values;
    };

    unsigned maxLoadings = 32;

    std::mutex cacheLock;
    TileCache::Cache cache;

    std::mutex loadingLock;
    std::vector<int64_t> loadings;

    unsigned mapsCount;


public:
    TextureDrawer* textureDrawer;

    TileCache(unsigned mapsCount);
    ~TileCache();

	void setMaxLoadings(unsigned maxLoadings);

	/**
	 * @param layer
	 * @param row
	 * @param col
	 * @param sLeft
	 * @param sTop
	 * @param sRight
	 * @param sBottom
	 * @param env
	 * @param obj
	 * @return texture or -1 if no any upper tile available
	 */
	GLint getTile(unsigned layer, unsigned row, unsigned col, float *sLeft, float *sTop, float *sRight, float *sBottom, JNIEnv *env, jobject obj);

    void discard(int64_t tileIndex);
    void put(int64_t tileIndex, void* pixels, bool tileExists, float alpha);
};


#endif //MAPV15_TILELOADER_H
