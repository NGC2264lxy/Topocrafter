//
// Generated file, do not edit! Created by nedtool 5.6 from packet/Packet.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include "Packet_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp


// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');
    
    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

EXECUTE_ON_STARTUP(
    omnetpp::cEnum *e = omnetpp::cEnum::find("ALL_MSGS");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("ALL_MSGS"));
    e->insert(APP_MSG, "APP_MSG");
    e->insert(DATA_MSG, "DATA_MSG");
    e->insert(FC_MSG, "FC_MSG");
    e->insert(SW_FLAG, "SW_FLAG");
    e->insert(SETUP_MSG, "SETUP_MSG");
    e->insert(BUFLEN_MSG, "BUFLEN_MSG");
    e->insert(PATHLEN_MSG, "PATHLEN_MSG");
)

Register_Class(appMsg)

appMsg::appMsg(const char *name, short kind) : ::omnetpp::cPacket(name,kind)
{
    this->srcLid = 0;
    this->dstLid = 0;
    this->msgId = 0;
    this->flitLength = 0;
    this->pathLength = 0;
}

appMsg::appMsg(const appMsg& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

appMsg::~appMsg()
{
}

appMsg& appMsg::operator=(const appMsg& other)
{
    if (this==&other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void appMsg::copy(const appMsg& other)
{
    this->srcLid = other.srcLid;
    this->dstLid = other.dstLid;
    this->msgId = other.msgId;
    this->flitLength = other.flitLength;
    this->pathLength = other.pathLength;
}

void appMsg::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->srcLid);
    doParsimPacking(b,this->dstLid);
    doParsimPacking(b,this->msgId);
    doParsimPacking(b,this->flitLength);
    doParsimPacking(b,this->pathLength);
}

void appMsg::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->srcLid);
    doParsimUnpacking(b,this->dstLid);
    doParsimUnpacking(b,this->msgId);
    doParsimUnpacking(b,this->flitLength);
    doParsimUnpacking(b,this->pathLength);
}

int appMsg::getSrcLid() const
{
    return this->srcLid;
}

void appMsg::setSrcLid(int srcLid)
{
    this->srcLid = srcLid;
}

int appMsg::getDstLid() const
{
    return this->dstLid;
}

void appMsg::setDstLid(int dstLid)
{
    this->dstLid = dstLid;
}

int appMsg::getMsgId() const
{
    return this->msgId;
}

void appMsg::setMsgId(int msgId)
{
    this->msgId = msgId;
}

int appMsg::getFlitLength() const
{
    return this->flitLength;
}

void appMsg::setFlitLength(int flitLength)
{
    this->flitLength = flitLength;
}

int appMsg::getPathLength() const
{
    return this->pathLength;
}

void appMsg::setPathLength(int pathLength)
{
    this->pathLength = pathLength;
}

class appMsgDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    appMsgDescriptor();
    virtual ~appMsgDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(appMsgDescriptor)

appMsgDescriptor::appMsgDescriptor() : omnetpp::cClassDescriptor("appMsg", "omnetpp::cPacket")
{
    propertynames = nullptr;
}

appMsgDescriptor::~appMsgDescriptor()
{
    delete[] propertynames;
}

bool appMsgDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<appMsg *>(obj)!=nullptr;
}

const char **appMsgDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *appMsgDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int appMsgDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount() : 5;
}

unsigned int appMsgDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<5) ? fieldTypeFlags[field] : 0;
}

const char *appMsgDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "srcLid",
        "dstLid",
        "msgId",
        "flitLength",
        "pathLength",
    };
    return (field>=0 && field<5) ? fieldNames[field] : nullptr;
}

int appMsgDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "srcLid")==0) return base+0;
    if (fieldName[0]=='d' && strcmp(fieldName, "dstLid")==0) return base+1;
    if (fieldName[0]=='m' && strcmp(fieldName, "msgId")==0) return base+2;
    if (fieldName[0]=='f' && strcmp(fieldName, "flitLength")==0) return base+3;
    if (fieldName[0]=='p' && strcmp(fieldName, "pathLength")==0) return base+4;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *appMsgDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "int",
        "int",
        "int",
        "int",
    };
    return (field>=0 && field<5) ? fieldTypeStrings[field] : nullptr;
}

const char **appMsgDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *appMsgDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int appMsgDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    appMsg *pp = (appMsg *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *appMsgDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    appMsg *pp = (appMsg *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string appMsgDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    appMsg *pp = (appMsg *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getSrcLid());
        case 1: return long2string(pp->getDstLid());
        case 2: return long2string(pp->getMsgId());
        case 3: return long2string(pp->getFlitLength());
        case 4: return long2string(pp->getPathLength());
        default: return "";
    }
}

bool appMsgDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    appMsg *pp = (appMsg *)object; (void)pp;
    switch (field) {
        case 0: pp->setSrcLid(string2long(value)); return true;
        case 1: pp->setDstLid(string2long(value)); return true;
        case 2: pp->setMsgId(string2long(value)); return true;
        case 3: pp->setFlitLength(string2long(value)); return true;
        case 4: pp->setPathLength(string2long(value)); return true;
        default: return false;
    }
}

const char *appMsgDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *appMsgDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    appMsg *pp = (appMsg *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(dataMsg)

dataMsg::dataMsg(const char *name, short kind) : ::omnetpp::cPacket(name,kind)
{
    this->srcLid = 0;
    this->dstLid = 0;
    this->msgId = 0;
    this->flitSn = 0;
    this->flitLength = 0;
    this->pathLength = 0;
    this->curTime = 0;
}

dataMsg::dataMsg(const dataMsg& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

dataMsg::~dataMsg()
{
}

dataMsg& dataMsg::operator=(const dataMsg& other)
{
    if (this==&other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void dataMsg::copy(const dataMsg& other)
{
    this->srcLid = other.srcLid;
    this->dstLid = other.dstLid;
    this->msgId = other.msgId;
    this->flitSn = other.flitSn;
    this->flitLength = other.flitLength;
    this->pathLength = other.pathLength;
    this->curTime = other.curTime;
}

void dataMsg::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->srcLid);
    doParsimPacking(b,this->dstLid);
    doParsimPacking(b,this->msgId);
    doParsimPacking(b,this->flitSn);
    doParsimPacking(b,this->flitLength);
    doParsimPacking(b,this->pathLength);
    doParsimPacking(b,this->curTime);
}

void dataMsg::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->srcLid);
    doParsimUnpacking(b,this->dstLid);
    doParsimUnpacking(b,this->msgId);
    doParsimUnpacking(b,this->flitSn);
    doParsimUnpacking(b,this->flitLength);
    doParsimUnpacking(b,this->pathLength);
    doParsimUnpacking(b,this->curTime);
}

int dataMsg::getSrcLid() const
{
    return this->srcLid;
}

void dataMsg::setSrcLid(int srcLid)
{
    this->srcLid = srcLid;
}

int dataMsg::getDstLid() const
{
    return this->dstLid;
}

void dataMsg::setDstLid(int dstLid)
{
    this->dstLid = dstLid;
}

int dataMsg::getMsgId() const
{
    return this->msgId;
}

void dataMsg::setMsgId(int msgId)
{
    this->msgId = msgId;
}

int dataMsg::getFlitSn() const
{
    return this->flitSn;
}

void dataMsg::setFlitSn(int flitSn)
{
    this->flitSn = flitSn;
}

int dataMsg::getFlitLength() const
{
    return this->flitLength;
}

void dataMsg::setFlitLength(int flitLength)
{
    this->flitLength = flitLength;
}

int dataMsg::getPathLength() const
{
    return this->pathLength;
}

void dataMsg::setPathLength(int pathLength)
{
    this->pathLength = pathLength;
}

::omnetpp::simtime_t dataMsg::getCurTime() const
{
    return this->curTime;
}

void dataMsg::setCurTime(::omnetpp::simtime_t curTime)
{
    this->curTime = curTime;
}

class dataMsgDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    dataMsgDescriptor();
    virtual ~dataMsgDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(dataMsgDescriptor)

dataMsgDescriptor::dataMsgDescriptor() : omnetpp::cClassDescriptor("dataMsg", "omnetpp::cPacket")
{
    propertynames = nullptr;
}

dataMsgDescriptor::~dataMsgDescriptor()
{
    delete[] propertynames;
}

bool dataMsgDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<dataMsg *>(obj)!=nullptr;
}

const char **dataMsgDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *dataMsgDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int dataMsgDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 7+basedesc->getFieldCount() : 7;
}

unsigned int dataMsgDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<7) ? fieldTypeFlags[field] : 0;
}

const char *dataMsgDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "srcLid",
        "dstLid",
        "msgId",
        "flitSn",
        "flitLength",
        "pathLength",
        "curTime",
    };
    return (field>=0 && field<7) ? fieldNames[field] : nullptr;
}

int dataMsgDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "srcLid")==0) return base+0;
    if (fieldName[0]=='d' && strcmp(fieldName, "dstLid")==0) return base+1;
    if (fieldName[0]=='m' && strcmp(fieldName, "msgId")==0) return base+2;
    if (fieldName[0]=='f' && strcmp(fieldName, "flitSn")==0) return base+3;
    if (fieldName[0]=='f' && strcmp(fieldName, "flitLength")==0) return base+4;
    if (fieldName[0]=='p' && strcmp(fieldName, "pathLength")==0) return base+5;
    if (fieldName[0]=='c' && strcmp(fieldName, "curTime")==0) return base+6;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *dataMsgDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "int",
        "int",
        "int",
        "int",
        "int",
        "simtime_t",
    };
    return (field>=0 && field<7) ? fieldTypeStrings[field] : nullptr;
}

const char **dataMsgDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *dataMsgDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int dataMsgDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    dataMsg *pp = (dataMsg *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *dataMsgDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    dataMsg *pp = (dataMsg *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string dataMsgDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    dataMsg *pp = (dataMsg *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getSrcLid());
        case 1: return long2string(pp->getDstLid());
        case 2: return long2string(pp->getMsgId());
        case 3: return long2string(pp->getFlitSn());
        case 4: return long2string(pp->getFlitLength());
        case 5: return long2string(pp->getPathLength());
        case 6: return simtime2string(pp->getCurTime());
        default: return "";
    }
}

bool dataMsgDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    dataMsg *pp = (dataMsg *)object; (void)pp;
    switch (field) {
        case 0: pp->setSrcLid(string2long(value)); return true;
        case 1: pp->setDstLid(string2long(value)); return true;
        case 2: pp->setMsgId(string2long(value)); return true;
        case 3: pp->setFlitSn(string2long(value)); return true;
        case 4: pp->setFlitLength(string2long(value)); return true;
        case 5: pp->setPathLength(string2long(value)); return true;
        case 6: pp->setCurTime(string2simtime(value)); return true;
        default: return false;
    }
}

const char *dataMsgDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *dataMsgDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    dataMsg *pp = (dataMsg *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(SWFLAG)

SWFLAG::SWFLAG(const char *name, short kind) : ::omnetpp::cPacket(name,kind)
{
    this->outPortNum = 0;
}

SWFLAG::SWFLAG(const SWFLAG& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

SWFLAG::~SWFLAG()
{
}

SWFLAG& SWFLAG::operator=(const SWFLAG& other)
{
    if (this==&other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void SWFLAG::copy(const SWFLAG& other)
{
    this->outPortNum = other.outPortNum;
}

void SWFLAG::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->outPortNum);
}

void SWFLAG::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->outPortNum);
}

int SWFLAG::getOutPortNum() const
{
    return this->outPortNum;
}

void SWFLAG::setOutPortNum(int outPortNum)
{
    this->outPortNum = outPortNum;
}

class SWFLAGDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    SWFLAGDescriptor();
    virtual ~SWFLAGDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(SWFLAGDescriptor)

SWFLAGDescriptor::SWFLAGDescriptor() : omnetpp::cClassDescriptor("SWFLAG", "omnetpp::cPacket")
{
    propertynames = nullptr;
}

SWFLAGDescriptor::~SWFLAGDescriptor()
{
    delete[] propertynames;
}

bool SWFLAGDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<SWFLAG *>(obj)!=nullptr;
}

const char **SWFLAGDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *SWFLAGDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int SWFLAGDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 1+basedesc->getFieldCount() : 1;
}

unsigned int SWFLAGDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
    };
    return (field>=0 && field<1) ? fieldTypeFlags[field] : 0;
}

const char *SWFLAGDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "outPortNum",
    };
    return (field>=0 && field<1) ? fieldNames[field] : nullptr;
}

int SWFLAGDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='o' && strcmp(fieldName, "outPortNum")==0) return base+0;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *SWFLAGDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",
    };
    return (field>=0 && field<1) ? fieldTypeStrings[field] : nullptr;
}

const char **SWFLAGDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *SWFLAGDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int SWFLAGDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    SWFLAG *pp = (SWFLAG *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *SWFLAGDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    SWFLAG *pp = (SWFLAG *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string SWFLAGDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    SWFLAG *pp = (SWFLAG *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getOutPortNum());
        default: return "";
    }
}

bool SWFLAGDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    SWFLAG *pp = (SWFLAG *)object; (void)pp;
    switch (field) {
        case 0: pp->setOutPortNum(string2long(value)); return true;
        default: return false;
    }
}

const char *SWFLAGDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *SWFLAGDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    SWFLAG *pp = (SWFLAG *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(setupMsg)

setupMsg::setupMsg(const char *name, short kind) : ::omnetpp::cPacket(name,kind)
{
    this->MEMSID = 0;
    this->flag = false;
    this->ini = false;
}

setupMsg::setupMsg(const setupMsg& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

setupMsg::~setupMsg()
{
}

setupMsg& setupMsg::operator=(const setupMsg& other)
{
    if (this==&other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void setupMsg::copy(const setupMsg& other)
{
    this->MEMSID = other.MEMSID;
    this->configuration = other.configuration;
    this->flag = other.flag;
    this->ini = other.ini;
}

void setupMsg::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->MEMSID);
    doParsimPacking(b,this->configuration);
    doParsimPacking(b,this->flag);
    doParsimPacking(b,this->ini);
}

void setupMsg::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->MEMSID);
    doParsimUnpacking(b,this->configuration);
    doParsimUnpacking(b,this->flag);
    doParsimUnpacking(b,this->ini);
}

int setupMsg::getMEMSID() const
{
    return this->MEMSID;
}

void setupMsg::setMEMSID(int MEMSID)
{
    this->MEMSID = MEMSID;
}

IntVector& setupMsg::getConfiguration()
{
    return this->configuration;
}

void setupMsg::setConfiguration(const IntVector& configuration)
{
    this->configuration = configuration;
}

bool setupMsg::getFlag() const
{
    return this->flag;
}

void setupMsg::setFlag(bool flag)
{
    this->flag = flag;
}

bool setupMsg::getIni() const
{
    return this->ini;
}

void setupMsg::setIni(bool ini)
{
    this->ini = ini;
}

class setupMsgDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    setupMsgDescriptor();
    virtual ~setupMsgDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(setupMsgDescriptor)

setupMsgDescriptor::setupMsgDescriptor() : omnetpp::cClassDescriptor("setupMsg", "omnetpp::cPacket")
{
    propertynames = nullptr;
}

setupMsgDescriptor::~setupMsgDescriptor()
{
    delete[] propertynames;
}

bool setupMsgDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<setupMsg *>(obj)!=nullptr;
}

const char **setupMsgDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *setupMsgDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int setupMsgDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount() : 4;
}

unsigned int setupMsgDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<4) ? fieldTypeFlags[field] : 0;
}

const char *setupMsgDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "MEMSID",
        "configuration",
        "flag",
        "ini",
    };
    return (field>=0 && field<4) ? fieldNames[field] : nullptr;
}

int setupMsgDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='M' && strcmp(fieldName, "MEMSID")==0) return base+0;
    if (fieldName[0]=='c' && strcmp(fieldName, "configuration")==0) return base+1;
    if (fieldName[0]=='f' && strcmp(fieldName, "flag")==0) return base+2;
    if (fieldName[0]=='i' && strcmp(fieldName, "ini")==0) return base+3;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *setupMsgDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "IntVector",
        "bool",
        "bool",
    };
    return (field>=0 && field<4) ? fieldTypeStrings[field] : nullptr;
}

const char **setupMsgDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *setupMsgDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int setupMsgDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    setupMsg *pp = (setupMsg *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *setupMsgDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    setupMsg *pp = (setupMsg *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string setupMsgDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    setupMsg *pp = (setupMsg *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getMEMSID());
        case 1: {std::stringstream out; out << pp->getConfiguration(); return out.str();}
        case 2: return bool2string(pp->getFlag());
        case 3: return bool2string(pp->getIni());
        default: return "";
    }
}

bool setupMsgDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    setupMsg *pp = (setupMsg *)object; (void)pp;
    switch (field) {
        case 0: pp->setMEMSID(string2long(value)); return true;
        case 2: pp->setFlag(string2bool(value)); return true;
        case 3: pp->setIni(string2bool(value)); return true;
        default: return false;
    }
}

const char *setupMsgDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case 1: return omnetpp::opp_typename(typeid(IntVector));
        default: return nullptr;
    };
}

void *setupMsgDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    setupMsg *pp = (setupMsg *)object; (void)pp;
    switch (field) {
        case 1: return (void *)(&pp->getConfiguration()); break;
        default: return nullptr;
    }
}

Register_Class(BufferLength)

BufferLength::BufferLength(const char *name, short kind) : ::omnetpp::cPacket(name,kind)
{
    this->srcGroupIdx = 0;
}

BufferLength::BufferLength(const BufferLength& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

BufferLength::~BufferLength()
{
}

BufferLength& BufferLength::operator=(const BufferLength& other)
{
    if (this==&other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void BufferLength::copy(const BufferLength& other)
{
    this->srcGroupIdx = other.srcGroupIdx;
    this->bufLen = other.bufLen;
}

void BufferLength::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->srcGroupIdx);
    doParsimPacking(b,this->bufLen);
}

void BufferLength::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->srcGroupIdx);
    doParsimUnpacking(b,this->bufLen);
}

int BufferLength::getSrcGroupIdx() const
{
    return this->srcGroupIdx;
}

void BufferLength::setSrcGroupIdx(int srcGroupIdx)
{
    this->srcGroupIdx = srcGroupIdx;
}

IntVector& BufferLength::getBufLen()
{
    return this->bufLen;
}

void BufferLength::setBufLen(const IntVector& bufLen)
{
    this->bufLen = bufLen;
}

class BufferLengthDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    BufferLengthDescriptor();
    virtual ~BufferLengthDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(BufferLengthDescriptor)

BufferLengthDescriptor::BufferLengthDescriptor() : omnetpp::cClassDescriptor("BufferLength", "omnetpp::cPacket")
{
    propertynames = nullptr;
}

BufferLengthDescriptor::~BufferLengthDescriptor()
{
    delete[] propertynames;
}

bool BufferLengthDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<BufferLength *>(obj)!=nullptr;
}

const char **BufferLengthDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *BufferLengthDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int BufferLengthDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount() : 2;
}

unsigned int BufferLengthDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
    };
    return (field>=0 && field<2) ? fieldTypeFlags[field] : 0;
}

const char *BufferLengthDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "srcGroupIdx",
        "bufLen",
    };
    return (field>=0 && field<2) ? fieldNames[field] : nullptr;
}

int BufferLengthDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "srcGroupIdx")==0) return base+0;
    if (fieldName[0]=='b' && strcmp(fieldName, "bufLen")==0) return base+1;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *BufferLengthDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "IntVector",
    };
    return (field>=0 && field<2) ? fieldTypeStrings[field] : nullptr;
}

const char **BufferLengthDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *BufferLengthDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int BufferLengthDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    BufferLength *pp = (BufferLength *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *BufferLengthDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    BufferLength *pp = (BufferLength *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string BufferLengthDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    BufferLength *pp = (BufferLength *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getSrcGroupIdx());
        case 1: {std::stringstream out; out << pp->getBufLen(); return out.str();}
        default: return "";
    }
}

bool BufferLengthDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    BufferLength *pp = (BufferLength *)object; (void)pp;
    switch (field) {
        case 0: pp->setSrcGroupIdx(string2long(value)); return true;
        default: return false;
    }
}

const char *BufferLengthDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case 1: return omnetpp::opp_typename(typeid(IntVector));
        default: return nullptr;
    };
}

void *BufferLengthDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    BufferLength *pp = (BufferLength *)object; (void)pp;
    switch (field) {
        case 1: return (void *)(&pp->getBufLen()); break;
        default: return nullptr;
    }
}

Register_Class(pathLength)

pathLength::pathLength(const char *name, short kind) : ::omnetpp::cPacket(name,kind)
{
    this->recPathLength = 0;
}

pathLength::pathLength(const pathLength& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

pathLength::~pathLength()
{
}

pathLength& pathLength::operator=(const pathLength& other)
{
    if (this==&other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void pathLength::copy(const pathLength& other)
{
    this->recPathLength = other.recPathLength;
}

void pathLength::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->recPathLength);
}

void pathLength::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->recPathLength);
}

double pathLength::getRecPathLength() const
{
    return this->recPathLength;
}

void pathLength::setRecPathLength(double recPathLength)
{
    this->recPathLength = recPathLength;
}

class pathLengthDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    pathLengthDescriptor();
    virtual ~pathLengthDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(pathLengthDescriptor)

pathLengthDescriptor::pathLengthDescriptor() : omnetpp::cClassDescriptor("pathLength", "omnetpp::cPacket")
{
    propertynames = nullptr;
}

pathLengthDescriptor::~pathLengthDescriptor()
{
    delete[] propertynames;
}

bool pathLengthDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<pathLength *>(obj)!=nullptr;
}

const char **pathLengthDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *pathLengthDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int pathLengthDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 1+basedesc->getFieldCount() : 1;
}

unsigned int pathLengthDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
    };
    return (field>=0 && field<1) ? fieldTypeFlags[field] : 0;
}

const char *pathLengthDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "recPathLength",
    };
    return (field>=0 && field<1) ? fieldNames[field] : nullptr;
}

int pathLengthDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='r' && strcmp(fieldName, "recPathLength")==0) return base+0;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *pathLengthDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "double",
    };
    return (field>=0 && field<1) ? fieldTypeStrings[field] : nullptr;
}

const char **pathLengthDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *pathLengthDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int pathLengthDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    pathLength *pp = (pathLength *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *pathLengthDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    pathLength *pp = (pathLength *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string pathLengthDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    pathLength *pp = (pathLength *)object; (void)pp;
    switch (field) {
        case 0: return double2string(pp->getRecPathLength());
        default: return "";
    }
}

bool pathLengthDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    pathLength *pp = (pathLength *)object; (void)pp;
    switch (field) {
        case 0: pp->setRecPathLength(string2double(value)); return true;
        default: return false;
    }
}

const char *pathLengthDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *pathLengthDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    pathLength *pp = (pathLength *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}


