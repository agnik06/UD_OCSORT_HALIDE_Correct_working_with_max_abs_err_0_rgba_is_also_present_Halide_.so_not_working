package vizinsight.atl.myapplication;

import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.media.ExifInterface;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.util.Log;
import android.webkit.MimeTypeMap;

import java.io.File;
import java.net.URISyntaxException;
import java.util.ArrayList;

public class Utils {

    public static String getFilePath(Context context, Uri uri) throws URISyntaxException {
        String selection = null;
        String[] selectionArgs = null;
        String[] split;
        if (Build.VERSION.SDK_INT >= 19 && DocumentsContract.isDocumentUri(context.getApplicationContext(), uri)) {
            String docId;
            if (isExternalStorageDocument(uri)) {
                docId = DocumentsContract.getDocumentId(uri);
                split = docId.split(":");
                return Environment.getExternalStorageDirectory() + "/" + split[1];
            }

            if (isDownloadsDocument(uri)) {
                docId = DocumentsContract.getDocumentId(uri);
                uri = ContentUris.withAppendedId(Uri.parse("content://downloads/public_downloads"), Long.valueOf(docId));
            } else if (isMediaDocument(uri)) {
                docId = DocumentsContract.getDocumentId(uri);
                split = docId.split(":");
                String type = split[0];
                if ("image".equals(type)) {
                    uri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
                } else if ("video".equals(type)) {
                    uri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
                } else if ("audio".equals(type)) {
                    uri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
                }

                selection = "_id=?";
                selectionArgs = new String[]{split[1]};
            }
        }

        if ("content".equalsIgnoreCase(uri.getScheme())) {
            String[] projection = new String[]{"_data"};
            split = null;

            try {
                Cursor cursor = context.getContentResolver().query(uri, projection, selection, selectionArgs, (String)null);
                int column_index = cursor.getColumnIndexOrThrow("_data");
                if (cursor.moveToFirst()) {
                    return cursor.getString(column_index);
                }
            } catch (Exception var7) {
            }
        } else if ("file".equalsIgnoreCase(uri.getScheme())) {
            return uri.getPath();
        }

        return null;
    }

    public static String getPathFromUri(Context context, Uri uri) {
        String[] projection = { "_data" };
        Cursor cursor = context.getContentResolver().query(uri, projection, null, null, null);
        if (cursor != null) {
            int column_index = cursor.getColumnIndexOrThrow("_data");
            cursor.moveToFirst();
            return cursor.getString(column_index);
        } else {
            return null;
        }
    }

    public static boolean isExternalStorageDocument(Uri uri) {
        return "com.android.externalstorage.documents".equals(uri.getAuthority());
    }

    public static boolean isDownloadsDocument(Uri uri) {
        return "com.android.providers.downloads.documents".equals(uri.getAuthority());
    }

    public static boolean isMediaDocument(Uri uri) {
        return "com.android.providers.media.documents".equals(uri.getAuthority());
    }

    public static Bitmap get_exif_corrected_bitmap(String image_path){
        Bitmap bm = BitmapFactory.decodeFile(image_path);
        try {
            ExifInterface exif = new ExifInterface(image_path);
            int rotation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_NORMAL);
            Matrix matrix = new Matrix();
            switch(rotation){
                case ExifInterface.ORIENTATION_ROTATE_90:
                    matrix.preRotate(90);
                    break;
                case ExifInterface.ORIENTATION_ROTATE_180:
                    matrix.preRotate(180);
                    break;
                case ExifInterface.ORIENTATION_ROTATE_270:
                    matrix.preRotate(270);
                    break;
            }
            if(bm != null) {
                bm = Bitmap.createBitmap(bm, 0, 0, bm.getWidth(), bm.getHeight(), matrix, false);
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
        return bm;
    }

    static byte [] getRGBA(int inputWidth, int inputHeight, Bitmap bitmap) throws OutOfMemoryError
    {
        int [] data = new int[inputWidth * inputHeight];
        byte [] array = new byte[4* inputWidth * inputHeight];
        bitmap.getPixels(data, 0, inputWidth, 0, 0, inputWidth, inputHeight);

        //Do the conversion
        int count = 0;
        int index = 0;
        int a, R, G, B;
        for (int j = 0; j < inputHeight; j++) {
            for (int i = 0; i < inputWidth; i++) {

                a = (data[index] & 0xff000000) >> 24; // a is not used obviously
                R = (data[index] & 0xff0000) >> 16;
                G = (data[index] & 0xff00) >> 8;
                B = (data[index] & 0xff);

                array[count++] = (byte) ((R < 0) ? 0 : ((R > 255) ? 255 : R));
                array[count++] = (byte) ((G < 0) ? 0 : ((G > 255) ? 255 : G));
                array[count++] = (byte) ((B < 0) ? 0 : ((B > 255) ? 255 : B));
                array[count++] = (byte) ((a < 0) ? 0 : ((a > 255) ? 255 : a));

                index++;
            }
        }

        return  array;
    }



        public static Bitmap resizeBitmap(Bitmap originalBitmap, int newWidth, int newHeight) {
            int width = originalBitmap.getWidth();
            int height = originalBitmap.getHeight();
            float scaleWidth = ((float) newWidth) / width;
            float scaleHeight = ((float) newHeight) / height;

            // Create a new Bitmap with the desired dimensions
            Bitmap resizedBitmap = Bitmap.createBitmap(newWidth, newHeight, originalBitmap.getConfig());

            // Create a Canvas to draw the original Bitmap onto the new Bitmap
            Canvas canvas = new Canvas(resizedBitmap);
            Matrix matrix = new Matrix();
            matrix.postScale(scaleWidth, scaleHeight);

            // Draw the original Bitmap onto the new Bitmap, scaled appropriately
            canvas.drawBitmap(originalBitmap, matrix, null);

            return resizedBitmap;
        }

    public static void list_files(String directoryName, ArrayList<File> files) {
        File directory = new File(directoryName);

        // get all the files from a directory
        File[] fList = directory.listFiles();
        if(fList == null || fList.length == 0)
            return;
        for (File file : fList) {
            if (file.isFile() && isImageFile(file) && !isFileNameTooLarge(file)) {
                files.add(file);
            } else if (file.isDirectory()) {
                list_files(file.getAbsolutePath(), files);
            }
        }
    }

    public static boolean isFileNameTooLarge(File file){
        String fileName = file.getName();
        int fieNameLength = fileName.length();
        Log.d("Utils:","isImageFile fileName : "+fileName+"  fieNameLength : "+fieNameLength);

        if(fieNameLength > 1024){
            return true;
        }
        return false;
    }

    public static boolean isImageFile(File file){
        String fileName = file.getName();
        if(fileName.contains(".JPEG") || fileName.contains(".jpg") || fileName.contains(".JPG") ||fileName.contains(".jpeg")
                || fileName.contains(".png") || fileName.contains(".PNG") || fileName.contains(".bmp") || fileName.contains(".BMP")){
            return true;
        }
        return false;
    }

    public static boolean isValidImagePath(String filepath){
        if(filepath != null && filepath.lastIndexOf(".") != -1){
            String ext = filepath.substring(filepath.lastIndexOf(".")+1);
            String mime_type = MimeTypeMap.getSingleton().getMimeTypeFromExtension(ext);
            String type = mime_type.split("/")[0];
            return type.equals("image");
        }
        return false;
    }

    public static Bitmap overlay(Bitmap b1, Bitmap b2){
        Bitmap overlayedBitmap = Bitmap.createBitmap(b1.getWidth(), b1.getHeight(), b1.getConfig());
        Canvas canvas = new Canvas(overlayedBitmap);
        canvas.drawBitmap(b1, new Matrix(), null);
        canvas.drawBitmap(b2, new Matrix(), null);
        return overlayedBitmap;
    }
}
