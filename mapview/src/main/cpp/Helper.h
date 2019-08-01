//
// Created by jaakko on 13.12.2018.
//

#ifndef MAPV15_HELPER_H
#define MAPV15_HELPER_H


#include <android/log.h>
#define TAG "native-lib"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__);

template <class T>
inline T map(T value, T imin, T imax, T omin, T omax) {
    return omin + (value - imin) / (imax - imin) * (omax - omin);
}

template<class T>
inline void map2D(T *dst, T *imin, T *imax, T *omin, T* omax) {
	float Xr = (omax[0] - omin[0]) / (imax[0] - imin[0]);
	float Yr = (omax[1] - omin[1]) / (imax[1] - imin[1]);

	dst[0] = Xr;
	dst[4] = Yr;
	dst[6] = Xr * -imin[0] + omin[0];
	dst[7] = Yr * -imin[1] + omin[1];
	dst[8] = 1;
}


inline GLuint loadShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    if (shader) {
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);
    }

    return shader;
}
inline GLuint createProgram(const char* vertexShaderSource, const char* fragmentShaderSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    return program;
}
inline GLuint createProgram(const char* vertexShaderSource, const char* fragmentShaderSource, GLuint* vertexShader, GLuint* fragmentShader) {
    *vertexShader = loadShader(GL_VERTEX_SHADER, vertexShaderSource);
    *fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint program = glCreateProgram();

    glAttachShader(program, *vertexShader);
    glAttachShader(program, *fragmentShader);
    glLinkProgram(program);

    return program;
}


#endif //MAPV15_HELPER_H
