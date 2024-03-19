#include <QCoreApplication>
#include <QDebug>
#include <QJsonObject>
#include <QTemporaryFile>
#include "test.h"

#include <QFileInfo>
#include <QDir>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    AAA aaa0;aaa0.setA_int(99);


    QList<AAA> aaaList;
    AAA aaa1;aaa1.setA_int(999);
    AAA aaa2;aaa2.setA_int(888);
    aaaList.append(aaa1);
    aaaList.append(aaa2);
#if 0

    QVariant test_aaaList = QVariant(qMetaTypeId<QList<AAA>>(), &aaaList);
    qInfo() << "test_aaaList.canConvert(QList<AAA>)=" << test_aaaList.canConvert<QList<AAA>>();
    qInfo() << "test_aaaList========" << test_aaaList.value<QSequentialIterable>().size();
    foreach (const QVariant &valueItem, test_aaaList.value<QSequentialIterable>()) {

        qInfo() << "test_aaaList========" << valueItem.typeName();
    }
#endif

    BBB bbb;
    bbb.setB_int(666);
    bbb.setB_aaa(aaa0);
    bbb.setB_list_aaa(aaaList);

    //============================ test serialize==========================================
    QString jsonStr = QJsonDocument::fromVariant(gadgetToVariantMap(&bbb, &BBB::staticMetaObject)).toJson(QJsonDocument::Compact);
    qInfo() << "serialize:" << jsonStr;

    //============================ test deserialize==========================================

    QVariant retValue = deserializeJsonMap(QJsonDocument::fromJson(jsonStr.toUtf8()).toVariant().toMap(), QMetaType::fromType<BBB>());

    BBB asdsad = retValue.value<BBB>();

    qInfo() << asdsad.b_aaa().a_int() << asdsad.b_int() << asdsad.b_list_aaa().size();

    return a.exec();
}
