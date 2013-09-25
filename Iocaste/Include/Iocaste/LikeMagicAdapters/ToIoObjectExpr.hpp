// LikeMagic C++ Binding Library
// Copyright 2008-2013 Dennis Ferron
// Co-founder DropEcho Studios, LLC.
// Visit our website at dropecho.com.
//
// LikeMagic is BSD-licensed.
// (See the license file in LikeMagic/Licenses.)


#pragma once

#include "LikeMagic/Exprs/Expr.hpp"
#include "LikeMagic/Utility/TypeDescr.hpp"
#include "Iocaste/LikeMagicAdapters/API_Io.hpp"
#include "Iocaste/LikeMagicAdapters/ToIoTypeInfo.hpp"

#include <iostream>
#include <stdexcept>

namespace Iocaste { namespace LMAdapters {

using LM::ExprPtr;
using LM::Expr;
using LM::TypeIndex;

class AbstractToIoObjectExpr : public Expr
{
public:
    AbstractToIoObjectExpr(ValuePtr ptr, TypeIndex type) : Expr(ptr, type) {}

    virtual bool is_terminal() const { return true; }
    virtual bool is_lazy() const { return false; }

    virtual IoObject* eval_in_context(IoObject *self, IoObject *locals, IoMessage *m) = 0;

    // It's already a script object, so don't need to try to convert it a second time.
    virtual bool disable_to_script_conv() const { return true; }
};

template <typename T, typename F>
class ToIoObjectExpr : public AbstractToIoObjectExpr
{
private:
    ExprPtr from_expr;

private:
    ToIoObjectExpr(ExprPtr from_expr_) : AbstractToIoObjectExpr(nullptr, ToIoTypeInfo::create_index()), from_expr(from_expr_) {}

public:

    static ExprPtr create(ExprPtr from_expr)
    {
        return new ToIoObjectExpr<T, F>(from_expr);
    }

    virtual IoObject* eval_in_context(IoObject *self, IoObject *locals, IoMessage *m)
    {
        ExprPtr warden;
        return F::eval_in_context(self, locals, m, EvalAs<T>::value(from_expr, warden));
    }

    virtual std::string description() const
    {
        return "ToIoObjectExpr from " + from_expr->description();
    }

    virtual void mark() const { from_expr->mark(); }

};

}}
