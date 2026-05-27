/*
**************************************************************************************************
* Copyright (c) 2014 Mobile Communication Division, Samsung Electronics, Inc.
*
* All right reserved.
*
* This is the confidential and proprietary information of Samsung
* Electronics, Inc.
* ("Confidential Information"). You shall not disclose such Confidential
* Information and shall use it only in accordance with the terms of
* the license agreement you entered into with Samsung Electronics.
*
**************************************************************************************************/

/*!
\file    srib_cnn.h
\brief   definition of srib_snap class
\author  Ambati Venkatesh(v.ambati@samsung.com)
\date    2019/07/08

<b>Revision History: </b>
- 2019/07/08 : Ambati Venkatesh(v.ambati@samsung.com) \n
Initial Release
*/

#pragma once

//#include "srib_cnn_interface.h"
#include "snap_interface.h"
#include <string>
#include <vector>
#include "core/logger/logger.hpp"
// using namespace sribcnn;

class SRIBSnapWrapper {
public:
  bool m_dlopenState;
  bool m_snapState;

  SRIBSnapWrapper();
  ~SRIBSnapWrapper();

  snap::ErrCode OpenSNAP(const struct snap::SnapOptionsV5 &options);
  snap::ErrCode ExecuteSNAP(const std::vector<snap::DataBuffer> &inputs,
                            std::vector<snap::DataBuffer> *outputs);
  snap::ErrCode CloseSNAP();
  snap::ErrCode GetModelInputShapeSNAP(int inputIndex, std::vector<int> *shape);

private:
  void *m_pDlHandle;
  void *m_pfuncTable[2];
  snap::SnapSessionVx *m_pSession;

  void init_sym_table();
};

extern "C" SRIBSnapWrapper *CreateSribSnapWrapperSession(void);
extern "C" snap::ErrCode
DestroySribSnapWrapperSession(SRIBSnapWrapper *session);
