#include <jni.h>
#include <string>
#include <malloc.h>
#include "android/bitmap.h"
#include "giflib/gif_lib.h"
#include "giflib/gif_lib_private.h"
#include "gif.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_inke_myndkgif_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_inke_myndkgif_GifHelper_openFile(JNIEnv *env, jobject instance, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    int err;
    GifFileType *gif = DGifOpenFileName(path, &err);
    err = DGifSlurp(gif);
    env->ReleaseStringUTFChars(path_, path);
    return reinterpret_cast<jlong>(gif);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_inke_myndkgif_GifHelper_getLengthN(JNIEnv *env, jobject thiz, jlong gifInfo) {
    return ((GifFileType *) gifInfo)->ImageCount;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_inke_myndkgif_GifHelper_getHeightN(JNIEnv *env, jobject thiz, jlong gifInfo) {
    return ((GifFileType *) gifInfo)->SHeight;
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_inke_myndkgif_GifHelper_getWidthN(JNIEnv *env, jobject thiz, jlong gifInfo) {
    return ((GifFileType *) gifInfo)->SWidth;
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_inke_myndkgif_GifHelper_renderFrameN(JNIEnv *env, jobject thiz, jlong gifInfo,
                                              jobject bitmap, jint index) {
    // 绘制帧 c/c++ bitmap
    GifFileType *gif_info = (GifFileType *) gifInfo;
    AndroidBitmapInfo info;
    void *pixels;
    int ret;
    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        return -1;
    }
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        return -1;
    }
    //将bitmap转成数组
    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &pixels)) < 0) {
        return -1;
    }
    //渲染帧 处理像素矩阵
    int delay_time = drawFrame(gif_info, &info, (int *)pixels, index, false);
    //解锁 转为bitmap
    AndroidBitmap_unlockPixels(env, bitmap);
    return delay_time;
}

#define argb(a, r, g, b) (((a) & 0xff) << 24) | (((b) & 0xff) << 16) | (((g) & 0xff) << 8) | ((r) & 0xff)
#define dispose(ext) (((ext)->Bytes[0] & 0x1c) >> 2)
#define trans_index(ext) ((ext)->Bytes[3])
#define transparency(ext) ((ext)->Bytes[0] & 1)
#define delay(ext) (10*((ext)->Bytes[2] << 8 | (ext) ->Bytes[1]))

int drawFrame(GifFileType *gif, AndroidBitmapInfo *info, int *pixels, int frame_no,
              bool force_dispose_1) {
    GifColorType *bg;
    GifColorType *color;
    SavedImage *frame;
    ExtensionBlock *ext = 0;
    GifImageDesc *frameInfo;
    ColorMapObject *colorMap;
    int *line;
    int width, height, x, y, j, loc, n, inc, p;
    int *px;
    width = gif->SWidth;
    height = gif->SHeight;
    frame = &(gif->SavedImages[frame_no]);
    frameInfo = &(frame->ImageDesc);
    if (frameInfo->ColorMap) {
        colorMap = frameInfo->ColorMap;
    } else {
        colorMap = gif->SColorMap;
    }
    bg = &colorMap->Colors[gif->SBackGroundColor];

    for (j = 0; j < frame->ExtensionBlockCount; j++) {
        if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
            ext = &(frame->ExtensionBlocks[j]);
            break;
        }
    }
    //For dispose = 1, We assume its been drawn
    px = pixels;
    if (ext && dispose(ext) == 1 && force_dispose_1 && frame_no > 0) {
        //覆盖前一帧
        drawFrame(gif, info, pixels, frame_no - 1, true);
    } else if (ext && dispose(ext) == 2 && bg) {
        for (y = 0; y < height; y++) {
            line = (int *) px;
            for (x = 0; x < width; x++) {
                line[x] = argb(255, bg->Red, bg->Green, bg->Blue);
            }
            px = (int *) ((char *) px + info->stride);
        }
    } else if (ext && dispose(ext) == 3 && frame_no > 1) {
        //被前一帧覆盖
        drawFrame(gif, info, pixels, frame_no - 2, true);
    }
    px = pixels;
    if (frameInfo->Interlace) {
        n = 0;
        inc = 0;
        p = 0;
        px = (int *) ((char *) px + info->stride * frameInfo->Top);
        for (y = frameInfo->Top; y < frameInfo->Top + frameInfo->Height; y++) {
            for (x = frameInfo->Left; x < frameInfo->Left + frameInfo->Width; x++) {
                loc = (y - frameInfo->Top) * frameInfo->Width + (x - frameInfo->Left);
                if (ext && frame->RasterBits[loc] == trans_index(ext) && transparency(ext)) {
                    continue;
                }
                color = (ext && frame->RasterBits[loc] == trans_index(ext)) ? bg
                                                                            : &colorMap->Colors[frame->RasterBits[loc]];
                if (color) {
                    line[x] = argb(255, color->Red, color->Green, color->Blue);
                }
            }
            px = (int *) ((char *) px + info->stride * inc);
            n += inc;
            if (n > frameInfo->Height) {
                n = 0;
                switch (p) {
                    case 0:
                        px = (int *) ((char *) pixels + info->stride * (4 + frameInfo->Top));
                        inc = 8;
                        p++;
                        break;
                    case 1:
                        px = (int *) ((char *) pixels + info->stride * (2 + frameInfo->Top));
                        inc = 4;
                        p++;
                        break;
                    case 2:
                        px = (int *) ((char *) pixels + info->stride * (1 + frameInfo->Top));
                        inc = 2;
                        p++;
                        break;
                }
            }
        }
    } else {
        px = (int *) ((char *) px + info->stride * frameInfo->Top);
        for (y = frameInfo->Top; y < frameInfo->Top + frameInfo->Height; y++) {
            line = (int *) px;
            for (x = frameInfo->Left; x < frameInfo->Left + frameInfo->Width; x++) {
                loc = (y - frameInfo->Top) * frameInfo->Width + (x - frameInfo->Left);
                if (ext && frame->RasterBits[loc] == trans_index(ext) && transparency(ext)) {
                    continue;
                }
                color = (ext && frame->RasterBits[loc] == trans_index(ext)) ? bg
                                                                            : &colorMap->Colors[frame->RasterBits[loc]];
                if (color) {
                    line[x] = argb(255, color->Red, color->Green, color->Blue);
                }
            }
            px = (int *) ((char *) px + info->stride);
        }
    }
    return delay(ext);
}