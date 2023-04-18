#include "Camera.hpp"
//#include "Debug.hpp"

#include <assert.h>
#include <stdlib.h>

// callback when get new buffer from stream
void Camera::newBufferCallback(ArvStream *_stream, void *_userData) {
    printf("==========================================================\n");
    ArvBuffer *buffer;
	gint n_input_buffers, n_output_buffers;

    assert(_stream != NULL);
    assert(_userData != NULL);
    Camera *camera = (Camera *)_userData;
    assert(camera != NULL);

    // do not block when trying to get an image
    buffer = arv_stream_try_pop_buffer(_stream);
    assert(buffer != NULL);
    if (buffer == NULL) {
        printf("arv_stream_try_pop_buffer() returned no buffer: %d\n", arv_buffer_get_status(buffer));
        return;
    }

	arv_stream_get_n_buffers(_stream, &n_input_buffers, &n_output_buffers);
	printf("have %d in, %d out buffers \n", n_input_buffers, n_output_buffers);

    // if buffer wrong
    if (arv_buffer_get_status(buffer) != ARV_BUFFER_STATUS_SUCCESS) {
        camera->camDebug.numErrors++;
        printf("arv_buffer_get_status() failed: %d\n", arv_buffer_get_status(buffer));
        // return the buffer to the stream
        arv_stream_push_buffer(_stream, buffer);
        return;
    }

    // buffer contains our image
    int imageWidth = arv_buffer_get_image_width(buffer);
    assert(imageWidth > 0);
    int imageHeight = arv_buffer_get_image_height(buffer);
    assert(imageHeight > 0);

    printf("buffer size %dx%d\n", imageWidth, imageHeight);

    size_t size = 0;
    // raw buffer bytes
    const void *raw = arv_buffer_get_data(buffer, &size);
    assert(raw != NULL);
    camera->camConfig.currentPixelFormat = arv_buffer_get_image_pixel_format(buffer);

    int imageDepth = ARV_PIXEL_FORMAT_BIT_PER_PIXEL(camera->camConfig.currentPixelFormat);
    // switch (pixelFormat) {
    // case ARV_PIXEL_FORMAT_MONO_16:
    //     // D("pixel format ARV_PIXEL_FORMAT_MONO_16\n");
    //     imageDepth = 16;
    //     break;
    // case ARV_PIXEL_FORMAT_MONO_8:
    //     // D("pixel format ARV_PIXEL_FORMAT_MONO_8\n");
    //     imageDepth = 8;
    //     break;
    // default:
    //     E("unsupportable pixel format 0x%X\n", pixelFormat);
    //     break;
    // }
    assert(imageDepth != 0);
    printf("RGB image %lu bytes, pixel format RGB\n", size);

    if (camera->image->imageData == NULL) {
        // allocate room for a RGB image, aravis provided payload data may be
        // in other pixel formats (8 bit, 10 bit, 12 bit, 16 bit,..)
        camera->image->imageData = malloc(size);
        printf("allocated %zu bytes for image data\n", size);
    } else if (camera->image->imageData != NULL) {
        // if size buffer large then priviase image, relloc
        // do i need relloc when smaller?
        assert(camera->image->imageSize > 0);
        if (camera->image->imageSize < size) {
            camera->image->imageData = realloc(camera->image->imageData, size);
        }
    }

    camera->image->imageSize = size;
    assert(camera->image->imageSize > 0);
    assert(camera->image->imageData != NULL);

    // save new image data
    memcpy(camera->image->imageData, raw, size);
    camera->image->imageWidth = imageWidth;
    camera->image->imageHeight = imageHeight;
    camera->image->imageDepth = imageDepth;

    camera->camDebug.numImages++;
    camera->camDebug.numBytes += size;

    // update image
    camera->image->imageUpdate = true;

    // return the buffer to the stream
    arv_stream_push_buffer(_stream, buffer);
    printf("==========================================================\n");
}

void Camera::controlLostCallback(void *_userData) {

    assert(_userData != NULL);
    Camera *camera = (Camera *)_userData;

    printf("camera %s control lost\n", camera->camInfo.deviceId);

    // WHAT TO DO HERE?!?
    assert(1 == 0);

}

void Camera::streamCallback (void *user_data, ArvStreamCallbackType type, ArvBuffer *buffer)
{
	/* This code is called from the stream receiving thread, which means all the time spent there is less time
	 * available for the reception of incoming packets */

	switch (type) {
		case ARV_STREAM_CALLBACK_TYPE_INIT:
			/* Stream thread started.
			 *
			 * Here you may want to change the thread priority arv_make_thread_realtime() or
			 * arv_make_thread_high_priority() */
			break;
		case ARV_STREAM_CALLBACK_TYPE_START_BUFFER:
			/* The first packet of a new frame was received */
			break;
		case ARV_STREAM_CALLBACK_TYPE_BUFFER_DONE:
			/* The buffer is received, successfully or not.
			 *
			 * You could here signal the new buffer to another thread than the main one, and pull/push the
			 * buffer from there. */
			break;
		case ARV_STREAM_CALLBACK_TYPE_EXIT:
			/* Stream thread ended */
			break;
	}
}

Camera::Camera(const unsigned int _index){
    GError *error = NULL;
    index = _index;

    /* get camera info */
    camInfo.protocol = strdup(arv_get_device_protocol(index));
    camInfo.deviceId = strdup(arv_get_device_id(index));
    camInfo.vendor = strdup(arv_get_device_vendor(index));
    camInfo.model = strdup(arv_get_device_model(index));
    camInfo.serialNumber = strdup(arv_get_device_serial_nbr(index));
    camInfo.physicalId = strdup(arv_get_device_physical_id(index));

    assert(camera == NULL);

    /* init camera */
    //camera = arv_camera_new(camInfo.deviceId, &error);
    camera = arv_camera_new(NULL, &error);
    assert(camera != NULL);

    // bind callback when lose conrol camera
    g_signal_connect(arv_camera_get_device(camera), "control-lost", G_CALLBACK(Camera::controlLostCallback), this);

    /*init image*/
    image = (Image*)malloc(sizeof(Image));

    if (! ARV_IS_CAMERA(camera)) {
        printf("arv_camera_new() failed : %s\n", (error != NULL) ? error->message : "");
		g_clear_error(&error);
        return;
    }else{
        printf("Succsecful connect with camera %s\n", camInfo.deviceId);
    }

    /* get camera available config */
    //pixel format
    
    arv_camera_set_pixel_format(camera, ARV_PIXEL_FORMAT_RGB_8_PACKED, &error);
    guint n_val = -1;
    camConfig.availablePixelFormatsStrings = arv_camera_dup_available_pixel_formats_as_strings(camera, &n_val, &error);
    assert(error == NULL);
    camConfig.availablePixelFormats = (ArvPixelFormat*)arv_camera_dup_available_pixel_formats(camera, &n_val, &error);
    for (unsigned int i = 0; i < n_val; i++) {
		printf("%d\n",camConfig.availablePixelFormats[i]);
	}
    assert(error == NULL);
    camConfig.currentPixelFormat = arv_camera_get_pixel_format(camera, &error);
    assert(error == NULL);
    camConfig.currentPixelFormatString = arv_camera_get_pixel_format_as_string(camera, &error);
    assert(error == NULL);
    assert(n_val != -1);
    camConfig.numPixelFormats = n_val;

    //frame rate
    camConfig.frameRateAvailable = arv_camera_is_frame_rate_available(camera, &error);
    assert(error == NULL);
    arv_camera_get_frame_rate_bounds(camera,
    &camConfig.minFrameRate,
    &camConfig.maxFrameRate,
    &error);
    assert(error == NULL);
    camConfig.currentFrameRate = arv_camera_get_frame_rate(camera, &error);
    assert(error == NULL);

    //gain
    camConfig.gainAvailable = arv_camera_is_gain_available(camera, &error);
    assert(error == NULL);
    arv_camera_get_gain_bounds(camera, &(camConfig.minGain), &(camConfig.maxGain), &error);
    assert(error == NULL);
    camConfig.currentGain = arv_camera_get_gain(camera, &error);
    assert(error == NULL);


    // payload
    camConfig.imagePayload = arv_camera_get_payload(camera, &error);
    assert(error == NULL);


    //image
    gint tempImgWidth = 0;
    gint tempImgHeight = 0;
    arv_camera_get_width_bounds(camera, NULL, &tempImgWidth, &error);
    assert(error == NULL);
    arv_camera_get_height_bounds(camera, NULL, &tempImgHeight, &error);
    assert(error == NULL);
    image->imageWidth = (unsigned int)tempImgWidth;
    image->imageHeight = (unsigned int)tempImgHeight;
    image->imageDepth = 0;
    image->imageUpdate = false;
    image->imageSize = camConfig.imagePayload;
    image->imageData = NULL;


    // TO DO init other shit ???

    // debug
    camDebug.numBytes = 0;
    camDebug.numErrors = 0;
    camDebug.numImages = 0;

    g_clear_error(&error);

}

void Camera::startVideo(void){
    if (!ARV_IS_CAMERA(camera)){
        printf("camera not connected %s\n", camInfo.deviceId);
        return;
    }
    assert(camera != NULL);

    GError *error = NULL;

    //start stream
    stream = arv_camera_create_stream(camera, Camera::streamCallback, NULL, &error);
	if (! ARV_IS_STREAM(stream)) {
        printf("arv_camera_create_stream() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
		g_clear_error(&error);
		return;
	}

    // FROM DOC, WHAT IS MEAN?
    bool autoSocketBuffer = false;
    bool packetResend = true;
    // in milli seconds
    unsigned int packetTimeout = 20;
    unsigned int frameRetention = 100;
    double packetRequestRatio = -1.0;

    if (ARV_IS_GV_STREAM(stream)) {
		if (autoSocketBuffer)
			g_object_set(stream,
				      "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO,
				      "socket-buffer-size", 0,
				      NULL);
		if (! packetResend)
			g_object_set(stream,
				      "packet-resend", ARV_GV_STREAM_PACKET_RESEND_NEVER,
				      NULL);
        if (packetRequestRatio >= 0.0)
            g_object_set(stream,
                      "packet-request-ratio", packetRequestRatio,
                      NULL);
		g_object_set(stream,
			      "packet-timeout", (unsigned) packetTimeout * 1000,
			      "frame-retention", (unsigned) frameRetention * 1000,
			      NULL);
	}

    // set emit signal in stream
    arv_stream_set_emit_signals(stream, TRUE);
    for (unsigned int i = 0; i < 5; i++) {
        arv_stream_push_buffer(stream, arv_buffer_new(camConfig.imagePayload, NULL));
    }

    assert(error == NULL);


    // start astream cquisition 
    arv_camera_start_acquisition(camera, &error);
    assert(error == NULL);

    //bind stream and callback function
    g_signal_connect_data(stream, "new-buffer", (GCallback)Camera::newBufferCallback, this, NULL, (GConnectFlags)0);

}

void Camera::stopVideo(void){
    assert(camera != NULL);

    // remove stream
    if (ARV_IS_STREAM(stream)) {
        arv_stream_set_emit_signals(stream, FALSE);
    }
	g_clear_object(&stream);

    GError *error = NULL;
    // stop acquisition
    if (ARV_IS_CAMERA(camera)) {
        arv_camera_stop_acquisition(camera, &error);
        if (error != NULL) {
            printf("arv_camera_stop_acquisition() failed : %s\n", (error != NULL) ? error->message : "???");
            assert(error == NULL);
            g_clear_error(&error);
        }
    }

    g_clear_error(&error);

}

void Camera::setGain(const float _value){
    GError *error = NULL;

    arv_camera_set_gain(camera, _value, &error);
    if (error != NULL) {
        printf("arv_camera_set_gain() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }
    camConfig.currentGain = _value;

    g_clear_error(&error);

}


void Camera::setFrameRate(const float _value){

    GError *error = NULL;

    if (! camConfig.frameRateAvailable) {
        return;
    }

    arv_camera_set_frame_rate(camera, _value, &error);
    if (error != NULL) {
        printf("arv_camera_set_frame_rate() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }
    camConfig.currentFrameRate = _value;

    g_clear_error(&error);

}

void Camera::setPixelFormat(const unsigned int _value) {
    GError *error = NULL;

    assert(_value < camConfig.numPixelFormats);

    // set pixel format as string
    // maybe better use ArvPixelFormat? TO DO
    // arv_camera_set_pixel_format_from_string(camera,
    // camConfig->availablePixelFormatsStrings[_value],
    // &error);
    arv_camera_set_pixel_format(camera,
    camConfig.availablePixelFormats[_value],
    &error);
    if (error != NULL) {
        printf("arv_camera_set_pixel_format() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }
    camConfig.currentPixelFormat = camConfig.availablePixelFormats[_value];
    camConfig.currentPixelFormatString = camConfig.availablePixelFormatsStrings[_value];

    // image payload might have changed with pixel format change
    camConfig.imagePayload = arv_camera_get_payload(camera, &error);
    if (error != NULL) {
        printf("arv_camera_get_payload() failed : %s\n", (error != NULL) ? error->message : "???");
        assert(error == NULL);
        g_clear_error(&error);
    }

    // need set new image depth???
    // image->imageDepth = 0;
    // switch (camConfig->currentPixelFormat) {
    // case ARV_PIXEL_FORMAT_MONO_16:
    //     // D("pixel format ARV_PIXEL_FORMAT_MONO_16\n");
    //     image->imageDepth = 0;
    //     break;
    // case ARV_PIXEL_FORMAT_MONO_8:
    //     // D("pixel format ARV_PIXEL_FORMAT_MONO_8\n");
    //     image->imageDepth = 0;
    //     break;
    // default:
    //     E("unhandled pixel format 0x%X\n", camConfig->currentPixelFormatString);
    //     break;
    // }
    // assert(image->imageDepth != 0);

    g_clear_error(&error);
}

Camera::~Camera(){
    printf("Clear cameras stuff\n");
    stopVideo();
    free(image);
    g_clear_object(&camera);
}

/* Callbacks */

