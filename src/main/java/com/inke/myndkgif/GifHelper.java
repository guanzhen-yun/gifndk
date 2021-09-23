package com.inke.myndkgif;

import android.graphics.Bitmap;

public class GifHelper {
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private volatile long gifInfo;

    public GifHelper(String path) {
        //加载Gif图片
        gifInfo = openFile(path);
    }

    public synchronized int getWidth() {
        return getWidthN(gifInfo);
    }

    public synchronized int getHeight() {
        return getHeightN(gifInfo);
    }

    public synchronized int getLength() {
        return getLengthN(gifInfo);
    }

    public long renderFrame(Bitmap bitmap, int index) {
        return renderFrameN(gifInfo, bitmap, index);
    }

    private native long renderFrameN(long gifInfo, Bitmap bitmap, int index);

    private native int getLengthN(long gifInfo);

    private native int getHeightN(long gifInfo);

    private native int getWidthN(long gifInfo);

    public native long openFile(String path);

}
