package vizinsight.atl.sceneoptimizerpro;

import android.Manifest;
import android.app.Activity;
import android.content.ContentValues;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.provider.MediaStore;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.ImageButton;

import androidx.appcompat.app.AppCompatActivity;

import java.io.FileNotFoundException;
import java.io.InputStream;
import java.net.URISyntaxException;

import vizinsight.atl.image_segmenter.VZImageSegmenter;

import static android.Manifest.permission.WRITE_EXTERNAL_STORAGE;

public class MainActivity extends AppCompatActivity {

    private static final int MULTIPLE_PERMISSIONS = 1;
    private static String[] PERMISSIONS_LIST = {
            Manifest.permission.CAMERA,
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if(savedInstanceState != null){
            mCapturedImagePath = savedInstanceState.getString("mCapturedImagePath");
        }
        else{
            mCapturedImagePath = "";
        }

        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getSupportActionBar().hide();
        setContentView(R.layout.activity_main);

        Utils.verifyPermissions(this, PERMISSIONS_LIST, MULTIPLE_PERMISSIONS);

        VZGlobals.getSegmenter(VZImageSegmenter.MODE_OD, "/vendor/etc/saiv/image_understanding/db/unified_detector/model.info");

        ImageButton cameraInputButton = (ImageButton)findViewById(R.id.camera_input);
        cameraInputButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                captureImage();
            }
        });

        ImageButton galleryButton = (ImageButton)findViewById(R.id.gallery_button);
        galleryButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //galleryIntent();
//                startActivityForSegments("/storage/emulated/0/Pictures/1574577031687.jpg");
		  start_folder_select_activity();
            }
        });

        try{
            String intent_mode = getIntent().getStringExtra("intent_mode");
            if(intent_mode == "camera"){
                cameraInputButton.callOnClick();
            }
            else if(intent_mode == "gallery"){
                galleryButton.callOnClick();
            }
        }catch(Exception e){

        }

    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        VZGlobals.releaseSegmenter();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putString("mCapturedImagePath", mCapturedImagePath);
    }

    final int SELECT_FILE = 2;
    private void galleryIntent(){
        Intent intent = new Intent();
        intent.setType("image/*");
        intent.setAction(Intent.ACTION_GET_CONTENT);
        startActivityForResult(Intent.createChooser(intent, "Pick me an image"), SELECT_FILE);
    }

    final int RC_TAKE_PHOTO = 1;
    String mCapturedImagePath;
    private void captureImage() {
        String fileName = "/sdcard/temp.jpg";
        ContentValues values = new ContentValues();
        values.put(MediaStore.Images.Media.TITLE, fileName);
        Uri capturedImageURI;
        capturedImageURI = getContentResolver().insert(
                MediaStore.Images.Media.EXTERNAL_CONTENT_URI, values);
        try {
            mCapturedImagePath = Utils.getFilePath(this, capturedImageURI);
        } catch (URISyntaxException e) {
            e.printStackTrace();
        }

        

        Intent intent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        intent.putExtra(MediaStore.EXTRA_OUTPUT, capturedImageURI);
        values.clear();
        startActivityForResult(intent, RC_TAKE_PHOTO);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        //Camera
        if (requestCode == RC_TAKE_PHOTO && resultCode == RESULT_OK) {
//            Log.e("Camera", data.getData()+"");
//            classify_camera_img(Utils.getFilePath(this, mCapturedImageURI));
            Log.e("SemanticMap_Camera path", mCapturedImagePath);
            startActivityForSegments(mCapturedImagePath);
        }

        //Gallery
        if(requestCode == SELECT_FILE && resultCode == RESULT_OK){
            Uri image_uri = data.getData();
            try {
                String galleryImagePath = Utils.getFilePath(this, image_uri);
                startActivityForSegments(galleryImagePath);
            } catch (URISyntaxException e) {
                e.printStackTrace();
            }
        }
    }

    private void startActivityForSegments(String imagePath) {
        Intent intent = new Intent(this, SegmentViewer.class);
        intent.putExtra("inputImagePath", imagePath);
        startActivity(intent);
    }

    public void start_folder_select_activity(){
	 Intent intent = new Intent(this, BatchTestActivity.class);
        startActivity(intent);
    }

}
