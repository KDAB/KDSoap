/*
    This file is part of KDE Schema Parser

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2006 Michaël Larouche <michael.larouche@kdemail.net>
                       based on wsdlpull parser by Vivek Krishna

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */

#include <QDir>
#include <QFile>
#include <QUrl>
#include <QXmlSimpleReader>
#include <QtDebug>
#include <QtCore/QLatin1String>

#include <common/fileprovider.h>
#include <common/messagehandler.h>
#include <common/nsmanager.h>
#include <common/parsercontext.h>
#include "parser.h"

static const QString XMLSchemaURI( QLatin1String("http://www.w3.org/2001/XMLSchema") );
static const QString WSDLSchemaURI( QLatin1String("http://schemas.xmlsoap.org/wsdl/") );
static const QString soapEncNs = QLatin1String("http://schemas.xmlsoap.org/soap/encoding/");
static const QString soap12EncNs = QLatin1String("http://www.w3.org/2003/05/soap-encoding");

namespace XSD {

class Parser::Private
{
public:
    QString mNameSpace;
    bool mDefaultQualified;

    SimpleType::List mSimpleTypes;
    ComplexType::List mComplexTypes;
    Element::List mElements;
    Attribute::List mAttributes;
    AttributeGroup::List mAttributeGroups;
    Annotation::List mAnnotations;

    QStringList mImportedSchemas;
    QStringList mIncludedSchemas;
    QStringList mNamespaces;
};

Parser::Parser( ParserContext *context, const QString &nameSpace )
  : d(new Private)
{
  d->mNameSpace = nameSpace;
  d->mDefaultQualified = false;
  init(context);
}

Parser::Parser( const Parser &other )
  : d(new Private)
{
  *d = *other.d;
}

Parser::~Parser()
{
  clear();
  delete d;
}

Parser &Parser::operator=( const Parser &other )
{
  if ( this == &other )
    return *this;

  *d = *other.d;

  return *this;
}

void Parser::clear()
{
  d->mImportedSchemas.clear();
  d->mNamespaces.clear();
  d->mComplexTypes.clear();
  d->mSimpleTypes.clear();
  d->mElements.clear();
  d->mAttributes.clear();
  d->mAttributeGroups.clear();
}

void Parser::init(ParserContext *context)
{
#if 0
  if (!parseFile(context, ":/schema/XMLSchema.xsd")) {
      qWarning("Error parsing builtin file XMLSchema.xsd");
  }
#else
  Q_UNUSED(context);
#endif

    // From the XML schema XSD
    {
        Element schema(XMLSchemaURI);
        schema.setName(QLatin1String("schema"));
        schema.setType(QName(XMLSchemaURI, QLatin1String("anyType")));
        d->mElements.append(schema);
    }

    {
        Attribute langAttr(XMLSchemaURI);
        langAttr.setName(QLatin1String("lang"));
        langAttr.setType(QName(XMLSchemaURI, QLatin1String("string")));
        d->mAttributes.append(langAttr);
    }


  // From http://schemas.xmlsoap.org/wsdl/soap/encoding
  {
      ComplexType array(soapEncNs);
      array.setArrayType(QName(XMLSchemaURI, QString::fromLatin1("any")));
      array.setName(QLatin1String("Array"));
      d->mComplexTypes.append(array);
  }

  // From http://schemas.xmlsoap.org/soap/encoding/, so that <attribute ref="soap-enc:arrayType" arrayType="kdab:EmployeeAchievement[]"/>
  // can be resolved.
  {
      Attribute arrayTypeAttr(soapEncNs);
      arrayTypeAttr.setName(QLatin1String("arrayType"));
      arrayTypeAttr.setType(QName(XMLSchemaURI, QLatin1String("string")));
      d->mAttributes.append(arrayTypeAttr);
  }

  // Same thing, but for SOAP-1.2: from http://www.w3.org/2003/05/soap-encoding
  {
      ComplexType array(soap12EncNs);
      array.setArrayType(QName(XMLSchemaURI, QString::fromLatin1("any")));
      array.setName(QLatin1String("Array"));
      d->mComplexTypes.append(array);
  }
  {
      Attribute arrayTypeAttr(soap12EncNs);
      arrayTypeAttr.setName(QLatin1String("arrayType"));
      arrayTypeAttr.setType(QName(XMLSchemaURI, QLatin1String("string")));
      d->mAttributes.append(arrayTypeAttr);
  }

}

bool Parser::parseFile( ParserContext *context, const QString &fileName )
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Error opening file %s", qPrintable(fileName));
        return false;
    } else {
        QXmlInputSource source( &file );
        return parse( context, &source );
    }
}

// currently unused
bool Parser::parseString( ParserContext *context, const QString &data )
{
  QXmlInputSource source;
  source.setData( data );
  return parse( context, &source );
}

bool Parser::parse( ParserContext *context, QXmlInputSource *source )
{
  QXmlSimpleReader reader;
  reader.setFeature( QLatin1String("http://xml.org/sax/features/namespace-prefixes"), true );

  QString errorMsg;
  int errorLine, errorCol;
  QDomDocument doc;
  if ( !doc.setContent( source, &reader, &errorMsg, &errorLine, &errorCol ) ) {
    qDebug( "%s at (%d,%d)", qPrintable( errorMsg ), errorLine, errorCol );
    return false;
  }

  QDomElement element = doc.documentElement();
  const QName name( element.tagName() );
  if ( name.localName() != QLatin1String("schema") ) {
    qDebug( "document element is '%s'", qPrintable( element.tagName() ) );
    return false;
  }

  return parseSchemaTag( context, element );
}

bool Parser::parseSchemaTag( ParserContext *context, const QDomElement &root )
{
  QName name(root.tagName());
  if ( name.localName() != QLatin1String("schema") ) {
    qDebug() << "ERROR localName=" << name.localName();
    return false;
  }

  // Already done by caller when coming from type.cpp, but doesn't hurt to do twice
  context->namespaceManager()->enterChild(root);

  // This method can call itself recursively, so save/restore the member attribute.
  QString oldNamespace = d->mNameSpace;
  if ( root.hasAttribute( QLatin1String("targetNamespace") ) )
    d->mNameSpace = root.attribute( QLatin1String("targetNamespace") );

  if ( root.hasAttribute( QLatin1String("elementFormDefault") ) )
      if (root.attribute( QLatin1String("elementFormDefault") ) == QLatin1String("qualified"))
          d->mDefaultQualified = true;

 // mTypesTable.setTargetNamespace( mNameSpace );

  QDomElement element = root.firstChildElement();
  while ( !element.isNull() ) {
    NSManager namespaceManager( context, element );
    const QName name( element.tagName() );
    if (debugParsing())
        qDebug() << "Schema: parsing" << name.localName();
    if ( name.localName() == QLatin1String("import") ) {
      parseImport( context, element );
    } else if ( name.localName() == QLatin1String("element") ) {
      addGlobalElement( parseElement( context, element, d->mNameSpace, element ) );
    } else if ( name.localName() == QLatin1String("complexType") ) {
      ComplexType ct = parseComplexType( context, element );
      d->mComplexTypes.append( ct );
    } else if ( name.localName() == QLatin1String("simpleType") ) {
      SimpleType st = parseSimpleType( context, element );
      d->mSimpleTypes.append( st );
    } else if ( name.localName() == QLatin1String("attribute") ) {
      addGlobalAttribute( parseAttribute( context, element ) );
    } else if ( name.localName() == QLatin1String("attributeGroup") ) {
      d->mAttributeGroups.append( parseAttributeGroup( context, element ) );
    } else if ( name.localName() == QLatin1String("annotation") ) {
      d->mAnnotations = parseAnnotation( context, element );
    } else if ( name.localName() == QLatin1String("include") ) {
      parseInclude( context, element );
    }

    element = element.nextSiblingElement();
  }

  d->mNamespaces = joinNamespaces( d->mNamespaces, context->namespaceManager()->uris() );
  d->mNamespaces = joinNamespaces( d->mNamespaces, QStringList( d->mNameSpace ) );

  if (!resolveForwardDeclarations())
      return false;

  d->mNameSpace = oldNamespace;

  return true;
}

void Parser::parseImport( ParserContext *context, const QDomElement &element )
{
  QString location = element.attribute( QLatin1String("schemaLocation") );

  if ( location.isEmpty() )
    return; // Testcase: <s:import namespace="http://microsoft.com/wsdl/types/" /> in the WSDL at https://www.elogbook.org/logbookws/logbookifv3.asmx

  // don't import a schema twice
  if ( d->mImportedSchemas.contains( location ) )
      return;
  else
    d->mImportedSchemas.append( location );

  importSchema( context, location );
}

void Parser::parseInclude( ParserContext *context, const QDomElement &element )
{
  QString location = element.attribute( QLatin1String("schemaLocation") );

  if( !location.isEmpty() ) {
    // don't include a schema twice
    if ( d->mIncludedSchemas.contains( location ) )
      return;
    else
      d->mIncludedSchemas.append( location );

    includeSchema( context, location );
  }
  else {
    context->messageHandler()->warning( QString::fromLatin1("include tag found at (%1, %2) contains no schemaLocation tag.").arg( element.lineNumber(), element.columnNumber() ) );
  }
}

Annotation::List Parser::parseAnnotation( ParserContext *context, const QDomElement &element )
{
  Annotation::List result;

  QDomElement child;
  for( child = element.firstChildElement(); !child.isNull();
       child = child.nextSiblingElement() ) {
    NSManager namespaceManager( context, child );
    const QName name( child.tagName() );
    if ( name.localName() == QLatin1String("documentation") ) {
      result.append( Annotation( child ) );
    } else if ( name.localName() == QLatin1String("appinfo") ) {
      result.append( Annotation( child ) );
    }
  }

  return result;
}

ComplexType Parser::parseComplexType( ParserContext *context, const QDomElement &element )
{
  ComplexType newType( d->mNameSpace );

  newType.setName( element.attribute( QLatin1String("name") ) );

  if (debugParsing())
      qDebug() << "complexType:" << d->mNameSpace << newType.name();

  if ( element.hasAttribute( QLatin1String("mixed") ) )
    newType.setContentModel( XSDType::MIXED );

  QDomElement childElement = element.firstChildElement();

  AttributeGroup::List attributeGroups;

  while ( !childElement.isNull() ) {
    NSManager namespaceManager( context, childElement );
    const QName name( childElement.tagName() );
    if ( name.localName() == QLatin1String("all") ) {
      all( context, childElement, newType );
    } else if ( name.localName() == QLatin1String("sequence") ) {
      parseCompositor( context, childElement, newType );
    } else if ( name.localName() == QLatin1String("choice") ) {
      parseCompositor( context, childElement, newType );
    } else if ( name.localName() == QLatin1String("attribute") ) {
      newType.addAttribute( parseAttribute( context, childElement ) );
    } else if ( name.localName() == QLatin1String("attributeGroup") ) {
      AttributeGroup g = parseAttributeGroup( context, childElement );
      attributeGroups.append( g );
    } else if ( name.localName() == QLatin1String("anyAttribute") ) {
      addAnyAttribute( context, childElement, newType );
    } else if ( name.localName() == QLatin1String("complexContent") ) {
      parseComplexContent( context, childElement, newType );
    } else if ( name.localName() == QLatin1String("simpleContent") ) {
      parseSimpleContent( context, childElement, newType );
    } else if ( name.localName() == QLatin1String("annotation") ) {
      Annotation::List annotations = parseAnnotation( context, childElement );
      newType.setDocumentation( annotations.documentation() );
      newType.setAnnotations( annotations );
    }

    childElement = childElement.nextSiblingElement();
  }

  newType.setAttributeGroups( attributeGroups );

  return newType;
}

void Parser::all( ParserContext *context, const QDomElement &element, ComplexType &ct )
{
  QDomElement childElement = element.firstChildElement();

  while ( !childElement.isNull() ) {
    NSManager namespaceManager( context, childElement );
    const QName name( childElement.tagName() );
    if ( name.localName() == QLatin1String("element") ) {
      ct.addElement( parseElement( context, childElement, ct.nameSpace(),
        childElement ) );
    } else if ( name.localName() == QLatin1String("annotation") ) {
      Annotation::List annotations = parseAnnotation( context, childElement );
      ct.setDocumentation( annotations.documentation() );
      ct.setAnnotations( annotations );
    }

    childElement = childElement.nextSiblingElement();
  }
}

static int readMaxOccurs( const QDomElement& element )
{
    const QString value = element.attribute( QLatin1String("maxOccurs"), QLatin1String("1") );
    if ( value == QLatin1String("unbounded") )
        return Parser::UNBOUNDED;
    else
        return value.toInt();
}

void Parser::parseCompositor( ParserContext *context,
  const QDomElement &element, ComplexType &ct )
{
  const QName name( element.tagName() );
  bool isChoice = name.localName() == QLatin1String("choice");
  bool isSequence = name.localName() == QLatin1String("sequence");

  Compositor compositor;
  if ( isChoice ) compositor.setType( Compositor::Choice );
  else if ( isSequence ) compositor.setType( Compositor::Sequence );
  compositor.setMaxOccurs( readMaxOccurs( element ) );

  if ( isChoice || isSequence ) {
    Element::List newElements;
    QDomElement childElement = element.firstChildElement();

    while ( !childElement.isNull() ) {
      NSManager namespaceManager( context, childElement );
      const QName csName( childElement.tagName() );
      if ( csName.localName() == QLatin1String("element") ) {
        Element newElement;
        if ( isChoice ) {
          newElement = parseElement( context, childElement,
            ct.nameSpace(), element );
        } else {
          newElement = parseElement( context, childElement,
            ct.nameSpace(), childElement );
        }
        newElements.append( newElement );
        compositor.addChild( csName );
      } else if ( csName.localName() == QLatin1String("any") ) {
        addAny( context, childElement, ct );
      } else if ( isChoice ) {
        parseCompositor( context, childElement, ct );
      } else if ( isSequence ) {
        parseCompositor( context, childElement, ct );
      }

      childElement = childElement.nextSiblingElement();
    }

    foreach( Element e, newElements ) {
      e.setCompositor( compositor );
      ct.addElement( e );
    }
  }
}

Element Parser::parseElement( ParserContext *context,
  const QDomElement &element, const QString &nameSpace,
  const QDomElement &occurrenceElement )
{
  Element newElement( nameSpace );

  newElement.setName( element.attribute( QLatin1String("name") ) );
  if (debugParsing())
      qDebug() << "newElement namespace=" << nameSpace << "name=" << newElement.name() << "defaultQualified=" << d->mDefaultQualified;

  newElement.setIsQualified(d->mDefaultQualified);
  if ( element.hasAttribute( QLatin1String("form") ) ) {
    if ( element.attribute( QLatin1String("form") ) == QLatin1String("qualified") )
      newElement.setIsQualified( true );
    else if ( element.attribute( QLatin1String("form") ) == QLatin1String("unqualified") )
      newElement.setIsQualified( false );
    else
      newElement.setIsQualified( false );
  }

  if ( element.hasAttribute( QLatin1String("ref") ) ) {
    QName reference( element.attribute( QLatin1String("ref") ) );
    reference.setNameSpace( context->namespaceManager()->uri( reference.prefix() ) );
    newElement.setReference( reference );
  }

  setOccurrenceAttributes( newElement, occurrenceElement );

  newElement.setDefaultValue( element.attribute( QLatin1String("default") ) );
  newElement.setFixedValue( element.attribute( QLatin1String("fixed") ) );

  //bool nill = false;
  //if ( element.hasAttribute( "nillable" ) )
    //nill = true;

  if ( element.hasAttribute( QLatin1String("type") ) ) {
    QName typeName( element.attribute( QLatin1String("type") ) );
    typeName.setNameSpace( context->namespaceManager()->uri( typeName.prefix() ) );
    if (debugParsing())
        qDebug() << "typeName=" << typeName.qname() << "namespace=" << context->namespaceManager()->uri( typeName.prefix() );
    newElement.setType( typeName );
  } else {
    QDomElement childElement = element.firstChildElement();

    while ( !childElement.isNull() ) {
      NSManager namespaceManager( context, childElement );
      const QName childName( childElement.tagName() );
      //qDebug() << "childName:" << childName.localName();
      if ( childName.localName() == QLatin1String("complexType") ) {
        ComplexType ct = parseComplexType( context, childElement );

        ct.setName( newElement.name() );
        ct.setAnonymous( true );
        d->mComplexTypes.append( ct );

        if (debugParsing())
            qDebug() << " found nested complexType element, type name is now element name, i.e. " << ct.name() << "newElement.setType" << ct.qualifiedName();
        newElement.setType( ct.qualifiedName() );
      } else if ( childName.localName() == QLatin1String("simpleType") ) {
        SimpleType st = parseSimpleType( context, childElement );

        st.setName( newElement.name() );
        d->mSimpleTypes.append( st );

        newElement.setType( st.qualifiedName() );
      } else if ( childName.localName() == QLatin1String("annotation") ) {
        Annotation::List annotations = parseAnnotation( context, childElement );
        newElement.setDocumentation( annotations.documentation() );
        newElement.setAnnotations( annotations );
      }

      childElement = childElement.nextSiblingElement();
    }
  }

  return newElement;
}

// Testcase: salesforce-partner.wsdl has <any namespace="##targetNamespace" [...]/>
void Parser::addAny( ParserContext*, const QDomElement &element, ComplexType &complexType )
{
  Element newElement( complexType.nameSpace() );
  newElement.setName( QLatin1String("any") );
  QName anyType( QLatin1String("http://www.w3.org/2001/XMLSchema"), QLatin1String("any") );
  newElement.setType( anyType );
  setOccurrenceAttributes( newElement, element );

  complexType.addElement( newElement );
}

void Parser::setOccurrenceAttributes( Element &newElement,
  const QDomElement &element )
{
    newElement.setMinOccurs( element.attribute( QLatin1String("minOccurs"), QLatin1String("1") ).toInt() );
    newElement.setMaxOccurs( readMaxOccurs( element ) );
}

void Parser::addAnyAttribute( ParserContext*, const QDomElement &element, ComplexType &complexType )
{
  Attribute newAttribute;
  newAttribute.setName( QLatin1String("anyAttribute") );

  newAttribute.setNameSpace( element.attribute( QLatin1String("namespace") ) );

  // ### Hmm, technically, this should be a list of anys, I think.
  newAttribute.setType(QName(XMLSchemaURI, QString::fromLatin1("anyType")));

  complexType.addAttribute( newAttribute );
}

Attribute Parser::parseAttribute( ParserContext *context,
  const QDomElement &element )
{
  Attribute newAttribute;

  newAttribute.setName( element.attribute( QLatin1String("name") ) );

  if ( element.hasAttribute( QLatin1String("type") ) ) {
    // TODO pass nsmanager to QName so that it can resolve namespaces?
    QName typeName( element.attribute( QLatin1String("type") ) );
    typeName.setNameSpace( context->namespaceManager()->uri( typeName.prefix() ) );
    newAttribute.setType( typeName );
  }

  if ( element.hasAttribute( QLatin1String("form") ) ) {
    if ( element.attribute( QLatin1String("form") ) == QLatin1String("qualified") )
      newAttribute.setIsQualified( true );
    else if ( element.attribute( QLatin1String("form") ) == QLatin1String("unqualified") )
      newAttribute.setIsQualified( false );
    else
      newAttribute.setIsQualified( false );
  }

  if ( element.hasAttribute( QLatin1String("ref") ) ) {
    QName reference;
    reference = element.attribute( QLatin1String("ref") );
    reference.setNameSpace( context->namespaceManager()->uri( reference.prefix() ) );

    // MS Exchange doesn't define the xml prefix, and yet uses xml:lang...
    if (reference.nameSpace().isEmpty() && reference.prefix() == "xml")
        reference.setNameSpace( XMLSchemaURI );

    newAttribute.setReference( reference );
  }

  newAttribute.setDefaultValue( element.attribute( QLatin1String("default") ) );
  newAttribute.setFixedValue( element.attribute( QLatin1String("fixed") ) );

  if ( element.hasAttribute( QLatin1String("use") ) ) {
    if ( element.attribute( QLatin1String("use") ) == QLatin1String("optional") )
      newAttribute.setIsUsed( false );
    else if ( element.attribute( QLatin1String("use") ) == QLatin1String("required") )
      newAttribute.setIsUsed( true );
    else
      newAttribute.setIsUsed( false );
  }

  QDomElement childElement = element.firstChildElement();

  while ( !childElement.isNull() ) {
    NSManager namespaceManager( context, childElement );
    const QName childName( childElement.tagName() );
    if ( childName.localName() == QLatin1String("simpleType") ) {
      SimpleType st = parseSimpleType( context, childElement );
      st.setName( newAttribute.name() );
      d->mSimpleTypes.append( st );

      newAttribute.setType( st.qualifiedName() );
    } else if ( childName.localName() == QLatin1String("annotation") ) {
      Annotation::List annotations = parseAnnotation( context, childElement );
      newAttribute.setDocumentation( annotations.documentation() );
      newAttribute.setAnnotations( annotations );
    }

    childElement = childElement.nextSiblingElement();
  }

  return newAttribute;
}

SimpleType Parser::parseSimpleType( ParserContext *context, const QDomElement &element )
{
  SimpleType st( d->mNameSpace );

  st.setName( element.attribute( QLatin1String("name") ) );

  if (debugParsing())
      qDebug() << "simpleType:" << d->mNameSpace << st.name();

  QDomElement childElement = element.firstChildElement();

  while ( !childElement.isNull() ) {
    NSManager namespaceManager( context, childElement );
    const QName name( childElement.tagName() );
    if ( name.localName() == QLatin1String("restriction") ) {
      st.setSubType( SimpleType::TypeRestriction );

      QName typeName( childElement.attribute( QLatin1String("base") ) );
      typeName.setNameSpace( context->namespaceManager()->uri( typeName.prefix() ) );
      st.setBaseTypeName( typeName );

      parseRestriction( context, childElement, st );
    } else if ( name.localName() == QLatin1String("union") ) {
      st.setSubType( SimpleType::TypeUnion );
      // It means "the contents can be either one of my child elements, or one of the types listed in memberTypes".
      // For now we'll just use QVariant.
      // For more compile-time checking we would need to actually parse and store
      // the references to the possible types. And then generate methods for each;
      // but we won't have a good name for these methods, just some type name...
      // setSizebyno / setSizebystring reads weird.
      st.setBaseTypeName( QName(XMLSchemaURI, QString::fromLatin1("anyType")) ); // to get QVariant

    } else if ( name.localName() == QLatin1String("list") ) {
      st.setSubType( SimpleType::TypeList );
      if ( childElement.hasAttribute( QLatin1String("itemType") ) ) {
        QName typeName( childElement.attribute( QLatin1String("itemType") ) );
        if ( !typeName.prefix().isEmpty() )
          typeName.setNameSpace( context->namespaceManager()->uri( typeName.prefix() ) );
        else
          typeName.setNameSpace( st.nameSpace() );
        st.setListTypeName( typeName );
      } else {
        // Anonymous type
        QDomElement typeElement = childElement.firstChildElement();
        for ( ; !typeElement.isNull() ; typeElement = typeElement.nextSiblingElement() ) {
          NSManager namespaceManager( context, typeElement );
          const QName typeName( typeElement.tagName() );
          //qDebug() << "childName:" << childName.localName();
          if ( typeName.localName() == QLatin1String("complexType") ) {
            ComplexType ctItem = parseComplexType( context, typeElement );
            ctItem.setName( st.name() + QLatin1String("ListItem") ); // need to make something up, so that the classname looks good
            st.setListTypeName( ctItem.qualifiedName() );
            d->mComplexTypes.append( ctItem );
          } else if ( typeName.localName() == QLatin1String("simpleType") ) {
            SimpleType stItem = parseSimpleType( context, typeElement );
            stItem.setName( st.name() + QLatin1String("ListItem") ); // need to make something up, so that the classname looks good
            st.setListTypeName( stItem.qualifiedName() );
            d->mSimpleTypes.append( stItem );
          } else {
              qDebug() << "ERROR: parseSimpleType: unhandled: " << typeName.localName() << "in list" << d->mNameSpace << st.name();
          }
        }
      }
    } else if ( name.localName() == QLatin1String("annotation") ) {
      Annotation::List annotations = parseAnnotation( context, childElement );
      st.setDocumentation( annotations.documentation() );
      st.setAnnotations( annotations );
    }

    childElement = childElement.nextSiblingElement();
  }

  return st;
}

void Parser::parseRestriction( ParserContext* context, const QDomElement &element, SimpleType &st )
{
  if ( st.baseTypeName().isEmpty() )
    qDebug( "<restriction>: unknown BaseType" );

  QDomElement childElement = element.firstChildElement();

  while ( !childElement.isNull() ) {
    NSManager namespaceManager( context, childElement );
    const QName tagName( childElement.tagName() );
    if ( tagName.localName() == QLatin1String("annotation") ) {
      // Skip annotations here.
    } else {
        SimpleType::FacetType ft = st.parseFacetId( tagName.localName() );
        if ( ft == SimpleType::NONE ) {
            qDebug( "<restriction>: %s is not a valid facet for the simple type '%s'", qPrintable( childElement.tagName() ), qPrintable(st.name()) );
        } else {
            st.setFacetValue( ft, childElement.attribute( QLatin1String("value") ) );
        }
    }
    childElement = childElement.nextSiblingElement();
  }
}

void Parser::parseComplexContent( ParserContext *context, const QDomElement &element, ComplexType &complexType )
{
  QName typeName;

  complexType.setContentModel( XSDType::COMPLEX );

  QDomElement childElement = element.firstChildElement();

  while ( !childElement.isNull() ) {
    const QName name( childElement.tagName() );

    if ( name.localName() == QLatin1String("restriction") || name.localName() == QLatin1String("extension") ) {
      typeName = childElement.attribute( QLatin1String("base") );
      typeName.setNameSpace( context->namespaceManager()->uri( typeName.prefix() ) );

      if (typeName != QName(XMLSchemaURI, QString::fromLatin1("anyType"))) { // ignore this
        complexType.setBaseTypeName( typeName );
      }

      // if the base soapenc:Array, then read the arrayType attribute, and possibly the desired name for the child elements
      // TODO check namespace is really soap-encoding
      if ( typeName.localName() == QLatin1String("Array") ) {

          QString typeStr;
          QString arrayElementStr;
          QDomElement arrayElement = childElement.firstChildElement();
          while ( !arrayElement.isNull() ) {
              NSManager namespaceManager( context, arrayElement );
              const QName tagName( arrayElement.tagName() );
              if ( context->namespaceManager()->uri( tagName.prefix() ) == XMLSchemaURI ) {
                  const QString localName = tagName.localName();
                  if ( localName == QLatin1String("attribute") ) {
                      const QString prefix = context->namespaceManager()->prefix( WSDLSchemaURI );
                      const QString attributeName = ( prefix.isEmpty() ? QString::fromLatin1("arrayType") : prefix + QLatin1String(":arrayType") );

                      typeStr = arrayElement.attribute( attributeName );
                      if (typeStr.isEmpty()) {
                          qWarning("ERROR: arrayType attribute missing in Array element.");
                      }
                      if ( typeStr.endsWith( QLatin1String("[]") ) )
                          typeStr.truncate( typeStr.length() - 2 );
                  } else if ( localName == QLatin1String("sequence") ) { // detosagent-legacy.wsdl
                      arrayElement = arrayElement.firstChildElement(); // go down
                  }
                  if ( QName(arrayElement.tagName()).localName() == QLatin1String("element" ) ) {
                      arrayElementStr = arrayElement.attribute("name");
                      if ( localName == QLatin1String("sequence") ) { // go up again
                          arrayElement = arrayElement.parentNode().toElement();
                      }
                  }
              }
              arrayElement = arrayElement.nextSiblingElement();
          }
          if (typeStr.isEmpty()) {
              qWarning("ERROR: <attribute> element not found");
          } else {
              if (arrayElementStr.isEmpty())
                  arrayElementStr = QLatin1String("items"); // we have to call it something...

              QName arrayType( typeStr );
              arrayType.setNameSpace( context->namespaceManager()->uri( arrayType.prefix() ) );
              complexType.setArrayType( arrayType );

              Element items( complexType.nameSpace() );
              items.setName( arrayElementStr );
              items.setType( arrayType );
              //items.setArrayType( arrayType );
              complexType.addElement( items );

              //qDebug() << complexType.name() << "is array of" << arrayType;
          }

      } else {
        QDomElement ctElement = childElement.firstChildElement();
        while ( !ctElement.isNull() ) {
          NSManager namespaceManager( context, ctElement );
          const QName name( ctElement.tagName() );

          if ( name.localName() == QLatin1String("all") ) {
            all( context, ctElement, complexType );
          } else if ( name.localName() == QLatin1String("sequence") ) {
            parseCompositor( context, ctElement, complexType );
          } else if ( name.localName() == QLatin1String("choice") ) {
            parseCompositor( context, ctElement, complexType );
          } else if ( name.localName() == QLatin1String("attribute") ) {
            complexType.addAttribute( parseAttribute( context, ctElement ) );
          } else if ( name.localName() == QLatin1String("anyAttribute") ) {
            addAnyAttribute( context, ctElement, complexType );
          }

          ctElement = ctElement.nextSiblingElement();
        }
      }
    }

    childElement = childElement.nextSiblingElement();
  }
  if ( element.attribute( QLatin1String("mixed") ) == QLatin1String("true") ) {
    qDebug( "<complexContent>: No support for mixed=true" );
  }
}

void Parser::parseSimpleContent( ParserContext *context, const QDomElement &element, ComplexType &complexType )
{
  complexType.setContentModel( XSDType::SIMPLE );

  QDomElement childElement = element.firstChildElement();

  while ( !childElement.isNull() ) {
    NSManager namespaceManager( context, childElement );
    const QName name( childElement.tagName() );
    if ( name.localName() == QLatin1String("restriction") ) {
      SimpleType st( d->mNameSpace );

      if ( childElement.hasAttribute( QLatin1String("base") ) ) {
        QName typeName( childElement.attribute( QLatin1String("base") ) );
        typeName.setNameSpace( context->namespaceManager()->uri( typeName.prefix() ) );
        st.setBaseTypeName( typeName );
      }

      parseRestriction( context, childElement, st );
    } else if ( name.localName() == QLatin1String("extension") ) {
      // This extension does not use the full model that can come in ComplexContent.
      // It uses the simple model. No particle allowed, only attributes

      if ( childElement.hasAttribute( QLatin1String("base") ) ) {
        QName typeName( childElement.attribute( QLatin1String("base") ) );
        typeName.setNameSpace( context->namespaceManager()->uri( typeName.prefix() ) );
        complexType.setBaseTypeName( typeName );

        QDomElement ctElement = childElement.firstChildElement();
        while ( !ctElement.isNull() ) {
          NSManager namespaceManager( context, ctElement );
          const QName name( ctElement.tagName() );
          if ( name.localName() == QLatin1String("attribute") )
            complexType.addAttribute( parseAttribute( context, ctElement ) );

          ctElement = ctElement.nextSiblingElement();
        }
      }
    }

    childElement = childElement.nextSiblingElement();
  }
}

void Parser::addGlobalElement( const Element &newElement )
{
    //qDebug() << "Adding global element" << newElement.qualifiedName();

    // don't add elements twice
  bool found = false;
  for ( int i = 0; i < d->mElements.count(); ++i ) {
    if ( d->mElements[ i ].qualifiedName() == newElement.qualifiedName() ) {
      found = true;
      break;
    }
  }

  if ( !found ) {
    d->mElements.append( newElement );
  }
}

void Parser::addGlobalAttribute( const Attribute &newAttribute )
{
  // don't add attributes twice
  bool found = false;
  for ( int i = 0; i < d->mAttributes.count(); ++i ) {
    if ( d->mAttributes[ i ].qualifiedName() == newAttribute.qualifiedName() ) {
      found = true;
      break;
    }
  }

  if ( !found ) {
    d->mAttributes.append( newAttribute );
  }
}

AttributeGroup Parser::parseAttributeGroup( ParserContext *context,
  const QDomElement &element )
{
  Attribute::List attributes;

  AttributeGroup group;

  if ( element.hasAttribute( QLatin1String("ref") ) ) {
    QName reference;
    reference = element.attribute( QLatin1String("ref") );
    reference.setNameSpace(
      context->namespaceManager()->uri( reference.prefix() ) );
    group.setReference( reference );

    return group;
  }

  QDomNode n;
  for( n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement e = n.toElement();
    QName childName = QName( e.tagName() );
    if ( childName.localName() == QLatin1String("attribute") ) {
      Attribute a = parseAttribute( context, e );
      addGlobalAttribute( a );
      attributes.append( a );
    }
  }

  group.setName( element.attribute( QLatin1String("name") ) );
  group.setAttributes( attributes );

  return group;
}

QString Parser::targetNamespace() const
{
  return d->mNameSpace;
}

static QUrl urlForLocation(ParserContext *context, const QString& location)
{
    QUrl url( location );
    if ((url.scheme().isEmpty() || url.scheme() == QLatin1String("file"))) {
        QDir dir( location );
        if (dir.isRelative()) {
            url = context->documentBaseUrl();
            url.setPath( url.path() + QLatin1Char('/') + location );
        }
    }
    return url;
}

// Note: http://www.w3.org/TR/xmlschema-0/#schemaLocation paragraph 3 (for <import>) says
// "schemaLocation is only a hint"
void Parser::importSchema( ParserContext *context, const QString &location )
{
    // Ignore this one, we have it built into the typemap
    if (location == soapEncNs)
        return;
    // Ignore this one, we don't need it, and it relies on soap/encoding
    if (location == QLatin1String("http://schemas.xmlsoap.org/wsdl/"))
        return;

    if (location.startsWith(QLatin1String("urn:"))) // Can't download that :-)
        return;

  FileProvider provider;
  QString fileName;
  const QUrl schemaLocation = urlForLocation(context, location);
  qDebug("importing schema at %s", schemaLocation.toEncoded().constData());
  if ( provider.get( schemaLocation, fileName ) ) {
    QFile file( fileName );
    if ( !file.open( QIODevice::ReadOnly ) ) {
      qDebug( "Unable to open file %s", qPrintable( file.fileName() ) );
      return;
    }

    QXmlInputSource source( &file );
    QXmlSimpleReader reader;
    reader.setFeature( QLatin1String("http://xml.org/sax/features/namespace-prefixes"), true );

    QDomDocument doc( QLatin1String("kwsdl") );
    QString errorMsg;
    int errorLine, errorColumn;
    bool ok = doc.setContent( &source, &reader, &errorMsg, &errorLine, &errorColumn );
    if ( !ok ) {
      qDebug( "Error[%d:%d] %s", errorLine, errorColumn, qPrintable( errorMsg ) );
      return;
    }

    QDomElement node = doc.documentElement();

    NSManager namespaceManager( context, node );

    const QName tagName( node.tagName() );
    if ( tagName.localName() == QLatin1String("schema") ) {
      parseSchemaTag( context, node );
    } else {
      qDebug( "No schema tag found in schema file %s", schemaLocation.toEncoded().constData());
    }

    d->mNamespaces = joinNamespaces( d->mNamespaces, namespaceManager.uris() );

    file.close();

    provider.cleanUp();
  }
}

// TODO: Try to merge import and include schema
// The main difference is that <include> can only
//     "pull in definitions and declarations from a schema whose
//      target namespace is the same as the including schema's target namespace"
void Parser::includeSchema( ParserContext *context, const QString &location )
{
  FileProvider provider;
  QString fileName;
  const QUrl schemaLocation = urlForLocation(context, location);
  qDebug("including schema at %s", schemaLocation.toEncoded().constData());
  if ( provider.get( schemaLocation, fileName ) ) {
    QFile file( fileName );
    if ( !file.open( QIODevice::ReadOnly ) ) {
      qDebug( "Unable to open file %s", qPrintable( file.fileName() ) );
      return;
    }

    QXmlInputSource source( &file );
    QXmlSimpleReader reader;
    reader.setFeature( QLatin1String("http://xml.org/sax/features/namespace-prefixes"), true );

    QDomDocument doc( QLatin1String("kwsdl") );
    QString errorMsg;
    int errorLine, errorColumn;
    bool ok = doc.setContent( &source, &reader, &errorMsg, &errorLine, &errorColumn );
    if ( !ok ) {
      qDebug( "Error[%d:%d] %s", errorLine, errorColumn, qPrintable( errorMsg ) );
      return;
    }

    QDomElement node = doc.documentElement();
    NSManager namespaceManager( context, node );
    const QName tagName( node.tagName() );
    if ( tagName.localName() == QLatin1String("schema") ) {
      // For include, targetNamespace must be the same as the current document.
      if ( node.hasAttribute( QLatin1String("targetNamespace") ) ) {
        if( node.attribute( QLatin1String("targetNamespace") ) != d->mNameSpace ) {
          context->messageHandler()->error( QLatin1String("Included schema must be in the same namespace of the resulting schema.") );
          return;
        }
      }
      parseSchemaTag( context, node );
    } else {
      qDebug("No schema tag found in schema file %s", schemaLocation.toEncoded().constData());
    }

    d->mNamespaces = joinNamespaces( d->mNamespaces, namespaceManager.uris() );

    file.close();

    provider.cleanUp();
  }
}

QString Parser::schemaUri()
{
  return XMLSchemaURI;
}

QStringList Parser::joinNamespaces( const QStringList &list, const QStringList &namespaces )
{
  QStringList retval( list );

  for ( int i = 0; i < namespaces.count(); ++i ) {
    if ( !retval.contains( namespaces[ i ] ) )
      retval.append( namespaces[ i ] );
  }

  return retval;
}

Element Parser::findElement( const QName &name )
{
  for ( int i = 0; i < d->mElements.count(); ++i ) {
    if ( d->mElements[ i ].nameSpace() == name.nameSpace() && d->mElements[ i ].name() == name.localName() )
      return d->mElements[ i ];
  }

  qDebug() << "Not found:" << name.nameSpace() << name.localName();

  return Element();
}

Attribute Parser::findAttribute( const QName &name )
{
  for ( int i = 0; i < d->mAttributes.count(); ++i ) {
    if ( d->mAttributes[ i ].nameSpace() == name.nameSpace() && d->mAttributes[ i ].name() == name.localName() )
      return d->mAttributes[ i ];
  }

  return Attribute();
}

AttributeGroup Parser::findAttributeGroup( const QName &name )
{
  foreach ( AttributeGroup g, d->mAttributeGroups ) {
    if ( g.nameSpace() == name.nameSpace() && g.name() == name.localName() ) {
      return g;
    }
  }

  return AttributeGroup();
}

bool Parser::resolveForwardDeclarations()
{
  const QName any( QLatin1String("http://www.w3.org/2001/XMLSchema"), QLatin1String("any") );
  //const QName anyType( "http://www.w3.org/2001/XMLSchema", "anyType" );
  for ( int i = 0; i < d->mComplexTypes.count(); ++i ) {

    Element::List elements = d->mComplexTypes[ i ].elements();
    Element::List finalElementList;
    for ( int j = 0; j < elements.count(); ++j ) {
      Element element = elements[ j ];
      if ( !element.isResolved() ) {
        Element resolvedElement = findElement( element.reference() );
        if (resolvedElement.qualifiedName().isEmpty()) {
            qWarning("ERROR in %s: resolving element ref to '%s': not found!", qPrintable(d->mComplexTypes[i].qualifiedName().qname()), qPrintable(element.reference().qname()));
            d->mElements.dump();
            return false;
        } else {
            resolvedElement.setMinOccurs( element.minOccurs() );
            resolvedElement.setMaxOccurs( element.maxOccurs() );
            resolvedElement.setCompositor( element.compositor() );
            element = resolvedElement;
        }
      }
      if (j > 0 && finalElementList.last().type() == any) {
          if (element.type() == any) {
              // Keep only one any. The alternative would be to implement namespace "filtering"...
              //qWarning("ERROR: two 'any' values in the same type %s", qPrintable(d->mComplexTypes[i].name()));
              //return false;
              continue;
          }
          // Hack for deserialization: keep "any" last.
          Element lastElem = finalElementList.takeLast();
          finalElementList.append( element );
          finalElementList.append( lastElem );
      } else {
          finalElementList.append( element );
      }
    }
    d->mComplexTypes[ i ].setElements( finalElementList );

    Attribute::List attributes = d->mComplexTypes[ i ].attributes();

    for ( int j = 0; j < attributes.count(); ++j ) {
      if ( !attributes[ j ].isResolved() ) {
        Attribute refAttribute = findAttribute( attributes[ j ].reference() );
        if (refAttribute.qualifiedName().isEmpty()) {
            qWarning("ERROR in %s: resolving attribute ref to '%s': not found!", qPrintable(d->mComplexTypes[i].qualifiedName().qname()), qPrintable(attributes[j].reference().qname()));
            d->mAttributes.dump();
            return false;
        } else {
           attributes[ j ] = refAttribute;
        }
      }
    }

    AttributeGroup::List attributeGroups = d->mComplexTypes[ i ].attributeGroups();
    foreach( const AttributeGroup& group, attributeGroups ) {
      if ( !group.isResolved() ) {
        AttributeGroup refAttributeGroup =
          findAttributeGroup( group.reference() );
        Attribute::List groupAttributes = refAttributeGroup.attributes();
        foreach ( const Attribute& ga, groupAttributes ) {
          attributes.append( ga );
        }
      }
    }

    d->mComplexTypes[ i ].setAttributes( attributes );
  }
  return true;
}

Types Parser::types() const
{
  Types types;

  types.setSimpleTypes( d->mSimpleTypes );
  types.setComplexTypes( d->mComplexTypes );
  //qDebug() << "Parser::types: elements:";
  //d->mElements.dump();
  types.setElements( d->mElements );
  types.setAttributes( d->mAttributes );
  types.setAttributeGroups( d->mAttributeGroups );
  types.setNamespaces( d->mNamespaces );

  return types;
}

Annotation::List Parser::annotations() const
{
  return d->mAnnotations;
}


bool Parser::debugParsing()
{
    static bool s_debug = qgetenv("KDSOAP_DEBUG_PARSER").toInt();
    return s_debug;
}

}
