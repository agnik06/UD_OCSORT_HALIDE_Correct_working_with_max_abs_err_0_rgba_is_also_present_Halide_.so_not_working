#include "image_segmenter.h"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <string.h>
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
std::vector<std::string> split(std::string s, char c)
{
     int x=0;
     std::string cur = "";
     std::vector<std::string> res;
     while(s[x] != '\0')
     {
           if(s[x]!=c)
           {
            cur+=s[x];
           }
           else
           {
                res.push_back(cur);
                cur = "";
           }
           x+=1;
     }
     if(cur.length()>0)
     {
        res.push_back(cur);
     }
     return res;
}
cv::Mat get_visualisation_image(cv::Mat mask_image)
{
    unsigned int color_map_scene_old[8][3] = {{0,0,0}, {221,178,250}, {255,191,0}, {87,122,59}, {2, 222, 132}, {102, 230, 255}, {24,38,227}, {133,140,197}};
    unsigned int color_map_scene[8][3] = {{0,0,0}, {221,178,250}, {255,191,0}, {87,122,59}, {2, 222, 132}, {102, 230, 255}, {color_map_scene_old[7][0], color_map_scene_old[7][1], color_map_scene_old[7][2]}, {color_map_scene_old[6][0], color_map_scene_old[6][1], color_map_scene_old[6][2]}};

    int rows = mask_image.rows;
    int cols = mask_image.cols;
    int dims [] = {rows,cols};
    std::cout<<"dimensions are "<<rows<<" "<<cols<<std::endl;
    cv::Mat colored_image(rows,cols,CV_8UC3);
    cv::Size s = colored_image.size();
    std::cout<<" info "<<s.height<<" "<<s.width<<" "<<colored_image.channels()<<std::endl;
    //int sp[4] = colored_image.size();
    //std::cout<<" spp "<<sp[0]<<" "<<sp[1]<<" "<<sp[2]<<std::endl;
    for(int i=0; i<rows; i++){
        for(int j=0; j<cols; j++){
            //std::cout<<"chagne for "<<i<<" "<<j<<" "<< colored_image.at<uchar>(i,j,0)<<" "<<(int)mask_image.at<uchar>(i,j)<<" "<<colored_image.at<uchar>(i,j,1)<<" "<<colored_image.at<uchar>(i,j,2)<<std::endl;
            colored_image.at<cv::Vec3b>(i,j)[0] = (char)color_map_scene[(int)mask_image.at<uchar>(i,j)][0];
            colored_image.at<cv::Vec3b>(i,j)[1] = (char)color_map_scene[(int)mask_image.at<uchar>(i,j)][1];
            colored_image.at<cv::Vec3b>(i,j)[2] = (char)color_map_scene[(int)mask_image.at<uchar>(i,j)][2];
        }
    }
    return colored_image;
}
int test_image_segmenter_batch(std::string image_file_path,std::string output_png_dump_path,std::string output_vis_dump_path){
    std::vector<std::string> image_files;
    std::ifstream infile;
    std::string file_name;
    infile.open(image_file_path);
    while (!infile.eof()) // To get you all the lines.
    {
        std::getline(infile, file_name); // Saves the line in STRING.
        image_files.push_back(file_name);
    }
    infile.close();
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
    int i =0;
    for(std::string image_path: image_files)
    {
    //Execute
        cv::Mat image = cv::imread(image_path); // absolute image path
        std::vector<std::string> vector_splits ; vector_splits = split(image_path,'/');
        std::string image_name = split(vector_splits[vector_splits.size()-1],'.')[0];
        std::cout<<"image name from split is "<<image_name;
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
            cv::resize(mask_image, mask_image, image.size(),cv::INTER_NEAREST);
            
            mask_image.convertTo(mask_image, CV_8UC1);
            cv::Mat visualisation_image = get_visualisation_image(mask_image);
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
            //cv::hconcat(image_result, image_masked, image_result);
            //cv::resize(image_result, image_result, cv::Size(3*500, 500));
            std::string output_file = output_png_dump_path + "/" + image_name +".png";
            cv::imwrite(output_file, image_result);
            std::string output_vis_file = output_vis_dump_path + "/" + image_name +".png";
            cv::imwrite(output_vis_file,visualisation_image);
        }
        i += 1;
        //cv::waitKey(0);
    }
    
    // Segmenter->Destroy();
    // viz::ImageSegmenter::Destroy(Segmenter);

    return 0;
}

int main(int argc, char** argv){
    std::string image_path("test/image_list.txt");
    std::string output_png_dump_path("/data/Himadri/work-2022/Engines/Night_mode/Outputs/png_dumps");
    std::string output_vis_dump_path("/data/Himadri/work-2022/Engines/Night_mode/Outputs/color_visualisations");
    if(argc > 1) image_path = std::string(argv[1]);

    test_image_segmenter_batch(image_path,output_png_dump_path,output_vis_dump_path);
    // test_data_handler();
    return 0;
}
