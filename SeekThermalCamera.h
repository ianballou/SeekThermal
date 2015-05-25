#pragma once
#ifndef _SEEK_THERMAL_LIB_H
#define _SEEK_THERMAL_LIB_H

#include <cstdint>
#include <cstddef>

#define MAX_THERMAL_PIXELS  (0x7ec0)

//Forward declarations
typedef struct libusb_context libusb_context;
typedef struct libusb_device_handle libusb_device_handle;

//========================================================

///@brief An enumeration to hold frame type.
enum class Frame_Type : uint8_t
{
    Calibration_Frame = 1,
    Usable_Frame = 3
};

//========================================================

///@brief An exception enumeration for the SeekThermalCamera object.
enum class SeekThermalCamera_Exception
{
	Camera_Not_Connected,
	Data_Error,
	Could_Not_Init_LibUSB_Context
};

//========================================================

///@brief Get a string version of a Seek Thermal Camera exception enumeration.
///@param [in] e    A SeekThermalCamera_Exception enum.
///@return A const C-string.
const char* getSeekThermalCameraExceptionString(SeekThermalCamera_Exception e);

//========================================================

//@brief A class to hold a thermal frame.
class ThermalFrame
{
private:
    uint16_t _pixelData[MAX_THERMAL_PIXELS];

public:

    ///@brief Get access to raw pixel data as unsigned bytes.
    ///@return A uint8_t pointer to the pixel buffer.
    inline uint8_t* getRawData()
    {
        return (uint8_t*)_pixelData;
    }

    ///@brief Get access to raw thermal data as unsigned 16-bit pixel values.
    ///@return A uint16_t pointer to the pixel buffer.
    inline uint16_t* getData()
    {
        return _pixelData;
    }

    ///@brief Get an individual 16-bit unsigned pixel value.
    ///@param [in] x    The x-coordinate.
    ///@param [in] y    The y-coordinate.
    ///@return A uint16_t pixel value.
    inline uint16_t getPixel(uint32_t x, uint32_t y)
    {
        return _pixelData[y * 208 + x];
    }

    ///@brief Get the type of frame.
    ///@return A frame type enumeration value.
    inline Frame_Type getFrameType()
    {
        return ((Frame_Type*)(_pixelData))[20];
    }

    ///@brief Get min pixel intensity.
    ///@return A minimum uint16_t pixel value.
    inline uint16_t getMin()
    {
        uint16_t minvalue = 0xFFFF;

        uint16_t value = 0xFFFF;

        for(uint32_t i = 0; i < MAX_THERMAL_PIXELS; ++i)
        {
            value = _pixelData[i];

            if(value < minvalue)
                minvalue = value;
        }

        return minvalue;
    }

    ///@brief Get max pixel intensity.
    ///@return A maximum uint16_t pixel value.
    inline uint16_t getMax()
    {
        uint16_t maxvalue = 0x0000;

        uint16_t value = 0x0000;

        for(uint32_t i = 0; i < MAX_THERMAL_PIXELS; ++i)
        {
            value = _pixelData[i];

            if(value > maxvalue)
                maxvalue = value;
        }

        return maxvalue;
    }

};

//========================================================

///@brief A function to populate an RGB buffer with the contents of a ThermalFrame object.
///@param [in] rgb_array    A pointer to an RGB buffer.
///@param [in] frame        A pointer to a ThermalFrame.
///@param [in] scaled       A boolean value that tells whether to scale the image colors by the min and max temperature in the image.
void createBitmap(uint8_t *rgb_array, ThermalFrame* frame, bool scaled = true);

//========================================================

//@brief A class to facilitate communication with a Seek Thermal Camera.
class SeekThermalCamera
{
	private:
		//The libusb context
		libusb_context			*ctx;

		//The Seek Thermal Camera device handle.
		libusb_device_handle	*seek;

		//A timeout in milliseconds to consider a usb data transaction failed.
		unsigned int timeout_value;

        //A pointer to the current calibration frame.
		ThermalFrame *calibrationFrame = nullptr;

        //A pointer to the last image frame.
		ThermalFrame *lastFrame = nullptr;

        //A two frame rotary buffer used to store incoming data.
		ThermalFrame frames[2];

        //The index of the last written frame in the buffer.
		uint32_t curFrame;

	public:
		///@brief Constructor (creates a libusb context)
		SeekThermalCamera();

		///@brief Destructor (destroys libusb device handle and context if extant)
		~SeekThermalCamera();

		///@brief Sends a deinitialization packet to the Seek Thermal Camera.
		void sendDeinit();

		///@brief Connects to and initializes the Seek Thermal Camera.
		///@return n/a
		void initialize();

		///@brief Deinitializes and disconnects from the Seek Thermal Camera.
		///@return n/a
		void deinitialize();

		///@brief Grabs a frame and saves it to a buffer from the camera given a buffer and a buffer size.
		///@return Returns the number of bytes received.
		uint32_t grabFrame(uint8_t *data, uint32_t size);

        ///@brief Get a pointer to a calibrated thermal frame.
        ///@return A pointer to a ThermalFrame object.
		ThermalFrame* getFrame();
};

//========================================================

#endif	//_SEEK_THERMAL_LIB_H
