/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "IonMapper"
#include <android-base/logging.h>

#include "IonMapper.h"

#include <sys/mman.h>
#include "ion.h"

#include "IonMemory.h"

namespace android {
namespace hidl {
namespace memory {
namespace V1_0 {
namespace implementation {

// Methods from ::android::hidl::memory::V1_0::IMapper follow.
Return<sp<IMemory>> IonMapper::mapMemory(const hidl_memory& mem) {
    if (mem.handle()->numFds == 0) {
        return nullptr;
    }

    int fd = mem.handle()->data[0];
    void* data = ion_map(fd, mem.size(), 0);
    if (data == MAP_FAILED) {
        LOG(ERROR) << "MemoryHeapIon : ION mmap failed(size[" << mem.size() << "], fd[" << fd << "]) : " << strerror(errno);
        ion_free(fd);
        return nullptr;
    }

    return new IonMemory(mem, data);
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace memory
}  // namespace hidl
}  // namespace android
