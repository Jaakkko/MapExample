//
// Created by jaakko on 19.6.2019.
//


#include <jni.h>
#include <android/bitmap.h>
#include <GLES2/gl2.h>
#include <cmath>
#include <android/log.h>

#include "Instance.h"

/**
 * ---------------------------------------
 * UI Thread
 * ---------------------------------------
 */
extern "C" JNIEXPORT void JNICALL Java_jaakko_leinonen_com_mapview_MapView_draw(JNIEnv* env, jobject obj, jlong handle, jdouble viewPortCenterX, jdouble viewPortCenterY, jdouble viewPortWidth, jdouble viewPortHeight, jdouble viewPortAngle) {
    auto instance = reinterpret_cast<Instance*>(handle);

    float screenWidth = instance->screenWidth;
    float screenHeight = instance->screenHeight;
    float halfScreenWidth = instance->halfScreenWidth;
    float halfScreenHeight = instance->halfScreenHeight;

    double hw = .5 * viewPortWidth;
    double hh = .5 * viewPortHeight;

    double left = viewPortCenterX - hw;
    double top = viewPortCenterY + hh;
    double right = viewPortCenterX + hw;
    double bottom = viewPortCenterY - hh;

    double mapWidthInPixels = 2.0 / viewPortWidth * screenWidth;
    double mapWidthInTiles = mapWidthInPixels / 256.0;
    auto layer = unsigned(fmin(fmax(log2(mapWidthInTiles), 0.0), 22.0));
    auto tileCount = double(1U << layer);
    auto tileWidth = float(1.0 + ceil(mapWidthInPixels / tileCount));
    double halfTileWidth = tileWidth / 2.0;
    double htwsqrt = halfTileWidth * 1.41421356237f;

    auto centerCol = (unsigned)round((viewPortCenterX + 1.0) / 2.0 * tileCount);
    auto centerRow = (unsigned)round((viewPortCenterY - 1.0) / -2.0 * tileCount);

    float c = cos((float)viewPortAngle);
    float s = sin((float)viewPortAngle);

    /**
     * Jaetaan 4, koska max width = 2 ja radius on 1/2 leveydestä
     */
    int maxRadius = 1 + (int)ceil(fmax(viewPortWidth, viewPortHeight) / 4.0 * tileCount);

    // lock display list
    std::lock_guard<std::mutex> lck(instance->displayListLock);

    // Clear display list
    instance->displayList.clear();

    for (int r = 1; r <= maxRadius; r++) {
        unsigned rtopRow = centerRow - r;
        unsigned rbottomRow = centerRow + r - 1;
        double n_h = bottom - top;
        double w = right - left;

        float rytop = float(((1.0 - rtopRow / tileCount * 2.0 - top) / n_h * screenHeight) + halfTileWidth);
        float rybottom = float(((1.0 + rbottomRow / tileCount * -2.0 - top) / n_h * screenHeight) + halfTileWidth);
        for (unsigned h = 0; h < r; h++) {
            unsigned leftCol = centerCol - h - 1;
            unsigned rightCol = centerCol + h;
            float xleft = float(((leftCol / tileCount * 2.0 - 1.0 - left) / w * screenWidth) + halfTileWidth);
            float xright = float(((rightCol / tileCount * 2.0 - 1.0 - left) / w * screenWidth) + halfTileWidth);

            // Top left
            float xtleft = xleft * c + s * rytop - c * halfScreenWidth - s * halfScreenHeight + halfScreenWidth;
            float ytleft = xleft *-s + c * rytop + s * halfScreenWidth - c * halfScreenHeight + halfScreenHeight;
            if (xtleft > -htwsqrt && xtleft < screenWidth + htwsqrt && ytleft > -htwsqrt && ytleft < screenHeight + htwsqrt) {
                instance->addTileToDisplayList(layer, rtopRow, leftCol, xtleft, ytleft, tileWidth, c, s, env, obj);
            }

            // Bottom left
            float xbleft = xleft * c + s * rybottom - c * halfScreenWidth - s * halfScreenHeight + halfScreenWidth;
            float ybleft = xleft *-s + c * rybottom + s * halfScreenWidth - c * halfScreenHeight + halfScreenHeight;
            if (xbleft > -htwsqrt && xbleft < screenWidth + htwsqrt && ybleft > -htwsqrt && ybleft < screenHeight + htwsqrt) {
                instance->addTileToDisplayList(layer, rbottomRow, leftCol, xbleft, ybleft, tileWidth, c, s, env, obj);
            }

            // Top right
            float xtright = xright * c + s * rytop - c * halfScreenWidth - s * halfScreenHeight + halfScreenWidth;
            float ytright = xright *-s + c * rytop + s * halfScreenWidth - c * halfScreenHeight + halfScreenHeight;
            if (xtright > -htwsqrt && xtright < screenWidth + htwsqrt && ytright > -htwsqrt && ytright < screenHeight + htwsqrt) {
                instance->addTileToDisplayList(layer, rtopRow, rightCol, xtright, ytright, tileWidth, c, s, env, obj);
            }

            // Bottom right
            float xbright = xright * c + s * rybottom - c * halfScreenWidth - s * halfScreenHeight + halfScreenWidth;
            float ybright = xright *-s + c * rybottom + s * halfScreenWidth - c * halfScreenHeight + halfScreenHeight;
            if (xbright > -htwsqrt && xbright < screenWidth + htwsqrt && ybright > -htwsqrt && ybright < screenHeight + htwsqrt) {
                instance->addTileToDisplayList(layer, rbottomRow, rightCol, xbright, ybright, tileWidth, c, s, env, obj);
            }
        }

        unsigned rleftCol = centerCol - r;
        unsigned rrightCol = centerCol + r - 1;
        float rxleft = float(((rleftCol / tileCount * 2 - 1 - left) / w * screenWidth) + halfTileWidth);
        float rxright = float(((rrightCol / tileCount * 2 - 1 - left) / w * screenWidth) + halfTileWidth);
        for (unsigned v = 1; v < r; v++) {
            unsigned topRow = centerRow - v;
            unsigned bottomRow = centerRow + v - 1;
            float ytop = float(((topRow / tileCount * -2 + 1 - top) / n_h * screenHeight) + halfTileWidth);
            float ybottom = float(((bottomRow / tileCount * -2 + 1 - top) / n_h * screenHeight) + halfTileWidth);

            // Left top
            float xltop = rxleft * c + s * ytop - c * halfScreenWidth - s * halfScreenHeight + halfScreenWidth;
            float yltop = rxleft *-s + c * ytop + s * halfScreenWidth - c * halfScreenHeight + halfScreenHeight;
            if (xltop > -htwsqrt && xltop < screenWidth + htwsqrt && yltop > -htwsqrt && yltop < screenHeight + htwsqrt) {
                instance->addTileToDisplayList(layer, topRow, rleftCol, xltop, yltop, tileWidth, c, s, env, obj);
            }

            // Right top
            float xrtop = rxright * c + s * ytop - c * halfScreenWidth - s * halfScreenHeight + halfScreenWidth;
            float yrtop = rxright *-s + c * ytop + s * halfScreenWidth - c * halfScreenHeight + halfScreenHeight;
            if (xrtop > -htwsqrt && xrtop < screenWidth + htwsqrt && yrtop > -htwsqrt && yrtop < screenHeight + htwsqrt) {
                instance->addTileToDisplayList(layer, topRow, rrightCol, xrtop, yrtop, tileWidth, c, s, env, obj);
            }

            // Left bottom
            float xlbottom = rxleft * c + s * ybottom - c * halfScreenWidth - s * halfScreenHeight + halfScreenWidth;
            float ylbottom = rxleft *-s + c * ybottom + s * halfScreenWidth - c * halfScreenHeight + halfScreenHeight;
            if (xlbottom > -htwsqrt && xlbottom < screenWidth + htwsqrt && ylbottom > -htwsqrt && ylbottom < screenHeight + htwsqrt) {
                instance->addTileToDisplayList(layer, bottomRow, rleftCol, xlbottom, ylbottom, tileWidth, c, s, env, obj);
            }

            // Right bottom
            float xrbottom = rxright * c + s * ybottom - c * halfScreenWidth - s * halfScreenHeight + halfScreenWidth;
            float yrbottom = rxright *-s + c * ybottom + s * halfScreenWidth - c * halfScreenHeight + halfScreenHeight;
            if (xrbottom > -htwsqrt && xrbottom < screenWidth + htwsqrt && yrbottom > -htwsqrt && yrbottom < screenHeight + htwsqrt) {
                instance->addTileToDisplayList(layer, bottomRow, rrightCol, xrbottom, yrbottom, tileWidth, c, s, env, obj);
            }
        }
    }
}


/**
 * ---------------------------------------
 * GL Thread
 * ---------------------------------------
 */
extern "C" JNIEXPORT jlong JNICALL Java_jaakko_leinonen_com_mapview_MapView_initialize(JNIEnv* env, jobject obj, jint mapsCount) {
    auto instance = new Instance(unsigned(mapsCount));
    return reinterpret_cast<jlong>(instance);
}
extern "C" JNIEXPORT void JNICALL Java_jaakko_leinonen_com_mapview_MapView_screenSizeChanged(JNIEnv* env, jobject obj, jlong handle, jint screenWidth, jint screenHeight) {
    auto instance = reinterpret_cast<Instance*>(handle);

    instance->screenWidth = (unsigned)screenWidth;
    instance->screenHeight = (unsigned)screenHeight;
    instance->halfScreenWidth = (unsigned)screenWidth / 2;
    instance->halfScreenHeight = (unsigned)screenHeight / 2;

    instance->tileCache->setMaxLoadings(unsigned(ceil(screenWidth / 256.0) * ceil(screenHeight / 256.0)));

    glViewport(0, 0, screenWidth, screenHeight);
}
extern "C" JNIEXPORT void JNICALL Java_jaakko_leinonen_com_mapview_MapView_render(JNIEnv* env, jobject obj, jlong handle) {
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    auto instance = reinterpret_cast<Instance*>(handle);

    instance->tile->prepareToDraw();

    {
        std::lock_guard<std::mutex> lck(instance->displayListLock);

        for (auto &tileData : instance->displayList) {
            instance->tile->draw(tileData);
        }
    }

    instance->tile->finishDrawing();
}
extern "C" JNIEXPORT void JNICALL Java_jaakko_leinonen_com_mapview_MapView_cleanUp(JNIEnv* env, jobject obj, jlong handle) {
    auto instance = reinterpret_cast<Instance*>(handle);
    delete(instance);
}


extern "C" JNIEXPORT void JNICALL Java_jaakko_leinonen_com_mapview_MapView_prepareToUpload(JNIEnv* env, jobject obj, jlong handle) {
    glViewport(0, 0, 256, 256);

    auto instance = reinterpret_cast<Instance*>(handle);
    instance->textureDrawer->prepareToDraw();
}
extern "C" JNIEXPORT void JNICALL Java_jaakko_leinonen_com_mapview_MapView_upload(JNIEnv* env, jobject obj, jlong handle, jlong tileIndex, jobject bitmap, jboolean tileExists, jfloat alpha) {
    auto instance = reinterpret_cast<Instance*>(handle);
    if (tileExists && nullptr == bitmap) {
        instance->tileCache->discard(tileIndex);
    }
    else {
        void* pixels;
        AndroidBitmap_lockPixels(env, bitmap, &pixels);
        instance->tileCache->put(tileIndex, pixels, tileExists, alpha);
        AndroidBitmap_unlockPixels(env, bitmap);
    }
}
extern "C" JNIEXPORT void JNICALL Java_jaakko_leinonen_com_mapview_MapView_finishUploading(JNIEnv* env, jobject obj, jlong handle) {
    auto instance = reinterpret_cast<Instance*>(handle);
    instance->textureDrawer->finishDrawing();
    glViewport(0, 0, instance->screenWidth, instance->screenHeight);
}