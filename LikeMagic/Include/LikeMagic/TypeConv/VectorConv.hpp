// LikeMagic C++ Binding Library
// Copyright 2008-2013 Dennis Ferron
// Co-founder DropEcho Studios, LLC.
// Visit our website at dropecho.com.
//
// LikeMagic is BSD-licensed.
// (See the license file in LikeMagic/Licenses.)

#pragma once

#include "ConvertibleTo.hpp"
#include "LikeMagic/Exprs/Adapter.hpp"

#include "boost/type_traits.hpp"

namespace LM {



// cannot use with vector<bool>
template <typename From, typename To>
struct VectorConvImpl
{
    static_assert(boost::is_reference<From>::value,
            "From type must be a reference.  Cannot convert a vector-by-value to pointer because the temporary copy is lost when do_conv returns.");

    inline static To do_conv(From obj)
    {
        return &obj[0];
    }
};



template <typename From, typename To>
class VectorConv : public ConvertibleTo<To>
{
    static_assert(false && boost::is_pointer<To>::value,
            "For VectorConv From type must be std::vector& and To type must be pointer to an element of the vector.");
};

template <typename F, typename T>
class VectorConv<std::vector<F>, T*> : public ConvertibleTo<T*>
{
    static_assert(false && boost::is_pointer<T*>::value,
            "For VectorConv From type must be std::vector& and To type must be pointer to an element of the vector.");

//public:
//    typedef std::vector<F> From;
//    typedef T* To;
//
//    virtual ExprPtr wrap_expr(ExprPtr expr) const
//    {
//        return Adapter<From, To, VectorConvImpl<From, To>>::create(
//            boost::intrusive_ptr<Expression<From>>(expr));
//    }
//
//    virtual std::string description() const { return describe_converter<From, To>("VectorConv"); }

};

template <typename F, typename T>
class VectorConv<std::vector<F>&, T*> : public ConvertibleTo<T*>
{
public:
    typedef std::vector<F>& From;
    typedef T* To;

    virtual ExprPtr wrap_expr(ExprPtr expr) const
    {
        return Adapter<From, To, VectorConvImpl<From, To>>::create(expr);
    }

    virtual std::string description() const { return describe_converter<From, To>("VectorConv"); }

};

template <typename F, typename T>
class VectorConv<std::vector<F> const&, T*> : public ConvertibleTo<T*>
{
public:
    typedef std::vector<F> const& From;
    typedef T* To;

    virtual ExprPtr wrap_expr(ExprPtr expr) const
    {
        return Adapter<From, To, VectorConvImpl<From, To>>::create(expr);
    }

    virtual std::string description() const { return describe_converter<From, To>("VectorConv"); }

};


}
