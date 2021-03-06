#pragma once

#include "LikeMagic/TypeSystem.hpp"
#include "LikeMagic/Interprocess/AbstractSharedArgMarshaller.hpp"

namespace LM {

template <typename T>
struct ReferenceToSharedArray
{
    ReferenceToSharedArray(boost::offset_ptr<T> data_, size_t size_)
        : data(data_), size(size_) {}

    boost::offset_ptr<T> data;
    size_t size;
};

class SharedMemoryAllocator
{
public:
    template <typename T> ReferenceToSharedArray<T> allocate(size_t count);
    template <typename T> void release(ReferenceToSharedArray<T> reference);
};

template <typename ContainerType, typename ElementType>
struct MarshalByStdCopyToArray : public AbstractSharedArgMarshaller
{
    LM::Interprocess::SharedMemoryAllocator& allocator;

    MarshalByCopy(SharedMemoryAllocator& allocator_) : allocator(allocator_) {}

    virtual void write(void* location, ExprPtr arg)
    {
        Container container = try_conv<T>(arg);
        size_t size = container.size();
        offset_ptr<T> buffer = allocator.allocate_buffer<ElementType>(size);
        std::copy(container.begin(), container.end(), buffer.data);
        *reinterpret_cast<ReferenceToSharedArray<ElementType>*>(location) =
            ReferenceToSharedArray<ElementType>(data, size);
    }

    virtual ExprPtr read(void* location)
    {
        ReferenceToSharedArray<ElementType> buffer =
            *reinterpret_cast<ReferenceToSharedArray<ElementType>*>(location);
        return Term<T>::create(Container(buffer.data, buffer.data + size));
    }

    virtual size_t size() const
    {
        return sizeof(ReferenceToSharedArray<ElementType>);
    }

    virtual void release(ExprPtr expr)
    {
        ReferenceToSharedArray<ElementType> buffer = try_conv<T>(arg);
        allocator.release(buffer);
    }
};

}
