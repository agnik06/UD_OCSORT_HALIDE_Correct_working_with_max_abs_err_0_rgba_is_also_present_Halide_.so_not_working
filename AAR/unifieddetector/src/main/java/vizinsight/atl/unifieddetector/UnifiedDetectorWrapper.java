package vizinsight.atl.unifieddetector;

import android.util.Log;

import java.io.File;

public class UnifiedDetectorWrapper {
    private static final String TAG = "UnifiedDetector";

    public static UnifiedDetector getUnifiedDetector(){
        Log.d(TAG, "AAR/JAR version : "+VersionInfo.VERSION_NAME);
        return new UnifiedDetector();
    }

    public static boolean isUnifiedAvailable(){
        File soFile = new File("/vendor/lib64/libUnifiedDetector.camera.samsung.so");
        boolean exists = soFile.exists();
        Log.d(TAG,"isUnifiedDetectorAvailable : " +exists);
        return exists;
    }

}
