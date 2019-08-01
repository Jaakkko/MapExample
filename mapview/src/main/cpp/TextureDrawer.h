//
// Created by jaakko on 17.2.2019.
//

#ifndef SUOMENKARTTA_TEXTUREDRAWER_H
#define SUOMENKARTTA_TEXTUREDRAWER_H


#include <GLES2/gl2.h>

class TextureDrawer {
private:
    GLuint program;
    GLuint VBO;

    GLuint holderTextureID;
    GLuint FBO;

    GLuint alphaHandle;

public:
    TextureDrawer();
    ~TextureDrawer();

    void prepareToDraw();
    void finishDrawing();

    void blend(GLuint textureID, void* pixels, float alpha);
    void clear(GLuint textureID);
};


#endif //SUOMENKARTTA_TEXTUREDRAWER_H
