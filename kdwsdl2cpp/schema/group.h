#ifndef GROUP_H
#define GROUP_H

#include <schema/xmlelement.h>
#include <schema/element.h>
#include <common/qname.h>
#include <qdebug.h>

#include <kode_export.h>

namespace XSD
{

class SCHEMA_EXPORT Group : public XmlElement
{
public:
    typedef QList<Group> List;

    Group();
    Group(const Group &other);
    ~Group();

    Group &operator=(const Group &other);

    void setReference(const QName &reference);
    QName reference() const;

    void setElements(const Element::List &elements);
    Element::List elements() const;

    bool isResolved() const;

private:
    class Private;
    Private *d;
};

}

SCHEMA_EXPORT QDebug operator<<(QDebug dbg, const XSD::Group &group);

#endif // GROUP_H
