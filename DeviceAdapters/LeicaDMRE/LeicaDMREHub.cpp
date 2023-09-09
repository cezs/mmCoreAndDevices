///////////////////////////////////////////////////////////////////////////////
// FILE:          LeicaDMREHub.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   LeicaDMRxE hub module. Required for operation of all 
//                LeicaDMRxE devices
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

#define _CRT_SECURE_NO_DEPRECATE

#include "assert.h"
#include <memory.h>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include "LeicaDMREHub.h"
#include "LeicaDMRE.h"

#ifdef WIN32
#include <windows.h>
#endif

using namespace std;

LeicaDMREHub::LeicaDMREHub() :
    port_(""),
    initialized_(false)
{
    ClearRcvBuf();
}

LeicaDMREHub::~LeicaDMREHub()
{
    initialized_ = false;
}

int LeicaDMREHub::Initialize(MM::Device& device, MM::Core& core)
{
    if (initialized_)
        return DEVICE_OK;

    int ret = GetVersion(device, core, version_);
    if (ret != DEVICE_OK) {
        // some serial ports do not open correctly right away, so try once more:
        ret = GetVersion(device, core, version_);
        if (ret != DEVICE_OK)
            return ret;
    }

    ret = GetMicroscope(device, core, microscope_);
    if (ret != DEVICE_OK)
        return ret;

    std::ostringstream os;
    os << "Microscope type: " << microscope_;
    core.LogMessage(&device, os.str().c_str(), false);
    os.str("");
    os << "Firmware version: " << version_;
    core.LogMessage(&device, os.str().c_str(), false);

    initialized_ = true;

    return DEVICE_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Device commands
///////////////////////////////////////////////////////////////////////////////

int LeicaDMREHub::GetChecksum(MM::Device& device, MM::Core& core, std::string& checksum)
{
    return GetCommand(device, core, 24, checksum);
}

int LeicaDMREHub::GetVersion(MM::Device& device, MM::Core& core, std::string& version)
{
    return GetCommand(device, core, 25, version);
}

int LeicaDMREHub::GetMicroscope(MM::Device& device, MM::Core& core, std::string& microscope)
{
    int ret = GetCommand(device, core, 26, microscope);
    if (ret != DEVICE_OK) {
        // the DM RxE does not have this command and will time out
        microscope = "RxE";
        return DEVICE_OK;
    }
    else
        return !DEVICE_OK;
}

int LeicaDMREHub::SetManual(MM::Device& device, MM::Core& core, bool& manual)
{
    return SetCommand(device, core, manual ? 15 : 14);
}

// Nosepiece

int LeicaDMREHub::GetObjNosepieceId(MM::Device& device, MM::Core& core, int& id)
{
    return GetCommand(device, core, 20, id); // 0 - none, 1 - uncoded, 2 - with encoded position
}

int LeicaDMREHub::GetObjNosepiecePosition(MM::Device& device, MM::Core& core, int& pos)
{
    return GetCommand(device, core, 10, pos);
}

int LeicaDMREHub::GetObjNosepieceMagnification(MM::Device& device, MM::Core& core, int& mag)
{
    return GetCommand(device, core, 21, mag);
}

// Halogen Lamp

int LeicaDMREHub::GetLampIntensity(MM::Device& device, MM::Core& core, int& intensity)
{
    return GetCommand(device, core, 9, intensity);
}

int LeicaDMREHub::SetLampIntensity(MM::Device& device, MM::Core& core, int intensity)
{
    return SetCommand(device, core, 6, intensity);
}

// Z Drive

int LeicaDMREHub::SetZAbs(MM::Device& device, MM::Core& core, long position)
{
    return SetCommand(device, core, 1, (int)position);
}

int LeicaDMREHub::SetZRel(MM::Device& device, MM::Core& core, long position)
{
    return SetCommand(device, core, 2, (int)position);
}

int LeicaDMREHub::MoveZMax(MM::Device& device, MM::Core& core)
{
    return SetCommand(device, core, 17);
}

int LeicaDMREHub::MoveZMin(MM::Device& device, MM::Core& core)
{
    return SetCommand(device, core, 16);
}

int LeicaDMREHub::MoveZConst(MM::Device& device, MM::Core& core, int speed)
{
    return SetCommand(device, core, 4, speed); //TODO
}

int LeicaDMREHub::StopZ(MM::Device& device, MM::Core& core)
{
    return SetCommand(device, core, 6); //TODO
}

int LeicaDMREHub::GetZ(MM::Device& device, MM::Core& core, long& position)
{
    int pos;
    int ret = GetCommand(device, core, 8, pos);
    if (ret != DEVICE_OK)
        return ret;
    position = (long)pos;

    return DEVICE_OK;
}

int LeicaDMREHub::SetZUpperThreshold(MM::Device& device, MM::Core& core)
{
    return SetCommand(device, core, 19);
}

bool LeicaDMREHub::GetZLowerThreholdSet(MM::Device& device, MM::Core& core)
{
    std::string isSet;
    int ret = GetCommand(device, core, 10, isSet);
    if (ret != DEVICE_OK)
        return ret;
    return isSet.at(3) != '0';
}

bool LeicaDMREHub::GetZUpperThreholdSet(MM::Device& device, MM::Core& core)
{
    std::string isSet;
    int ret = GetCommand(device, core, 10, isSet);
    if (ret != DEVICE_OK)
        return ret;
    return isSet.at(2) != '0';
}

///////////////////////////////////////////////////////////////////////////////
// HUB generic methods
///////////////////////////////////////////////////////////////////////////////

/**
 * Clears the serial receive buffer.
 */
void LeicaDMREHub::ClearAllRcvBuf(MM::Device& device, MM::Core& core)
{
    // Read whatever has been received so far:
    unsigned long read = RCV_BUF_LENGTH;
    while (read == (unsigned long)RCV_BUF_LENGTH)
    {
        core.ReadFromSerial(&device, port_.c_str(), (unsigned char*)rcvBuf_, RCV_BUF_LENGTH, read);
    }
    // Delete it all:
    memset(rcvBuf_, 0, RCV_BUF_LENGTH);
}

void LeicaDMREHub::ClearRcvBuf()
{
    memset(rcvBuf_, 0, RCV_BUF_LENGTH);
}

/**
 * Sends serial command to the MMCore virtual serial port.
 * returns data from the microscope as a string
 */
int LeicaDMREHub::GetCommand(MM::Device& device, MM::Core& core, int command, std::string& answer)
{
    if (port_ == "")
        return ERR_PORT_NOT_SET;

    ClearAllRcvBuf(device, core);

    int deviceId = 50;

    // send command
    std::ostringstream os;
    os << std::setw(2) << std::setfill('0') << deviceId;
    os << std::setw(3) << std::setfill('0') << command;
    int ret = core.SetSerialCommand(&device, port_.c_str(), os.str().c_str(), "\r");
    if (ret != DEVICE_OK)
        return ret;

    char rcvBuf[RCV_BUF_LENGTH];
    ret = core.GetSerialAnswer(&device, port_.c_str(), RCV_BUF_LENGTH, rcvBuf, "\r");
    if (ret != DEVICE_OK)
        return ret;

    std::stringstream s1, s2, s3;
    int commandCheck, deviceIdCheck;
    s1 << rcvBuf[0] << rcvBuf[1];
    s1 >> deviceIdCheck;
    s2 << rcvBuf[2] << rcvBuf[3] << rcvBuf[4];
    s2 >> commandCheck;
    if ((deviceId != deviceIdCheck) || (command != commandCheck))
        return ERR_UNEXPECTED_ANSWER;

    if (strlen(rcvBuf) > 5)
        s3 << rcvBuf + 5;

    answer = s3.str();

    return DEVICE_OK;

}

/**
 * Sends serial command to the MMCore virtual serial port.
 * returns data from the microscope as an integer
 */
int LeicaDMREHub::GetCommand(MM::Device& device, MM::Core& core, int command, int& answer)
{
    std::string reply;
    int ret = GetCommand(device, core, command, reply);
    if (ret != DEVICE_OK)
        return ret;

    std::stringstream ss;
    ss << reply;
    ss >> answer;

    return DEVICE_OK;
}

/**
 * Sends serial command to the MMCore virtual serial port appended with data.
 */
int LeicaDMREHub::SetCommand(MM::Device& device, MM::Core& core, int command, int data)
{
    if (port_ == "")
        return ERR_PORT_NOT_SET;

    ClearAllRcvBuf(device, core);

    int deviceId = 50;

    // send command
    std::ostringstream os;
    os << std::setw(2) << std::setfill('0') << deviceId;
    os << std::setw(3) << std::setfill('0') << command << data;
    int ret = core.SetSerialCommand(&device, port_.c_str(), os.str().c_str(), "\r");
    if (ret != DEVICE_OK)
        return ret;

    char rcvBuf[RCV_BUF_LENGTH];
    ret = core.GetSerialAnswer(&device, port_.c_str(), RCV_BUF_LENGTH, rcvBuf, "\r");
    if (ret != DEVICE_OK)
        return ret;

    std::stringstream s1, s2, s3;
    int commandCheck, deviceIdCheck;
    s1 << rcvBuf[0] << rcvBuf[1];
    s1 >> deviceIdCheck;
    s2 << rcvBuf[2] << rcvBuf[3] << rcvBuf[4];
    s2 >> commandCheck;
    if ((deviceId != deviceIdCheck) || (command != commandCheck))
        return ERR_UNEXPECTED_ANSWER;

    // TODO: error checking in the answer

    return DEVICE_OK;
}


/**
 * Sends serial command to the MMCore virtual serial port.
 */
int LeicaDMREHub::SetCommand(MM::Device& device, MM::Core& core, int command)
{
    if (port_ == "")
        return ERR_PORT_NOT_SET;

    ClearAllRcvBuf(device, core);

    int deviceId = 50;

    // send command
    std::ostringstream os;
    os << std::setw(2) << std::setfill('0') << deviceId;
    os << std::setw(3) << std::setfill('0') << command;
    int ret = core.SetSerialCommand(&device, port_.c_str(), os.str().c_str(), "\r");
    if (ret != DEVICE_OK)
        return ret;

    char rcvBuf[RCV_BUF_LENGTH];
    ret = core.GetSerialAnswer(&device, port_.c_str(), RCV_BUF_LENGTH, rcvBuf, "\r");
    if (ret != DEVICE_OK)
        return ret;

    std::stringstream s1, s2, s3;
    int commandCheck, deviceIdCheck;
    s1 << rcvBuf[0] << rcvBuf[1];
    s1 >> deviceIdCheck;
    s2 << rcvBuf[2] << rcvBuf[3] << rcvBuf[4];
    s2 >> commandCheck;
    if ((deviceId != deviceIdCheck) || (command != commandCheck))
        return ERR_UNEXPECTED_ANSWER;

    // TODO: error checking in the answer

    return DEVICE_OK;
}
