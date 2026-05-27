package vizinsight.atl.myapplication;


import android.app.assist.AssistStructure;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;
import android.widget.Toast;

import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.PickVisualMediaRequest;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.AppCompatActivity;

import com.google.android.material.button.MaterialButton;

import java.util.ArrayList;
import java.util.concurrent.CountDownLatch;

import vizinsight.atl.unifieddetector.DetectorBoxInfo;
import vizinsight.atl.unifieddetector.InitCallBack;
import vizinsight.atl.unifieddetector.UnifiedDetector;
import vizinsight.atl.unifieddetector.UnifiedDetectorWrapper;

public class MainActivity extends AppCompatActivity {
    ImageView imageView;
    private final int MODE_OD =0;
    private final String  TAG = "WineDetectorJARUsage";
    private CountDownLatch mCountDownLatch;
    InitCallBack initCallBack;
    public static UnifiedDetector unifiedDetector;
    Uri mUrl;
    boolean mInitStatus;
    InitCallBack mInitCallBack;
    ActivityResultLauncher<PickVisualMediaRequest> launcher = registerForActivityResult(new ActivityResultContracts.PickVisualMedia(), new ActivityResultCallback<Uri>() {
        @Override
        public void onActivityResult(Uri url) {
            if (url == null) {
                Toast.makeText(MainActivity.this, "No image Selected", Toast.LENGTH_SHORT).show();
            } else {
                mUrl = url;
            }
        }
    });
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        imageView = findViewById(R.id.imageView);
        MaterialButton pickImage = findViewById(R.id.pickImage);


        mCountDownLatch = new CountDownLatch(1);  // To Stop activity exiting while Snap execution is in progress
        Log.d(TAG,"on Create");

        boolean unifiedDetectorAvailable = true;//unifiedDetector.isUnifiedDetectorAvailable();
        //Log.d(TAG,"Wine Available : "+wineDetectorAvailable);
        unifiedDetector = UnifiedDetectorWrapper.getUnifiedDetector();
        initCallBack = new InitCallBack() {
            @Override
            public void initCompleteCallBackReceived(boolean initStatus) {
                mInitStatus = initStatus;
                mCountDownLatch.countDown();
            }
        };

        unifiedDetector.setModelInitListener(initCallBack);
        try {
            unifiedDetector.unifiedDetectorModelInit(getApplicationContext(),MODE_OD);
        } catch (SecurityException ex) {
            Toast.makeText(getApplicationContext(), "App is not allowed to use the library", Toast.LENGTH_LONG).show();
        }

        waitForCallback(); // To Stop activity exiting while Snap execution is in progress

        pickImage.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                /*launcher.launch(new PickVisualMediaRequest.Builder()
                        .setMediaType(ActivityResultContracts.PickVisualMedia.VideoOnly.INSTANCE)
                        .build());*/
                // Launch the photo picker and let the user choose only images/videos of a
                /*launcher.launch(new PickVisualMediaRequest.Builder()
                        .setMediaType(ActivityResultContracts.PickVisualMedia.ImageOnly.INSTANCE)
                        .build());*/
                Intent intent = new Intent(getApplicationContext(), BatchTestActivity.class);
                startActivity(intent);
            }
        });
    }

    private void waitForCallback() {
        try {
            mCountDownLatch.await();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}