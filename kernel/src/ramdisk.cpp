//=======================================================================
// Copyright Baptiste Wicht 2013-2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ramdisk.hpp"
#include "errors.hpp"
#include "disks.hpp"
#include "paging.hpp"
#include "logging.hpp"

namespace {

constexpr const size_t MAX_RAMDISK = 3;

size_t current = 0;
ramdisk::disk_descriptor ramdisks[MAX_RAMDISK];

} //end of anonymous namespace

ramdisk::disk_descriptor* ramdisk::make_disk(uint64_t max_size){
    if(current == MAX_RAMDISK){
        return nullptr;
    }

    auto pages = paging::pages(max_size);

    ramdisks[current].id = current;
    ramdisks[current].max_size = max_size;
    ramdisks[current].pages = pages;
    ramdisks[current].allocated = new char*[pages];

    for(size_t i = 0; i < pages; ++i){
        ramdisks[current].allocated[i] = nullptr;
    }

    ++current;
    return &ramdisks[current - 1];
}

size_t ramdisk::ramdisk_driver::read(void* data, char* destination, size_t count, size_t offset, size_t& read){
    read = 0;

    auto descriptor = reinterpret_cast<disks::disk_descriptor*>(data);
    auto disk = reinterpret_cast<ramdisk::disk_descriptor*>(descriptor->descriptor);

    if(offset + count >= disk->max_size){
        logging::logf(logging::log_level::ERROR, "ramdisk: Tried to read too far\n");
        return std::ERROR_INVALID_OFFSET;
    }

    while(read != count){
        auto page = offset / paging::PAGE_SIZE;
        auto page_offset = offset % paging::PAGE_SIZE;

        auto to_read = std::min(paging::PAGE_SIZE - page_offset, count - read);

        // If the page is not allocated, we simply consider it full of zero
        if(!disk->allocated[page]){
            std::fill_n(destination, to_read, 0);
        } else {
            std::copy_n(destination, disk->allocated[page] + page_offset, to_read);
        }

        read += to_read;
        offset += to_read;
    }

    return 0;
}

size_t ramdisk::ramdisk_driver::write(void* data, const char* source, size_t count, size_t offset, size_t& written){
    written = 0;

    auto descriptor = reinterpret_cast<disks::disk_descriptor*>(data);
    auto disk = reinterpret_cast<ramdisk::disk_descriptor*>(descriptor->descriptor);

    if(offset + count >= disk->max_size){
        logging::logf(logging::log_level::ERROR, "ramdisk: Tried to write too far\n");
        return std::ERROR_INVALID_OFFSET;
    }

    while(written != count){
        auto page = offset / paging::PAGE_SIZE;
        auto page_offset = offset % paging::PAGE_SIZE;

        if(!disk->allocated[page]){
            logging::logf(logging::log_level::TRACE, "ramdisk: Disk %u Allocated page %u \n", disk->id, page);

            disk->allocated[page] = new char[paging::PAGE_SIZE];
            std::fill_n(disk->allocated[page], paging::PAGE_SIZE, 0);
        }

        uint64_t to_write = std::min(paging::PAGE_SIZE - page_offset, count - written);
        std::copy_n(disk->allocated[page] + page_offset, source, to_write);
        written += to_write;

        offset += to_write;
    }

    return 0;
}

size_t ramdisk::ramdisk_driver::size(void* data){
    auto descriptor = reinterpret_cast<disks::disk_descriptor*>(data);
    auto disk = reinterpret_cast<ramdisk::disk_descriptor*>(descriptor->descriptor);
    return disk->max_size;
}
