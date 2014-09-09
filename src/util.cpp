#include "util.h"

#include <QStringList>

bool isValidMacAddress(const QString &address)
{
    QStringList list = address.split(":");

    return ((list.size() == 6) &&
            ((list.at(0).toInt() != 0) ||
             (list.at(1).toInt() != 0) ||
             (list.at(2).toInt() != 0) ||
             (list.at(3).toInt() != 0) ||
             (list.at(4).toInt() != 0) ||
             (list.at(5).toInt() != 0)));
}
