#pragma once

#include <arv.h>
#include "Image.hpp"
#include <stdio.h>


struct CameraInfo
{
    char * protocol;
    char * deviceId;
    char * vendor;
    char * model;
    char * serialNumber;
    char * physicalId;

    ~CameraInfo(){
        free(protocol);
        free(deviceId);
        free(vendor);
        free(model);
        free(serialNumber);
        free(physicalId);
    }
};

struct CameraConfig
{
    gboolean gainAvailable;
    double maxGain;
    double minGain;
    double currentGain;

    gboolean frameRateAvailable;
    double minFrameRate;
    double maxFrameRate;
    double currentFrameRate;

    unsigned int numPixelFormats;
    const char** availablePixelFormatsStrings;
    const char* currentPixelFormatString;
    ArvPixelFormat* availablePixelFormats;
    ArvPixelFormat currentPixelFormat;

    unsigned int imagePayload;

    ~CameraConfig(){
        g_free(availablePixelFormatsStrings);
        g_free(availablePixelFormats);
    }

};

struct CameraDebug
{

    unsigned int numImages;
    unsigned int numBytes;
    unsigned int numErrors;

};

class Camera {
    public:
        unsigned int index;
        Image* image;
        CameraInfo camInfo;
        CameraConfig camConfig;
        CameraDebug camDebug;

        Camera(const unsigned int _index);
        ~Camera();
        void startVideo(void);
        void stopVideo(void);
        void setGain(const float _value);
        void setPixelFormat(const unsigned int _value);
        void setFrameRate(const float _value);

    private:
        ArvCamera *camera = NULL;
        ArvStream *stream = NULL;
        static void newBufferCallback(ArvStream *_stream, void *_userData);
        static void controlLostCallback(void *_userData);
        static void streamCallback (void *user_data, ArvStreamCallbackType type, ArvBuffer *buffer);
};
