#include "serial_control/ftd2xx_serial_interface.hh"
#include "logging.hh"

#include <assert.h>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace serial
{

//
// ### constructor ############################################################
//

ftd2xx_serial_interface::ftd2xx_serial_interface(const size_t baudrate, const size_t device_number)
{
    DWORD number_of_devices;
    assert(FT_CreateDeviceInfoList(&number_of_devices) == FT_OK);
    if (number_of_devices < 1)
    {
        LOG_ERROR("Need at least one device");
        return;
    }

    //
    // Try to open it
    //
    assert(FT_Open(device_number, &ft_handle) == FT_OK);
    assert(FT_ResetDevice(ft_handle) == FT_OK);
    LOG_DEBUG("Device opened successfully!");


    // Set BAUD rate, this needs to have this factor divided out in bitbang mode...
    constexpr size_t NON_ARBITRARY_FACTOR = 5;
    assert(FT_SetBaudRate(ft_handle, baudrate / NON_ARBITRARY_FACTOR) == FT_OK);
    // Enable RS485 mode
    //constexpr uint8_t RS485_MODE = 0x20;
    constexpr uint8_t ASYNC_BITBANG = 0x01;
    constexpr uint8_t PIN_MASK = 0xFF;
    assert(FT_SetBitMode(ft_handle, PIN_MASK, ASYNC_BITBANG) == FT_OK);
}

//
// ############################################################################
//

ftd2xx_serial_interface::~ftd2xx_serial_interface()
{
    FT_SetBitMode(ft_handle, 0x0, 0x00);
    FT_Close(ft_handle);
}

//
// ### public methods #########################################################
//

bool ftd2xx_serial_interface::write_data(ByteVector_t data) const
{
    const unsigned int bytes_to_send = data.size();
    unsigned int bytes_sent = 0;
    FT_STATUS ft_status = FT_Write(ft_handle, data.data(), bytes_to_send, &bytes_sent);

    if (status_okay(ft_status) == false)
    {
        return false;
    };

    return bytes_sent == bytes_to_send;
}

//
// ### private methods ########################################################
//

inline bool ftd2xx_serial_interface::status_okay(const FT_STATUS ft_status) const
{
    if (ft_status != FT_OK)
    {
        LOG_ERROR("ft_status = " << ft_status);
        return false;
    }
    return true;
}

} // namespace serial
