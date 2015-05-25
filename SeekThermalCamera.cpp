#include "SeekThermalCamera.h"
#include <libusb-1.0/libusb.h>

#include <iostream>

//TODO:
//Should remove this include after removing the debug statement in grabFrame()
#include <iostream>

#define CONTROL_DIRECTION_IN    0x80

float map(float val, float istart, float istop, float ostart, float ostop)
{
	return ostart + (ostop - ostart) * ((val - istart) / (istop - istart));
}


void createBitmap(uint8_t *rgb_array, ThermalFrame* frame, bool scaled)
{
    uint32_t c = 0;
    for(uint32_t i = 0; i < MAX_THERMAL_PIXELS; ++i)
    {
        int v = frame->getData()[i];
        /*if(scaled)
        {
            uint16_t minvalue = frame->getMin();
            uint16_t maxvalue = frame->getMax();

            v = (v - minvalue) * 255 / (maxvalue - minvalue);

            if(v < 0)
                v = 0;
            else if(v > 255)
                v = 255;
        }
        else
        {
            uint16_t val = (float)v;
	    if (v < 32000 || v > 32400) v = 0;
            else v = val;
        }*/

	uint16_t val = (float)v;
        if (v < 32000 || v > 32400) v = 0;
        else v = val;

        rgb_array[c++] = (uint8_t)v;	//Set red
        rgb_array[c++] = (uint8_t)v;	//Set green
        rgb_array[c++] = (uint8_t)v;	//Set blue
    }
	//std::cout << maxTempPixel << std::endl;
	//std::cout << map(maxTemp, 0, 65535, -40, 330) - 75 << std::endl;
}

const char* SeekThermalCamera_Exception_Strings[] =
{
	"Camera Not Connected",
	"Data Error",
	"Could Not Init LibUSB Context"
};

const char* getSeekThermalCameraExceptionString(SeekThermalCamera_Exception e)
{
    return SeekThermalCamera_Exception_Strings[(uint32_t)e];
}


SeekThermalCamera::SeekThermalCamera()
{
	seek = nullptr;

	timeout_value = 1000;

	curFrame = 0;

	//Create the libusb library session.
	int retval = libusb_init(&ctx);

	//Check for problems initializuing the libusb context.
	if(retval < 0)
		throw SeekThermalCamera_Exception::Could_Not_Init_LibUSB_Context;
}

SeekThermalCamera::~SeekThermalCamera()
{
	if(seek)
	{
		//This will deinitialize and delete the device handle, setting its pointer to null.
		deinitialize();
	}

	if(ctx)
	{
		//Close libusb library session.
		libusb_exit(ctx);

		ctx = nullptr;
	}
}

void SeekThermalCamera::sendDeinit()
{
	uint8_t out[] = { 0x00, 0x00 };
	libusb_control_transfer(seek, 0x41, 0x3c, 0, 0, out, 2, timeout_value);
}

void SeekThermalCamera::deinitialize()
{
	if(seek)
	{
		sendDeinit();

		libusb_close(seek);

		seek = nullptr;
	}
}

void SeekThermalCamera::initialize()
{
	//Check to ensure camera is not already
	if(!seek)
	{
		//Attempt to open a device handle.
		seek = libusb_open_device_with_vid_pid(ctx, 10397, 16);

		//If we fail, the camera is likely not connected.
		if(!seek)
			throw SeekThermalCamera_Exception::Camera_Not_Connected;

		unsigned char arr[] = {0x01};

		//Keep trying!
		while(libusb_control_transfer(seek, 0x41, 0x54, 0, 0, arr, 1, timeout_value) != 1)
		{
			sendDeinit();
		}


		//Send bytes
		unsigned char arr2[] = { 0x00, 0x00 };
		libusb_control_transfer(seek, 0x41, 0x3c, 0, 0, arr2, 2, timeout_value);

		//Receive bytes
		uint8_t data1[4];
		libusb_control_transfer(seek, 0xC1 | CONTROL_DIRECTION_IN, 0x4e, 0, 0, data1, 4, timeout_value);

		//Receive bytes
		uint8_t data2[12];
		libusb_control_transfer(seek, 0xC1 | CONTROL_DIRECTION_IN, 0x36, 0, 0, data2, 12, timeout_value);

		//Send bytes
		unsigned char arr3[] = { 0x20, 0x00, 0x30, 0x00, 0x00, 0x00 };
		libusb_control_transfer(seek, 0x41, 0x56, 0, 0, arr3, 6, timeout_value);

		//Receive bytes
		uint8_t data3[0x40];
		libusb_control_transfer(seek, 0xC1 | CONTROL_DIRECTION_IN, 0x58, 0, 0, data3, 0x40, timeout_value);

		//Send bytes
		unsigned char arr4[] = { 0x20, 0x00, 0x50, 0x00, 0x00, 0x00 };
		libusb_control_transfer(seek, 0x41, 0x56, 0, 0, arr4, 6, timeout_value);

		//Receive bytes
		uint8_t data4[0x40];
		libusb_control_transfer(seek, 0xC1 | CONTROL_DIRECTION_IN, 0x58, 0, 0, data4, 0x40, timeout_value);

		//Send bytes
		unsigned char arr5[] = { 0x0C, 0x00, 0x70, 0x00, 0x00, 0x00 };
		libusb_control_transfer(seek, 0x41, 0x56, 0, 0, arr5, 6, timeout_value);

		//Receive bytes
		uint8_t data5[0x18];
		libusb_control_transfer(seek, 0xC1 | CONTROL_DIRECTION_IN, 0x58, 0, 0, data5, 0x18, timeout_value);

		//Send bytes
		unsigned char arr6[] = { 0x06, 0x00, 0x08, 0x00, 0x00, 0x00 };
		libusb_control_transfer(seek, 0x41, 0x56, 0, 0, arr6, 6, timeout_value);

		//Receive bytes
		uint8_t data6[0x0c];
		libusb_control_transfer(seek, 0xC1 | CONTROL_DIRECTION_IN, 0x58, 0, 0, data6, 0x0c, timeout_value);

		//Send bytes
		unsigned char arr7[] = { 0x08, 0x00 };
		libusb_control_transfer(seek, 0x41, 0x3E, 0, 0, arr7, 2, timeout_value);

		//Receive bytes
		uint8_t data7[2];
		libusb_control_transfer(seek, 0xC1 | CONTROL_DIRECTION_IN, 0x3D, 0, 0, data7, 2, timeout_value);


		//Send bytes
		unsigned char arr8[] = { 0x08, 0x00 };
		libusb_control_transfer(seek, 0x41, 0x3E, 0, 0, arr8, 2, timeout_value);

		//Send bytes
		unsigned char arr9[] = { 0x01, 0x00 };
		libusb_control_transfer(seek, 0x41, 0x3C, 0, 0, arr9, 2, timeout_value);

		//Receive bytes
		uint8_t data8[2];
		libusb_control_transfer(seek, 0xC1 | CONTROL_DIRECTION_IN, 0x3D, 0, 0, data8, 2, timeout_value);
	}
}

uint32_t SeekThermalCamera::grabFrame(uint8_t *data, uint32_t size)
{
	uint8_t out[] = { 0xc0, 0x7e, 0, 0 };
	
    libusb_control_transfer(seek, 0x41, 0x53, 0, 0, out, 4, timeout_value);

    ////device.ReadExactPipe(0x81, 0x7ec0 * 2);

    //uint8_t frame[0x7ec0 * 2];

	//Size should be (0x7ec0 * 2).
	
	uint32_t bytes = 0;

	while(bytes < MAX_THERMAL_PIXELS * sizeof(uint16_t))
	{
		int numBytes = 0;
		libusb_bulk_transfer(seek, 0x81, data + bytes, size - bytes, &numBytes, timeout_value);
		bytes += numBytes;
	}

    std::cout << "Got frame! (" << bytes << " bytes)\n";

	return bytes;
}

//========================================================

ThermalFrame* SeekThermalCamera::getFrame()
{
    do
    {
        //Get a frame.
        grabFrame((uint8_t*)frames[curFrame].getData(), MAX_THERMAL_PIXELS * sizeof(uint16_t));

        //Loop until we've got a valid calibration frame and a usable frame.
        switch(frames[curFrame].getFrameType())
        {
            case Frame_Type::Calibration_Frame:
                calibrationFrame = &frames[curFrame];

                curFrame++;
                curFrame %= 2;

                lastFrame = nullptr;

				std::cout << "Got calibration frame!\n";
				
                break;

            case Frame_Type::Usable_Frame:
                lastFrame = &frames[curFrame];

                //Use calibration frame if available.
                if(calibrationFrame)
                {
                    //Subtract calibration frame data.
                    for(uint32_t i = 0; i < MAX_THERMAL_PIXELS; ++i)
                    {
                        int v = lastFrame->getData()[i];
                        int c = calibrationFrame->getData()[i];

                        v = v - c + 0x8000;

                        if(v < 0)
                            v = 0;
                        else if(v > 0xFFFF)
                            v = 0xFFFF;

                        lastFrame->getData()[i] = (uint16_t)v;
                    }
                }
				else
					std::cout << "Did not have a calibration frame but received usable frame!\n";
        }
    }
    while(lastFrame == nullptr || calibrationFrame == nullptr);
	
	std::cout << "Created frame.\n";
	
    return lastFrame;
}
