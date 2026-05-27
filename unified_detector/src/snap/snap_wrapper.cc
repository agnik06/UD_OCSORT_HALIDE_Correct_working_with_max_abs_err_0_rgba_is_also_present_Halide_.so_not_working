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
\brief   Implementation of srib_snap class
\author  Ambati Venkatesh(v.ambati@samsung.com)
\date    2019/07/08

<b>Revision History: </b>
- 2019/07/08 : Ambati Venkatesh(v.ambati@samsung.com) \n

<b>Revision History: </b>
- 2023/01/18 : Biplab Ch Das(biplab.das@samsung.com) \n
Initial Release
*/

#include "snap_wrapper.h"
#include <dlfcn.h>
#include "core/logger/logger.hpp"

SRIBSnapWrapper *CreateSribSnapWrapperSession(void) {
  SRIBSnapWrapper *srib_snap = new SRIBSnapWrapper();
  return srib_snap;
}

snap::ErrCode DestroySribSnapWrapperSession(SRIBSnapWrapper *srib_snap) {
  delete srib_snap;
  return snap::ErrCode::OK;
}

SRIBSnapWrapper::SRIBSnapWrapper() 
: m_pSession(NULL), m_pfuncTable{NULL}{
  m_dlopenState = false;
  m_pDlHandle = NULL;
  m_snapState = false;
  int lib_type = 1; // 1 - hidl.so 2 - hidl.snap.samsung.so 3 - snap_vndk.so
  m_pDlHandle = dlopen("libsnap_hidl.so", RTLD_LAZY | RTLD_GLOBAL);
  if (m_pDlHandle == NULL) {
    m_pDlHandle =
        dlopen("libsnap_hidl.snap.samsung.so", RTLD_LAZY | RTLD_GLOBAL);
    lib_type = 2;
  }
  if (m_pDlHandle == NULL) {
    m_pDlHandle =
        dlopen("libsnap_aidl.snap.samsung.so", RTLD_LAZY | RTLD_GLOBAL);
    lib_type = 3;
  }
  if (m_pDlHandle == NULL) {
    m_pDlHandle = dlopen("libsnap_vndk.so", RTLD_LAZY | RTLD_GLOBAL);
    lib_type = 4;
  }
  if (m_pDlHandle != NULL) {
    std::string libname;
    if (lib_type == 1)
      libname = "libsnap_hidl.so";
    else if (lib_type == 2)
      libname = "libsnap_hidl.snap.samsung.so";
    else if (lib_type == 3)
      libname = "libsnap_aidl.snap.samsung.so";
    else
      libname = "libsnap_vndk.so";

    LOGV( "snap library found : %s", libname.c_str());
    m_dlopenState = true;
    init_sym_table();
    if (m_dlopenState == true) {
      snap::SnapSessionVx *(*fptr)(void);
      fptr = (snap::SnapSessionVx * (*)(void)) m_pfuncTable[0];
      m_pSession = static_cast<snap::SnapSessionVx *>((*fptr)());
      if (m_pSession != NULL) {
        m_snapState = true;
      } else {
        LOGV("Snap wrapper SnapSession not created");
      }
    }
  } else {
    LOGV("snap library not found");
    m_dlopenState = false;
  }
}

SRIBSnapWrapper::~SRIBSnapWrapper() {
  if (m_dlopenState == true) {
    if (m_pSession != NULL) {
      snap::ErrCode (*fptr)(snap::SnapSessionV2 *);
      fptr = (snap::ErrCode(*)(snap::SnapSessionV2 *))m_pfuncTable[1];
      snap::ErrCode errCode = (*fptr)(m_pSession);
      if (errCode != snap::ErrCode::OK) {
        LOGV("Snap wrapper DestroySnapSession error = %d", errCode);
      }
    } else {
      LOGV("Snap wrapper snapSession is NULL");
    }
  }

  if (m_pDlHandle)
    dlclose(m_pDlHandle);
}

/* Symbol Table
0 : SnapSessionV2 * CreateSnapSession(void);
1 : ErrCode DestroySnapSession(SnapSessionV2 *session);
 */

void SRIBSnapWrapper::init_sym_table() {
  m_pfuncTable[0] = dlsym(m_pDlHandle, "CreateSnapSession");
  m_pfuncTable[1] = dlsym(m_pDlHandle, "DestroySnapSession");

  if (m_pfuncTable[0] == NULL || m_pfuncTable[1] == NULL) {
    m_dlopenState = false;
    LOGV(
        "Snap wrapper CreateSession and DestroySession symbols not found");
  }
}

snap::ErrCode
SRIBSnapWrapper::OpenSNAP(const struct snap::SnapOptionsV5 &options) {
  snap::ErrCode errCode;
  if (m_dlopenState == true && m_snapState == true) {
    errCode = m_pSession->Open(options);
    if (errCode != snap::ErrCode::OK) {
      m_snapState = false;
      LOGV("Snap wrapper openSnap error = %d", errCode);
    }
  } else {
    if (m_dlopenState == false)
      LOGV("Snap wrapper dlopen fail");
    if (m_snapState == false)
      LOGV("Snap wrapper snap session fail");
    errCode = snap::ErrCode::ERR;
  }
  LOGV("SNAP Open %s",((errCode == snap::ErrCode::OK)? "success" : "fail"));
  return errCode;
}

snap::ErrCode
SRIBSnapWrapper::ExecuteSNAP(const std::vector<snap::DataBuffer> &inputs,
                             std::vector<snap::DataBuffer> *outputs) {
  snap::ErrCode errCode;
  if (m_dlopenState == true && m_snapState == true) {
    errCode = m_pSession->Execute(inputs, outputs);
  } else {
    if (m_dlopenState == false)
      LOGV("Snap wrapper snapExecute skipped - dlopen fail");
    if (m_snapState == false)
      LOGV("Snap wrapper snapExecute skipped - snap init fail");
    errCode = snap::ErrCode::ERR;
  }

  LOGV("SNAP Execute : [%s]", ((errCode == snap::ErrCode::OK)? "success" : "fail"));
  return errCode;
}

snap::ErrCode SRIBSnapWrapper::CloseSNAP() {
  snap::ErrCode errCode;
  if (m_dlopenState == true && m_snapState == true && m_pSession!= NULL) {
    errCode = m_pSession->Close();
  } else {
    if (m_dlopenState == false)
      LOGV("Snap wrapper snapClose skipped - dlopen fail");
    if (m_snapState == false)
      LOGV("Snap wrapper snapClose skipped - snap init fail");
    if(m_pSession == NULL)
      LOGV("Snap wrapper snapClose skipped - snap session null");
    errCode = snap::ErrCode::ERR;
  }
  return errCode;
}

snap::ErrCode SRIBSnapWrapper::GetModelInputShapeSNAP(int inputIndex,
                                                      std::vector<int> *shape) {
  snap::ErrCode errCode = snap::ERR;
  LOGV("function not implemented in CNNInterface linking library");
  return errCode;
}
