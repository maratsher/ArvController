#pragma once


struct Image
{
    void *imageData;
    unsigned int imageSize;
    unsigned int imageWidth;
    unsigned int imageHeight;
    unsigned int imageDepth;
    bool imageUpdate;
};