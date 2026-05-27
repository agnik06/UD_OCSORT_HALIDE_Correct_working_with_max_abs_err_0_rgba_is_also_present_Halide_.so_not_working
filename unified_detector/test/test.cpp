#include "image_segmenter.h"
#include "opencv2/opencv.hpp"
#include <iostream>

#include "modules/detector/mapper/data/roi_data_handler.hpp"

int test_data_handler(){
    ROIDataHandler data_handler;
    std::cout<<data_handler.get_label(0)<<"\n";
    std::cout<<data_handler.get_priority(0)<<"\n";
    std::cout<<data_handler.is_valid_id(0)<<"\n";
    return 0;
}

cv::Mat get_mat_from_mask_buffer(char* buffer, int width, int height){
    cv::Mat mask_image = cv::Mat(height, width, CV_8UC1);
    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            mask_image.at<uchar>(cv::Point(j,i)) = (uchar)buffer[i*width+j];
        }
    }
    return mask_image;
}

int test_image_segmenter(std::string image_path){
    std::shared_ptr<viz::ImageSegmenter> Segmenter(viz::ImageSegmenter::Create(), viz::ImageSegmenter::Destroy);
    // viz::ImageSegmenter* Segmenter = viz::ImageSegmenter::Create();

    //Initialize
    // bool init_status = Segmenter->initialize(viz::ImageSegmenter::Mode::SALIENCY, (char*)"models/");
    // bool init_status = Segmenter->initialize(viz::ImageSegmenter::Mode::LIVE_FOCUS, (char*)"models/");
    bool init_status = Segmenter->initialize(viz::ImageSegmenter::Mode::SCENE, (char*)"models/");
    std::cout<<"Init status: "<<init_status<<"\n";
    if(!init_status){
        std::cerr<<"Initialization of ImageSegmenter failed\n";
    }

    //Execute
    cv::Mat image = cv::imread(image_path);
    char* image_buffer = reinterpret_cast<char*> (image.data);
    int n_faces = 1;
    bool execute_status = Segmenter->execute(image_buffer, image.cols, image.rows, 0, viz::ImageSegmenter::ImageFormat::BGR,n_faces);
    // bool execute_status = Segmenter->execute_on_touch(image_buffer, image.cols, image.rows, 0, viz::ImageSegmenter::ImageFormat::BGR, 100, 300);
    std::cout<<"Execute status: "<<execute_status<<"\n";
    if(!execute_status){
        std::cerr<<"Execution of segmentation failed\n";
    }

    //Extract segmentation mask
    viz::ImageSegmenter::SegmentationMask mask;
    bool mask_extraction_staus = Segmenter->get_segmentation_mask(mask);
    std::cout<<"Mask Extraction status: "<<mask_extraction_staus<<"\n";
    if(!mask_extraction_staus){
        std::cerr<<"Extraction of segmentation mask failed\n";
    }
    else{
        std::cout<<"Result: "<<mask.tag<<" | ("<<mask.width<<","<<mask.height<<")\n";
        std::cout<<"Result: "<<"("<<mask.left<<","<<mask.top<<","<<mask.right<<","<<mask.bottom<<")\n";
    }

    //Display result
    if(mask_extraction_staus){
        // cv::Mat mask_image = cv::Mat(mask.height, mask.width, CV_8UC1, mask.mask);
        cv::Mat mask_image = get_mat_from_mask_buffer(mask.mask_buffer, mask.width, mask.height);
        cv::resize(mask_image, mask_image, image.size());
        mask_image.convertTo(mask_image, CV_8UC1);

        cv::Mat image_overlayed, mask_image_3c;
        std::vector<cv::Mat> imgs = {mask_image, mask_image, mask_image};
        cv::merge(imgs, mask_image_3c);

        // cv::addWeighted(image, 0.3, mask_image_3c, 0.7, 0.0, image_overlayed);

        cv::Mat mask_image_th;
        cv::threshold(mask_image, mask_image_th, 128, 255, 0);
        cv::Mat image_masked;
        image.copyTo(image_masked, mask_image_th);

        cv::Mat image_result;
        cv::hconcat(image, mask_image_3c, image_result);
        cv::hconcat(image_result, image_masked, image_result);
        cv::resize(image_result, image_result, cv::Size(3*500, 500));
        cv::imwrite("Output.png", image_result);
        //cv::waitKey(0);
    }
    
    // Segmenter->Destroy();
    // viz::ImageSegmenter::Destroy(Segmenter);

    return 0;
}

int main(int argc, char** argv){
    std::string image_path("test/test_wall.jpg");
    if(argc > 1) image_path = std::string(argv[1]);

    test_image_segmenter(image_path);
    // test_data_handler();
    return 0;
}
