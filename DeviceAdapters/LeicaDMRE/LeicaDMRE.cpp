///////////////////////////////////////////////////////////////////////////////
// FILE:          LeicaDMRE.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   LeicaDMRE hub module. Required for operation of the Leica DM RE,
//                DM RXE, DM RME and DM IRBE.
//
//                In this device adapter we use the "General microscope" set
//                of serial commands without addressing individual components with
//                a different interface controller ID as in the LeiceDMR device adapater.
// 
//                For details, please refer to the Leica's "The serial interface 
//                for the stands DM Rxx and DM IRBE" (1998).
//
// LICENSE:       This library is free software; you can redistribute it and/or
//                modify it under the terms of the GNU Lesser General Public
//                License as published by the Free Software Foundation.
//                
//                You should have received a copy of the GNU Lesser General Public
//                License along with the source distribution; if not, write to
//                the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
//                Boston, MA  02111-1307  USA
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,                   
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.           
//
// AUTHOR:        github.com/cezs, 08/07/2023

#include "LeicaDMRE.h"
#include "LeicaDMREHub.h"
#include <cstdio>
#include <string>
#include <math.h>
#include "ModuleInterface.h"

using namespace std;

// Device strings
const char* g_LeicaDMREHub = "Leica DM microscope";
const char* g_LeicaDMRELamp = "Halogen Lamp";
const char* g_LeicaDMREZDrive = "Z Drive";
const char* g_LeicaDMREObjNosepiece = "Objective Nosepiece";

// Property strings
const char* g_Threshold = "Threshold";
const char* g_Set = "Set";
const char* g_Update = "Update";

const char* g_OperatingMode = "Operating Mode";
const char* g_ImmMode = "Immersion";
const char* g_DryMode = "Dry";

const char* g_RotationMode = "Rotation Mode";
const char* g_LowerMode = "Lower";
const char* g_NoLowerMode = "Do not lower";

const char* g_Break = "Interrupt";
const char* g_On = "Now";
const char* g_Off = " ";

const char* g_Condensor = "Condensor Top";
const char* g_In = "In";
const char* g_Out = "Out";
const char* g_Undefined = "Undefined";

// global hub object, very important!
LeicaDMREHub g_hub;

///////////////////////////////////////////////////////////////////////////////
// Exported MMDevice API
///////////////////////////////////////////////////////////////////////////////
MODULE_API void InitializeModuleData()
{
    RegisterDevice(g_LeicaDMREHub, /*MM::HubDevice*/ MM::GenericDevice, "LeicaDM (RE, RXE, RME, RBE) Controller");
    RegisterDevice(g_LeicaDMRELamp, MM::ShutterDevice, "Halogen Lamp");
    RegisterDevice(g_LeicaDMREZDrive, MM::StageDevice, "Z Drive");
    RegisterDevice(g_LeicaDMREObjNosepiece, MM::StateDevice, "Objective Nosepiece");
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
    if (deviceName == 0)
        return 0;

    if (strcmp(deviceName, g_LeicaDMREHub) == 0) {
        return new Hub();
    }
    else if (strcmp(deviceName, g_LeicaDMRELamp) == 0) {
        return new Lamp();
    }
    else if (strcmp(deviceName, g_LeicaDMREZDrive) == 0) {
        return new ZStage;
    }
    else if (strcmp(deviceName, g_LeicaDMREObjNosepiece) == 0) {
        return new ObjNosepiece;
    }

    return 0;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
    delete pDevice;
}

///////////////////////////////////////////////////////////////////////////////
// LeicaDMRE Hub
///////////////////////////////////////////////////////////////////////////////

Hub::Hub() :
    initialized_(false),
    port_("Undefined")
{
    InitializeDefaultErrorMessages();

    // custom error messages
    SetErrorText(ERR_COMMAND_CANNOT_EXECUTE, "Command cannot be executed");
    SetErrorText(ERR_NO_ANSWER, "No answer received.  Is the Leica microscope connected to the correct serial port and switched on?");
    SetErrorText(ERR_NOT_CONNECTED, "No answer received.  Is the Leica microscope connected to the correct serial port and switched on?");

    // create pre-initialization properties
    // ------------------------------------

    // Port
    CPropertyAction* pAct = new CPropertyAction(this, &Hub::OnPort);
    CreateProperty(MM::g_Keyword_Port, "Undefined", MM::String, false, pAct, true);

}

Hub::~Hub()
{
    Shutdown();
}

void Hub::GetName(char* name) const
{
    CDeviceUtils::CopyLimitedString(name, g_LeicaDMREHub);
}

bool Hub::Busy()
{
    return false;
}

int Hub::Initialize()
{
    int ret;
    if (!g_hub.Initialized()) {
        ret = g_hub.Initialize(*this, *GetCoreCallback());
        if (ret != DEVICE_OK)
            return ret;
    }

    // set property list
    // -----------------

    // Name
    ret = CreateProperty(MM::g_Keyword_Name, g_LeicaDMREHub, MM::String, true);
    if (DEVICE_OK != ret)
        return ret;

    // Description
    ret = CreateProperty(MM::g_Keyword_Description, "LeicaDMRxE controller", MM::String, true);
    if (DEVICE_OK != ret)
        return ret;

    // Version
    std::string version = g_hub.Version();
    ret = CreateProperty("Firmware version", version.c_str(), MM::String, true);
    if (DEVICE_OK != ret)
        return ret;

    // Microscope
    std::string microscope = g_hub.Microscope();
    ret = CreateProperty("Microscope", microscope.c_str(), MM::String, true);
    if (DEVICE_OK != ret)
        return ret;

    // We might also get the available pieces of hardware at this point

    ret = UpdateStatus();
    if (ret != DEVICE_OK)
        return ret;

    initialized_ = true;

    return DEVICE_OK;
}

int Hub::Shutdown()
{
    if (initialized_) {
        initialized_ = false;
        g_hub.DeInitialize();
    }
    return DEVICE_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Action handlers
///////////////////////////////////////////////////////////////////////////////
/*
 * Sets the Serial Port to be used.
 * Should be called before initialization
 */
int Hub::OnPort(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (eAct == MM::BeforeGet)
    {
        pProp->Set(port_.c_str());
    }
    else if (eAct == MM::AfterSet)
    {
        if (initialized_)
        {
            // revert
            pProp->Set(port_.c_str());
            //return ERR_PORT_CHANGE_FORBIDDEN;
        }

        pProp->Get(port_);
        g_hub.SetPort(port_.c_str());
    }
    return DEVICE_OK;
}


///////////////////////////////////////////////////////////////////////////////
// LeicaDMRE Lamp
///////////////////////////////////////////////////////////////////////////////
Lamp::Lamp() :
    initialized_(false),
    name_(g_LeicaDMRELamp),
    open_(false),
    changedTime_(0.0)
{
    InitializeDefaultErrorMessages();
    SetErrorText(ERR_DEVICE_NOT_FOUND, "No Lamp found in this microscope");
    SetErrorText(ERR_PORT_NOT_SET, "No serial port found.  Did you include the Leica DM microscope and set its serial port property?");

    // Todo: Add custom messages

    // EnableDelay();
}

Lamp::~Lamp()
{
    Shutdown();
}

void Lamp::GetName(char* name) const
{
    assert(name_.length() < CDeviceUtils::GetMaxStringLength());
    CDeviceUtils::CopyLimitedString(name, name_.c_str());
}

int Lamp::Initialize()
{
    int ret;
    if (!g_hub.Initialized()) {
        ret = g_hub.Initialize(*this, *GetCoreCallback());
        if (ret != DEVICE_OK)
            return ret;
    }

    // Name
    ret = CreateProperty(MM::g_Keyword_Name, name_.c_str(), MM::String, true);
    if (DEVICE_OK != ret)
        return ret;

    // Description
    ret = CreateProperty(MM::g_Keyword_Description, "LeicaDMRxE Lamp", MM::String, true);
    if (DEVICE_OK != ret)
        return ret;

    // Set timer for the Busy signal, or we'll get a time-out the first time we check the state of the shutter, for good measure, go back 5s into the past
    changedTime_ = GetCurrentMMTime() - MM::MMTime::fromSeconds(5);

    // Check current intensity of lamp
    ret = g_hub.GetLampIntensity(*this, *GetCoreCallback(), intensity_);
    if (DEVICE_OK != ret)
        return ret;
    // The following seemed a good idea, but ends up being annoying
    /*
    if (intensity_ > 0)
       open_ = true;
    */

    // State
    CPropertyAction* pAct = new CPropertyAction(this, &Lamp::OnState);
    if (intensity_ > 0)
        ret = CreateProperty(MM::g_Keyword_State, "1", MM::Integer, false, pAct);
    else
        ret = CreateProperty(MM::g_Keyword_State, "0", MM::Integer, false, pAct);

    if (ret != DEVICE_OK)
        return ret;

    AddAllowedValue(MM::g_Keyword_State, "0"); // Closed
    AddAllowedValue(MM::g_Keyword_State, "1"); // Open

    // Intensity
    pAct = new CPropertyAction(this, &Lamp::OnIntensity);
    CreateProperty("Intensity", "0", MM::Integer, false, pAct);
    SetPropertyLimits("Intensity", 0, 255);

    EnableDelay();

    ret = UpdateStatus();
    if (ret != DEVICE_OK)
        return ret;

    initialized_ = true;

    return DEVICE_OK;
}

bool Lamp::Busy()
{
    MM::MMTime interval = GetCurrentMMTime() - changedTime_;
    return interval < MM::MMTime::fromMs(GetDelayMs());
}

int Lamp::Shutdown()
{
    if (initialized_)
    {
        initialized_ = false;
    }
    return DEVICE_OK;
}

int Lamp::SetOpen(bool open)
{
    if (open) {
        int ret = g_hub.SetLampIntensity(*this, *GetCoreCallback(), intensity_);
        if (ret != DEVICE_OK)
            return ret;
        open_ = true;
    }
    else {
        int ret = g_hub.SetLampIntensity(*this, *GetCoreCallback(), 0);
        if (ret != DEVICE_OK)
            return ret;
        open_ = false;
    }
    changedTime_ = GetCurrentMMTime();

    return DEVICE_OK;
}

int Lamp::GetOpen(bool& open)
{
    open = open_;
    return DEVICE_OK;
}

int Lamp::Fire(double /*deltaT*/)
{
    return DEVICE_UNSUPPORTED_COMMAND;
}

///////////////////////////////////////////////////////////////////////////////
// Action handlers                                                           
///////////////////////////////////////////////////////////////////////////////

int Lamp::OnState(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (eAct == MM::BeforeGet)
    {
        // return pos as we know it
        bool open;
        GetOpen(open);
        if (open)
        {
            pProp->Set(1L);
        }
        else
        {
            pProp->Set(0L);
        }
    }
    else if (eAct == MM::AfterSet)
    {
        int ret;
        long pos;
        pProp->Get(pos);
        if (pos == 1) {
            ret = this->SetOpen(true);
        }
        else {
            ret = this->SetOpen(false);
        }
        if (ret != DEVICE_OK)
            return ret;
        pProp->Set(pos);
    }
    return DEVICE_OK;
}


int Lamp::OnIntensity(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (eAct == MM::BeforeGet) {
        if (open_) {
            int ret = g_hub.GetLampIntensity(*this, *GetCoreCallback(), intensity_);
            if (ret != DEVICE_OK)
                return ret;
        }
        else {
            // shutter is closed.  Return the cached value
            // TODO: check if user increased brightness
        }
        pProp->Set((long)intensity_);
    }
    else if (eAct == MM::AfterSet) {
        long intensity;
        pProp->Get(intensity);
        intensity_ = (int)intensity;
        if (open_) {
            int ret = g_hub.SetLampIntensity(*this, *GetCoreCallback(), intensity_);
            if (ret != DEVICE_OK)
                return ret;
        }
    }

    return DEVICE_OK;
}

///////////////////////////////////////////////////////////////////////////////
// LeicaDMRE ZDrive 
///////////////////////////////////////////////////////////////////////////////
ZStage::ZStage() :
    stepSize_um_(0.1),
    initialized_(false),
    lowerLimit_(0.0),
    upperLimit_(25000.0),
    name_(g_LeicaDMREZDrive)
{
    InitializeDefaultErrorMessages();
    SetErrorText(ERR_DEVICE_NOT_FOUND, "No Z-Drive found in this microscope");
    SetErrorText(ERR_PORT_NOT_SET, "No serial port found.  Did you include the Leica DM microscope and set its serial port property?");
}

ZStage::~ZStage()
{
    Shutdown();
}

int ZStage::Shutdown()
{
    initialized_ = false;
    return DEVICE_OK;
}

void ZStage::GetName(char* name) const
{
    assert(name_.length() < CDeviceUtils::GetMaxStringLength());
    CDeviceUtils::CopyLimitedString(name, name_.c_str());
}

int ZStage::Initialize()
{
    int ret;
    if (!g_hub.Initialized()) {
        ret = g_hub.Initialize(*this, *GetCoreCallback());
        if (ret != DEVICE_OK)
            return ret;
    }

    // Name
    ret = CreateProperty(MM::g_Keyword_Name, name_.c_str(), MM::String, true);
    if (DEVICE_OK != ret)
        return ret;

    // Description
    ret = CreateProperty(MM::g_Keyword_Description, "Leica DM RXE/RME/RBE/RE Z Drive", MM::String, true);
    if (DEVICE_OK != ret)
        return ret;

    // Set timer for the Busy signal, or we'll get a time-out the first time we check the state of the shutter, for good measure, go back 5s into the past
    changedTime_ = GetCurrentMMTime() - MM::MMTime::fromSeconds(5);

    // Position
    // There are two reference frames.  An absolute reference frame (implemeted here)
    // and a relative reference frame, implemented with a upper and lower lower limit
    // The display on the DMRxE shows the difference with the upper limit
    //lowerLimit_ = 25000.0;


    // To implement the relative reference frame we need the upper threshold
    //g_hub.GetZUpperThreshold(*this, *GetCoreCallback(), upperThreshold_);

    // Allow user to update threshold
    CPropertyAction* pAct = new CPropertyAction(this, &ZStage::OnThreshold);
    ret = CreateProperty(g_Threshold, g_Set, MM::String, false, pAct);
    if (ret != DEVICE_OK)
        return ret;
    AddAllowedValue(g_Threshold, g_Set);
    AddAllowedValue(g_Threshold, g_Update);


    // Do not implement the position property as it can lead to trouble
    
    //pAct = new CPropertyAction (this, &ZStage::OnPosition);
    //ret = CreateProperty("Position", "0", MM::Float, false, pAct);
    //if (ret != DEVICE_OK)
    //   return ret;
    //SetPropertyLimits("Position", lowerLimit_, upperLimit_);
    

    //// Emergency stop
    //pAct = new CPropertyAction(this, &ZStage::OnStop);
    //ret = CreateProperty("Emergency Stop", "Off", MM::String, false, pAct);
    //if (ret != DEVICE_OK)
    //    return ret;
    //AddAllowedValue("Emergency Stop", "Off");
    //AddAllowedValue("Emergency Stop", "Apply Now!");

    ret = UpdateStatus();
    if (ret != DEVICE_OK)
        return ret;

    initialized_ = true;

    return DEVICE_OK;
}

bool ZStage::Busy()
{
    MM::MMTime interval = GetCurrentMMTime() - changedTime_;
    return interval < MM::MMTime::fromMs(GetDelayMs());
}

int ZStage::SetPositionUm(double position)
{
    long positionSteps = (long)(position / stepSize_um_);
    //if (upperThreshold_ != -1)
    //    positionSteps = upperThreshold_ + positionSteps;

    return SetPositionSteps(positionSteps);
}

int ZStage::SetRelativePositionUm(double position)
{
    long positionSteps = (long)(position / stepSize_um_);

    return SetRelativePositionSteps(positionSteps);
}

int ZStage::GetPositionUm(double& position)
{
    long positionSteps;
    int ret = GetPositionSteps(positionSteps);
    if (ret != DEVICE_OK)
        return ret;
    //if (upperThreshold_ != -1)
    //    positionSteps = positionSteps - upperThreshold_;
    position = (double)positionSteps * stepSize_um_;

    return DEVICE_OK;
}

int ZStage::SetPositionSteps(long position)
{
    return g_hub.SetZAbs(*this, *GetCoreCallback(), position);
}

int ZStage::SetRelativePositionSteps(long position)
{
    return g_hub.SetZRel(*this, *GetCoreCallback(), position);
}

int ZStage::GetPositionSteps(long& position)
{
    return g_hub.GetZ(*this, *GetCoreCallback(), position);
}

int ZStage::SetOrigin()
{
    return DEVICE_OK;
}

///*
// * Assume that the parameter is in um/sec!
// */
//int ZStage::Move(double speed)
//{
//    int speedNumber;
//    double  minSpeed, maxSpeed;
//    if (g_hub.Microscope() == "DMRXA" || g_hub.Microscope() == "DMRA") {
//        minSpeed = 18.4;
//        maxSpeed = 4700;
//    }
//    else {
//        minSpeed = 4.6;
//        maxSpeed = 1175;
//    }
//    speedNumber = (int)(speed / maxSpeed * 255);
//    if (speedNumber > 255)
//        speedNumber = 255;
//    if (speedNumber < -255)
//        speedNumber = -255;
//
//    return g_hub.MoveZConst(*this, *GetCoreCallback(), speedNumber);
//}

int ZStage::OnPosition(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (eAct == MM::BeforeGet) {
        double pos;
        int ret = GetPositionUm(pos);
        if (ret != DEVICE_OK)
            return ret;
        pProp->Set(pos);
    }
    else if (eAct == MM::AfterSet) {
        double pos;
        pProp->Get(pos);
        int ret = SetPositionUm(pos);
        if (ret != DEVICE_OK)
            return ret;
    }

    return DEVICE_OK;
}

//int ZStage::OnStop(MM::PropertyBase* pProp, MM::ActionType eAct)
//{
//    if (eAct == MM::BeforeGet) {
//        pProp->Set("Off");
//    }
//    else if (eAct == MM::AfterSet) {
//        std::string value;
//        pProp->Get(value);
//        if (value == "Apply Now!") {
//            g_hub.StopZ(*this, *GetCoreCallback());
//        }
//    }
//    return DEVICE_OK;
//}

int ZStage::OnThreshold(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (eAct == MM::BeforeGet) {
        pProp->Set(g_Set);
    }
    else if (eAct == MM::AfterSet) {
        std::string value;
        pProp->Get(value);
        if (value == g_Update) {
            g_hub.SetZUpperThreshold(*this, *GetCoreCallback());
        }
    }
    return DEVICE_OK;
}


///////////////////////////////////////////////////////////////////////////////
// LeicaDMRE Objective Nosepiece
///////////////////////////////////////////////////////////////////////////////
ObjNosepiece::ObjNosepiece() :
    initialized_(false),
    name_(g_LeicaDMREObjNosepiece),
    pos_(0),
    numPos_(7) // assume septuple
{
    InitializeDefaultErrorMessages();

    // Todo: Add custom messages
    SetErrorText(ERR_INVALID_POSITION, "Objective nosepiece reports an invalid position. Is it clicked into position correctly?");
    SetErrorText(ERR_DEVICE_NOT_FOUND, "No objective nosepiece in this microscope.");
    SetErrorText(ERR_OBJECTIVE_SET_FAILED, "Failed changing objectives.  Is the Immersion mode appropriate for the new objective?");


    //
    // create pre-initialization properties
    // ------------------------------------

}


ObjNosepiece::~ObjNosepiece()
{
    Shutdown();
}

void ObjNosepiece::GetName(char* name) const
{
    assert(name_.length() < CDeviceUtils::GetMaxStringLength());
    CDeviceUtils::CopyLimitedString(name, name_.c_str());
}

int ObjNosepiece::Initialize()
{
    int ret;
    if (!g_hub.Initialized()) {
        ret = g_hub.Initialize(*this, *GetCoreCallback());
        if (ret != DEVICE_OK)
            return ret;
    }

    // Name
    ret = CreateProperty(MM::g_Keyword_Name, name_.c_str(), MM::String, true);
    if (DEVICE_OK != ret)
        return ret;

    // Description
    ret = CreateProperty(MM::g_Keyword_Description, "LeicaDMRE Objective Nosepiece", MM::String, true);
    if (DEVICE_OK != ret)
        return ret;

    // Find current position to avoid initialisation issues
    int cpos;
    ret = g_hub.GetObjNosepiecePosition(*this, *GetCoreCallback(), cpos);
    if (ret != DEVICE_OK)
        return ret;
    //if (cpos == 0)
    //    return ERR_INVALID_POSITION;
    char cstate[8];
    sprintf(cstate, "%dx", (long)(cpos - 1));

    // State
    CPropertyAction* pAct = new CPropertyAction(this, &ObjNosepiece::OnState);
    ret = CreateProperty(MM::g_Keyword_State, cstate, MM::Integer, true, pAct);
    if (ret != DEVICE_OK)
        return ret;

    //// Get the number of Objectives
    //ret = g_hub.GetObjNosepieceNumberOfPositions(*this, *GetCoreCallback(), numPos_);
    //if (ret != DEVICE_OK)
    //    return ret;
    //char pos[3];
    //for (int i = 0; i < numPos_; i++)
    //{
    //    sprintf(pos, "%d", i);
    //    AddAllowedValue(MM::g_Keyword_State, pos);
    //}

    // Label
    char clabel[8];
    int cmag;
    pAct = new CPropertyAction(this, &CStateBase::OnLabel);
    ret = g_hub.GetObjNosepieceMagnification(*this, *GetCoreCallback(), cmag);
    if (ret != DEVICE_OK)
        return ret;
    sprintf(clabel, "%dx", cmag);
    ret = CreateProperty(MM::g_Keyword_Label, clabel, MM::String, true, pAct);
    if (ret != DEVICE_OK)
        return ret;

    // create default positions and labels
    SetPositionLabel(0, "Undefined");
    for (int i = 1; i < numPos_; i++)
    {
        char label[8];
        int mag;
        ret = g_hub.GetObjNosepieceMagnification(*this, *GetCoreCallback(), mag);
        //if (ret != DEVICE_OK)
        //    return ret;
        sprintf(label, "%dx", i);
        SetPositionLabel(i, label);
    }

    //pAct = new CPropertyAction(this, &ObjNosepiece::OnImmersionMode);
    //ret = CreateProperty(g_OperatingMode, g_DryMode, MM::String, false, pAct);
    //if (ret != DEVICE_OK)
    //    return ret;
    //AddAllowedValue(g_OperatingMode, g_ImmMode);
    //AddAllowedValue(g_OperatingMode, g_DryMode);

    //pAct = new CPropertyAction(this, &ObjNosepiece::OnRotationMode);
    //ret = CreateProperty(g_RotationMode, g_LowerMode, MM::String, false, pAct);
    //if (ret != DEVICE_OK)
    //    return ret;
    //AddAllowedValue(g_RotationMode, g_LowerMode);
    //AddAllowedValue(g_RotationMode, g_NoLowerMode);

    ret = UpdateStatus();
    if (ret != DEVICE_OK)
        return ret;

    initialized_ = true;

    return DEVICE_OK;
}

bool ObjNosepiece::Busy()
{
    return false;
}

int ObjNosepiece::Shutdown()
{
    if (initialized_)
    {
        initialized_ = false;
    }
    return DEVICE_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Action handlers                                                           
///////////////////////////////////////////////////////////////////////////////


int ObjNosepiece::OnState(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (eAct == MM::BeforeGet) {
        int pos;
        int ret = g_hub.GetObjNosepiecePosition(*this, *GetCoreCallback(), pos);
        if (ret != DEVICE_OK)
            return ret;
        if (pos == 0)
            return ERR_INVALID_POSITION;
        pProp->Set((long)pos - 1);
    }
    else if (eAct == MM::AfterSet) {
        long pos;
        //int ret;
        pProp->Get(pos);
        // sanity check
        if (pos < 0)
            pos = 0;
        if (pos >= numPos_)
            pos = numPos_ - 1;
        if (pos == pos_)
            return DEVICE_OK;
        /*
        // lower the stage by 4 cm if so requested (funny that this is not done in the firmware)
        char mode[MM::MaxStrLength];
        GetProperty(g_RotationMode, mode);
        if (strcmp(mode, g_LowerMode) == 0)
           g_hub.SetZRel(*this, *GetCoreCallback(), -40000);
        */
        // Actual nosepiece move
        //ret = g_hub.SetObjNosepiecePosition(*this, *GetCoreCallback(), pos + 1);
        // return focus if needed
        /*
        if (strcmp(mode, g_LowerMode) == 0)
           g_hub.SetZRel(*this, *GetCoreCallback(), 40000);
        */
        //if (ret != DEVICE_OK)
        //    return ERR_OBJECTIVE_SET_FAILED;

        pos_ = pos;
        pProp->Set(pos_);
    }
    return DEVICE_OK;
}

//
//int ObjNosepiece::OnImmersionMode(MM::PropertyBase* pProp, MM::ActionType eAct)
//{
//    if (eAct == MM::BeforeGet) {
//        int mode;
//        int ret = g_hub.GetObjNosepieceImmMode(*this, *GetCoreCallback(), mode);
//        if (ret != DEVICE_OK)
//            return ret;
//        if (mode == 0)
//            pProp->Set(g_ImmMode);
//        else
//            pProp->Set(g_DryMode);
//    }
//    else if (eAct == MM::AfterSet) {
//        std::string mode;
//        pProp->Get(mode);
//        if (mode == g_ImmMode)
//            return g_hub.SetObjNosepieceImmMode(*this, *GetCoreCallback(), 0);
//        else
//            return g_hub.SetObjNosepieceImmMode(*this, *GetCoreCallback(), 1);
//    }
//
//    return DEVICE_OK;
//}
