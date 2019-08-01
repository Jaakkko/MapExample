//
// Created by jaakko on 13.12.2018.
//

#include <sstream>
#include <jni.h>
#include "TileCache.h"

#include "Helper.h"


static int64_t indexOf(unsigned layer, unsigned row, unsigned col) {
    unsigned index = 0;
    for (unsigned i = 0; i < layer; i++)
    {
        index |= (1U << (i << 1U));
    }

    return index + (1U << layer) * row + col;
}


TileCache::TileCache(unsigned mapsCount) {
    GLint maxSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);

    // Maastokartta on maastokartalle ja ilmakuvalle
    cache = Cache{(unsigned)(maxSize * maxSize) / (256 * 256) / 2};
    // cache = Cache{(unsigned)(32 * 1024 * 1024 / (256 * 256 * 4))};

    this->mapsCount = mapsCount;

    textureDrawer = new TextureDrawer();
}


TileCache::~TileCache() {
    for (auto const& [tileIndex, tile] : cache.values) {
        glDeleteTextures(1, &tile.textureID);
    }

    cache.values.clear();
    cache.keys.clear();

    std::lock_guard<std::mutex> lck(loadingLock);
    loadings.clear();

    delete textureDrawer;
}


void TileCache::setMaxLoadings(unsigned maxLoadings) {
    this->maxLoadings = maxLoadings;
}


GLint TileCache::getTile(unsigned layer, unsigned row, unsigned col, float *sLeft, float *sTop, float *sRight, float *sBottom, JNIEnv *env, jobject obj) {
    const int64_t tileIndex = indexOf(layer, row, col);

    std::unordered_map<int64_t, Cache::Tile>::const_iterator tile;
    bool isNull;
    {
        std::lock_guard<std::mutex> lck(cacheLock);

        tile = cache.values.find(tileIndex);
        isNull = tile == cache.values.end();
    }

    if (isNull) {
        {
            std::lock_guard<std::mutex> lck(loadingLock);
            if (loadings.size() < maxLoadings && std::find(loadings.begin(), loadings.end(), tileIndex) == loadings.end())
            {
                jclass mapViewClass = env->FindClass("jaakko/leinonen/com/mapview/MapView");
                jmethodID getMethodID = env->GetMethodID(mapViewClass, "get", "(IJIII)Z");
                bool canLoad = env->CallBooleanMethod(obj, getMethodID, 0, (jint)tileIndex, layer, row, col);
                if (canLoad)
                {
                    loadings.push_back(tileIndex);
                }
            }
        }

        // Näytä pohjakartta heti kun ladattu
        float tileWidth = 128.0f;
        unsigned rows = 1U << layer;
        unsigned upperLayer = layer - 1;
        while (upperLayer >= 1) {
            auto a = 1U << upperLayer;
            unsigned upperRow = a * row / rows;
            unsigned upperCol = a * col / rows;
            unsigned upperIdx = indexOf(upperLayer, upperRow, upperCol);

            std::unordered_map<int64_t, Cache::Tile>::const_iterator upperTile;
            bool upperTileExists;
            {
                std::lock_guard<std::mutex> lck(cacheLock);

                upperTile = cache.values.find(upperIdx);
                upperTileExists = upperTile != cache.values.end();
            }

            if (upperTileExists) {
                *sLeft = 256 * (col * a / float(rows) - upperCol);
                *sRight = *sLeft + tileWidth;
                *sTop = 256 * (row * a / float(rows) - upperRow);
                *sBottom = *sTop + tileWidth;
                return (GLuint)upperTile->second.textureID;
            }

            upperLayer--;
            tileWidth *= .5f;
        }

        // No tile found
        return -1;
    }
    else if (tile->second.loadIndex < mapsCount) {
        std::lock_guard<std::mutex> lck(loadingLock);
        if (loadings.size() < maxLoadings && std::find(loadings.begin(), loadings.end(), tileIndex) == loadings.end())
        {
            jclass mapViewClass = env->FindClass("jaakko/leinonen/com/mapview/MapView");
            jmethodID getMethodID = env->GetMethodID(mapViewClass, "get", "(IJIII)Z");
            bool canLoad = env->CallBooleanMethod(obj, getMethodID, tile->second.loadIndex, tileIndex, layer, row, col);
            if (canLoad)
            {
                loadings.push_back(tileIndex);
            }
        }
    }

    auto moveToBack = std::find(cache.keys.begin(), cache.keys.end(), tileIndex);
    cache.keys.splice(cache.keys.end(), cache.keys, moveToBack);


    *sLeft = 0;
    *sTop = 0;
    *sRight = 256;
    *sBottom = 256;
    return (GLuint)tile->second.textureID;
}


void TileCache::discard(int64_t tileIndex) {
    std::lock_guard<std::mutex> lck(loadingLock);
    loadings.erase(std::find(loadings.begin(), loadings.end(), tileIndex));
}


void TileCache::put(int64_t tileIndex, void *pixels, bool tileExists, float alpha) {
    {
        std::lock_guard<std::mutex> clck(cacheLock);
        const auto& mapTile = cache.values.find(tileIndex);
        if (mapTile != cache.values.end())
        {
            GLuint textureID = mapTile->second.textureID;
            mapTile->second.loadIndex++;

            if (tileExists)
            {
                textureDrawer->blend(textureID, pixels, alpha);
            }

            if (glGetError() != GL_NO_ERROR) {
                cache.values.erase(mapTile);

                const auto& removeTileIndex = std::find(cache.keys.begin(), cache.keys.end(), tileIndex);
                cache.keys.erase(removeTileIndex);

                glDeleteTextures(1, &textureID);
            }
        }
        else if (tileExists)
        {
            GLuint textureID;

            if (cache.keys.size() == cache.size)
            {
                const auto& to_remove = cache.values.find(cache.keys.front());
                cache.values.erase(to_remove);
                cache.keys.pop_front();

                textureID = to_remove->second.textureID;
            }
            else
            {
                glGenTextures(1, &textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            }

            textureDrawer->clear(textureID);
            textureDrawer->blend(textureID, pixels, alpha);

            if (glGetError() == GL_NO_ERROR) {
                cache.values[tileIndex] = Cache::Tile { textureID, 0 };
                cache.keys.push_back(tileIndex);
            }
            else {
                // Poista tekstuuri jos tapahtui virhe
                glDeleteTextures(1, &textureID);
            }
        }
    }

    std::lock_guard<std::mutex> lck(loadingLock);
    loadings.erase(std::find(loadings.begin(), loadings.end(), tileIndex));
}
