package vizinsight.atl.unifieddetector;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;

import java.io.File;
import java.nio.ByteBuffer;
import java.util.ArrayList;

@SuppressLint("UnsafeDynamicallyLoadedCode")
public class UnifiedDetector {
    private static final String TAG = "UnifiedDetector_1.0.0";
    private static final int MODE_OD   = 0;
    private static final int MODE_CLS   = 1;
    private static final int MODE_OD_AND_CLS   = 2;
    private static final int IMAGE_FORMAT_NV21 = 0;
    private static final int IMAGE_FORMAT_BGR  = 1;
    private static final int IMAGE_FORMAT_RGBA = 2;
    private static final int IMAGE_FORMAT_NV12 = 3;
    static{
        try {
            Log.i(TAG, "Loading library");
            System.load("/system/lib64/libUnifiedDetector.camera.samsung.so");
            Log.i(TAG, "Successfully loaded the libUnifiedDetector.camera.samsung.so library");
        }catch (Exception e) {
            Log.e(TAG, "Failed to loadLibrary libUnifiedDetector.camera.samsung.so : " + e);
            e.printStackTrace();

        }
    }
    private native boolean initializeJNI(int mode,String modelPath);
    private native boolean executeJNI(ByteBuffer imageByteBuffer, int width, int height, int resizedWidth, int resizedHeight, int angularRotation, int colorFormat,int mode);
    private native String getagInfosJNI(int mode);
    private native boolean executeWithByteArrayJNI(byte[] bgrByteArray, int width, int height, int angularRotation, int colorFormat, int mode);

    private native boolean releaseJNI(int mode);
    private native boolean isPackageAllowedJNI(int mode, String packageName);
    private long od_engine_reference;
    private long system_engine_reference;
    private int mMode;
    private String mModelPath = "/vendor/etc/saiv/image_understanding/db/unified_detector/model.info";
    InitCallBack mInitCallBack;
    public interface VZStatusChangeListener {
        void onInitComplete(boolean isSuccess);
    }
    private VZStatusChangeListener statusChangeListener = null;

    private void callback(int status){ //called from jni
        int a=1;
        if(statusChangeListener != null)
            statusChangeListener.onInitComplete(status==1);
    }

    private ByteBuffer image_byte_buffer;

    public UnifiedDetector(){

    }
    public void setVZStatusChangeListener(VZStatusChangeListener listener){
        this.statusChangeListener = listener;
    }
    public void setModelInitListener(InitCallBack initCallBack){
        mInitCallBack= initCallBack;
    }
    public void unifiedDetectorModelInit(Context context, int mode){
        mMode = mode;
        //if(!isPackageAllowedJNI(mMode, context.getPackageName())){
          //  throw new SecurityException("This library can only be used in the allowed app");
        //}*/
        this.setVZStatusChangeListener(new VZStatusChangeListener() {
            @Override
            public void onInitComplete(boolean isSuccess) {
                final boolean is_success = isSuccess;

                if (is_success) {
                    Log.v(TAG,"Initialization of Unified detector models completed");
                } else
                    Log.v(TAG,"Initialization of Unified detector models failed");
                mInitCallBack.initCompleteCallBackReceived(is_success);
            }});
        boolean initResult = initializeJNI(mMode, mModelPath);
        mInitCallBack.initCompleteCallBackReceived(initResult);
    }

    public ArrayList<DetectorBoxInfo> getDetectorBoxInfo(byte[] bgrByteArray, int width, int height, int angularRotation, int colorFormat){
        Log.d(TAG,"getBoxInfo byteArray");
        //boolean execute_status = false;
        boolean execute_status = executeWithByteArrayJNI(bgrByteArray, width, height, angularRotation,colorFormat,mMode);
        ArrayList<DetectorBoxInfo> entities= getBoxEntities(execute_status);
        dumpBoxInfo(entities);
        return entities;
    }

    public void releaseUnifiedDetector(){
        Log.d(TAG,"releaseUnifiedDetector");
        releaseJNI(mMode);
    }

    private void dumpBoxInfo(ArrayList<DetectorBoxInfo> entities){
        if(entities != null) {
            for (int i = 0; i < entities.size(); i++) {
                DetectorBoxInfo boxInfo = entities.get(i);
                Log.d(TAG, "boxInfo : tag : " + boxInfo.tag + " tag ID : " + +boxInfo.tagId+ " score : " + boxInfo.score
                        + " left : " + boxInfo.left + " top : " + boxInfo.top + " right : " + boxInfo.right + " bottom : " + boxInfo.bottom );
            }
        }
    }

    private ArrayList<DetectorBoxInfo> getBoxEntities(boolean execute_status) {
        if(!execute_status){
            Log.d(TAG,"getBoxEntities execute_status :"+execute_status);
            return null;
        }
        String result_info_json_str = getagInfosJNI(mMode);
        if(result_info_json_str.isEmpty()){
            return null;
        }

        return Utils.convert_result_info_to_box_entities(result_info_json_str );
    }
}
