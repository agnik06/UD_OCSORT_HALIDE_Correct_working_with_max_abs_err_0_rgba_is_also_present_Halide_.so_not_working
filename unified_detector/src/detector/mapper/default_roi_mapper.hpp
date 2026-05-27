/**
 * @file   default_roi_mapper.hpp
 * @brief  This file defines the mapping of boxes with labels
 *
 * This provides the interfaces for assigning the boxes with labels
 */

#ifndef __SSD_ROI_MAPPER__
#define __SSD_ROI_MAPPER__

#include "roi_mapper.hpp"
#include "framework/rectangle.hpp"
 #include "core/logger/logger.hpp"

class DefaultROIMapper: public IROIMapper{
public:
    DefaultROIMapper(std::shared_ptr<ROIDataHandler> data_handler);
    ~DefaultROIMapper();

    /**
     * @brief maps the ROI bounding boxes
     *
     * @param[in] boxes representing bounding boxes
     *
     * @return bounding boxes
     */
    std::vector<unified_detector::BoundingBox> map(std::vector<unified_detector::BoundingBox> boxes);

private:
    /**
     * @brief assigns the labels to the bounding boxes
     *
     * @param[in] boxes representing the bounding boxes
     *
     * @return bounding boxes with labels set
     */
    std::vector<unified_detector::BoundingBox> assign_labels(std::vector<unified_detector::BoundingBox> boxes);
};

#endif //__SSD_ROI_MAPPER__