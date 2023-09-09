///////////////////////////////////////////////////////////////////////////////
// FILE:          LeicaMStereo.cpp
///////////////////////////////////////////////////////////////////////////////
// FILE:          NikonNi.cpp
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

#include "NikonNi.h"

#include "../../MMDevice/ModuleInterface.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <sstream>
#include <bitset>

using namespace std;

const char* g_DeviceName_NikonNiHub = "NikonNi-Hub";
const char* g_DeviceName_NikonNiStage = "NikonNi-Stage";
const char* g_DeviceName_NikonNiDiaLamp = "NikonNi-DiaLamp";
const char* g_DeviceName_NikonNiXYStage = "NikonNi-XYStage";

///////////////////////////////////////////////////////////////////////////////
// Exported MMDevice API
///////////////////////////////////////////////////////////////////////////////

MODULE_API void InitializeModuleData()
{
    RegisterDevice(g_DeviceName_NikonNiHub, MM::HubDevice, "Nikon Ni microscope");
    RegisterDevice(g_DeviceName_NikonNiStage, MM::StageDevice, "Nikon Ni objective stage");
    RegisterDevice(g_DeviceName_NikonNiDiaLamp, MM::ShutterDevice, "Nikon Ni transillumination");
    RegisterDevice(g_DeviceName_NikonNiXYStage, MM::XYStageDevice, "Nikon Ni XY stage");
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
	if (deviceName == 0) {
		return 0;
	}

    if (strcmp(deviceName, g_DeviceName_NikonNiHub) == 0) {
        return new NikonNiHub();
    }
    else if (strcmp(deviceName, g_DeviceName_NikonNiStage) == 0) {
        return new NikonNiStage();
    }
    else if (strcmp(deviceName, g_DeviceName_NikonNiDiaLamp) == 0) {
        return new NikonNiDiaLamp();
    }
    else if (strcmp(deviceName, g_DeviceName_NikonNiXYStage) == 0) {
        return new NikonNiXYStage();
    }

	return 0;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
	delete pDevice;
}

///////////////////////////////////////////////////////////////////////////////
// Hub 
///////////////////////////////////////////////////////////////////////////////

NikonNiHub::NikonNiHub() 
    : m_initialized(false)
#if SDK_PRESENT
    , m_uiAccesory(0)
#endif
{
    InitializeDefaultErrorMessages();
}

NikonNiHub::~NikonNiHub()
{
}

bool NikonNiHub::Busy()
{
    return false;
}

int NikonNiHub::Initialize()
{
#if SDK_PRESENT
    lx_result lResult = LX_OK;
    lx_wchar szError[256];
    ZeroMemory(szError, sizeof(szError));
    lResult = MIC_Open(0, m_uiAccesory, 256, szError); // first device

    if (lResult != LX_OK)
        lResult = MIC_SimulatorOpen(0, m_uiAccesory, 256, szError); // simulator

    if (lResult != LX_OK)
        lResult = MIC_SimulatorOpen(101, m_uiAccesory, 256, szError); // simulator force Ni-E

    
    //std::stringstream stream;
    //stream << "Mounted accessories:" << std::bitset<64>(m_uiAccesory).to_string();
    //stream << "\n";
    //stream << "Mounted ZSTAGE:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_ZSTAGE).to_string();
    //stream << "\n";
    //stream << "Mounted ZOBJECTIVESTAGE:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_ZOBJECTIVESTAGE).to_string();
    //stream << "\n";
    //stream << "Mounted XYSTAGE:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_XYSTAGE).to_string();
    //stream << "\n";
    //stream << "Mounted LIGHTPATH:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_LIGHTPATH).to_string();
    //stream << "\n";
    //stream << "Mounted PIEZOZSTAGE:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_PIEZOZSTAGE).to_string();
    //stream << "\n";
    //stream << "Mounted NOSEPIECE:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_NOSEPIECE).to_string();
    //stream << "\n";
    //stream << "Mounted CONDENSER:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_CONDENSER).to_string();
    //stream << "\n";
    //stream << "Mounted TURRET1:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_TURRET1).to_string();
    //stream << "\n";
    //stream << "Mounted TURRET1SHUTTER:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_TURRET1SHUTTER).to_string();
    //stream << "\n";
    //stream << "Mounted TURRET2:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_TURRET2).to_string();
    //stream << "\n";
    //stream << "Mounted TURRET2SHUTTER:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_TURRET2SHUTTER).to_string();
    //stream << "\n";
    //stream << "Mounted FILTERWHEEL_EXC:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_FILTERWHEEL_EXC).to_string();
    //stream << "\n";
    //stream << "Mounted FILTERWHEEL_BAR:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_FILTERWHEEL_BAR).to_string();
    //stream << "\n";
    //stream << "Mounted TIRF:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_TIRF).to_string();
    //stream << "\n";
    //stream << "Mounted LIGHTPATH:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_LIGHTPATH).to_string();
    //stream << "\n";
    //stream << "Mounted ANALYZER:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_ANALYZER).to_string();
    //stream << "\n";
    //stream << "Mounted PFS:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_PFS).to_string();
    //stream << "\n";
    //stream << "Mounted SHUTTER_EPI:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_SHUTTER_EPI).to_string();
    //stream << "\n";
    //stream << "Mounted SHUTTER_DIA:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_SHUTTER_DIA).to_string();
    //stream << "\n";
    //stream << "Mounted SHUTTER_AUX:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_SHUTTER_AUX).to_string();
    //stream << "\n";
    //stream << "Mounted DSC1:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_DSC1).to_string();
    //stream << "\n";
    //stream << "Mounted DSC2:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_DSC2).to_string();
    //stream << "\n";
    //stream << "Mounted DIALAMP:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_DIALAMP).to_string();
    //stream << "\n";
    //stream << "Mounted INTENSILIGHT:" << std::bitset<64>(m_uiAccesory & MIC_ACCESSORY_MASK_INTENSILIGHT).to_string();
    //stream << "\n";
    //LogMessage(stream.str(), false);


    // Name
    int ret = CreateStringProperty(MM::g_Keyword_Name, g_DeviceName_NikonNiHub, true);
    if (DEVICE_OK != ret) return ret;

    // Description
    ret = CreateStringProperty(MM::g_Keyword_Description, "Microscope base", true);
    if (DEVICE_OK != ret)
        return ret;

    m_initialized = true;
    if (lResult == LX_OK) return DEVICE_OK;
    else return DEVICE_ERR;
#endif
    return DEVICE_OK;
}

int NikonNiHub::Shutdown()
{
#if SDK_PRESENT
    lx_result lResult = LX_OK;
    m_uiAccesory = 0;
    m_initialized = false;
    MIC_Close();
    if (lResult == LX_OK) return DEVICE_OK;
    else return DEVICE_ERR;
#endif
    return DEVICE_OK;
}

void NikonNiHub::GetName(char *name) const
{
    CDeviceUtils::CopyLimitedString(name, g_DeviceName_NikonNiHub);
}

int NikonNiHub::DetectInstalledDevices()
{
#if SDK_PRESENT
    ClearInstalledDevices();

    InitializeModuleData();

    //lx_uint32 uiDeviceCount;
    //lx_int32* ppiDeviceTypeList = new lx_int32;
    //MIC_GetDeviceList(uiDeviceCount, &ppiDeviceTypeList);
    //LogMessage("Detected:" + *ppiDeviceTypeList);


    std::stringstream stream;
    stream << "Mounted accessories:" << std::bitset<64>(m_uiAccesory).to_string();
    stream << "\n";
    LogMessage(stream.str(), false);

    if (m_uiAccesory & MIC_ACCESSORY_MASK_ZSTAGE) {
        MM::Device* pDev = ::CreateDevice(g_DeviceName_NikonNiStage);
        if (pDev) {
            LogMessage("Creating z stage", false);
        }
            AddInstalledDevice(pDev);
    }
    if (m_uiAccesory & MIC_ACCESSORY_MASK_ZOBJECTIVESTAGE) {
        MM::Device* pDev = ::CreateDevice(g_DeviceName_NikonNiStage);
        if (pDev) {
            LogMessage("Creating objective stage", false);
            AddInstalledDevice(pDev);
        }
    }
    if (m_uiAccesory & MIC_ACCESSORY_MASK_DIALAMP) {
        MM::Device* pDev = ::CreateDevice(g_DeviceName_NikonNiDiaLamp);
        if (pDev) {
            LogMessage("Creating dialamp", false);
            AddInstalledDevice(pDev);
        }
    }
    if (m_uiAccesory & MIC_ACCESSORY_MASK_XYSTAGE) {
        MM::Device* pDev = ::CreateDevice(g_DeviceName_NikonNiXYStage);
        if (pDev) {
            LogMessage("Creating xy stage", false);
            AddInstalledDevice(pDev);
        }
    }


#endif
    return DEVICE_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Stage
///////////////////////////////////////////////////////////////////////////////

NikonNiStage::NikonNiStage()
    : m_initialized(false)
{
    InitializeDefaultErrorMessages();
    EnableDelay();
    CreateHubIDProperty();
}

NikonNiStage::~NikonNiStage()
{
    Shutdown();
}

bool NikonNiStage::Busy()
{
    return false;
}

int NikonNiStage::Initialize()
{

    NikonNiHub* pHub = static_cast<NikonNiHub*>(GetParentHub());
    if (pHub) {
        char hubLabel[MM::MaxStrLength];
        pHub->GetLabel(hubLabel);
        SetParentID(hubLabel);
    }
    else {
        LogMessage("Parent Hub not defined.");
    }

    if (m_initialized)
        return DEVICE_OK;

    // Name
    int ret = CreateStringProperty(MM::g_Keyword_Name, g_DeviceName_NikonNiStage, true);
    if (DEVICE_OK != ret) return ret;

    // Description
    ret = CreateStringProperty(MM::g_Keyword_Description, "Stage", true);
    if (DEVICE_OK != ret)
        return ret;

    return 0;
}

int NikonNiStage::Shutdown()
{
    return 0;
}

void NikonNiStage::GetName(char *name) const
{
    CDeviceUtils::CopyLimitedString(name, g_DeviceName_NikonNiStage);
}

int NikonNiStage::SetPositionSteps(long steps)
{
    return ERR_MISSING_LICENSE_FILE;
}

int NikonNiStage::GetPositionSteps(long & steps)
{
    return ERR_MISSING_LICENSE_FILE;
}

int NikonNiStage::SetOrigin()
{
    return ERR_MISSING_LICENSE_FILE;
}

int NikonNiStage::GetLimits(double &lower, double &upper)
{
    return ERR_MISSING_LICENSE_FILE;
    //MIC_MetaData sMetaData;
    //ZeroMemory(&sMetaData, sizeof(sMetaData));
    //sMetaData.uiMetaDataUsageMask |= MIC_METADATA_MASK_ZDrive_RangePhys;

    //if (MIC_MetadataGet(sMetaData) != LX_OK) {
    //    LogMessage("Error: MIC_MetadataGet(sMetaData)", false);
    //}
    //else {
    //    lower = sMetaData.iZDrive_RangePhys[0];
    //    upper = sMetaData.iZDrive_RangePhys[1];
    //}
    //return 0;
}

int NikonNiStage::IsStageSequenceable(bool &isSequenceable) const
{
    isSequenceable = false; return DEVICE_OK;
}

bool NikonNiStage::IsContinuousFocusDrive() const
{
    return false;
}

int NikonNiStage::GetPositionUm(double &pos)
{
#if SDK_PRESENT
    MIC_Data sData;
    ZeroMemory(&sData, sizeof(sData));
    sData.uiDataUsageMask |= MIC_DATA_MASK_ZPOSITION;

    if (MIC_DataGet(sData) != LX_OK)
        LogMessage("MIC_DataGet(sData) != LX_OK", false);
    else {
        MIC_Convert_Dev2Phys(MIC_DATA_MASK_ZPOSITION, sData.iZPOSITION, pos);
    }

#endif
    return DEVICE_OK;
}

int NikonNiStage::SetPositionUm(double pos)
{
#if SDK_PRESENT
    lx_int32 iDevValue;

    MIC_Data sDataIn, sDataOut;
    ZeroMemory(&sDataIn, sizeof(sDataIn));
    ZeroMemory(&sDataOut, sizeof(sDataOut));

    MIC_Convert_Phys2Dev(MIC_DATA_MASK_ZPOSITION, pos, iDevValue);
    sDataIn.uiDataUsageMask |= MIC_DATA_MASK_ZPOSITION;
    sDataIn.iZPOSITION = iDevValue;

    if (MIC_DataSet(sDataIn, sDataOut, false) != LX_OK) {
        LogMessage("Was unable to change the position", false);
    }

#endif
    return DEVICE_OK;
}

//int NikonNiStage::SetRelativePositionUm(double pos)
//{
//    return 0;
//}

///////////////////////////////////////////////////////////////////////////////
// Transillumination
///////////////////////////////////////////////////////////////////////////////

NikonNiDiaLamp::NikonNiDiaLamp()
    : m_initialized(false)
{
    InitializeDefaultErrorMessages();
    EnableDelay();
    CreateHubIDProperty();
}

NikonNiDiaLamp::~NikonNiDiaLamp()
{
}

bool NikonNiDiaLamp::Busy()
{
    return false;
}

int NikonNiDiaLamp::Initialize()
{
#if SDK_PRESENT
    NikonNiHub* pHub = static_cast<NikonNiHub*>(GetParentHub());
    if (pHub) {
        char hubLabel[MM::MaxStrLength];
        pHub->GetLabel(hubLabel);
        SetParentID(hubLabel);
    }
    else {
        LogMessage("Parent Hub not defined.");
    }

    if (m_initialized) return DEVICE_OK;

    int ret;
    CPropertyAction* pAct;
    unsigned char ans[16];
    MIC_Data sData;

    // Name
    ret = CreateStringProperty(MM::g_Keyword_Name, g_DeviceName_NikonNiDiaLamp, true);
    if (DEVICE_OK != ret) return ret;

    // Description
    ret = CreateStringProperty(MM::g_Keyword_Description, "Transillumination", true);
    if (DEVICE_OK != ret) return ret;

    // Computer vs manual control
    sData = MIC_Data();
    ZeroMemory(&sData, sizeof(sData));
    sData.uiDataUsageMask |= MIC_DATA_MASK_DIALAMP_CTRLMODE;
    if (MIC_DataGet(sData) != LX_OK)
        LogMessage("MIC_DataGet(sData) != LX_OK", false);
    else {
        pAct = new CPropertyAction(this, &NikonNiDiaLamp::OnControl);
        std::string controlSwitch = (sData.iDIALAMP_CTRLMODE == 1 ? "Computer" : "Manual");
        ret = CreateProperty("ControlMode", controlSwitch.c_str(), MM::String, false, pAct);
        if (DEVICE_OK != ret) { return ret; }
        ret = SetAllowedValues("ControlMode", std::vector<std::string>{"Computer", "Manual"});
        if (DEVICE_OK != ret) { return ret; }
    }

    // Lamp intensity
    std::string intensity;

    sData = MIC_Data();
    ZeroMemory(&sData, sizeof(sData));
    sData.uiDataUsageMask |= MIC_DATA_MASK_DIALAMP_VOLTAGE;

    MIC_MetaData sMetaData;
    ZeroMemory(&sMetaData, sizeof(sMetaData));
    sMetaData.uiMetaDataUsageMask |= MIC_METADATA_MASK_DiaLampVoltage_RangePhys;

    if ((MIC_MetadataGet(sMetaData) != LX_OK) || (MIC_DataGet(sData) != LX_OK)) {
        LogMessage("Error: MIC_MetadataGet(sMetaData)", false);
    }
    else {
        m_intensity = sData.iDIALAMP_VOLTAGE;
        pAct = new CPropertyAction(this, &NikonNiDiaLamp::OnIntensity);
        ret = CreateProperty("Intensity", std::to_string(m_intensity).c_str(), MM::Float, false, pAct);
        if (DEVICE_OK != ret) { return ret; }
        ret = SetPropertyLimits("Intensity", sMetaData.iDialampVoltage_RangePhys[0], sMetaData.iDialampVoltage_RangePhys[1]);
        if (DEVICE_OK != ret) { return ret; }
    }

    sData = MIC_Data();
    ZeroMemory(&sData, sizeof(sData));
    sData.uiDataUsageMask |= MIC_DATA_MASK_DIALAMP_SWITCH;

    if (MIC_DataGet(sData) != LX_OK) {
        LogMessage("Error: MIC_MetadataGet(sMetaData)", false);
    }
    else {
        std::string state = sData.iDIALAMP_SWITCH == 1 ? "On" : "Off";
        LogMessage("Current state of lamp: " + state, false);
        pAct = new CPropertyAction(this, &NikonNiDiaLamp::OnSwitchLamp);
        ret = CreateProperty("State", state.c_str(), MM::String, false, pAct);
        if (DEVICE_OK != ret) { return ret; }
        ret = SetAllowedValues("State", std::vector<std::string>{"Off", "On"});
        if (DEVICE_OK != ret) { return ret; }
    }

    ret = UpdateStatus();
    if (ret != DEVICE_OK)
        return ret;


    m_initialized = true;
    
#endif
    return 0;
}

int NikonNiDiaLamp::Shutdown()
{
    return 0;
}

void NikonNiDiaLamp::GetName(char* name) const
{
    CDeviceUtils::CopyLimitedString(name, g_DeviceName_NikonNiDiaLamp);
}

int NikonNiDiaLamp::SetOpen(bool open=true)
{
#if SDK_PRESENT
    MIC_Data sDataIn, sDataOut;
    ZeroMemory(&sDataIn, sizeof(sDataIn));
    ZeroMemory(&sDataOut, sizeof(sDataOut));

    sDataIn.uiDataUsageMask |= MIC_DATA_MASK_DIALAMP_SWITCH;
    sDataIn.iDIALAMP_SWITCH = open;

    if (MIC_DataSet(sDataIn, sDataOut, false) != LX_OK) {
        LogMessage("Error: NikonNiDiaLamp::SetOpen", false);
    }

#endif
    return 0;
}

int NikonNiDiaLamp::GetOpen(bool& open)
{
#if SDK_PRESENT
    MIC_Data sData;
    ZeroMemory(&sData, sizeof(sData));
    sData.uiDataUsageMask |= MIC_DATA_MASK_DIALAMP_SWITCH;

    if (MIC_DataGet(sData) != LX_OK)
        LogMessage("MIC_DataGet(sData) != LX_OK", false);
    else {
        open = sData.iDIALAMP_SWITCH;
    }
#endif
    return DEVICE_OK;
}

int NikonNiDiaLamp::Fire(double deltaT)
{
    return DEVICE_UNSUPPORTED_COMMAND;
}

int NikonNiDiaLamp::OnSwitchLamp(MM::PropertyBase* pProp, MM::ActionType eAct)
{
#if SDK_PRESENT
    if (eAct == MM::BeforeGet)
    {
        MIC_Data sData;
        ZeroMemory(&sData, sizeof(sData));
        sData.uiDataUsageMask |= MIC_DATA_MASK_DIALAMP_SWITCH;

        if (MIC_DataGet(sData) != LX_OK)
            LogMessage("MIC_DataGet(sData) != LX_OK", false);
        else {
            std::string state = sData.iDIALAMP_SWITCH ? "On" : "Off";
            pProp->Set(state.c_str());
        }
    }
    else if (eAct == MM::AfterSet)
    {
        std::string selectedState;
        pProp->Get(selectedState);
        int nextState = selectedState.compare("On") == 0 ? 1 : 0;

        MIC_Data sDataIn, sDataOut;
        ZeroMemory(&sDataIn, sizeof(sDataIn));
        ZeroMemory(&sDataOut, sizeof(sDataOut));

        sDataIn.uiDataUsageMask |= MIC_DATA_MASK_DIALAMP_SWITCH;
        sDataIn.iDIALAMP_SWITCH = nextState;

        if (MIC_DataSet(sDataIn, sDataOut, false) != LX_OK) {
            LogMessage("Error: NikonNiDiaLamp::SetOpen", false);
        }
    }
#endif
    return DEVICE_OK;
}

int NikonNiDiaLamp::OnControl(MM::PropertyBase* pProp, MM::ActionType eAct)
{
#if SDK_PRESENT
    if (eAct == MM::BeforeGet)
    {
        MIC_Data sData;
        std::string controlSwitch;
        ZeroMemory(&sData, sizeof(sData));
        sData.uiDataUsageMask |= MIC_DATA_MASK_DIALAMP_CTRLMODE;
        if (MIC_DataGet(sData) != LX_OK)
            LogMessage("MIC_DataGet(sData) != LX_OK", false);
        else {
            controlSwitch = sData.iDIALAMP_CTRLMODE == 1 ? "Computer" : "Manual";
            pProp->Set(controlSwitch.c_str());
        }
        
    }
    else if (eAct == MM::AfterSet)
    {
        MIC_Data sDataIn, sDataOut;
        ZeroMemory(&sDataIn, sizeof(sDataIn));
        ZeroMemory(&sDataOut, sizeof(sDataOut));

        std::string controlSwitch;
        pProp->Get(controlSwitch);
        
        std::stringstream stream;
        stream << "Read property:" << controlSwitch;
        stream << "\n";
        LogMessage(stream.str(), false);
        stream.clear();

        stream << "Found control set to computer:" << controlSwitch.compare("Computer");
        stream << "\n";
        LogMessage(stream.str(), false);
        

        sDataIn.uiDataUsageMask |= MIC_DATA_MASK_DIALAMP_CTRLMODE;
        sDataIn.iDIALAMP_CTRLMODE = controlSwitch.compare("Computer") == 0 ? 1 : 0;
        if (MIC_DataSet(sDataIn, sDataOut, false) != LX_OK) {
            LogMessage("Error: NikonNiDiaLamp::OnControl", false);
        }
        stream.clear();
        stream << "Changed control mode to:" << sDataIn.iDIALAMP_CTRLMODE;
        stream << "\n";
        stream << "Actual control mode to:" << sDataOut.iDIALAMP_CTRLMODE;
        stream << "\n";
        LogMessage(stream.str(), false);

    }
#endif
    return DEVICE_OK;
}

int NikonNiDiaLamp::OnIntensity(MM::PropertyBase* pProp, MM::ActionType eAct)
{
#if SDK_PRESENT
    if (eAct == MM::BeforeGet)
    {
        long intensity;
        MIC_Data sData;
        ZeroMemory(&sData, sizeof(sData));
        sData.uiDataUsageMask |= MIC_DATA_MASK_DIALAMP_VOLTAGE;
        if (MIC_DataGet(sData) != LX_OK) {
            LogMessage("MIC_DataGet(sData) != LX_OK", false);
        }
        else {
            intensity = sData.iDIALAMP_VOLTAGE;
            pProp->Set((long)intensity);
        }
    }
    else if (eAct == MM::AfterSet) {
        MIC_Data sDataIn, sDataOut;
        ZeroMemory(&sDataIn, sizeof(sDataIn));
        ZeroMemory(&sDataOut, sizeof(sDataOut));
        long intensity;
        pProp->Get(intensity);
        sDataIn.uiDataUsageMask |= MIC_DATA_MASK_DIALAMP_VOLTAGE;
        sDataIn.iDIALAMP_VOLTAGE = intensity;
        if (MIC_DataSet(sDataIn, sDataOut, false) != LX_OK) {
            LogMessage("Error: NikonNiDiaLamp::OnIntensity", false);
        }
    }
#endif
    return DEVICE_OK;
}

///////////////////////////////////////////////////////////////////////////////
// XYStage
///////////////////////////////////////////////////////////////////////////////

NikonNiXYStage::NikonNiXYStage()
    : m_initialized(false)
{
    InitializeDefaultErrorMessages();
    EnableDelay();
    CreateHubIDProperty();
}

NikonNiXYStage::~NikonNiXYStage()
{
}

bool NikonNiXYStage::Busy()
{
    return false;
}

int NikonNiXYStage::Initialize()
{
    NikonNiHub* pHub = static_cast<NikonNiHub*>(GetParentHub());
    if (pHub) {
        char hubLabel[MM::MaxStrLength];
        pHub->GetLabel(hubLabel);
        SetParentID(hubLabel);
    }
    else {
        LogMessage("Parent Hub not defined.");
    }

    if (m_initialized) return DEVICE_OK;

    // Name
    int ret = CreateStringProperty(MM::g_Keyword_Name, g_DeviceName_NikonNiXYStage, true);
    if (DEVICE_OK != ret) return ret;

    // Description
    ret = CreateStringProperty(MM::g_Keyword_Description, "XY Stage", true);
    if (DEVICE_OK != ret)
        return ret;


    ret = UpdateStatus();
    if (DEVICE_OK != ret) return ret;

    m_initialized = true;

    return DEVICE_OK;
}

int NikonNiXYStage::Shutdown()
{
    return 0;
}

void NikonNiXYStage::GetName(char* name) const
{
    CDeviceUtils::CopyLimitedString(name, g_DeviceName_NikonNiXYStage);
}

int NikonNiXYStage::GetLimitsUm(double& xMin, double& xMax, double& yMin, double& yMax)
{
    return 0;
}

int NikonNiXYStage::SetPositionSteps(long x, long y)
{
    return 0;
}

int NikonNiXYStage::GetPositionSteps(long& x, long& y)
{
    return 0;
}

int NikonNiXYStage::Home()
{
    return 0;
}

int NikonNiXYStage::Stop()
{
    return 0;
}

int NikonNiXYStage::SetOrigin()
{
    return 0;
}

int NikonNiXYStage::GetStepLimits(long& xMin, long& xMax, long& yMin, long& yMax)
{
#if SDK_PRESENT
    MIC_MetaData sMetaData;
    ZeroMemory(&sMetaData, sizeof(sMetaData));
    sMetaData.uiMetaDataUsageMask |= MIC_METADATA_MASK_XYStage_XRangePhys;
    sMetaData.uiMetaDataUsageMask |= MIC_METADATA_MASK_XYStage_YRangePhys;

    if (MIC_MetadataGet(sMetaData) != LX_OK) {
        LogMessage("Error: MIC_MetadataGet(sMetaData)", false);
    }
    else {
        xMin = sMetaData.iXYStage_XRangePhys[0];
        xMax = sMetaData.iXYStage_XRangePhys[1];
        yMin = sMetaData.iXYStage_YRangePhys[0];
        yMax = sMetaData.iXYStage_YRangePhys[1];
    }
#endif
    return 0;
}

double NikonNiXYStage::GetStepSizeXUm()
{
    return 0.0;
}

double NikonNiXYStage::GetStepSizeYUm()
{
    return 0.0;
}

int NikonNiXYStage::IsXYStageSequenceable(bool& isSequenceable) const
{
    return 0;
}

int NikonNiXYStage::SetPositionUm(double x, double y)
{
#if SDK_PRESENT
    lx_int32 iDevValueX;
    lx_int32 iDevValueY;

    MIC_Data sDataIn, sDataOut;
    ZeroMemory(&sDataIn, sizeof(sDataIn));
    ZeroMemory(&sDataOut, sizeof(sDataOut));

    MIC_Convert_Phys2Dev(MIC_DATA_MASK_XPOSITION, x, iDevValueX);
    MIC_Convert_Phys2Dev(MIC_DATA_MASK_YPOSITION, y, iDevValueY);

    sDataIn.uiDataUsageMask |= MIC_DATA_MASK_XPOSITION;
    sDataIn.uiDataUsageMask |= MIC_DATA_MASK_YPOSITION;

    sDataIn.iXPOSITION = iDevValueX;
    sDataIn.iYPOSITION = iDevValueY;

    if (MIC_DataSet(sDataIn, sDataOut, false) != LX_OK) {
        LogMessage("Was unable to change the position", false);
    }

#endif
    return DEVICE_OK;
}

int NikonNiXYStage::GetPositionUm(double& x, double& y)
{
#if SDK_PRESENT
    MIC_Data sData;
    ZeroMemory(&sData, sizeof(sData));
    sData.uiDataUsageMask |= MIC_DATA_MASK_XPOSITION;
    sData.uiDataUsageMask |= MIC_DATA_MASK_YPOSITION;

    if (MIC_DataGet(sData) != LX_OK)
        LogMessage("MIC_DataGet(sData) != LX_OK", false);
    else {
        MIC_Convert_Dev2Phys(MIC_DATA_MASK_XPOSITION, sData.iXPOSITION, x);
        MIC_Convert_Dev2Phys(MIC_DATA_MASK_YPOSITION, sData.iYPOSITION, y);
    }
#endif
    return DEVICE_OK;
}

int NikonNiXYStage::SetRelativePositionUm(double dx, double dy)
{
    double x, y;
    GetPositionUm(x, y);
    x += dx;
    y += dy;
    SetPositionUm(x, y);
    return DEVICE_OK;
}

