package vizinsight.atl.myapplication;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.documentfile.provider.DocumentFile;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.ArrayList;

import vizinsight.atl.unifieddetector.DetectorBoxInfo;

public class BatchTestActivity extends AppCompatActivity {
    Context mContext;
    boolean ENABLE_BOX=false;
    TextView directoryText;
    ImageButton select_button;
    ImageButton play_stop_button;
    ImageView progress_image;
    TextView progress_text;
    String image_folder = null;
    boolean isTestRunning = false;
    TestRunner test_runner;
    final int IMAGEFORMAT_NV21 = 0;
    final int IMAGEFORMAT_BGR  = 1;
    final int IMAGEFORMAT_RGBA = 2;
    final int IMAGEFORMAT_NV12 = 3;
    final int IMAGEFORMAT_P010 = 3;
    String Mode;

    private String TAG = "BatchTestActivity_APP1";
    private Handler handler = new Handler(Looper.getMainLooper());
    DocumentFile pickedDir;
    Uri treeUri;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        View decorView = getWindow().getDecorView();
        int uiOptions = View.SYSTEM_UI_FLAG_FULLSCREEN;
        decorView.setSystemUiVisibility(uiOptions);
        setContentView(R.layout.activity_batch_test);
        setTitle("Batch Test");
        Mode = getIntent().getStringExtra("mode");
        mContext = this.getApplicationContext();

        directoryText       = findViewById(R.id.directory_text);
        play_stop_button    = findViewById(R.id.play_stop_folder);
        progress_image      = findViewById(R.id.progress_image);
        progress_text       = findViewById(R.id.progress_text);

        select_button = findViewById(R.id.select_folder_button);
        select_button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                select_folder();
            }
        });

        play_stop_button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(isTestRunning){
                    Toast.makeText(mContext, "Testing stopped", Toast.LENGTH_SHORT).show();
                    stopTesting();
                }
                else{
                    startTesting();
                }
            }
        });
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {

        if(true){
            return true;
        }
        boolean val = super.onCreateOptionsMenu(menu);
        menu.findItem(R.id.style_select).setEnabled(false);
        return val;
    }

    private void startTesting() {
        test_runner = new TestRunner();
        test_runner.execute(image_folder);
        play_stop_button.setImageResource(R.drawable.stop);
        select_button.setAlpha(0.5f);
        select_button.setClickable(false);
        isTestRunning = true;
    }

    private void stopTesting() {
        if(test_runner!= null && !test_runner.isCancelled()){
            test_runner.cancel(true);
        }
        clearProgress();
        play_stop_button.setImageResource(R.drawable.play);
        select_button.setAlpha(1f);
        select_button.setClickable(true);
        isTestRunning = false;
    }
    private void clearProgress(){
        progress_image.setImageResource(android.R.color.transparent);
        progress_text.setText("");

    }
    int SELECT_FOLDER_REQUEST_CODE = 0;
    void select_folder(){
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        startActivityForResult(intent, SELECT_FOLDER_REQUEST_CODE);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if(requestCode == SELECT_FOLDER_REQUEST_CODE && resultCode != 0){
            treeUri = data.getData();
            //try {
                pickedDir = DocumentFile.fromTreeUri(this, treeUri);
            //Log.d(TAG,"selected URI : "+treeUri);
                //image_folder = Utils.getPathFromUri(getApplicationContext(), treeUri);
            //Log.d(TAG,"selected image_folder : "+image_folder);
            //} catch (URISyntaxException e) {
            //    throw new RuntimeException(e);
            //}
            //directoryText.setText(Html.fromHtml("Image Directory: <u>"+image_folder+"</u>"));
            play_stop_button.setVisibility(View.VISIBLE);
        }
        super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        boolean itemSelectStatus = super.onOptionsItemSelected(item);
        if(!itemSelectStatus) return false;
        switch(item.getItemId()){
            /*case R.id.mode_live_focus:
                Toast.makeText(this, "Live-Focus Mode not supported in Batch Testing", Toast.LENGTH_LONG).show();
                return true;*/
            default:
                if(isTestRunning){
                    stopTesting();
                }
                clearProgress();
                return true;
        }
    }

    private class TestRunner extends AsyncTask<String, Integer, Integer>{
        Bitmap progressbar_bitmap_full;
        Bitmap progressbar_bitmap_boundary;
        Bitmap progressbar_bitmap;
        final int progressbar_size = 500;
        int progress = 0;
        int previous_progress = 0;
        String result_folder_name = "";

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            progressbar_bitmap_full = BitmapFactory.decodeResource(getResources(), R.drawable.mask_on_logo);
            progressbar_bitmap_full = progressbar_bitmap_full.createScaledBitmap(progressbar_bitmap_full, progressbar_size, progressbar_size, false);
            progressbar_bitmap = Bitmap.createBitmap(progressbar_size, progressbar_size, Bitmap.Config.ARGB_8888);
            progressbar_bitmap_boundary = BitmapFactory.decodeResource(getResources(), R.drawable.mask_on_boundary);
            progressbar_bitmap_boundary = progressbar_bitmap_boundary.createScaledBitmap(progressbar_bitmap_boundary, progressbar_size, progressbar_size, false);

        }

        private void update_progress_text(final String image_path){
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if(isTestRunning) {
                        progress_text.setText(image_path);
                    }
                }
            });
        }

        private void showToast(final String message){
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(mContext, message, Toast.LENGTH_SHORT).show();
                }
            });
        }

        @Override
        protected Integer doInBackground(String... strings) {
            for (DocumentFile file : pickedDir.listFiles()) {
                if (file.isFile() && (file.getName().toLowerCase().endsWith(".jpg") || file.getName().toLowerCase().endsWith(".jpeg")
                                        || file.getName().toLowerCase().endsWith(".png"))) {
                    try {
                        InputStream inputStream = getContentResolver().openInputStream(file.getUri());
                        Bitmap bitmap = BitmapFactory.decodeStream(inputStream);

                        int height = bitmap.getHeight();
                        int width = bitmap.getWidth();
                        Log.d(TAG,"Bitmap Width : "+width+" Height : "+height);
                        ByteBuffer buffer = ByteBuffer.allocateDirect(width * height * 3);
                        int[] pixels = new int[width * height];
                        bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
                        for (int j = 0; j < pixels.length; j++) {
                            int pixel = pixels[j];
                            buffer.put((byte) Color.blue(pixel)); // B
                            buffer.put((byte) Color.green(pixel)); // G
                            buffer.put((byte) Color.red(pixel)); // R
                        }
                        buffer.rewind();
                        byte[] bgrByteArr = buffer.array();
                        ArrayList<DetectorBoxInfo> boxInfos = MainActivity.unifiedDetector.getDetectorBoxInfo(bgrByteArr,bitmap.getWidth(), bitmap.getHeight(), 0,1);
                        Bitmap bitmap1 = bitmap.copy(Bitmap.Config.ARGB_8888,true);
                        Canvas canvas = new Canvas(bitmap1);
                        Paint paint = new Paint();
                        paint.setColor(Color.RED);
                        paint.setStyle(Paint.Style.STROKE);
                        paint.setStrokeWidth(10); // Adjust width
                        // Process the bitmap here
                        if(boxInfos != null) {
                            for (int i = 0; i < boxInfos.size(); i++) {
                                DetectorBoxInfo boxInfo = boxInfos.get(i);
                                Log.d(TAG, "boxinfo : tag : " + boxInfo.tag + " tag ID : " + boxInfo.tagId + " score : " + boxInfo.score
                                        + " left : " + boxInfo.left + " top : " + boxInfo.top + "right : " + boxInfo.right + "bottom : " + boxInfo.bottom);
                                canvas.drawRect(boxInfo.left, boxInfo.top, boxInfo.right, boxInfo.bottom, paint);
                            }
                        }


                        // Determine output filename and format
                        String fileName = file.getName();
                        int extIndex = fileName.lastIndexOf('.');
                        String baseName = (extIndex == -1) ? fileName : fileName.substring(0, extIndex);
                        String ext = (extIndex == -1) ? ".png" : fileName.substring(extIndex).toLowerCase();
                        Bitmap.CompressFormat format;

                        if (ext.equals(".jpg") || ext.equals(".jpeg")) {
                            format = Bitmap.CompressFormat.JPEG;
                        } else {
                            format = Bitmap.CompressFormat.PNG;
                        }

                        String outputFileName = "sdcard/Download/UD_Test_App/" + "0_result" + ext;
                        File outputFile = new File(outputFileName);

                        // Save the modified image
                        bitmap1.compress(format, 100, new FileOutputStream(outputFile));
                        Log.d(TAG,"Saved as: " + outputFile.getAbsolutePath());

                        progress++;
                        //publishProgress(progress * 100 / n_images);
                        update_progress_text("Processed images : "+progress);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }


                // Update progress

            }
            return null;
        }
        @Override
        protected void onPostExecute(Integer integer) {
            super.onPostExecute(integer);
            stopTesting();
            isTestRunning = false;
            if(progress != 0) {
                Toast.makeText(mContext, "Test Completed", Toast.LENGTH_SHORT).show();
                progress_image.setImageBitmap(progressbar_bitmap_full);
            }
            progress_text.setText("Results are stored in \""+result_folder_name+"\"");
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            super.onProgressUpdate(values);
            int pro = values[0] * progressbar_size / 100;
            int px[] = new int[progressbar_size *(pro - previous_progress)];
            progressbar_bitmap_full.getPixels(px,0, progressbar_size, 0, progressbar_size -pro, progressbar_size, pro- previous_progress);
            progressbar_bitmap.setPixels(px, 0, progressbar_size, 0, progressbar_size -pro, progressbar_size, pro- previous_progress);
            progressbar_bitmap = Utils.overlay(progressbar_bitmap_boundary, progressbar_bitmap);
            progress_image.setImageBitmap(progressbar_bitmap);
            previous_progress = pro;
        }
    }

}
