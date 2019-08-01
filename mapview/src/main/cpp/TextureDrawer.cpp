//
// Created by jaakko on 17.2.2019.
//

#include "TextureDrawer.h"
#include "Helper.h"


static const float positions[] = {
        -1.0f, 1.0f,
        -1.0f, -1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f,
};


static const char* vertexShader = "attribute vec4 aPosition;"
                                  "varying vec2 vTexCoord;"
                                  "void main() {"
                                  " gl_Position = aPosition;"
                                  " vTexCoord = 0.5 * aPosition.xy + 0.5;"
                                  "}";

static const char* fragmentShader = "precision mediump float;"
                                    "uniform sampler2D uTexture;"
                                    "uniform float uAlpha;"
                                    "varying vec2 vTexCoord;"
                                    "void main() {"
                                    "   gl_FragColor = uAlpha * texture2D(uTexture, vTexCoord);"
                                    "}";


TextureDrawer::TextureDrawer() {
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

    glGenFramebuffers(1, &FBO);

    glGenTextures(1, &holderTextureID);
    glBindTexture(GL_TEXTURE_2D, holderTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    program = createProgram(vertexShader, fragmentShader);

    alphaHandle = (GLuint)glGetUniformLocation(program, "uAlpha");
}


TextureDrawer::~TextureDrawer() {
    glDeleteBuffers(1, &VBO);
    glDeleteFramebuffers(1, &FBO);
    glDeleteTextures(1, &holderTextureID);

    glDeleteProgram(program);
}


void TextureDrawer::prepareToDraw() {
    glUseProgram(program);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(0);
}


void TextureDrawer::finishDrawing() {
    glDisableVertexAttribArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void TextureDrawer::blend(GLuint textureID, void *pixels, float alpha) {
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

    glBindTexture(GL_TEXTURE_2D, holderTextureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glUniform1f(alphaHandle, alpha);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisable(GL_BLEND);
}


void TextureDrawer::clear(GLuint textureID) {
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
