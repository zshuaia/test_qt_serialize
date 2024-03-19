#ifndef TEST_H
#define TEST_H

#include <QList>
#include <QStringList>
#include <QMetaObject>
#include <QMetaProperty>
#include <QJsonDocument>
#include <QVariantMap>

#include <QMetaObject>
#include <QMetaType>


// The EXPAND macro here is only needed for MSVC:
// https://stackoverflow.com/questions/5134523/msvc-doesnt-expand-va-args-correctly
#define EXPAND( x ) x
#define PROP_GET_MACRO(_1, _2, _3, _4, NAME,...) NAME

// QObject
// ============

#define PROP_GETSET(...) \
EXPAND( PROP_GET_MACRO(__VA_ARGS__, PROP_GETSET4, PROP_GETSET3, PROP_GETSET2)(__VA_ARGS__) )
#define PROP_GET(...) \
EXPAND( PROP_GET_MACRO(__VA_ARGS__, PROP_GET4, PROP_GET3, PROP_GET2)(__VA_ARGS__) )

// A read-write prop both in C++ and in QML
// ========================================
#define _INT_DECL_PROP_GETSET(type, propname, Propname)    \
Q_PROPERTY(type propname READ propname WRITE set ## Propname NOTIFY propname ## Changed) \
    public:                                                \
    type propname() const { return m_##propname ; }    \
    Q_SIGNAL void propname ## Changed();                   \
    private:                                               \

#define PROP_GETSET_(type, propname, Propname)             \
    _INT_DECL_PROP_GETSET(type, propname, Propname)        \
    public Q_SLOTS:                                        \
    void set ## Propname(type value)                   \
{                                                  \
        m_ ## propname = value;                        \
        emit propname ## Changed();                    \
}                                                  \
    private:

#define PROP_GETSET4(type, name, Propname, defaultValue)   \
              PROP_GETSET_(type, name, Propname)                     \
              type m_##name = defaultValue;

#define PROP_GETSET3(type, name, Propname)                 \
PROP_GETSET_(type, name, Propname)                     \
    type m_##name;

// A readonly prop both in C++ and in QML
// ========================================
#define _INT_DECL_PROP_GET(type, propname)                 \
public:                                                \
    type propname() const { return m_##propname ; }    \

#define PROP_GET_(type, propname)                          \
    _INT_DECL_PROP_GET(type, propname)                     \
    private:

#define PROP_GET3(type, propname, defaultValue)            \
              PROP_GET_(type, propname)                              \
              type m_##propname = defaultValue;

#define PROP_GET2(type, propname)                          \
PROP_GET_(type, propname)                              \
    type m_##propname;


// QGadget
// ============

#define GPROP_GETSET(...) \
EXPAND( PROP_GET_MACRO(__VA_ARGS__, GPROP_GETSET4, GPROP_GETSET3, GPROP_GETSET2)(__VA_ARGS__) )
#define GPROP_GET(...) \
EXPAND( PROP_GET_MACRO(__VA_ARGS__, GPROP_GET4, GPROP_GET3, GPROP_GET2)(__VA_ARGS__) )

// A read-write prop both in C++ and in QML
// ========================================
#define _INT_DECL_GPROP_GETSET(type, propname, Propname)    \
Q_PROPERTY(type propname READ propname WRITE set ## Propname) \
    public:                                                \
    type propname() const { return m_##propname ; }    \
    private:                                               \

#define GPROP_GETSET_(type, propname, Propname)             \
    _INT_DECL_GPROP_GETSET(type, propname, Propname)        \
    public Q_SLOTS:                                        \
    void set ## Propname(type value)                   \
{                                                  \
        m_ ## propname = value;                        \
}                                                  \
    private:

#define GPROP_GETSET4(type, name, Propname, defaultValue)   \
              GPROP_GETSET_(type, name, Propname)                     \
              type m_##name = defaultValue;

#define GPROP_GETSET3(type, name, Propname)                 \
GPROP_GETSET_(type, name, Propname)                     \
    type m_##name;

// A readonly prop both in C++ and in QML
// ========================================
#define _INT_DECL_GPROP_GET(type, propname)                 \
public:                                                \
    type propname() const { return m_##propname ; }    \

#define PROP_GET_(type, propname)                          \
    _INT_DECL_GPROP_GET(type, propname)                     \
    private:

#define PROP_GET3(type, propname, defaultValue)            \
              PROP_GET_(type, propname)                              \
              type m_##propname = defaultValue;

#define PROP_GET2(type, propname)                          \
PROP_GET_(type, propname)                              \
    type m_##propname;

// ============================= serialize ====================================
static QVariantList _toVariantList     (const QVariant &list);
static QVariant     readVariant        (const QVariant &value);
static QVariantMap  gadgetToVariantMap (const void *gadget, const QMetaObject *staticMetaObject);

static QVariantList _toVariantList(const QVariant &list)
{
    QVariantList valueList;
    foreach (const QVariant &valueItem, list.value<QSequentialIterable>()) {
        qInfo() << "_toVariantList" << valueItem.typeName() << valueItem;
        valueList.append(readVariant(valueItem));
    }

    return valueList;
}

static QVariant readVariant(const QVariant &value)
{
    QMetaType metaType(QMetaType::type(value.typeName()));

    qInfo() << "readVariant" << value.typeName() << value;

    if(metaType.id() < QMetaType::User)
    {
        return value;
    }

    if(metaType.flags() & QMetaType::IsGadget)
    {
        return gadgetToVariantMap(value.data(), metaType.metaObject());
    }

    if(QString(value.typeName()).startsWith("QList"))
    {
        return _toVariantList(value);
    }
}

static QVariantMap gadgetToVariantMap(const void *gadget, const QMetaObject *staticMetaObject)
{
    QVariantMap variantMap;
    int propertyCount = staticMetaObject->propertyCount();

    for(int i = 0; i < staticMetaObject->propertyCount(); ++i)
    {
        const QMetaProperty metaProperty = staticMetaObject->property(i);
        QVariant value = metaProperty.readOnGadget(gadget);

        variantMap.insert(metaProperty.name(), readVariant(value));

    }
    return variantMap;
}

// ============================ deserialize =============================

static QVariant deserializeJsonValue(const QVariant &value, const QMetaType &metaType);
static QVariant deserializeJsonList(const QVariantList &valueList, const QMetaType &metaType);
static QVariant deserializeJsonMap(const QVariantMap &valueMap, const QMetaType &metaType);
/**
 * QJsonValue->toVariant()
*/
static QVariant deserializeJsonValue(const QVariant &value, const QMetaType &metaType)
{
    // const QMetaType fromMetaType(QMetaType::type(value.typeName()));

    if (value.canConvert<QVariantList>()) {
        const QVariantList valueList = value.value<QVariantList>();
        return deserializeJsonList(valueList, metaType);

    }
    else if (value.canConvert<QVariantMap>()) {
        const QVariantMap valueMap = value.value<QVariantMap>();
        return deserializeJsonMap(valueMap, metaType);
    }
    else if (value.canConvert(metaType.id()))
    {
        // value.convert(metaType.id());
        return value;
    }
}

static QVariant deserializeJsonList(const QVariantList &valueList, const QMetaType &metaType)
{
    const QMetaObject *metaObject = metaType.metaObject();

    // ???
    // metaType is "QList<AAA>", How create a QList<AAA> from metaType, and convert to QVarint;

    QVariant variant(metaType.id(), metaType.create());
    
    // ??? how append to `variant` from `AAA`

    return variant;
}

/**
 * @valueMap, QJsonValue->toVariant()
 * @metaType, Q_GADGET class type
*/
static QVariant deserializeJsonMap(const QVariantMap &valueMap, const QMetaType &metaType)
{
    const QMetaObject *metaObject = metaType.metaObject();

    QVariant gadgetVariant(metaType.id(), metaType.create());

    for(int i = 0; i < metaObject->propertyCount(); ++i)
    {
        const QMetaProperty metaProperty = metaObject->property(i);

        if(!valueMap.contains(metaProperty.name())) continue;

        QVariant value = valueMap[metaProperty.name()];
        const QMetaType toMetaType(QMetaType::type(metaProperty.typeName()));

        metaProperty.writeOnGadget(gadgetVariant.data(), deserializeJsonValue(value, toMetaType));
    }

    return gadgetVariant;
}


struct AAA
{
    Q_GADGET

    GPROP_GETSET(int, a_int, A_int)
};
Q_DECLARE_METATYPE(AAA)

struct BBB
{
    Q_GADGET

    GPROP_GETSET(int, b_int, B_int)

    GPROP_GETSET(AAA, b_aaa, B_aaa)

    GPROP_GETSET(QList<AAA>, b_list_aaa, B_list_aaa)
};
Q_DECLARE_METATYPE(BBB)

#endif // TEST_H
