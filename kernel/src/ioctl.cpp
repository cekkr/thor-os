//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ioctl.hpp"
#include "errors.hpp"
#include "fs/devfs.hpp"

int64_t ioctl(const std::string& device, ioctl_request request, void* data){
    if(request == ioctl_request::GET_BLK_SIZE){
        return devfs::get_device_size(device, *reinterpret_cast<size_t*>(data));
    }

    return std::ERROR_INVALID_REQUEST;
}
