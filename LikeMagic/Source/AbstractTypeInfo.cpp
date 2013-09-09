// LikeMagic C++ Binding Library
// Copyright 2008-2013 Dennis Ferron
// Co-founder DropEcho Studios, LLC.
// Visit our website at dropecho.com.
//
// LikeMagic is BSD-licensed.
// (See the license file in LikeMagic/Licenses.)

#include "LikeMagic/Utility/AbstractTypeInfo.hpp"

namespace LM {

const TypeInfoList empty_arg_list;

void intrusive_ptr_add_ref(AbstractTypeInfo const* p)
{
    ++(p->ref_count);
}

void intrusive_ptr_release(AbstractTypeInfo const* p)
{
    if (!--(p->ref_count))
        delete p;
}

AbstractTypeInfo::AbstractTypeInfo() :
    ref_count(0), has_cached_hash(false)
{
}

AbstractTypeInfo::~AbstractTypeInfo()
{
}

TypeIndex AbstractTypeInfo::get_index() const
{
    return TypeInfoCache::get_instance()->get_index(this, this->class_type());
}

std::size_t hash_value(AbstractTypeInfo const& info)
{
    return info.hash_value();
}

std::size_t hash_value(TypeInfoPtr info_p)
{
    return info_p->hash_value();
}

bool AbstractTypeInfo::operator <(const AbstractTypeInfo& that) const
{
    if (this->get_system() < that.get_system())
        return true;
    else if (that.get_system() < this->get_system())
        return false;
    else
        return this->less(that);
}

bool AbstractTypeInfo::operator ==(const AbstractTypeInfo& that) const
{
    if (this->get_system() != that.get_system())
        return false;
    else
        return this->equals(that);
}

bool AbstractTypeInfo::get_is_const() const { return false; }

TypeInfoPtr AbstractTypeInfo::as_const() const
{
    throw std::logic_error("Type variation as_const undefined on " + description());
}

TypeInfoPtr AbstractTypeInfo::as_nonconst() const
{
    throw std::logic_error("Type variation as_nonconst undefined on " + description());
}

TypeInfoPtr AbstractTypeInfo::as_ptr() const
{
    throw std::logic_error("Type variation as_ptr undefined on " + description());
}

TypeInfoPtr AbstractTypeInfo::as_value() const
{
    throw std::logic_error("Type variation as_value undefined on " + description());
}

TypeInfoPtr AbstractTypeInfo::class_type() const
{
    throw std::logic_error("Type variation class_type undefined on " + description());
}

std::size_t AbstractTypeInfo::hash_value() const
{

    if (!has_cached_hash)
    {
        cached_hash = 0;
        boost::hash_combine(cached_hash, get_system());
        boost::hash_combine(cached_hash, calc_hash());
        has_cached_hash = true;
    }
    return cached_hash;
}

}
