package com.inke.myndkgif;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.graphics.Bitmap;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends AppCompatActivity {

    private ImageView image;

    private String path = Environment.getExternalStorageDirectory().getAbsolutePath() + File.separator + "mycat.gif";

    private GifHelper gifHelper;

    private Bitmap bitmap;
    private int currentIndex = 0;
    private int maxIndex = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE}, 100);
        }
        image = findViewById(R.id.image);

        File file = new File(path);
        if(!file.exists()) {
            return;
        }
        gifHelper = new GifHelper(path);
        int width = gifHelper.getWidth();
        int height = gifHelper.getHeight();
        maxIndex = gifHelper.getLength();
        bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        //图像渲染
        long nextFrameTime = gifHelper.renderFrame(bitmap, currentIndex);
        image.setImageBitmap(bitmap);
        if(handler != null) {
            handler.sendEmptyMessageDelayed(1, nextFrameTime);
        }
    }

    private Handler handler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(@NonNull Message msg) {
            currentIndex++;
            if(currentIndex >= maxIndex) {
                currentIndex = 0;
            }
            long nextFrameTime = gifHelper.renderFrame(bitmap, currentIndex);
            image.setImageBitmap(bitmap);
            handler.sendEmptyMessageDelayed(1, nextFrameTime);
        }
    };
}