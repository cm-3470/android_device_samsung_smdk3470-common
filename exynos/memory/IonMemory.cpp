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
#include <sys/mman.h>
#include "ion.h"

#include "IonMemory.h"

namespace android {
namespace hidl {
namespace memory {
namespace V1_0 {
namespace implementation {

IonMemory::IonMemory(const hidl_memory& memory, void* data)
  : mMemory(memory),
    mData(data)
{}

IonMemory::~IonMemory()
{
    // TODO: Move implementation to mapper class
    ion_unmap(mData, mMemory.size());
}

// Methods from ::android::hidl::memory::V1_0::IMemory follow.
Return<void> IonMemory::update() {
    // NOOP (since non-remoted memory)
    return Void();
}

Return<void> IonMemory::updateRange(uint64_t /* start */, uint64_t /* length */) {
    // NOOP (since non-remoted memory)
    return Void();
}

Return<void> IonMemory::read() {
    // NOOP (since non-remoted memory)
    return Void();
}

Return<void> IonMemory::readRange(uint64_t /* start */, uint64_t /* length */) {
    // NOOP (since non-remoted memory)
    return Void();
}

Return<void> IonMemory::commit() {
    // NOOP (since non-remoted memory)
    return Void();
}

Return<void*> IonMemory::getPointer() {
    return mData;
}

Return<uint64_t> IonMemory::getSize() {
    return mMemory.size();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace memory
}  // namespace hidl
}  // namespace android
