// LikeMagic C++ Binding Library
// Copyright 2008-2013 Dennis Ferron
// Co-founder DropEcho Studios, LLC.
// Visit our website at dropecho.com.
//
// LikeMagic is BSD-licensed.
// (See the license file in LikeMagic/Licenses.)

#pragma once

#include "LikeMagic/TypeSystem.hpp"
#include "LikeMagic/Exprs/Term.hpp"
#include "LikeMagic/Exprs/methodcall_args.hpp"
#include "LikeMagic/Mirrors/CallTarget.hpp"
#include "LikeMagic/Utility/IndexPack.hpp"

namespace LM {

template <typename R, typename FirstArg, typename... Args>
class ExtensionMethodCallTarget : public CallTarget
{
public:
    typedef typename MakeIndexPack<sizeof...(Args)>::type IPack;
    typedef R (*F)(FirstArg, Args...);

private:

    F func_ptr;

    // Handle functions that return a value
    template<typename R_, int... Indices>
    ExprPtr build_method_call(ExprPtr target, ArgList args, TypePack<R_>, IndexPack<Indices...>) const
    {
        if (args.size() != sizeof...(Indices))
            throw std::logic_error("Wrong number of arguments.");

        ExprPtr result = Term<R>::create(
            (*func_ptr)(EvalAs<FirstArg>::value(target), EvalAs<Args>::value(args[Indices])...)
        );

        return result;
    }

    // Handle functions that do not return a value
    template<int... Indices>
    ExprPtr
    build_method_call(ExprPtr target, ArgList args, TypePack<void>, IndexPack<Indices...>) const
    {
        if (args.size() != sizeof...(Indices))
            throw std::logic_error("Wrong number of arguments.");

        (*func_ptr)(EvalAs<FirstArg>::value(target), EvalAs<Args>::value(args[Indices])...);

        return 0;
    }

public:

    ExtensionMethodCallTarget(F func_ptr_) : func_ptr(func_ptr_) {}

    virtual ExprPtr call(ExprPtr target, ArgList args) const
    {
        return build_method_call(target, args, TypePack<R>(), IPack());
    }

    virtual TypeInfoList const& get_arg_types() const
    {
        static TypeInfoList arg_types = make_arg_list(TypePack<Args...>());
        return arg_types;
    }

};

}
