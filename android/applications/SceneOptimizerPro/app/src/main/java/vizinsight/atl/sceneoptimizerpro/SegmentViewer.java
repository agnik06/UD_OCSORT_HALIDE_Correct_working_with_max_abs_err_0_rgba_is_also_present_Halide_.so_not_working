package vizinsight.atl.sceneoptimizerpro;

import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Rect;
//import android.support.annotation.ColorRes;
//import android.support.annotation.NonNull;

import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.widget.ImageView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import vizinsight.atl.image_segmenter.VZEntity;
import vizinsight.atl.image_segmenter.VZImageSegmenter;

public class SegmentViewer extends AppCompatActivity {

    ImageView segmentMaskView;
    //ImageView refreshView;
    Bitmap enhanced_image;
    Bitmap original_image;
    Bitmap mask;
    Bitmap result;
    Map<Integer,Integer> MASK_ON_FLAG=new HashMap<Integer, Integer>() ;
    Rect imgBox=new Rect();

    private View.OnTouchListener handleTouch = new View.OnTouchListener() {
        long prev = 0;
        long current = 0;
        long dif = 0;

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                prev = (long) System.currentTimeMillis();
                Log.v("SemanticMap Debug", "prev = "+prev);
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                current = (long) System.currentTimeMillis();
                Log.v("SemanticMap Debug", "current = "+current);
                dif = current -prev;
                Log.v("CD Debug", "dif = "+dif);
                if (dif > 1000) {
                    Log.v("SemanticMap Debug", "onLongClickListener");
                    segmentMaskView.setImageBitmap(enhanced_image);
                    return false;
                } else {
                    float x = (float) event.getX();
                    float y = (float) event.getY();
//            Toast.makeText(SegmentViewer.this,"points="+x+"y="+y,
//                    Toast.LENGTH_SHORT).show();
//            Toast(x+"+"+y);
                    float img_h = original_image.getHeight();
                    float img_w = original_image.getWidth();
                    float max_dim = img_h;

                    float img_left, img_right;
                    segmentMaskView.getLocalVisibleRect(imgBox);
                    float ht_act, width_act;
                    ht_act = imgBox.height();
                    width_act = (ht_act / img_h) * img_w;

                    Log.v("SemanticMap Debug:", "left=" + imgBox.left);

                    Log.v("SemanticMap Debug:", "top=" + imgBox.top);
                    x = (x - (imgBox.width() - width_act) / 2) / width_act;
                    y = (y - imgBox.top) / ht_act;
                    Log.v("SemanticMap Debug:", "computed x%f and y=%f" + x + " " + y);
                    displayOnTouch((int) (x * img_w), (int) (y * img_h));
                    return true;
                }
            }
            return true;
        }



    };
    private View.OnLongClickListener handleLongClickAction = new View.OnLongClickListener(){

        @Override
        public boolean onLongClick(View view) {
            Log.v("AF_Segmenter_CD Debug","onLongClickListener");
            segmentMaskView.setImageBitmap(enhanced_image);
            return true;
        }
    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getSupportActionBar().hide();
        setContentView(R.layout.activity_segment_viewer);

//        Bitmap inputImage = (Bitmap) getIntent().getParcelableExtra("inputImage");
        String inputImagePath = getIntent().getStringExtra("inputImagePath");
        if(inputImagePath.isEmpty()){
            finish();
            return;
        }




//        Bitmap inputImage = BitmapFactory.decodeFile(inputImagePath);
        Bitmap inputImage = Utils.get_exif_corrected_bitmap(inputImagePath);

        ImageView segmentImageView = (ImageView) findViewById(R.id.segment_image_view);
//        segmentImageView.setImageBitmap(inputImage);
        /*refreshView = (ImageView) findViewById(R.id.refreshBtton);
        refreshView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(original_image != null){
                    segmentMaskView.setImageBitmap(original_image);
                }
            }
        });*/
        segmentMaskView = (ImageView) findViewById(R.id.segment_mask_view);
        segmentMaskView.setOnTouchListener(handleTouch);
        segmentMaskView.setOnLongClickListener(handleLongClickAction);

        VZImageSegmenter segmenter = VZGlobals.getSegmenter(VZImageSegmenter.MODE_OD, "/vendor/etc/saiv/image_understanding/db/sm_segmenter");
        Log.v("AF_Segmenter_VZ Debug"," Segmenting on Image: "+inputImagePath);
        List<VZEntity> entities = segmenter.execute(inputImagePath);
        /*Log.v("AF_Segmenter_CD Debug",entities.get(0).mask.toString());
//        displaySegmentEntities(entities);
        segmentMaskView.setImageResource(android.R.color.transparent);*/
        try {
            File f=new File("/sdcard/", "test_dumped_image.png");
            enhanced_image = BitmapFactory.decodeStream(new FileInputStream(f));
            f=new File("/sdcard/", "original_image.png");
            original_image = BitmapFactory.decodeStream(new FileInputStream(f));
            f=new File("/sdcard/", "test_mask_resized.png");
            mask=BitmapFactory.decodeStream(new FileInputStream(f));
            result=original_image.copy(original_image.getConfig(),true);
            segmentImageView.setImageBitmap(result);

        }
        catch (FileNotFoundException e)
        {
            e.printStackTrace();
        }


    }

    private void displaySegmentEntities(List<VZEntity> entities) {
        if(entities!= null && !entities.isEmpty()){

//            Bitmap mask = entities.get(0).mask;
            segmentMaskView.setImageBitmap(mask);
//            segmentMaskView.setImageBitmap(mask);
        }
        else{
            segmentMaskView.setImageResource(android.R.color.transparent);
        }
    }


    private void displayOnTouch(int x,int y){

        Log.v("AF_Segmenter_CD Debug:","ontouch called"+x+"y="+y);

        int cls=mask.getPixel(x,y);
        if(!MASK_ON_FLAG.containsKey(cls)){
            MASK_ON_FLAG.put(cls,1);
        }else{
            if(MASK_ON_FLAG.get(cls)==1){
                MASK_ON_FLAG.put(cls,0);
            }
            else{
                MASK_ON_FLAG.put(cls,1);
            }
        }
        Log.v("AF_Segmenter_CD Debug:","got cls");

        int h=mask.getHeight();
        int w=mask.getWidth();
        Log.v("AF_Segmenter_CD Debug:",h+" "+w);
//        result=original_image.copy(original_image.getConfig(),true);
//        result.eraseColor(Color.TRANSPARENT);
        for(int i=0;i<h;i++){
            for(int j=0;j<w;j++){
//                Log.v("CD Debug","cls="+cls);
//                Log.v("CD Debug","mask="+mask.getPixel(x,y));
                if(cls==mask.getPixel(j,i)&&(MASK_ON_FLAG.get(cls)==1)) {
                    result.setPixel(j,i,Color.TRANSPARENT);
                    result.setPixel(j, i,enhanced_image.getPixel(j,i));
//                    result.setPixel(j, i,Color.BLACK);
                }
                else if(cls==mask.getPixel(j,i)&&(MASK_ON_FLAG.get(cls)==0)){
                    result.setPixel(j,i,Color.TRANSPARENT);
                    result.setPixel(j, i,original_image.getPixel(j,i));
                }
                else{

                }
            }
        }

//                ImageView img=(ImageView)findViewById(R.id.image);
//                img.setImageBitmap(b);

        Log.v("AF_Segmenter_CD Debug","setting image");
        segmentMaskView.setImageBitmap(result);
        Log.v("AF_Segmenter_CD Debug","image set");

//                ImageView img=(ImageView)findViewById(R.id.image);
//                img.setImageBitmap(b);


    }




    @Override
    public void onBackPressed() {
        super.onBackPressed();
    }
}
