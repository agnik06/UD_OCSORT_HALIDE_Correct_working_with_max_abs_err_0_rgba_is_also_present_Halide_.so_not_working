package vizinsight.atl.unifieddetector;
public class DetectorBoxInfo {
    public String tag;
    public int tagId;
    public int top, left, bottom, right;
    public float score;


    public DetectorBoxInfo(String tag, int tagId, int top, int left, int bottom, int right,float score) {
        this.tag = tag;
        this.tagId = tagId;
        this.top = top;
        this.left = left;
        this.bottom = bottom;
        this.right = right;
        this.score = ((float)(int)(score*100))/100;
    }
}
