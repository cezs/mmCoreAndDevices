///////////////////////////////////////////////////////////////////////////////
// FILE:          LeicaDMREHub.h
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


#ifndef _LeicaDMREHub_H_
#define _LeicaDMREHub_H_

#include "MMDevice.h"


class LeicaDMREHub
{
public:
    LeicaDMREHub();
    ~LeicaDMREHub();

    void SetPort(const char* port) { port_ = port; }
    int Initialize(MM::Device& device, MM::Core& core);
    int DeInitialize() { initialized_ = false; return DEVICE_OK; };
    bool Initialized() { return initialized_; };
    std::string Version() { return version_; };
    std::string Microscope() { return microscope_; };
    int SetManual(MM::Device& device, MM::Core& core, bool& manual);

    int GetLampIntensity(MM::Device& device, MM::Core& core, int& intensity);
    int SetLampIntensity(MM::Device& device, MM::Core& core, int intensity);
    bool LampPresent() { return true; };

    int SetZUpperThreshold(MM::Device& device, MM::Core& core);
    int SetZAbs(MM::Device& device, MM::Core& core, long position);
    int SetZRel(MM::Device& device, MM::Core& core, long position);
    int MoveZConst(MM::Device& device, MM::Core& core, int speed);
    int StopZ(MM::Device& device, MM::Core& core);
    int GetZ(MM::Device& device, MM::Core& core, long& position);
    bool ZDrivePresent() { return true; };
    int MoveZMin(MM::Device& device, MM::Core& core);
    int MoveZMax(MM::Device& device, MM::Core& core);
    bool GetZLowerThreholdSet(MM::Device& device, MM::Core& core);
    bool GetZUpperThreholdSet(MM::Device& device, MM::Core& core);


    int GetObjNosepieceId(MM::Device& device, MM::Core& core, int& id);
    int GetObjNosepiecePosition(MM::Device& device, MM::Core& core, int& pos);
    int GetObjNosepieceMagnification(MM::Device& device, MM::Core& core, int& mag);
    bool ObjNosepiecePresent() { return true; };

private:
    int GetChecksum(MM::Device& device, MM::Core& core, std::string& version);
    int GetVersion(MM::Device& device, MM::Core& core, std::string& version);
    int GetMicroscope(MM::Device& device, MM::Core& core, std::string& microscope);
    void ClearRcvBuf();
    void ClearAllRcvBuf(MM::Device& device, MM::Core& core);
    int GetCommand(MM::Device& device, MM::Core& core, int command, std::string& answer);
    int GetCommand(MM::Device& device, MM::Core& core, int command, int& answer);
    int SetCommand(MM::Device& device, MM::Core& core, int command, int data);
    int SetCommand(MM::Device& device, MM::Core& core, int command);

    static const int gMic_ = 50;

    static const int RCV_BUF_LENGTH = 1024;
    char rcvBuf_[RCV_BUF_LENGTH];

    std::string port_;
    std::string version_;
    std::string microscope_;
    bool initialized_;
};

#endif // _LeicaDMREHub_H_
