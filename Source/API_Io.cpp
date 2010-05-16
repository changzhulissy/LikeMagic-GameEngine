
#include "LikeMagic/Backends/Io/API_Io_Impl.hpp"
#include "LikeMagic/SFMO/Term.hpp"
#include "LikeMagic/SFMO/NullExpr.hpp"
#include "LikeMagic/Backends/Io/IoBlock.hpp"

#include "boost/lexical_cast.hpp"

namespace LikeMagic { namespace Backends { namespace Io {


IoMethodTable* make_io_method_table(std::vector<std::string> method_names)
{
    IoMethodTable* table;

    typedef IoMethodFunc* MethodPtr;

    int count = method_names.size();

    table = new IoMethodTable[count+1];

    int i = 0;
    for (auto it = method_names.begin(); it != method_names.end(); it++, i++)
    {
        // I believe Io makes a copy of the std::string's c_str.  I hope...
        IoMethodTable entry = { it->c_str(), &API_io_userfunc };  // All point to same userfunc
        table[i] = entry;
    }

    IoMethodTable term = { NULL, NULL };
    table[count] = term;

    return table;
}


IoObject* arg_at(IoObject *self, IoObject *locals, IoMessage *m, int pos)
{
    if (pos >= IoMessage_argCount(m))
        throw std::invalid_argument("Not enough arguments passed to function");

    IoObject* arg = IoMessage_locals_valueArgAt_(m, locals, pos);

    if (!arg)
        throw std::invalid_argument("IoMessage_locals_valueArgAt_ at pos is null!");

    return arg;
}

bool is_sfmo_obj(IoObject* io_obj)
{
    const std::string magic_tag("LikeMagic");
    return 0 == magic_tag.compare(0, 4, IoObject_tag(io_obj)->name, 0, 4) && IoObject_dataPointer(io_obj);
}


// Gets the IoState object from an IoObject
IoState* get_io_state(IoObject* self)
{
    return reinterpret_cast<IoState*>(IoObject_tag(self)->state);
}

IoMessage* new_message(IoObject* self, std::string name)
{
    return IoMessage_newWithName_(get_io_state(self), IOSYMBOL(name.c_str()));
}

// Gets the script object argument at a certain position.
boost::intrusive_ptr<LikeMagic::SFMO::AbstractExpression> arg_at(IoObject *self, IoObject *locals, IoMessage *m, int pos, std::vector<BetterTypeInfo> arg_types, AbstractTypeSystem const& type_sys)
{
    //std::cout << "Arg " << pos << " = " << IoObject_tag(self)->name << std::endl;
    return from_script(self, arg_at(self, locals, m, pos), arg_types[pos], type_sys);
}


}}}

//////////////////////////////////////////
// Some extern "C" functions below.
////////////////////////////////////////////

IoObject* API_io_rawClone(IoObject* proto)
{
    IoObject* clone = reinterpret_cast<IoObject*>(IoObject_rawClonePrimitive(
            reinterpret_cast<IoObject*>(proto)));

    // When we clone a methodset proto, there's no CppObjProxy.
    if (!IoObject_dataPointer(reinterpret_cast<IoObject*>(proto)))
    {
        //cout << "Raw clone called for proto " << proto << ", no cpp obj" << endl;
    }
    else
    {
        //cout << "Raw clone called for proto " << proto << " data pointer is " << IoObject_dataPointer(proto) << endl;

        LikeMagic::SFMO::AbstractCppObjProxy* proxy(
                reinterpret_cast<LikeMagic::SFMO::AbstractCppObjProxy*>(IoObject_dataPointer(proto)));

        IoObject_setDataPointer_(clone, proxy->clone());
    }

    return clone;
}

void API_io_free_proxy(IoObject* self)
{
    IoObject* io_obj = reinterpret_cast<IoObject*>(self);

    if (ISNUMBER(io_obj))
        throw std::logic_error("free sfmo proxy passed an IoNumber instead!");
    else if (ISSEQ(io_obj))
        throw std::logic_error("free sfmo proxy passed an IoSeq instead!");
    else if (IoObject_dataPointer(self))
    {
        if (is_sfmo_obj(io_obj))
        {
            //std::cout << "free sfmo proxy passed sfmo object with tag " << IoObject_tag(io_obj)->name << std::endl;
        }
        else
        {
            //std::cout << "free sfmo proxy passed a NON-sfmo object with tag " << IoObject_tag(io_obj)->name << std::endl;
            throw std::logic_error("free sfmo proxy passed a NON-sfmo object");
        }

        auto proxy = reinterpret_cast<LikeMagic::SFMO::AbstractCppObjProxy*>(IoObject_dataPointer(self));
        proxy->check_magic();
        delete proxy;
        IoObject_setDataPointer_(self, 0);
    }
}

void API_io_mark(IoObject* self)
{
    if (self && IoObject_dataPointer(self))
    {
        IoObject_shouldMark(self);
        auto proxy = reinterpret_cast<AbstractCppObjProxy*>(IoObject_dataPointer(self));
        proxy->mark();
    }
}

IoObject *API_io_proto(IoState* state)
{
    //std::cout << "Creating LikeMagic proto" <<std::endl;

    IoObject *self = IoObject_new(state);

    IoTag* tag = IoTag_newWithName_("LikeMagic");
    IoTag_state_(tag, state);
    IoTag_freeFunc_(tag, (IoTagFreeFunc*)API_io_free_proxy);
    IoTag_cloneFunc_(tag, (IoTagCloneFunc*)API_io_rawClone);
    IoTag_markFunc_(tag, (IoTagMarkFunc*)API_io_mark);
    IoObject_tag_(self, tag);

    IoState_registerProtoWithFunc_(state, self, (IoStateProtoFunc*)&API_io_proto);

    return self;
}


IoObject* API_io_userfunc(IoObject *self, IoObject *locals, IoMessage *m)
{
    IOASSERT(IoObject_dataPointer(self), "No C++ object");
    try
    {
        std::string method_name = CSTRING(IoMessage_name(m));

        //std::cout << "Method called is: "  << method_name << std::endl;

        auto proxy = reinterpret_cast<AbstractCppObjProxy*>(IoObject_dataPointer(self));
        proxy->check_magic();

        auto type_sys = proxy->get_type_system();

        std::vector<boost::intrusive_ptr<AbstractExpression>> args;
        std::vector<BetterTypeInfo> arg_types = proxy->get_arg_types(method_name, IoMessage_argCount(m));

        for (int i=0; i<arg_types.size(); i++)
        {
            ExprPtr expr = arg_at(self, locals, m, i, arg_types, type_sys);
            //std::cout << "arg " << i << " expects " << arg_types[i].describe() << " got " << expr->get_type().describe() << std::endl;
            args.push_back(expr);
        }

        auto result = proxy->call(method_name, args);
        return to_script(self, locals, m, result);
    }
    catch (std::logic_error le)
    {
        std::cout << "Caught exception: " << le.what() << std::endl;
        IoState_error_(IOSTATE,  m, "Caught C++ exception: type=%s, what()=%s", typeid(le).name(), le.what());
        return IONIL(self);
    }
    catch (std::exception e)
    {
        std::cout << "Caught exception: " << e.what() << std::endl;
        IoState_error_(IOSTATE,  m, "Caught C++ exception: type=%s, what()=%s", typeid(e).name(), e.what());
        return IONIL(self);
    }
}
