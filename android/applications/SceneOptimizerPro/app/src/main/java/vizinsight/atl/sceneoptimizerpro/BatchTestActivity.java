// package vizinsight.atl.sceneoptimizerpro;

// import android.content.Context;
// import android.content.Intent;
// import android.graphics.Bitmap;
// import android.graphics.BitmapFactory;
// import android.graphics.Canvas;
// import android.graphics.Color;
// import android.graphics.Paint;
// import android.os.AsyncTask;
// import android.os.Build;
// import android.os.Bundle;
// import android.net.Uri;
// import android.text.Html;
// import android.util.Log;
// import android.view.Menu;
// import android.graphics.Rect;
// import android.view.MenuItem;
// import android.view.View;
// import android.view.WindowManager;
// import android.widget.ImageButton;
// import android.widget.ImageView;
// import android.widget.TextView;
// import android.widget.Toast;
// import android.content.pm.ActivityInfo;
// import android.content.Context;
// import android.provider.DocumentsContract;

// import androidx.annotation.NonNull;
// import androidx.annotation.Nullable;

// import org.json.JSONArray;
// import org.json.JSONException;
// import org.json.JSONObject;

// import java.io.BufferedReader;
// import java.io.File;
// import java.io.FileNotFoundException;
// import java.io.FileInputStream;
// import java.io.FileOutputStream;
// import java.io.FileReader;
// import java.io.IOException;
// import java.net.URISyntaxException;
// import java.util.ArrayList;
// import java.util.Collection;
// import java.util.Collections;
// import java.util.Iterator;
// import java.util.List;
// import java.util.ListIterator;
// import java.io.FilenameFilter;
// import java.util.HashMap;

// import androidx.appcompat.app.AppCompatActivity;

// import lib.folderpicker.FolderPicker;

// import vizinsight.atl.image_segmenter.VZEntity;
// import vizinsight.atl.image_segmenter.VZImageSegmenter;

// public class BatchTestActivity extends AppCompatActivity {
//      private String TAG = "UOD_BatchTestActivity";
//     Context mContext;
//     boolean DISPLAY_BOX=false;
//     TextView directoryText;
//     ImageButton select_button;
//     ImageButton play_stop_button;
//     ImageView progress_image;
//     TextView progress_text;

//     String image_folder = null;
//     boolean isTestRunning = false;

//     TestRunner test_runner;
//     VZImageSegmenter object_detector;
//     Bitmap mask;
//     final int IMAGEFORMAT_NV21 = 0;
//     final int IMAGEFORMAT_BGR  = 1;
//     final int IMAGEFORMAT_RGBA = 2;
//     final int IMAGEFORMAT_NV12 = 3;

//     @Override
//     protected void onCreate(Bundle savedInstanceState) {
// //        segmenterMode = VZImageSegmenter.MODE_SALIENCY;
//         setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);


//         object_detector = VZGlobals.getSegmenter(VZImageSegmenter.MODE_OD, "/vendor/etc/saiv/image_understanding/db/unified_detector/model.info");

//         super.onCreate(savedInstanceState);
//         /*super.setInfo("Select the folder with images\n\n" +
//                 "After processing the results are generated in '/sdcard/results/' directory\n\n" +
//                 "Batch Testing is not supported in Live-Focus mode");*/

//         if (Build.VERSION.SDK_INT < 16) {
//             getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
//                     WindowManager.LayoutParams.FLAG_FULLSCREEN);
//         }
//         View decorView = getWindow().getDecorView();
//         int uiOptions = View.SYSTEM_UI_FLAG_FULLSCREEN;
//         decorView.setSystemUiVisibility(uiOptions);
//         setContentView(R.layout.activity_batch_test);
//         setTitle("Batch Test");

//         mContext = this.getApplicationContext();

//         directoryText       = findViewById(R.id.directory_text);
//         play_stop_button    = findViewById(R.id.play_stop_folder);
//         progress_image      = findViewById(R.id.progress_image);
//         progress_text       = findViewById(R.id.progress_text);

//         select_button = findViewById(R.id.select_folder_button);
//         select_button.setOnClickListener(new View.OnClickListener() {
//             @Override
//             public void onClick(View view) {
//                 select_folder();
//             }
//         });

//         play_stop_button.setOnClickListener(new View.OnClickListener() {
//             @Override
//             public void onClick(View view) {
//                 if(isTestRunning){
//                     Toast.makeText(mContext, "Testing stopped", Toast.LENGTH_SHORT).show();
//                     stopTesting();
//                 }
//                 else{
//                     startTesting();
//                 }
//             }
//         });
//     }

//     @Override
//     public boolean onCreateOptionsMenu(Menu menu) {

//         if(true){
//             return true;
//         }
//         /*boolean val = super.onCreateOptionsMenu(menu);
//         menu.findItem(R.id.style_select).setEnabled(false);
//         return val;*/
//         return true;
//     }

//     private void startTesting() {
//         test_runner = new TestRunner();
//         test_runner.execute(image_folder);
//         play_stop_button.setImageResource(R.drawable.stop);
//         select_button.setAlpha(0.5f);
//         select_button.setClickable(false);
//         isTestRunning = true;
//     }

//     private void stopTesting() {
//         if(test_runner!= null && !test_runner.isCancelled()){
//             test_runner.cancel(true);
//         }
//         clearProgress();
//         play_stop_button.setImageResource(R.drawable.play);
//         select_button.setAlpha(1f);
//         select_button.setClickable(true);
//         isTestRunning = false;
//     }
//     private void clearProgress(){
//         progress_image.setImageResource(android.R.color.transparent);
//         progress_text.setText("");

//     }
//     int SELECT_FOLDER_REQUEST_CODE = 0;
//     //private static final int REQUEST_CODE_FOLDER_PICKER = 101;
//     void select_folder(){
//         Intent intent = new Intent(this, FolderPicker.class);
//         startActivityForResult(intent, SELECT_FOLDER_REQUEST_CODE);
//         /*Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
//         startActivityForResult(intent, REQUEST_CODE_FOLDER_PICKER);*/
//     }

//     @Override
//     protected void onActivityResult(int requestCode, int resultCode, Intent data) {
//         if(requestCode == SELECT_FOLDER_REQUEST_CODE && resultCode != 0){
//             image_folder = data.getExtras().getString("data");
//             directoryText.setText(Html.fromHtml("Image Directory: <u>"+image_folder+"</u>"));
//             play_stop_button.setVisibility(View.VISIBLE);
//         }

//         /*if (requestCode == REQUEST_CODE_FOLDER_PICKER && resultCode == RESULT_OK) {
//             try{
//             Uri treeUri = data.getData();
//             Log.d("VZ Debug", "Selected Path:  "+treeUri);
//             image_folder = Utils.getFilePath(this,treeUri);
//             Log.d("VZ Debug", "Selected Path:  "+image_folder);
//             directoryText.setText(Html.fromHtml("Image Directory: <u>"+image_folder+"</u>"));
//             play_stop_button.setVisibility(View.VISIBLE);
//             } catch (URISyntaxException e) {
//                 e.printStackTrace();
//             }
//         }*/
//         super.onActivityResult(requestCode, resultCode, data);
//     }

//     @Override
//     public boolean onOptionsItemSelected(MenuItem item) {
//         boolean itemSelectStatus = super.onOptionsItemSelected(item);
//         if(!itemSelectStatus) return false;
//         switch(item.getItemId()){
//             /*case R.id.mode_live_focus:
//                 Toast.makeText(this, "Live-Focus Mode not supported in Batch Testing", Toast.LENGTH_LONG).show();
//                 return true;*/
//             default:
//                 if(isTestRunning){
//                     stopTesting();
//                 }
//                 clearProgress();
//                 return true;
//         }
//     }

//     private class TestRunner extends AsyncTask<String, Integer, Integer>{
//         Bitmap progressbar_bitmap_full;
//         Bitmap progressbar_bitmap_boundary;
//         Bitmap progressbar_bitmap;
//         final int progressbar_size = 500;
//         int progress = 0;
//         int previous_progress = 0;
//         String result_folder_name = "";

//         @Override
//         protected void onPreExecute() {
//             super.onPreExecute();
//             progressbar_bitmap_full = BitmapFactory.decodeResource(getResources(), R.drawable.mask_on_logo);
//             progressbar_bitmap_full = progressbar_bitmap_full.createScaledBitmap(progressbar_bitmap_full, progressbar_size, progressbar_size, false);
//             progressbar_bitmap = Bitmap.createBitmap(progressbar_size, progressbar_size, Bitmap.Config.ARGB_8888);
//             progressbar_bitmap_boundary = BitmapFactory.decodeResource(getResources(), R.drawable.mask_on_boundary);
//             progressbar_bitmap_boundary = progressbar_bitmap_boundary.createScaledBitmap(progressbar_bitmap_boundary, progressbar_size, progressbar_size, false);

//         }

//         private void update_progress_text(final String image_path, final int abs_progress, final int total_count){
//             runOnUiThread(new Runnable() {
//                 @Override
//                 public void run() {
//                     if(isTestRunning) {
//                         progress_text.setText("(" + abs_progress + "/" + total_count + "): " + image_path);
//                     }
//                 }
//             });
//         }

//         private void showToast(final String message){
//             runOnUiThread(new Runnable() {
//                 @Override
//                 public void run() {
//                     Toast.makeText(mContext, message, Toast.LENGTH_SHORT).show();
//                 }
//             });
//         }

//         @Override
//         protected Integer doInBackground(String... strings) {
//             if(object_detector == null || !object_detector.isValid){
//                 return null;
//             }
//             String image_folder_name = strings[0];
//             Log.d(TAG,"doInBackground image_folder_name : "+image_folder_name);

//             // Make result folder
//             result_folder_name = get_results_folder_name(image_folder_name);
//             Log.d(TAG,"doInBackground : result_folder_name : "+result_folder_name);
//             File dir = new File(result_folder_name);
//             if(dir.exists()){
//                 dir.delete();
//             }
//             dir.mkdirs();
//             showToast("Results will be stored in \""+result_folder_name+"\" directory");

//             // List images in the folder
//             ArrayList image_filepaths = new ArrayList();
//             Utils.list_files(image_folder_name, image_filepaths);
//             int n_images = image_filepaths.size();
//             //Collections.sort(image_filepaths);

//             Log.v(TAG, "Starting test on "+n_images+" images");

//             // Execute on images
//             for(int i = 0; i < n_images && !isCancelled(); i++){
//                 String image_path;
//                     image_path = image_filepaths.get(i).toString();
//                 String image_name = ""; //modification
//                     image_name = image_path.substring(image_path.lastIndexOf('/')+1,image_path.length()); //Changed to read any folder name  
//                     Log.d(TAG, "( " + i + " /  " + n_images + "  ) " + image_path );
//                     Log.d(TAG, "image_name : "+image_name);
//                 if(!Utils.isValidImagePath(image_path)){
//                     continue;
//                 }
//                 List<VZEntity> entities = null;
//                 try{
//                     entities = object_detector.execute(image_path,IMAGEFORMAT_NV21,image_name); //modified for raw dumps - using NV21 to leverage Halide AOT preprocessing path

//                 }catch(OutOfMemoryError e){
//                     showToast(image_path+" size is large. So ignoring the image to proess");
//                 }
//                 int width;
//                 // Save results in results folder
//                 HashMap<String,Integer> classes=new HashMap<String, Integer>();
//                 classes.put("object",1);
//                 classes.put("person",2);
//                 classes.put("food",3);
//                 classes.put("vehicle",4);
//                 classes.put("flower",5);
//                 classes.put("Animal",6);
//                 classes.put("drink",7);
//                 classes.put("petface",8);
//                 classes.put("moon",9);
//                 classes.put("human_head",10);
//                 classes.put("pets",11);
//                 classes.put("text",12);
//                 classes.put("scene_text",13);
//                 classes.put("Wine",15);
//                 classes.put("wine_label",16);
                
//                 String out_filepath = image_path.replace(image_folder_name, result_folder_name);
//                 String out_filepath_txt ;
//                 if(out_filepath.contains(".JPEG")){
//                     out_filepath_txt = (out_filepath.split("[.]JP"))[0] + ".txt";
//                 }
//                 else if(out_filepath.contains(".jpg")){
//                     out_filepath_txt = (out_filepath.split("[.]jp"))[0] + ".txt";
//                 }
//                 else if(out_filepath.contains(".jpeg")){
//                     out_filepath_txt = (out_filepath.split("[.]jp"))[0] + ".txt";
//                 }
//                 else if(out_filepath.contains(".png")){
//                     out_filepath_txt = (out_filepath.split("[.]pn"))[0] + ".txt";
//                 }
//                 else if(out_filepath.contains(".PNG")){
//                     out_filepath_txt = (out_filepath.split("[.]PN"))[0] + ".txt";
//                 }
//                 else if(out_filepath.contains(".bmp")){
//                     out_filepath_txt = (out_filepath.split("[.]bm"))[0] + ".txt";
//                 }
//                 else if(out_filepath.contains(".BMP")){
//                     out_filepath_txt = (out_filepath.split("[.]BM"))[0] + ".txt";
//                 }
//                 else{
//                     out_filepath_txt=out_filepath+".txt";
//                 }
//                 String txt = "";

//                 if(!(entities== null) && !entities.isEmpty()) {
//                     Bitmap bm = BitmapFactory.decodeFile(image_path);
//                     Bitmap temp_mask;


//                     bm = Utils.rotateBitmap(bm, Utils.getImageOrientation(image_path));
//                     temp_mask = bm.copy(Bitmap.Config.ARGB_8888,true);
//                     width=bm.getWidth();
                    
//                     Paint p = new Paint();
//                     p.setStyle(Paint.Style.STROKE);
//                     p.setAntiAlias(true);
//                     p.setFilterBitmap(true);
//                     p.setDither(true);
//                     p.setColor(Color.BLUE);
//                     p.setTextSize(width/25);

//                     Rect text_bounds = new Rect();
//                     Paint paint_box = new Paint();
//                     paint_box.setStyle(Paint.Style.FILL);
//                     paint_box.setAntiAlias(true);
//                     paint_box.setColor(Color.WHITE); //Color.BLUE
//                     paint_box.setStrokeWidth(width/320.0f);
//                     Paint paint_text_box = new Paint(paint_box);
//                     paint_text_box.setStyle(Paint.Style.FILL);
//                     paint_text_box.setTextSize(width/25);
//                     paint_box.setTextSize(width/25);
                    
//                     Canvas c = new Canvas(temp_mask);
//                     String[] lines = new String[10];
//                     for (int idx = 0; idx < entities.size(); idx++) {
//                         VZEntity ent = entities.get(idx);
//                         if(ent.left == 0 && ent.top == 0 && ent.right==0 &&ent.bottom ==0){
//                             /*p.setStyle(Paint.Style.STROKE);
//                             p.setColor(Color.GREEN);
//                             p.setTextSize(40);*/
//                             String tempString;
//                             float percentage_score = ent.score;
//                             String formattedPercentage = String.format("%.1f%%", percentage_score);
//                             Log.d("VZ Debug", ent.tag + " : "+ formattedPercentage);
//                             tempString = ent.tag + " : "+formattedPercentage;
//                             lines[idx] = tempString;
                            
//                         }else{
//                             if(ent.tag.contains("et")){
//                                 p.setColor(Color.RED);
//                                 paint_text_box.setColor(Color.WHITE);
//                                 paint_box.setColor(Color.RED);
//                             }
//                             else{
//                                 p.setColor(Color.BLUE);
//                                 paint_text_box.setColor(Color.TRANSPARENT);
//                                 paint_box.setColor(Color.BLUE);
//                             }
                            
//                             Log.d(TAG,"Drawing the box for Images.");
//                             c.drawRect(ent.left, ent.top, ent.right, ent.bottom, p);
//                             paint_text_box.getTextBounds(ent.trackId+"_"+ent.tag+"_"+(int)(ent.score*100), 0, (ent.associationId+"_"+ent.tag+"_"+(int)(ent.score*100)).length(), text_bounds);
//                             c.drawRect(ent.left, ent.top, ent.left + text_bounds.width(), ent.top - text_bounds.height(), paint_text_box);

//                             c.drawText(ent.trackId+"_"+ent.tag + "_" + (int)(ent.score*100), ent.left, ent.top, paint_box);
//                         }
//                         txt = txt + ent.trackId + " ";
//                         txt = txt + ent.tagId + " ";
//                         txt = txt + ent.score + " ";
//                         txt = txt + ent.left + " ";
//                         txt = txt + ent.top + " ";
//                         txt = txt + ent.right+ " ";
//                         txt = txt + ent.bottom + "\n";
                        
//                     }
//                     int y = 0;
//                     for (int idx = 0; idx < entities.size(); idx++) {
//                         VZEntity ent = entities.get(idx);
//                         if(ent.left == 0 && ent.top == 0 && ent.right==0 &&ent.bottom ==0){
//                             Log.d("VZ Debug", "line : "+lines[idx]);
//                             p.setColor(Color.GREEN);
//                             p.setStyle(Paint.Style.FILL);
//                             y += p.getTextSize();
//                             c.drawText(lines[idx], 0, y, p);
//                         }
//                     }



//                     try (FileOutputStream out_stream = new FileOutputStream(out_filepath)) {
//                         temp_mask.compress(Bitmap.CompressFormat.PNG, 100, out_stream);
//                     } catch (FileNotFoundException e) {
//                         e.printStackTrace();
//                     } catch (IOException e) {
//                         e.printStackTrace();
//                     }
//                 }

//                 try (FileOutputStream out_stream = new FileOutputStream(out_filepath_txt)) {
//                     out_stream.write(txt.getBytes());
//                 } catch (FileNotFoundException e) {
//                     e.printStackTrace();
//                 } catch (IOException e) {
//                     e.printStackTrace();
//                 }
//                 // Update progress
//                 progress++;
//                 publishProgress(progress * 100 / n_images);
//                 //update_progress_text(image_path.split(image_folder_name+"/")[1], progress, n_images);

//             }
            
               
//            return null;
//         }

//          private String get_results_folder_name(String folder_path) {
// //            return folder_path+"_results";
//             String result_folder_path;
            
//             result_folder_path = "/sdcard/results_UOD/Image";
            
//             result_folder_path += folder_path.split("/storage/emulated/0/")[1];

//             /*SimpleDateFormat sdf1 = new SimpleDateFormat("yyyy.MM.dd.HH.mm.ss");
//             Timestamp timestamp = new Timestamp(System.currentTimeMillis());
//             String currentTimeStamp = sdf1.format(timestamp);

//             result_folder_path += "_"+currentTimeStamp;*/

//             return result_folder_path;
//         }

//         private boolean isNFacesdiretoryExists(){
//             String NfacesDirPath = "storage/emulated/0/AFS_NFaces";
//             return new File(NfacesDirPath).exists();
//         }

//         private String getImageName(String imagePath){
//             File imageFile = new File(imagePath);
//             String imageName = imageFile.getName();
//             return imageName;
//         }

//         private int getNFaceCountFromTxtFile(String nFaceTxtFile){
//             String nFaceTextFilePath = "storage/emulated/0/AFS_NFaces/"+nFaceTxtFile;
//             Log.d("VZ Debug","nFaceText File path : "+nFaceTextFilePath);
//             BufferedReader reader = null;
//             String nFaceCountStr = null;
//             try {
//                 reader = new BufferedReader(new FileReader(nFaceTextFilePath));
//                 nFaceCountStr = reader.readLine();
//                 reader.close();
//             } catch (FileNotFoundException e) {
//                 e.printStackTrace();
//             } catch (IOException e) {
//                 e.printStackTrace();
//             }
//             int nFaceCount = Integer.parseInt(nFaceCountStr);
//             Log.d("VZ Debug", "NFaceCount : "+nFaceCount);
//             return nFaceCount;
//         }

//         @Override
//         protected void onPostExecute(Integer integer) {
//             super.onPostExecute(integer);
//             stopTesting();
//             isTestRunning = false;
//             if(progress != 0) {
//                 Toast.makeText(mContext, "Test Completed", Toast.LENGTH_SHORT).show();
//                 progress_image.setImageBitmap(progressbar_bitmap_full);
//             }
//             progress_text.setText("Results are stored in \""+result_folder_name+"\"");
//         }

//         @Override
//         protected void onProgressUpdate(Integer... values) {
//             super.onProgressUpdate(values);
// //            int pro = progress*progressbar_size/100;
//             int pro = values[0] * progressbar_size / 100;
//             int px[] = new int[progressbar_size *(pro - previous_progress)];
//             progressbar_bitmap_full.getPixels(px,0, progressbar_size, 0, progressbar_size -pro, progressbar_size, pro- previous_progress);
//             progressbar_bitmap.setPixels(px, 0, progressbar_size, 0, progressbar_size -pro, progressbar_size, pro- previous_progress);
//             progressbar_bitmap = Utils.overlay(progressbar_bitmap_boundary, progressbar_bitmap);
//             progress_image.setImageBitmap(progressbar_bitmap);
//             previous_progress = pro;
//         }
//     }

// }


package vizinsight.atl.sceneoptimizerpro;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.net.Uri;
import android.text.Html;
import android.util.Log;
import android.view.Menu;
import android.graphics.Rect;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import android.content.pm.ActivityInfo;
import android.content.Context;
import android.provider.DocumentsContract;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.io.FilenameFilter;
import java.util.HashMap;

import androidx.appcompat.app.AppCompatActivity;

import lib.folderpicker.FolderPicker;

import vizinsight.atl.image_segmenter.VZEntity;
import vizinsight.atl.image_segmenter.VZImageSegmenter;

public class BatchTestActivity extends AppCompatActivity {
     private String TAG = "UOD_BatchTestActivity";
    Context mContext;
    boolean DISPLAY_BOX=false;
    TextView directoryText;
    ImageButton select_button;
    ImageButton play_stop_button;
    ImageView progress_image;
    TextView progress_text;

    String image_folder = null;
    boolean isTestRunning = false;

    TestRunner test_runner;
    VZImageSegmenter object_detector;
    Bitmap mask;
    final int IMAGEFORMAT_NV21 = 0;
    final int IMAGEFORMAT_BGR  = 1;
    final int IMAGEFORMAT_RGBA = 2;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
//        segmenterMode = VZImageSegmenter.MODE_SALIENCY;
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);


        object_detector = VZGlobals.getSegmenter(VZImageSegmenter.MODE_OD, "/vendor/etc/saiv/image_understanding/db/unified_detector/model.info");

        super.onCreate(savedInstanceState);
        /*super.setInfo("Select the folder with images\n\n" +
                "After processing the results are generated in '/sdcard/results/' directory\n\n" +
                "Batch Testing is not supported in Live-Focus mode");*/

        if (Build.VERSION.SDK_INT < 16) {
            getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                    WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }
        View decorView = getWindow().getDecorView();
        int uiOptions = View.SYSTEM_UI_FLAG_FULLSCREEN;
        decorView.setSystemUiVisibility(uiOptions);
        setContentView(R.layout.activity_batch_test);
        setTitle("Batch Test");

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
        /*boolean val = super.onCreateOptionsMenu(menu);
        menu.findItem(R.id.style_select).setEnabled(false);
        return val;*/
        return true;
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
    //private static final int REQUEST_CODE_FOLDER_PICKER = 101;
    void select_folder(){
        Intent intent = new Intent(this, FolderPicker.class);
        startActivityForResult(intent, SELECT_FOLDER_REQUEST_CODE);
        /*Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        startActivityForResult(intent, REQUEST_CODE_FOLDER_PICKER);*/
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if(requestCode == SELECT_FOLDER_REQUEST_CODE && resultCode != 0){
            image_folder = data.getExtras().getString("data");
            directoryText.setText(Html.fromHtml("Image Directory: <u>"+image_folder+"</u>"));
            play_stop_button.setVisibility(View.VISIBLE);
        }

        /*if (requestCode == REQUEST_CODE_FOLDER_PICKER && resultCode == RESULT_OK) {
            try{
            Uri treeUri = data.getData();
            Log.d("VZ Debug", "Selected Path:  "+treeUri);
            image_folder = Utils.getFilePath(this,treeUri);
            Log.d("VZ Debug", "Selected Path:  "+image_folder);
            directoryText.setText(Html.fromHtml("Image Directory: <u>"+image_folder+"</u>"));
            play_stop_button.setVisibility(View.VISIBLE);
            } catch (URISyntaxException e) {
                e.printStackTrace();
            }
        }*/
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

        private void update_progress_text(final String image_path, final int abs_progress, final int total_count){
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if(isTestRunning) {
                        progress_text.setText("(" + abs_progress + "/" + total_count + "): " + image_path);
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

        private int naturalPathCompare(String left, String right){
            int i = 0;
            int j = 0;
            while(i < left.length() && j < right.length()){
                char c1 = left.charAt(i);
                char c2 = right.charAt(j);
                if(Character.isDigit(c1) && Character.isDigit(c2)){
                    int start1 = i;
                    int start2 = j;
                    while(start1 < left.length() && left.charAt(start1) == '0') start1++;
                    while(start2 < right.length() && right.charAt(start2) == '0') start2++;
                    int end1 = start1;
                    int end2 = start2;
                    while(end1 < left.length() && Character.isDigit(left.charAt(end1))) end1++;
                    while(end2 < right.length() && Character.isDigit(right.charAt(end2))) end2++;
                    int len1 = end1 - start1;
                    int len2 = end2 - start2;
                    if(len1 != len2) return len1 - len2;
                    for(int k = 0; k < len1; k++){
                        int diff = left.charAt(start1 + k) - right.charAt(start2 + k);
                        if(diff != 0) return diff;
                    }
                    int zeroDiff = (start1 - i) - (start2 - j);
                    if(zeroDiff != 0) return zeroDiff;
                    i = end1;
                    j = end2;
                }else{
                    c1 = Character.toLowerCase(c1);
                    c2 = Character.toLowerCase(c2);
                    if(c1 != c2) return c1 - c2;
                    i++;
                    j++;
                }
            }
            return left.length() - right.length();
        }

        @Override
        protected Integer doInBackground(String... strings) {
            if(object_detector == null || !object_detector.isValid){
                return null;
            }
            String image_folder_name = strings[0];
            Log.d(TAG,"doInBackground image_folder_name : "+image_folder_name);

            // Make result folder
            result_folder_name = get_results_folder_name(image_folder_name);
            Log.d(TAG,"doInBackground : result_folder_name : "+result_folder_name);
            File dir = new File(result_folder_name);
            if(dir.exists()){
                dir.delete();
            }
            dir.mkdirs();
            showToast("Results will be stored in \""+result_folder_name+"\" directory");

            // List images in the folder
            ArrayList<File> image_filepaths = new ArrayList<>();
            Utils.list_files(image_folder_name, image_filepaths);
            int n_images = image_filepaths.size();
            Collections.sort(image_filepaths, new java.util.Comparator<File>() {
                @Override
                public int compare(File left, File right) {
                    return naturalPathCompare(left.getAbsolutePath(), right.getAbsolutePath());
                }
            });

            Log.v(TAG, "Starting test on "+n_images+" images");

            // Execute on images
            for(int i = 0; i < n_images && !isCancelled(); i++){
                String image_path;
                    image_path = image_filepaths.get(i).toString();
                String image_name = ""; //modification
                    image_name = image_path.substring(image_path.lastIndexOf('/')+1,image_path.length()); //Changed to read any folder name  
                    Log.d(TAG, "( " + i + " /  " + n_images + "  ) " + image_path );
                    Log.d(TAG, "image_name : "+image_name);
                if(!Utils.isValidImagePath(image_path)){
                    continue;
                }
                List<VZEntity> entities = null;
                try{
                    entities = object_detector.execute(image_path,IMAGEFORMAT_NV21,image_name); //modified for raw dumps

                }catch(OutOfMemoryError e){
                    showToast(image_path+" size is large. So ignoring the image to proess");
                }
                int width;
                // Save results in results folder
                HashMap<String,Integer> classes=new HashMap<String, Integer>();
                classes.put("object",1);
                classes.put("person",2);
                classes.put("food",3);
                classes.put("vehicle",4);
                classes.put("flower",5);
                classes.put("Animal",6);
                classes.put("drink",7);
                classes.put("petface",8);
                classes.put("moon",9);
                classes.put("human_head",10);
                classes.put("pets",11);
                classes.put("text",12);
                classes.put("scene_text",13);
                classes.put("Wine",15);
                classes.put("wine_label",16);
                
                String out_filepath = image_path.replace(image_folder_name, result_folder_name);
                String out_filepath_txt ;
                if(out_filepath.contains(".JPEG")){
                    out_filepath_txt = (out_filepath.split("[.]JP"))[0] + ".txt";
                }
                else if(out_filepath.contains(".jpg")){
                    out_filepath_txt = (out_filepath.split("[.]jp"))[0] + ".txt";
                }
                else if(out_filepath.contains(".jpeg")){
                    out_filepath_txt = (out_filepath.split("[.]jp"))[0] + ".txt";
                }
                else if(out_filepath.contains(".png")){
                    out_filepath_txt = (out_filepath.split("[.]pn"))[0] + ".txt";
                }
                else if(out_filepath.contains(".PNG")){
                    out_filepath_txt = (out_filepath.split("[.]PN"))[0] + ".txt";
                }
                else if(out_filepath.contains(".bmp")){
                    out_filepath_txt = (out_filepath.split("[.]bm"))[0] + ".txt";
                }
                else if(out_filepath.contains(".BMP")){
                    out_filepath_txt = (out_filepath.split("[.]BM"))[0] + ".txt";
                }
                else{
                    out_filepath_txt=out_filepath+".txt";
                }
                String txt = "";

                if(!(entities== null) && !entities.isEmpty()) {
                    Bitmap bm = BitmapFactory.decodeFile(image_path);
                    Bitmap temp_mask;


                    bm = Utils.rotateBitmap(bm, Utils.getImageOrientation(image_path));
                    temp_mask = bm.copy(Bitmap.Config.ARGB_8888,true);
                    width=bm.getWidth();
                    
                    Paint p = new Paint();
                    p.setStyle(Paint.Style.STROKE);
                    p.setAntiAlias(true);
                    p.setFilterBitmap(true);
                    p.setDither(true);
                    p.setColor(Color.BLUE);
                    p.setTextSize(width/25);

                    Rect text_bounds = new Rect();
                    Paint paint_box = new Paint();
                    paint_box.setStyle(Paint.Style.FILL);
                    paint_box.setAntiAlias(true);
                    paint_box.setColor(Color.WHITE); //Color.BLUE
                    paint_box.setStrokeWidth(width/320.0f);
                    Paint paint_text_box = new Paint(paint_box);
                    paint_text_box.setStyle(Paint.Style.FILL);
                    paint_text_box.setTextSize(width/25);
                    paint_box.setTextSize(width/25);
                    
                    Canvas c = new Canvas(temp_mask);
                    String[] lines = new String[10];
                    for (int idx = 0; idx < entities.size(); idx++) {
                        VZEntity ent = entities.get(idx);
                        if(ent.left == 0 && ent.top == 0 && ent.right==0 &&ent.bottom ==0){
                            /*p.setStyle(Paint.Style.STROKE);
                            p.setColor(Color.GREEN);
                            p.setTextSize(40);*/
                            String tempString;
                            float percentage_score = ent.score;
                            String formattedPercentage = String.format("%.1f%%", percentage_score);
                            Log.d("VZ Debug", ent.tag + " : "+ formattedPercentage);
                            tempString = ent.tag + " : "+formattedPercentage;
                            lines[idx] = tempString;
                            
                        }else{
                            if(ent.tag.contains("et")){
                                p.setColor(Color.RED);
                                paint_text_box.setColor(Color.WHITE);
                                paint_box.setColor(Color.RED);
                            }
                            else{
                                p.setColor(Color.BLUE);
                                paint_text_box.setColor(Color.TRANSPARENT);
                                paint_box.setColor(Color.BLUE);
                            }
                            
                            Log.d(TAG,"Drawing the box for Images.");
                            c.drawRect(ent.left, ent.top, ent.right, ent.bottom, p);
                            paint_text_box.getTextBounds(ent.trackId+"_"+ent.tag+"_"+(int)(ent.score*100), 0, (ent.associationId+"_"+ent.tag+"_"+(int)(ent.score*100)).length(), text_bounds);
                            c.drawRect(ent.left, ent.top, ent.left + text_bounds.width(), ent.top - text_bounds.height(), paint_text_box);

                            c.drawText(ent.trackId+"_"+ent.tag + "_" + (int)(ent.score*100), ent.left, ent.top, paint_box);
                        }
                        txt = txt + ent.trackId + " ";
                        txt = txt + ent.tagId + " ";
                        txt = txt + ent.score + " ";
                        txt = txt + ent.left + " ";
                        txt = txt + ent.top + " ";
                        txt = txt + ent.right+ " ";
                        txt = txt + ent.bottom + "\n";
                        
                    }
                    int y = 0;
                    for (int idx = 0; idx < entities.size(); idx++) {
                        VZEntity ent = entities.get(idx);
                        if(ent.left == 0 && ent.top == 0 && ent.right==0 &&ent.bottom ==0){
                            Log.d("VZ Debug", "line : "+lines[idx]);
                            p.setColor(Color.GREEN);
                            p.setStyle(Paint.Style.FILL);
                            y += p.getTextSize();
                            c.drawText(lines[idx], 0, y, p);
                        }
                    }



                    try (FileOutputStream out_stream = new FileOutputStream(out_filepath)) {
                        temp_mask.compress(Bitmap.CompressFormat.PNG, 100, out_stream);
                    } catch (FileNotFoundException e) {
                        e.printStackTrace();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }

                try (FileOutputStream out_stream = new FileOutputStream(out_filepath_txt)) {
                    out_stream.write(txt.getBytes());
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                // Update progress
                progress++;
                publishProgress(progress * 100 / n_images);
                //update_progress_text(image_path.split(image_folder_name+"/")[1], progress, n_images);

            }
            
               
           return null;
        }

         private String get_results_folder_name(String folder_path) {
//            return folder_path+"_results";
            String result_folder_path;
            
            result_folder_path = "/sdcard/results_UOD/Image";
            
            result_folder_path += folder_path.split("/storage/emulated/0/")[1];

            /*SimpleDateFormat sdf1 = new SimpleDateFormat("yyyy.MM.dd.HH.mm.ss");
            Timestamp timestamp = new Timestamp(System.currentTimeMillis());
            String currentTimeStamp = sdf1.format(timestamp);

            result_folder_path += "_"+currentTimeStamp;*/

            return result_folder_path;
        }

        private boolean isNFacesdiretoryExists(){
            String NfacesDirPath = "storage/emulated/0/AFS_NFaces";
            return new File(NfacesDirPath).exists();
        }

        private String getImageName(String imagePath){
            File imageFile = new File(imagePath);
            String imageName = imageFile.getName();
            return imageName;
        }

        private int getNFaceCountFromTxtFile(String nFaceTxtFile){
            String nFaceTextFilePath = "storage/emulated/0/AFS_NFaces/"+nFaceTxtFile;
            Log.d("VZ Debug","nFaceText File path : "+nFaceTextFilePath);
            BufferedReader reader = null;
            String nFaceCountStr = null;
            try {
                reader = new BufferedReader(new FileReader(nFaceTextFilePath));
                nFaceCountStr = reader.readLine();
                reader.close();
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }
            int nFaceCount = Integer.parseInt(nFaceCountStr);
            Log.d("VZ Debug", "NFaceCount : "+nFaceCount);
            return nFaceCount;
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
//            int pro = progress*progressbar_size/100;
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


