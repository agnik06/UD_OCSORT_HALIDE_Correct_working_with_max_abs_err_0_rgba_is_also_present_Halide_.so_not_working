package vizinsight.atl.sceneoptimizerpro;

import vizinsight.atl.image_segmenter.VZImageSegmenter;

public class VZGlobals {
    private static VZImageSegmenter segmenter;
    public static VZImageSegmenter getSegmenter(int mode, String baseModelPath){
        if(segmenter == null){
            segmenter = new VZImageSegmenter(mode, baseModelPath);
        }
        return segmenter;
    }

    public static void releaseSegmenter(){
        segmenter.release();
        segmenter = null;
    }
}
