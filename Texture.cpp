#include "Texture.hpp"
#include <assert.h>
#include <stdio.h>

Texture::Texture(Image* _img){

    img = _img;

    // generate Texture
    glGenTextures(1, &imageTexture);
    glBindTexture(GL_TEXTURE_2D, imageTexture);

    // Setup filtering parameters for display
    // TO DO What is mean?!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // init empty texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}

GLuint Texture::getTexture(){
    assert(&imageTexture != NULL);
    return imageTexture;
}

void Texture::updateTexture(ArvPixelFormat pixelFormat){


    GLenum glFormat = -1;
    GLenum glPixelFormat = -1;
    /*byte format*/
    // if (img->imageDepth == 8) {
    //     // 8 bit image
    //     format = GL_UNSIGNED_BYTE;
    // } else if (img->imageDepth >= 16) {
    //     // 16 bit image
    //     format = GL_UNSIGNED_SHORT;
    // }
    glFormat = GL_UNSIGNED_BYTE;

    /*pixel format*/
    // switch (pixelFormat) {
    // case ARV_PIXEL_FORMAT_MONO_16:
    //     glPixelFormat = GL_R16;
    //     break;
    // case ARV_PIXEL_FORMAT_MONO_8:
    //     glPixelFormat = GL_R16;
    //     break;
    // default:
    //     break;
    // }
    glPixelFormat = GL_RGB;

    assert((int)glFormat != -1);

    glTexSubImage2D(GL_TEXTURE_2D,
        0,
        0,
        0,
        img->imageWidth,
        img->imageHeight,
        glPixelFormat,
        glFormat,
        img->imageData);

}

Texture::~Texture(){
}