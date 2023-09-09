///////////////////////////////////////////////////////////////////////////////
// FILE:          NikonNi.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Control Nikon Ni-series (Ni-E, Ni-U) microscopes 
//                
// AUTHOR:        Cezary Stankiewicz, cezary.stankiewicz@gmail.com, 15/04/2021
// COPYRIGHT:     Cezary Stankiewicz, 2021
// LICENSE:       This file is distributed under the BSD license.
//                License text is included with the source distribution.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.

#ifndef NIKONNI_H
#define NIKONNI_H

#define SDK_PRESENT 0

#if SDK_PRESENT
#include "new_mic_sdk.h"
#endif

#include "../../MMDevice/DeviceBase.h"
#include "../../MMDevice/DeviceThreads.h"
#include "../../MMDevice/DeviceUtils.h"
#include "../../MMDevice/ModuleInterface.h"

using namespace std;

#define ERR_MISSING_LICENSE_FILE    10002
#define ERR_CONNECTION_FAILED       10004

class NikonNiHub : public HubBase<NikonNiHub> {

public:
    NikonNiHub();
    ~NikonNiHub();

    // Inherited via HubBase (Pure)
    virtual bool Busy() override;
    virtual int Initialize() override;
    virtual int Shutdown() override;
    virtual void GetName(char *name) const override;

    // Inherited via HubBase
    virtual int DetectInstalledDevices() override;

private:
#if SDK_PRESENT
    lx_uint64 m_uiAccesory;
#endif
    bool m_initialized;
};

class NikonNiStage : public CStageBase<NikonNiStage> {
private:
    bool m_initialized;
public:
    NikonNiStage();
    ~NikonNiStage();
    
    // Inherited via CStageBase (Pure)
    virtual bool Busy() override;
    virtual int Initialize() override;
    virtual int Shutdown() override;
    virtual void GetName(char * name) const override;
    virtual int SetPositionSteps(long steps) override;
    virtual int GetPositionSteps(long & steps) override;
    virtual int SetOrigin() override;
    virtual int GetLimits(double & lower, double & upper) override;
    virtual int IsStageSequenceable(bool & isSequenceable) const override;
    virtual bool IsContinuousFocusDrive() const override;

    virtual int GetPositionUm(double & pos) override;
    virtual int SetPositionUm(double pos) override;
    //virtual int SetRelativePositionUm(double pos) override;
};

class NikonNiDiaLamp : public CShutterBase<NikonNiDiaLamp> {
    bool m_initialized;
    long m_intensity;
    long m_state;
public:
    NikonNiDiaLamp();
    ~NikonNiDiaLamp();

    // Inherited via CShutterBase
    virtual bool Busy() override;
    virtual int Initialize() override;
    virtual int Shutdown() override;
    virtual void GetName(char* name) const override;
    virtual int SetOpen(bool open) override;
    virtual int GetOpen(bool& open) override;
    virtual int Fire(double deltaT) override;

    // action interface
    int OnControl(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnIntensity(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnSwitchLamp(MM::PropertyBase* pProp, MM::ActionType eAct);
};

class NikonNiXYStage : public CXYStageBase<NikonNiXYStage> {
private:
    bool m_initialized;
public:
    NikonNiXYStage();
    ~NikonNiXYStage();

    // Inherited via CXYStageBase
    virtual bool Busy() override;
    virtual int Initialize() override;
    virtual int Shutdown() override;
    virtual void GetName(char* name) const override;
    virtual int GetLimitsUm(double& xMin, double& xMax, double& yMin, double& yMax) override;
    virtual int SetPositionSteps(long x, long y) override;
    virtual int GetPositionSteps(long& x, long& y) override;
    virtual int Home() override;
    virtual int Stop() override;
    virtual int SetOrigin() override;
    virtual int GetStepLimits(long& xMin, long& xMax, long& yMin, long& yMax) override;
    virtual double GetStepSizeXUm() override;
    virtual double GetStepSizeYUm() override;
    virtual int IsXYStageSequenceable(bool& isSequenceable) const override;


    int SetPositionUm(double x, double y);
    int GetPositionUm(double& x, double& y);
    int SetRelativePositionUm(double dx, double dy);
};

#endif // NIKONNI_H
