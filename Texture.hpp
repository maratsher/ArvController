#pragma once

#include <GLFW/glfw3.h> 
#include <arv.h>

#include "Image.hpp"

class Texture {
    public:
        Texture(Image* _img);
        ~Texture();
        GLuint getTexture();
        void updateTexture(ArvPixelFormat pixelFormat);
    
    private:
        GLuint imageTexture;
        Image* img;
};

