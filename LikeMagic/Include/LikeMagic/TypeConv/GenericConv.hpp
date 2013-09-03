// LikeMagic C++ Binding Library
// Copyright 2008-2013 Dennis Ferron
// Co-founder DropEcho Studios, LLC.
// Visit our website at dropecho.com.
//
// LikeMagic is BSD-licensed.
// (See the license file in LikeMagic/Licenses.)

#pragma once

#include "LikeMagic/TypeConv/StaticCastConv.hpp"

namespace LM {




template <typename From, typename To>
class StaticCastConv : public StaticCastConv<From, To>
{
private:
    TypeInfoPtr from_info;
    TypeInfoPtr to_info;

public:

    StaticCastConv(TypeInfoPtr from, TypeInfoPtr to) : from_info(from), to_info(to)
    {
    }

    virtual std::string description() const
    {
        return
            std::string("StaticCastConv ") +
            " from " + from_info->description() +
            " to " + to_info->description();
    }
};


}
