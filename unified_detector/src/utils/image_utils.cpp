/**
 * @file   image_utils.cpp
 * @brief  This file implements the image utils used in unified_detectorinsight
 *
 * This file implements image utils interface used in unified_detectorinsight
 */
#include "image_utils.hpp"
#include "core/logger/logger.hpp"
#include <opencv2/opencv.hpp>


char* halve_nv21_buffer(char* bitmap_buffer, const int image_width, int const image_height) {
    cv::Mat YUV(image_height / 2 + image_height / 4, image_width / 2, CV_8UC1, bitmap_buffer);
    char *yuv =(char*)YUV.data;
    // halve yuma
    int i = 0;
    for (int y = 0; y < image_height; y+=2) {
        for (int x = 0; x < image_width; x+=2) {
            yuv[i] = bitmap_buffer[y * image_width + x];
            i++;
        }
    }
    // halve U and V color components
    for (int y = 0; y < image_height / 2; y+=2) {
        for (int x = 0; x < image_width; x += 4) {
            yuv[i] = bitmap_buffer[(image_width * image_height) + (y * image_width) + x];
            i++;
            yuv[i] = bitmap_buffer[(image_width * image_height) + (y * image_width) + (x + 1)];
            i++;
        }
    }

    return yuv;
}
/**
 * @brief Get bgr mat from image buffer
 * 
 * @param[in] bitmap buffer of image
 * @param[in] width of image
 * @param[in] height of image
 * @param[in] Image format
 * @param[out] target image width
 * @param[out] target image height
 *
 * @return Converted image 
 */
 cv::Mat get_bgr_mat_from_buffer(char* bitmap_buffer, int width, int height, const unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat format,int& target_width,int& target_height){
    cv::Mat image;
    if (format == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat::NV21) {
        LOGV("VZ Debug: Converting image format - NV21");
        if(width*height>350000){
            //LG<<"CD Debug:resizing:"<<width<<"  ddd="<<height;
            auto start_time=CURRENT_TIME;
            char *resized_buffer=halve_nv21_buffer(bitmap_buffer,width,height);
            //LG<<"CD Debug: resize time:"<<CURRENT_TIME-start_time<<"ms";
            start_time=CURRENT_TIME;
            cv::Mat yuv_resized(height/2+height/4,width/2,CV_8UC1,resized_buffer);
            cv::Mat frame(height/2,width/2, CV_8UC3);
            cv::cvtColor(yuv_resized, frame, CV_YUV2BGR_NV21);  // CV_YUV2BGR_NV21 = 93,CV_YUV2BGRA_NV21
            //LG<<"CD Debug: convert time:"<<CURRENT_TIME-start_time<<"ms";
            image = frame;
        }
        else{
            cv::Mat yuv(height + height / 2, width, CV_8UC1, bitmap_buffer);
            cv::Mat frame(height,width, CV_8UC3);
            cv::cvtColor(yuv, frame, CV_YUV2BGR_NV21);  // CV_YUV2BGR_NV21 = 93,CV_YUV2BGRA_NV21
            image = frame;
        }
        
    }
    else if (format == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat::RGBA) {
        LOGV("VZ Debug: Converting image format - RGBA");
        cv::Mat rgba(height, width, CV_8UC4, bitmap_buffer);
        cv::Mat frame(height, width, CV_8UC3);
        cv::cvtColor(rgba, frame, CV_RGBA2BGR);  // CV_RGBA2BGR = 3
        image = frame;
    }
    else {
        LOGV("VZ Debug: Converting image format - BGR");
        cv::Mat bgr(height, width, CV_8UC3, bitmap_buffer);
        image = bgr;
    }
    auto start_time=CURRENT_TIME;
    cv::resize(image,image,cv::Size(target_width,target_height));
    //LG<<"VZ Debug: time in resize:"<<CURRENT_TIME-start_time;
    return image;
}

/**
 * @brief Applies exifcorrection on the input image
 *
 * @param[in] image on which exifcorrection needs to be applied
 * @param[in] orientation of the image
 *
 * @return rotated image
 */
 cv::Mat rotate_image(cv::Mat image, int clockwise_angle) {
    //LOGV("rotate_image : clockwise_angle : [%d]",clockwise_angle);
    int rotate_code;
    if (clockwise_angle == 90) {
        rotate_code = 2;
    } else if (clockwise_angle == 180) {
        rotate_code = 1;
    } else if (clockwise_angle == 270) {
        rotate_code = 0;
    } else {
        return image;
    }
    //LOGV("rotate_image : clockwise_angle : [%d], rotate_code : [%d]",clockwise_angle, rotate_code);
    cv::Mat rotated_image;
    cv::rotate(image, rotated_image, rotate_code);
    return rotated_image;
}

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? x : y)
#define MIN(x, y) ((x) < (y) ? x : y)
#endif

void fill_mask_buffer(std::string& buffer, cv::Mat mask){
    if(mask.empty()){
        LOGV("VZ Debug: Cannot fill mask buffer as given mask is empty.");
    }
    buffer.clear();
    for(int i=0; i < mask.rows; i++){
        for(int j=0; j < mask.cols; j++){
            uchar pixel_value = mask.at<uchar>(cv::Point(j,i));
            buffer.push_back((char)pixel_value);
        }
    }
}

bool checkSimilarColor(const cv::Mat &img) {
    // Convert image to HSV color space
    cv::Mat hsvImg;
    cv::cvtColor(img, hsvImg, cv::COLOR_BGR2HSV);

    // Extract the Hue channel
    cv::Mat hueImg;
    cv::extractChannel(hsvImg, hueImg, 0);

    // Calculate the mean value of the Hue channel
    cv::Scalar meanVal = cv::mean(hueImg);

    // Define the acceptable range around the mean value
    int lowerBound = static_cast<int>(meanVal[0]) - 10;
    int upperBound = static_cast<int>(meanVal[0]) + 10;
    //LG<<"lower bound: "<<lowerBound<<" upper bound: "<<upperBound << "meanVal[0] : "<<meanVal[0];

    // Check if all pixel values are within the acceptable range
    for (int y = 0; y < hueImg.rows; ++y) {
        for (int x = 0; x < hueImg.cols; ++x) {
            if (hueImg.at<uchar>(y, x) < lowerBound || hueImg.at<uchar>(y, x) > upperBound) {
                return false;
            }
        }
    }

    return true;
}

/*
bool boundscheck(const int iw, int const ih, int i, int j){
    if (i >=0 && i<=iw && j >=0 && j<=iw)
        return true;
    return false;
}

cv::Vec3b get_bgr_pixel_yuv(char* yuv, const int image_width, int const image_height, int i, int j){
    const int frame_size = image_width * image_height;
    //LG<<"VZ Debug: get_bgr_pixel_yuv" << frame_size << " i=" << i << " j=" << j;
    // if(!boundscheck(image_width, image_height, i, j))
    //    return cv::Vec3b();

    int y = yuv[i * image_width + j];
    int v = yuv[frame_size + (int)(i/2) * image_width + 2 * (int)(j/2)];
    int u = yuv[frame_size + (int)(i/2) * image_width + 2 * (int)(j/2) + 1];
    //LG<<"VZ Debug: yvu set successfully";

    float b = (1.164 * (y - 16) + 2.018 * (u - 128));
    float g = (1.164 * (y - 16) - 0.813 * (v - 128) - 0.391 * (u - 128));
    float r = (1.164 * (y - 16) + 1.596 * (v - 128));

    cv::Vec3b color;
    color[0] = b > 255.0f ? 255 : b < 0 ? 0 : char(b);
    color[1] = g > 255.0f ? 255 : g < 0 ? 0 : char(g);
    color[2] = r > 255.0f ? 255 : r < 0 ? 0 : char(r);
    return color;
}
*/

cv::Vec3b get_bgr_pixel_yvu(char* yuv, const int image_width, int const image_height, int i, int j){
    const int frame_size = image_width * image_height;        
    int y = yuv[i * image_width + j];
    int u = yuv[frame_size + (int)(i/2) * image_width + 2 * (int)(j/2)];
    int v = yuv[frame_size + (int)(i/2) * image_width + 2 * (int)(j/2) + 1];

    float b = (1.164 * (y - 16) + 2.018 * (u - 128));
    float g = (1.164 * (y - 16) - 0.813 * (v - 128) - 0.391 * (u - 128));
    float r = (1.164 * (y - 16) + 1.596 * (v - 128));

    cv::Vec3b color;
    color[0] = b > 255.0f ? 255 : b < 0 ? 0 : char(b);
    color[1] = g > 255.0f ? 255 : g < 0 ? 0 : char(g);
    color[2] = r > 255.0f ? 255 : r < 0 ? 0 : char(r);
    return color;
}

cv::Vec3b get_bgr_pixel_yuv(char* yuv, const int image_width, int const image_height, int i, int j){
    const int frame_size = image_width * image_height;        
    int y = yuv[i * image_width + j];
    int v = yuv[frame_size + (int)(i/2) * image_width + 2 * (int)(j/2)];
    int u = yuv[frame_size + (int)(i/2) * image_width + 2 * (int)(j/2) + 1];

    float b = (1.164 * (y - 16) + 2.018 * (u - 128));
    float g = (1.164 * (y - 16) - 0.813 * (v - 128) - 0.391 * (u - 128));
    float r = (1.164 * (y - 16) + 1.596 * (v - 128));

    cv::Vec3b color;
    color[0] = b > 255.0f ? 255 : b < 0 ? 0 : char(b);
    color[1] = g > 255.0f ? 255 : g < 0 ? 0 : char(g);
    color[2] = r > 255.0f ? 255 : r < 0 ? 0 : char(r);
    return color;
}

/**
 * @brief Resize and rotate the given image
 * 
 * @param[in] Image buffer with yuv format
 * @param[in] input image width
 * @param[in] input image height
 * @param[in] converted image width
 * @param[in] converted image height
 * @param[in] rotated angle
 * @param[in] image format
 *
 * @return Rotated Image
 */
 cv::Mat nv_convert_resize_rotate(char* yuv, const int image_width, 
    int const image_height, int const target_width,
    int const target_height, int rotation_angle,
    const unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat format) {

    float step_w = float(image_width) / target_width;
    float step_h = float(image_height) / target_height;

    auto *get_pixel = &get_bgr_pixel_yvu;
    if(format == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat::NV21){
        get_pixel = &get_bgr_pixel_yuv;
    }
    if (rotation_angle == 270){
        cv::Mat bgr(target_height, target_width, CV_8UC3);

        for(int i=0; i<target_height; i++){
            for(int j=0; j<target_width; j++){
                bgr.at<cv::Vec3b>(i, j) = (*get_pixel)(yuv, image_width, image_height, (target_height-j-1)*step_h, i*step_w);
            }
        }
        return bgr;
    }
    else if (rotation_angle == 180){
        cv::Mat bgr(target_width, target_height, CV_8UC3);

        for(int i=0; i<target_height; i++){
            for(int j=0; j<target_width; j++){
                bgr.at<cv::Vec3b>(i, j) = (*get_pixel)(yuv, image_width, image_height, (target_height-i-1)*step_h, (target_width-j-1)*step_w);
            }
        }
        return bgr;
    }
    else if (rotation_angle == 90){
        cv::Mat bgr(target_width, target_height, CV_8UC3);
        for(int i=0; i<target_height; i++){
            for(int j=0; j<target_width; j++){
                bgr.at<cv::Vec3b>(i, j) = (*get_pixel)(yuv, image_width, image_height, j*step_h, (target_width-i-1)*step_w);
            }
        }
        return bgr;
    }
    else{
        cv::Mat bgr(target_width, target_height , CV_8UC3);
        for(int i=0; i<target_height; i++){
            for(int j=0; j<target_width; j++){
                bgr.at<cv::Vec3b>(i, j) = (*get_pixel)(yuv, image_width, image_height, i*step_h, j*step_w);
            }
        }
        if (rotation_angle == 0){
            return bgr;
        }
        else{
            LOGV("Unsupported rotation parameter");
        }
    }
    cv::Mat bgr(target_width, target_height , CV_8UC3);
    return bgr;      
}

cv::Mat normalize_image(cv::Mat uchar_image, std::vector<float> means, std::vector<float> scales){
    cv::Mat image;
    uchar_image.convertTo(image, CV_32FC3);
    if(image.channels() != means.size() || image.channels() != scales.size()){
        LOGV("Image not normalized | image channels doesnt match means & scale");
    return image;
    }
    else{
        if(image.channels() < 5){
            cv::Scalar cv_means;
            cv::Scalar cv_scales;
            switch(image.channels()){
                case 1:
                    cv_means = cv::Scalar(means[0]);
                    cv_scales = cv::Scalar(scales[0]);
                break;
                case 3:
                    cv_means = cv::Scalar(means[0], means[1], means[2]);
                    cv_scales = cv::Scalar(scales[0], scales[1], scales[2]);
                break;
                case 4:
                    cv_means = cv::Scalar(means[0], means[1], means[2], means[3]);
                    cv_scales = cv::Scalar(scales[0], scales[1], scales[2], scales[3]);
                break;
                default: {
                    //LG << "Image not normalized as default case is reached";
                    return image;
                }
            }
            image = image - cv_means;
            // image = image / cv_scales;
            cv::divide(image, cv_scales, image);
        }
        else{
            std::vector<cv::Mat> image_channels;
            cv::split(image, image_channels);
            for(int i=0; i < image_channels.size(); i++){
            image_channels[i] = image_channels[i] - (means[i]); 
            image_channels[i] = image_channels[i] * (1.0f/scales[i]); 
            // cv::divide(image_channels[i], cv::Scalar(scales[i]), image_channels[i]); 
            }
            cv::Mat norm_image;
            cv::merge(image_channels, norm_image);
            return norm_image;
        }
    }   
    return image;
}

cv::Mat preprocess_image(char* image_buffer,int width,int height,int target_width,int target_height,int image_rotation,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format){
    cv::Mat image;    
    if(image_format == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat::NV21 || image_format == unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat::NV12){
        //This is done so that image orientation is resized and corrected before its passed to tagger
        #ifdef ANDROID_ARM_NEON
              //LG << "ARM Neon optimization is using";
              image = viz_arm_neon::nv21_or_nv12_resize_convert_rotate(image_buffer, width, height, target_width, target_height, image_rotation, image_format);
          #else
              image = nv_convert_resize_rotate(image_buffer,width,height,target_width,target_height,image_rotation,image_format);
          #endif
    }
    else{
        //rest of the image types
        image = get_bgr_mat_from_buffer(image_buffer, width, height, image_format,target_width,target_height);
        image = rotate_image(image, image_rotation);
    }
    return image;
}

cv::Mat get_cropped_image(cv::Mat image, unified_detector::Rectangle crop_rect) {
  unified_detector::Rectangle crop_rect_org(crop_rect.left, crop_rect.top, crop_rect.right, crop_rect.bottom);
  cv::Mat crop_image;
  float width = crop_rect.right - crop_rect.left + 1;
  float height = crop_rect.bottom - crop_rect.top + 1;
  int width_inc = (int)(0.025 * width);
  int height_inc = (int)(0.025 * height);

  crop_rect.left = crop_rect.left - width_inc;
  crop_rect.right = crop_rect.right + width_inc;
  crop_rect.top = crop_rect.top - height_inc;
  crop_rect.bottom = crop_rect.bottom + height_inc;

  float aspect_ratio = float(crop_rect.right - crop_rect.left) /
                       float(crop_rect.bottom - crop_rect.top);
  if (aspect_ratio > 2.0) {
    int required_height = (crop_rect.right - crop_rect.left) / 2;
    int deficit = required_height - (crop_rect.bottom - crop_rect.top);
    crop_rect.bottom += (deficit * 0.5);
    crop_rect.top -= (deficit * 0.5);
    std::cout << "deficit:" << deficit << std::endl;
  }
  if (aspect_ratio < 0.5) {
    int required_width = (crop_rect.bottom - crop_rect.top) / 2;
    int deficit = required_width - (crop_rect.right - crop_rect.left);
    crop_rect.right += (deficit * 0.5);
    crop_rect.left -= (deficit * 0.5);
    std::cout << "deficit:" << deficit << std::endl;
  }
  if (crop_rect.right > image.cols - 1) {
    crop_rect.right = image.cols - 1;
  }
  if (crop_rect.bottom > image.rows - 1) {
    crop_rect.bottom = image.rows - 1;
  }
  if (crop_rect.left < 0) {
    crop_rect.left = 0;
  }
  if (crop_rect.top < 0) {
    crop_rect.top = 0;
  }
	
  cv::Rect roi(crop_rect.left, crop_rect.top,
               (crop_rect.right - crop_rect.left + 1),
               (crop_rect.bottom - crop_rect.top + 1));
  crop_image = image(roi).clone();

  // cv::Mat image_roi = image(roi);
  // image_roi.copyTo(crop_image);

  return crop_image;
}

cv::Mat& cropResizeBGR(char* image, int width, int height, int left, int top, int right, int bottom, int target_width, int target_height, cv::Mat& preprocessed_image) {
	LOGV("AJ Debug: cropResizeCvtColorBGR w:%d, h:%d, l:%d, t:%d, r:%d, b:%d, tw:%d, th:%d",width, height, left, top, right, bottom, target_width, target_height);
	LOGV("VZ Debug: cropResizeCvtColorBGR called");
	// cv::Mat yuv(height+height/2, width, CV_8UC1, image);
	cv::Rect roi(left, top, right - left, bottom - top);
	cv::Size img_size(width, height);
	cv::Size op_size(target_width, target_height);
	cv::Mat output_image(img_size, CV_32FC3);

  cv::Mat image1(height, width, CV_8UC3, image);
	// cv::Mat image1;  
	// cv::Mat frame(height, width, CV_8UC3);
	// cv::cvtColor(yuv, frame, CV_YUV2BGRA_NV21); //CV_YUV2BGR_NV21 = 93,
	// cv::cvtColor(frame, image1, CV_BGRA2BGR);
	
	cv::Mat image2 = get_cropped_image(image1, {left, top, right, bottom});


	cv::Mat image3;
	cv::resize(image2, image3, op_size);

	//cv::Mat image4;
  image3.convertTo(preprocessed_image, CV_32FC3);

	LOGV("VZ Debug: cropResizeCvtColorBGR completed");
	return preprocessed_image;
}

/*
cv::Mat nv21_convert_resize_rotate(char* yuv, const int image_width, int const image_height, int const target_width, int const target_height, int rotation_angle) {
    float step_w = float(image_width) / target_width;
    float step_h = float(image_height) / target_height;

    LG<<"VZ Debug: nv21_convert_resize_rotate() from SmartScan";

    if (rotation_angle == 90){
        cv::Mat bgr(target_height, target_width, CV_8UC3);

        for(int i=0; i<target_height; i++){
            for(int j=0; j<target_width; j++){
                bgr.at<cv::Vec3b>(i, j) = get_bgr_pixel_yuv(yuv, image_width, image_height, (target_height-j-1)*step_h, i*step_w);
            }
        }
        return bgr;
    }
    else if (rotation_angle == 180){
        cv::Mat bgr(target_height, target_width, CV_8UC3);

        for(int i=0; i<target_height; i++){
            for(int j=0; j<target_width; j++){
                bgr.at<cv::Vec3b>(i, j) = get_bgr_pixel_yuv(yuv, image_width, image_height, (target_height-i-1)*step_h, (target_width-j-1)*step_w);
            }
        }
        return bgr;
    }
    else if (rotation_angle == 270){
        cv::Mat bgr(target_height, target_width, CV_8UC3);

        for(int i=0; i<target_height; i++){
            for(int j=0; j<target_width; j++){
                bgr.at<cv::Vec3b>(i, j) = get_bgr_pixel_yuv(yuv, image_width, image_height, j*step_h, (target_width-i-1)*step_w);
            }
        }
        return bgr;
    }
    else{
        cv::Mat bgr(target_height, target_width, CV_8UC3);
        for(int i=0; i<target_height; i++){
            for(int j=0; j<target_width; j++){
                bgr.at<cv::Vec3b>(i, j) = get_bgr_pixel_yuv(yuv, image_width, image_height, i*step_h, j*step_w);
            }
        }
        if (rotation_angle == 0){
           return bgr;
        }
        else{
            LG<<"Unsupported rotation parameter";
        }
    }       
}
*/

/*std::string get_type_string(int type)
{
    std::string r;

    uchar depth = type & CV_MAT_DEPTH_MASK;
    uchar chans = 1 + (type >> CV_CN_SHIFT);

    switch ( depth ) {
        case CV_8U:  r = "8U"; break;
        case CV_8S:  r = "8S"; break;
        case CV_16U: r = "16U"; break;
        case CV_16S: r = "16S"; break;
        case CV_32S: r = "32S"; break;
        case CV_32F: r = "32F"; break;
        case CV_64F: r = "64F"; break;
        default:     r = "User"; break;
    }

    r += "C";
    r += (chans+'0');

    return r;
}*/
