package vizinsight.atl.sceneoptimizerpro;

import android.app.Activity;
import android.content.ContentUris;
import android.content.Context;
import android.content.pm.PackageManager;
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
import android.webkit.MimeTypeMap;

import androidx.core.app.ActivityCompat;
import java.io.File;
import java.io.IOException;
import java.net.URISyntaxException;
import java.util.ArrayList;
import android.content.Context;
import java.io.FilenameFilter;


/**
 * Created by batman on 28/12/17.
 */

public class Utils {

    public static String getFilePath(Context context, Uri uri) throws URISyntaxException {
        String selection = null;
        String[] selectionArgs = null;
        // Uri is different in versions after KITKAT (Android 4.4), we need to
        if (Build.VERSION.SDK_INT >= 19 && DocumentsContract.isDocumentUri(context.getApplicationContext(), uri)) {
            if (isExternalStorageDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                return Environment.getExternalStorageDirectory() + "/" + split[1];
            } else if (isDownloadsDocument(uri)) {
                final String id = DocumentsContract.getDocumentId(uri);
                uri = ContentUris.withAppendedId(
                        Uri.parse("content://downloads/public_downloads"), Long.valueOf(id));
            } else if (isMediaDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];
                if ("image".equals(type)) {
                    uri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
                } else if ("video".equals(type)) {
                    uri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
                } else if ("audio".equals(type)) {
                    uri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
                }
                selection = "_id=?";
                selectionArgs = new String[]{
                        split[1]
                };
            }
        }
        if ("content".equalsIgnoreCase(uri.getScheme())) {
            String[] projection = {
                    MediaStore.Images.Media.DATA
            };
            Cursor cursor = null;
            try {
                cursor = context.getContentResolver()
                        .query(uri, projection, selection, selectionArgs, null);
                int column_index = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
                if (cursor.moveToFirst()) {
                    return cursor.getString(column_index);
                }
            } catch (Exception e) {
            }
        } else if ("file".equalsIgnoreCase(uri.getScheme())) {
            return uri.getPath();
        }
        return null;
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

    public static Bitmap rotateBitmap(Bitmap bitmap, int orientation) {

        Matrix matrix = new Matrix();
        switch (orientation) {
            case ExifInterface.ORIENTATION_NORMAL:
                return bitmap;
            case ExifInterface.ORIENTATION_FLIP_HORIZONTAL:
                matrix.setScale(-1, 1);
                break;
            case ExifInterface.ORIENTATION_ROTATE_180:
                matrix.setRotate(180);
                break;
            case ExifInterface.ORIENTATION_FLIP_VERTICAL:
                matrix.setRotate(180);
                matrix.postScale(-1, 1);
                break;
            case ExifInterface.ORIENTATION_TRANSPOSE:
                matrix.setRotate(90);
                matrix.postScale(-1, 1);
                break;
            case ExifInterface.ORIENTATION_ROTATE_90:
                matrix.setRotate(90);
                break;
            case ExifInterface.ORIENTATION_TRANSVERSE:
                matrix.setRotate(-90);
                matrix.postScale(-1, 1);
                break;
            case ExifInterface.ORIENTATION_ROTATE_270:
                matrix.setRotate(-90);
                break;
            default:
                return bitmap;
        }
        try {
            Bitmap bmRotated = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
            bitmap.recycle();
            return bmRotated;
        }
        catch (OutOfMemoryError e) {
            e.printStackTrace();
            return null;
        }
    }

    public static int getImageOrientation(String path){
        ExifInterface exif = null;
        try {
            exif = new ExifInterface(path);
        } catch (IOException e) {
            e.printStackTrace();
        }
        int orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION,
                ExifInterface.ORIENTATION_UNDEFINED);
        return orientation;
    }

    public static boolean verifyPermissions(Context context, String[] permission_list, int multiple_permissions) {
        int permission_status = 1;
        for(String permission: permission_list) {
            permission_status = ActivityCompat.checkSelfPermission(context, permission);
            if (permission_status != PackageManager.PERMISSION_GRANTED) {
                break;
            }
        }
        if(permission_status!=PackageManager.PERMISSION_GRANTED){
            ActivityCompat.requestPermissions(
                    (Activity) context,
                    permission_list,
                    multiple_permissions
            );
            return false;
        }
        return true;
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
            bm = Bitmap.createBitmap(bm, 0, 0, bm.getWidth(), bm.getHeight(), matrix, false);

        } catch (Exception e) {
            e.printStackTrace();
        }
        return bm;
    }

    public static void list_files(String directoryName, ArrayList<File> files) {
        File directory = new File(directoryName);

        // get all the files from a directory
        File[] fList = directory.listFiles();
        if(fList == null || fList.length == 0)
            return;
        for (File file : fList) {
            if (file.isFile()) {
                files.add(file);
            } else if (file.isDirectory()) {
                list_files(file.getAbsolutePath(), files);
            }
        }
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

   /* public static String getDirectoryPathFromTreeUri(Context context, Uri treeUri) {
        DocumentFile pickedDir = DocumentFile.fromTreeUri(context, treeUri);
        return pickedDir.getUri().getPath();
    }*/

    public static int countImages(String inputDir) {
        File inputDirectory = new File(inputDir);
        int imageCount = 0;

        // Traverse the input directory and count each image
        for (File categoryDir : inputDirectory.listFiles(new FilenameFilter() {
            @Override
            public boolean accept(File dir, String name) {
                return new File(dir, name).isDirectory();
            }
        })) {
            for (File levelDir : categoryDir.listFiles(new FilenameFilter() {
                @Override
                public boolean accept(File dir, String name) {
                    return new File(dir, name).isDirectory();
                }
            })) {
                for (File imageFile : levelDir.listFiles(new FilenameFilter() {
                    @Override
                    public boolean accept(File dir, String name) {
                        return name.endsWith(".jpg") || name.endsWith(".png")
                                || name.endsWith(".JPG") || name.endsWith(".PNG")
                                || name.endsWith(".JPEG") || name.endsWith(".jpeg");
                    }
                })) {
                    imageCount++;
                }
            }
        }

        return imageCount;
    }
}
