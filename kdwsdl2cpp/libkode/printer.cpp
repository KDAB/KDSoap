/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2010 David Faure <dfaure@kdab.com>

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

#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QFileInfo>
#include <QDebug>

#include "printer.h"

using namespace KODE;

class Printer::Private
{
  public:
    Private( Printer *parent )
      : mParent( parent ),
      mCreationWarning( false ),
      mLabelsDefineIndent( true ),
      mIndentLabels( true ),
      mGenerator( "libkode" )
    {
    }

    void addLabel( Code& code, const QString& label );
    QString classHeader( const Class &classObject, bool publicMembers, bool nestedClass = false );
    QString classImplementation( const Class &classObject, bool nestedClass = false );
    void addFunctionHeaders( Code& code,
                             const Function::List &functions,
                             const QString &className,
                             int access );
    QString formatType( const QString& type ) const;

    Printer *mParent;
    Style mStyle;
    bool mCreationWarning;
    bool mLabelsDefineIndent;
    bool mIndentLabels;
    QString mGenerator;
    QString mOutputDirectory;
    QString mSourceFile;
    QStringList mStatementsAfterIncludes;

    /**
     * @brief printCodeIntoFile
     * Writes the string passed through the code parameter to the file referenced
     * by the file parameter in the case if it differs from the content of the file pointed by the
     * file parameter.
     * @param code reference to a Code object which contents needs to be printed
     * @param file the target file in unopened state with filename set
     */
    void printCodeIntoFile( const Code &code, QFile *file );
};

void Printer::Private::addLabel( Code& code, const QString& label )
{
    if ( !mIndentLabels )
        code.unindent();
    code += label;
    if ( !mIndentLabels )
        code.indent();
}

QString Printer::Private::formatType( const QString& type ) const
{
  QString s = type;
  if ( s.endsWith( '*' ) || s.endsWith( '&' ) ) {
      if ( s.at( s.length() - 2 ) != ' ' ) {
          // Turn "Foo*" into "Foo *" for readability
          s.insert( s.length() - 1, ' ' );
      }
  } else {
      s += ' ';
  }
  return s;
}

QString Printer::Private::classHeader( const Class &classObject, bool publicMembers, bool nestedClass )
{
  Code code;

  int numNamespaces = 0;
  if ( !classObject.nameSpace().isEmpty() ) {
    const QStringList nsList = classObject.nameSpace().split("::");
    Q_FOREACH(const QString& ns, nsList) {
        code += "namespace " + ns + " {";
        code.indent();
        ++numNamespaces;
    }
  }

  if ( nestedClass )
    code.indent();

  if ( !classObject.docs().isEmpty() ) {
    code += "/**";
    code.indent();
    code.addFormattedText( classObject.docs() );
    code.unindent();
    code += " */";
  }

  QString txt = "class ";
  if ( !classObject.exportDeclaration().isEmpty() ) {
    txt += classObject.exportDeclaration().toUpper() + "_EXPORT ";
  }
  txt += classObject.name();

  Class::List baseClasses = classObject.baseClasses();
  if ( !baseClasses.isEmpty() ) {
    txt += " : ";
    Class::List::ConstIterator it;
    for ( it = baseClasses.constBegin(); it != baseClasses.constEnd(); ++it ) {
      Class bc = *it;

      if ( it != baseClasses.constBegin() )
        txt +=", ";

      txt += "public ";
      if ( !bc.nameSpace().isEmpty() )
        txt += bc.nameSpace() + "::";

      txt += bc.name();
    }
  }
  code += txt;

  if( nestedClass ) {
    code.indent();
    code += '{';
  }
  else {
    code += '{';
    // We always want to indent here; so that Q_OBJECT and enums etc. are indented.
    // However with mIndentLabels=false, we'll unindent before printing out "public:".
    code.indent();
  }

  if ( classObject.isQObject() ) {
    code += "Q_OBJECT";
    code.newLine();
  }
  Q_FOREACH( const QString& declMacro, classObject.declarationMacros() ) {
    code += declMacro;
    code.newLine();
  }

  Class::List nestedClasses = classObject.nestedClasses();
  // Generate nestedclasses
  if ( !classObject.nestedClasses().isEmpty() ) {
    addLabel( code, "public:" );

    Class::List::ConstIterator it, itEnd = nestedClasses.constEnd();
    for ( it = nestedClasses.constBegin(); it != itEnd; ++it ) {
      code += classHeader( (*it), false, true );
    }

    code.newLine();
  }

  Typedef::List typedefs = classObject.typedefs();
  if ( typedefs.count() > 0 ) {
    addLabel( code, "public:" );
    if ( mLabelsDefineIndent )
      code.indent();

    Typedef::List::ConstIterator it;
    for ( it = typedefs.constBegin(); it != typedefs.constEnd(); ++it )
      code += (*it).declaration();

    if ( mLabelsDefineIndent )
      code.unindent();
    code.newLine();
  }

  Enum::List enums = classObject.enums();
  if ( enums.count() > 0 ) {
    addLabel( code, "public:" );
    if ( mLabelsDefineIndent )
      code.indent();

    Enum::List::ConstIterator it;
    for ( it = enums.constBegin(); it != enums.constEnd(); ++it )
      code += (*it).declaration();

    if ( mLabelsDefineIndent )
      code.unindent();
    code.newLine();
  }

  Function::List functions = classObject.functions();

  addFunctionHeaders( code, functions, classObject.name(), Function::Public );

  if ( classObject.canBeCopied() && classObject.useDPointer() && !classObject.memberVariables().isEmpty() ) {
    Function cc( classObject.name() );
    cc.addArgument( "const " + classObject.name() + '&' );
    Function op( "operator=", classObject.name() + '&' );
    op.addArgument( "const " + classObject.name() + '&' );
    Function::List list;
    list << cc << op;
    addFunctionHeaders( code, list, classObject.name(), Function::Public );
  }

  addFunctionHeaders( code, functions, classObject.name(), Function::Public | Function::Slot );
  addFunctionHeaders( code, functions, classObject.name(), Function::Signal );
  addFunctionHeaders( code, functions, classObject.name(), Function::Protected );
  addFunctionHeaders( code, functions, classObject.name(), Function::Protected | Function::Slot );
  addFunctionHeaders( code, functions, classObject.name(), Function::Private );
  addFunctionHeaders( code, functions, classObject.name(), Function::Private | Function::Slot );

  if ( !classObject.memberVariables().isEmpty() ) {
    Function::List::ConstIterator it;
    // Do we have any private function?
    bool hasPrivateFunc = false;
    bool hasPrivateSlot = false;
    for ( it = functions.constBegin(); it != functions.constEnd(); ++it ) {
        if ( (*it).access() == Function::Private ) {
            hasPrivateFunc = true;
        } else if ( (*it).access() == (Function::Private | Function::Slot) ) {
            hasPrivateSlot = true;
        }
    }

    if ( publicMembers )
      addLabel( code, "public:" );
    else if ( !hasPrivateFunc || hasPrivateSlot )
      addLabel( code, "private:" );

    if (mLabelsDefineIndent)
      code.indent();

    if ( classObject.useDPointer() && !classObject.memberVariables().isEmpty() ) {
      code += "class PrivateDPtr;";
      if ( classObject.useSharedData() )
        code += "QSharedDataPointer<PrivateDPtr> " + classObject.dPointerName() + ";";
      else
        code += "PrivateDPtr *" + classObject.dPointerName() + ";";
    } else {
      MemberVariable::List variables = classObject.memberVariables();
      MemberVariable::List::ConstIterator it2;
      for ( it2 = variables.constBegin(); it2 != variables.constEnd(); ++it2 ) {
        MemberVariable v = *it2;

        QString decl;
        if ( v.isStatic() )
          decl += "static ";

        decl += formatType( v.type() );

        decl += v.name() + ';';

        code += decl;
      }
    }
    if (mLabelsDefineIndent)
      code.unindent();
  }

  code.unindent();
  code += "};";

  for (int i = 0; i < numNamespaces; ++i) {
      code.unindent();
      code += "} // namespace end";
  }

  return code.text();
}

QString Printer::Private::classImplementation( const Class &classObject, bool nestedClass )
{
  Code code;

  bool needNewLine = false;

  QString functionClassName = classObject.name();
  if (nestedClass)
      functionClassName.prepend( classObject.parentClassName() + QLatin1String("::") );
  else if ( !classObject.nameSpace().isEmpty() )
      functionClassName.prepend( classObject.nameSpace() + QLatin1String("::") );

  // Generate private class
  if ( classObject.useDPointer() && !classObject.memberVariables().isEmpty() ) {
    Class privateClass( functionClassName + "::PrivateDPtr" );
    if ( classObject.useSharedData() ) {
        privateClass.addBaseClass( Class("QSharedData") );
    }
    MemberVariable::List vars = classObject.memberVariables();
    MemberVariable::List::ConstIterator it;
    Function ctor("PrivateDPtr");
    bool hasInitializers = false;
    for ( it = vars.constBegin(); it != vars.constEnd(); ++it ) {
        const MemberVariable v = *it;
        privateClass.addMemberVariable( v );
        if ( !v.initializer().isEmpty() ) {
            ctor.addInitializer( v.name() + '(' + v.initializer() + ')' );
            hasInitializers = true;
        }
    }
    if ( hasInitializers )
        privateClass.addFunction( ctor );
    code += classHeader( privateClass, true /*publicMembers*/ );
    if ( hasInitializers )
        code += classImplementation( privateClass );
  }

  // Generate static vars
  MemberVariable::List vars = classObject.memberVariables();
  MemberVariable::List::ConstIterator itV;
  for ( itV = vars.constBegin(); itV != vars.constEnd(); ++itV ) {
    const MemberVariable v = *itV;
    if ( !v.isStatic() )
      continue;

    // ## I thought the static int foo = 42; syntax was not portable?
    code += v.type() + functionClassName + "::" + v.name() + " = " + v.initializer() + ';';
    needNewLine = true;
  }

  if ( needNewLine )
    code.newLine();

  Function::List functions = classObject.functions();
  Function::List::ConstIterator it;
  for ( it = functions.constBegin(); it != functions.constEnd(); ++it ) {
    Function f = *it;

    // Omit signals
    if ( f.access() == Function::Signal )
      continue;
    // Omit pure virtuals without a body
    if ( f.virtualMode() == Function::PureVirtual && f.body().isEmpty() )
      continue;

    code += mParent->functionSignature( f, functionClassName, true );

    QStringList inits = f.initializers();
    if ( classObject.useDPointer() && !classObject.memberVariables().isEmpty() &&
         f.name() == classObject.name() ) {
      inits.append( classObject.dPointerName() + "(new PrivateDPtr)" );
    }
    if ( !classObject.useDPointer() && f.name() == classObject.name()
         && f.arguments().isEmpty() ) {
      // Default constructor: add initializers for variables
      for ( itV = vars.constBegin(); itV != vars.constEnd(); ++itV ) {
          const MemberVariable v = *itV;
          if ( !v.initializer().isEmpty() ) {
              inits.append( v.name() + '(' + v.initializer() + ')' );
          }
      }
    }

    if (!inits.isEmpty()) {
      code.indent();
      code += ": " + inits.join( ", " );
      code.unindent();
    }

    code += '{';
    code.addBlock( f.body(), Code::defaultIndentation() );

    if ( classObject.useDPointer() && !classObject.useSharedData() &&
        !classObject.memberVariables().isEmpty() &&
        f.name() == '~' + classObject.name() ) {
      // Delete d pointer
      code.newLine();
      code.indent();
      code += "delete " + classObject.dPointerName() + ";";
      code += classObject.dPointerName() + " = 0;";
      code.unindent();
    }
    code += '}';
    code.newLine();
  }

  if ( classObject.useDPointer() && classObject.canBeCopied() && !classObject.memberVariables().isEmpty() ) {

    // print copy constructor
    Function cc( classObject.name() );
    cc.addArgument( "const " + functionClassName + "& other" );

    Code body;
    if ( !classObject.useSharedData() ) {
      body += classObject.dPointerName() + " = new PrivateDPtr;";
      body += "*" + classObject.dPointerName() + " = *other." + classObject.dPointerName() + ";";
    }
    cc.setBody( body );

    code += mParent->functionSignature( cc, functionClassName, true );

    // call copy constructor of base classes
    QStringList list;
    Class::List baseClasses = classObject.baseClasses();
    for ( int i = 0; i < baseClasses.count(); ++i ) {
      list.append( baseClasses[ i ].name() + "( other )" );
    }
    if ( classObject.useSharedData() ) {
      list.append( classObject.dPointerName() + "( other." + classObject.dPointerName() + " )" );
    }
    if ( !list.isEmpty() ) {
      code.indent();
      code += ": " + list.join( ", " );
      code.unindent();
    }

    code += '{';
    code.addBlock( cc.body(), Code::defaultIndentation() );
    code += '}';
    code.newLine();

    // print assignment operator
    Function op( "operator=", functionClassName + "& " );
    op.addArgument( "const " + functionClassName + "& other" );

    body.clear();
    body += "if ( this == &other )";
    body.indent();
    body += "return *this;";
    body.unindent();
    body.newLine();
    if ( classObject.useSharedData() )
      body += classObject.dPointerName() + " = other." + classObject.dPointerName() + ";";
    else
      body += "*" + classObject.dPointerName() + " = *other." + classObject.dPointerName() + ";";
    for ( int i = 0; i < baseClasses.count(); ++i ) {
        body += QLatin1String("* static_cast<") + baseClasses[i].name() + QLatin1String(" *>(this) = other;");
    }

    body.newLine();
    body += "return *this;";
    op.setBody( body );

    code += mParent->functionSignature( op, functionClassName, true );
    code += '{';
    code.addBlock( op.body(), Code::defaultIndentation() );
    code += '}';
    code.newLine();
  }

  // Generate nested class functions
  if( !classObject.nestedClasses().isEmpty() ) {
    foreach ( Class nestedClass, classObject.nestedClasses() ) {
      code += classImplementation( nestedClass, true );
    }
  }

  return code.text();
}

void Printer::Private::addFunctionHeaders( Code& code,
                                           const Function::List &functions,
                                           const QString &className,
                                           int access )
{
  bool needNewLine = false;
  bool hasAccess = false;

  Function::List::ConstIterator it;
  for ( it = functions.constBegin(); it != functions.constEnd(); ++it ) {
    Function f = *it;
    if ( f.access() == access ) {
      if ( !hasAccess ) {
        addLabel( code, f.accessAsString() + ':' );
        hasAccess = true;
      }
      if ( mLabelsDefineIndent )
        code.indent();
      if ( !(*it).docs().isEmpty() ) {
        code += "/**";
        code.indent();
        code.addFormattedText( (*it).docs() );
        code.unindent();
        code += " */";
      }
      code += mParent->functionSignature( *it, className, false ) + ';';
      if ( mLabelsDefineIndent )
        code.unindent();
      needNewLine = true;
    }
  }

  if ( needNewLine )
    code.newLine();
}



Printer::Printer()
  : d( new Private( this ) )
{
}

Printer::Printer( const Printer &other )
  : d( new Private( this ) )
{
  *d = *other.d;
  d->mParent = this;
}

Printer::Printer( const Style &style )
  : d( new Private( this ) )
{
  d->mStyle = style;
}

Printer::~Printer()
{
  delete d;
}

Printer& Printer::operator=( const Printer &other )
{
  if ( this == &other )
    return *this;

  *d = *other.d;
  d->mParent = this;

  return *this;
}

void Printer::setCreationWarning( bool v )
{
  d->mCreationWarning = v;
}

void Printer::setGenerator( const QString &generator )
{
  d->mGenerator = generator;
}

void Printer::setOutputDirectory( const QString &outputDirectory )
{
  d->mOutputDirectory = outputDirectory;
}

void Printer::setSourceFile( const QString &sourceFile )
{
  d->mSourceFile = sourceFile;
}

void Printer::setLabelsDefineIndent( bool b )
{
  d->mLabelsDefineIndent = b;
}

void Printer::setIndentLabels( bool b )
{
  d->mIndentLabels = b;
}

QString Printer::functionSignature( const Function &function,
                                    const QString &className,
                                    bool forImplementation )
{
  QString s;

  if ( function.isStatic() && !forImplementation ) {
    s += "static ";
  }

  if ( function.virtualMode() != Function::NotVirtual && !forImplementation ) {
    s += "virtual ";
  }

  QString ret = function.returnType();
  if ( !ret.isEmpty() ) {
    s += d->formatType( ret );
  }

  if ( forImplementation )
    s += className + "::";

  s += function.name();

  s += '(';
  if ( function.hasArguments() ) {
    QStringList arguments;
    foreach( Function::Argument argument, function.arguments() ) {
      if ( !forImplementation ) {
        arguments.append( argument.headerDeclaration() );
      } else {
        arguments.append( argument.bodyDeclaration() );
      }
    }
    s += ' ' + arguments.join( ", " ) + ' ';
  }
  s += ')';

  if ( function.isConst() )
    s += " const";
  
  if ( function.virtualMode() == Function::Override && !forImplementation ) {
    s += " override";
  }

  if ( function.virtualMode() == Function::PureVirtual )
    s += " = 0";

  return s;
}

QString Printer::creationWarning() const
{
  // Create warning about generated file
  QString str = "// This file is generated by " + d->mGenerator;
  if ( !d->mSourceFile.isEmpty() )
    str += " from " + d->mSourceFile;

  str += ".\n";

  str += "// All changes you do to this file will be lost.";

  return str;
}

QString Printer::licenseHeader( const File &file ) const
{
  Code code;

  const QStringList copyrights = file.copyrightStrings();
  if (!file.project().isEmpty() || !copyrights.isEmpty() || !file.license().text().isEmpty())
  {
      code += "/*";
      code.setIndent( 4 );

      if (!file.project().isEmpty()) {
          code += "This file is part of " + file.project() + '.';
          code.newLine();
      }

      if ( !copyrights.isEmpty() ) {
          code.addBlock( copyrights.join( "\n" ) );
          code.newLine();
      }

      code.addBlock( file.license().text() );
      code.setIndent( 0 );
      code += "*/";
  }

  return code.text();
}

void Printer::setStatementsAfterIncludes(const QStringList &statements)
{
  d->mStatementsAfterIncludes = statements;
}

static QStringList commonLeft(const QStringList& l1, const QStringList& l2) {
    QStringList r;
    const int l = qMin(l1.size(), l2.size());
    for ( int i = 0; i < l; ++i )
        if (l1.at(i) == l2.at(i))
            r.append(l1.at(i));
        else
            return r;
    return r;
}

void Printer::printHeader( const File &file )
{
  Code out;

  if ( d->mCreationWarning )
    out += creationWarning();

  out.addBlock( licenseHeader( file ) );

  // Create include guard
  QString className = file.filenameHeader();
  QFileInfo headerInfo(className);
  className = headerInfo.fileName(); // remove path, keep only filename
  className.replace( '-', "_" );

  QString includeGuard;
  if ( !file.nameSpace().isEmpty() )
    includeGuard += file.nameSpace().toUpper() + '_';

  includeGuard += className.toUpper();
  includeGuard.replace( '.', "_" );

  out += "#ifndef " + includeGuard;
  out += "#define " + includeGuard;

  out.newLine();

  // Create includes
  QSet<QString> processed;
  const Class::List classes = file.classes();
  Q_FOREACH( const Class& cl, classes )
  {
    Q_ASSERT( !cl.name().isEmpty() );
    QStringList includes = cl.headerIncludes();
    if ( cl.useSharedData() )
        includes.append( "QtCore/QSharedData" );
    //qDebug() << "includes=" << includes;
    QStringList::ConstIterator it2;
    for ( it2 = includes.constBegin(); it2 != includes.constEnd(); ++it2 ) {
      if ( !processed.contains( *it2 ) ) {
        out += "#include <" + *it2 + '>';
        processed.insert( *it2 );
      }
    }
  }

  if ( !processed.isEmpty() )
    out.newLine();

  for ( const QString &statement : d->mStatementsAfterIncludes ) {
      out += statement;
  }

  // Create enums
  Enum::List enums = file.fileEnums();
  Enum::List::ConstIterator enumIt;
  for ( enumIt = enums.constBegin(); enumIt != enums.constEnd(); ++enumIt ) {
    out += (*enumIt).declaration();
    out.newLine();
  }

  // Create forward declarations
  processed.clear();
  Class::List::ConstIterator it;
  for ( it = classes.constBegin(); it != classes.constEnd(); ++it ) {
    const QStringList decls = (*it).forwardDeclarations();
    processed += decls.toSet();
  }
  QStringList fwdClasses = processed.toList();
  fwdClasses.sort();
  fwdClasses += QString(); //for proper closing of the namespace blocks below

  QStringList prevNS;

  Q_FOREACH( const QString& fwd, fwdClasses ) {
    //handle namespaces by opening and closing namespace blocks accordingly
    //the sorting will ensure sensible grouping
    const QStringList seg = fwd.split(QLatin1String("::"));
    const QStringList ns = seg.mid(0, seg.size() - 1);
    const QString clas = seg.isEmpty() ? QString() : seg.last();
    const QStringList common = commonLeft(ns, prevNS);
    for (int i = common.size(); i < prevNS.size(); ++i) {
      out.unindent();
      out += "}";
      out.newLine();
    }
    for (int i = common.size(); i < ns.size(); ++i) {
      out += "namespace " + ns.at(i) + " {";
      out.indent();
    }

    if (!clas.isNull()) {
      const bool isQtClass = clas.startsWith(QLatin1Char('Q')) && !clas.contains(QLatin1Char('_'));
      if (isQtClass)
        out += QLatin1String("QT_BEGIN_NAMESPACE");
      out += "class " + clas + ';';
      if (isQtClass)
        out += QLatin1String("QT_END_NAMESPACE");
    }
    prevNS = ns;
  }

  if ( !processed.isEmpty() )
    out.newLine();


  if ( !file.nameSpace().isEmpty() ) {
    out += "namespace " + file.nameSpace() + " {";
    out.newLine();
  }

  // Create content
  for ( it = classes.constBegin(); it != classes.constEnd(); ++it ) {
    out.addBlock( d->classHeader( *it, false ) );
    out.newLine();
  }

  if ( !file.nameSpace().isEmpty() ) {
    out += '}';
    out.newLine();
  }

  // Finish file
  out += "#endif";


  // Print to file
  QString filename = file.filenameHeader();

  if ( !d->mOutputDirectory.isEmpty() )
    filename.prepend( d->mOutputDirectory + '/' );

//  KSaveFile::simpleBackupFile( filename, QString(), ".backup" );

  QFile header( filename );
  d->printCodeIntoFile( out, &header );
}

void Printer::printImplementation( const File &file, bool createHeaderInclude )
{
  Code out;

  if ( d->mCreationWarning )
    out += creationWarning();

  out.addBlock( licenseHeader( file ) );

  out.newLine();

  // Create includes
  if ( createHeaderInclude ) {
    out += "#include \"" + file.filenameHeader() + "\"";
    out.newLine();
  }

  QStringList includes = file.includes();
  QStringList::ConstIterator it2;
  for ( it2 = includes.constBegin(); it2 != includes.constEnd(); ++it2 )
    out += "#include <" + *it2 + '>';

  if ( !includes.isEmpty() )
    out.newLine();

  // Create class includes
  QStringList processed;
  Class::List classes = file.classes();
  Class::List::ConstIterator it;
  for ( it = classes.constBegin(); it != classes.constEnd(); ++it ) {
    QStringList includes = (*it).includes();
    QStringList::ConstIterator it2;
    for ( it2 = includes.constBegin(); it2 != includes.constEnd(); ++it2 ) {
      if ( !processed.contains( *it2 ) ) {
        out += "#include <" + *it2 + '>';
        processed.append( *it2 );
      }
    }
  }

  if ( !processed.isEmpty() )
    out.newLine();

  if ( !file.nameSpace().isEmpty() ) {
    out += "namespace " + file.nameSpace() + " {";
    out.newLine();
  }

  // 'extern "C"' declarations
  const QStringList externCDeclarations = file.externCDeclarations();
  if ( !externCDeclarations.isEmpty() ) {
    out += "extern \"C\" {";
    QStringList::ConstIterator it;
    for ( it = externCDeclarations.constBegin(); it != externCDeclarations.constEnd();
         ++it ) {
      out += *it + ';';
    }
    out += '}';
    out.newLine();
  }

  // File variables
  Variable::List vars = file.fileVariables();
  Variable::List::ConstIterator itV;
  for ( itV = vars.constBegin(); itV != vars.constEnd(); ++itV ) {
    Variable v = *itV;
    QString str;
    if ( v.isStatic() )
      str += "static ";
    str += v.type() + ' ' + v.name() + ';';
    out += str;
  }

  if ( !vars.isEmpty() )
    out.newLine();

  // File code
  if ( !file.fileCode().isEmpty() ) {
    out += file.fileCode();
    out.newLine();
  }

  // File functions
  Function::List funcs = file.fileFunctions();
  Function::List::ConstIterator itF;
  for ( itF = funcs.constBegin(); itF != funcs.constEnd(); ++itF ) {
    Function f = *itF;
    out += functionSignature( f );
    out += '{';
    out.addBlock( f.body(), Code::defaultIndentation() );
    out += '}';
    out.newLine();
  }

  // Classes
#ifdef KDAB_DELETED
  bool containsQObject = false;
#endif
  for ( it = classes.constBegin(); it != classes.constEnd(); ++it ) {
#ifdef KDAB_DELETED
    if ( (*it).isQObject() )
      containsQObject = true;
#endif

    QString str = d->classImplementation( *it );
    if ( !str.isEmpty() )
      out += d->classImplementation( *it );
  }

  if ( !file.nameSpace().isEmpty() ) {
    out += "}";
    out.newLine();
  }

  // KDAB: removed; 1) for removing .filename(), and 2) qmake would want moc_foo.cpp anyway
#ifdef KDAB_DELETED
  if ( containsQObject ) {
    out.newLine();
    //out += "#include \"" + file.filename() + ".moc\"";
  }
#endif

  // Print to file
  QString filename = file.filenameImplementation();

  if ( !d->mOutputDirectory.isEmpty() )
    filename.prepend( d->mOutputDirectory + '/' );

  QFile implementation( filename );
  d->printCodeIntoFile( out, &implementation );
}

void Printer::Private::printCodeIntoFile( const Code &code, QFile *file )
{
  const QString outText = code.text();
  bool identical = true;
  if ( file->exists() ) {
    if ( !file->open( QIODevice::ReadOnly ) ) {
      qWarning( "Can't open '%s' for reading.", qPrintable( file->fileName() ) );
      return;
    }

    QTextStream fileReaderStream( file );
    fileReaderStream.setCodec( QTextCodec::codecForName("UTF-8") );

    QTextStream codeStream( outText.toUtf8() );
    QString fileLine, outLine;
    while ( fileReaderStream.readLineInto( &fileLine ) && codeStream.readLineInto(&outLine) ) {
      if ( fileLine != outLine ) {
        identical = false;
        break;
      }
    }

    if ( identical )
      identical = fileReaderStream.atEnd() && codeStream.atEnd();
    file->close();
  } else {
    identical = false;
  }

  if ( !identical ) {
    if ( !file->open( QIODevice::WriteOnly ) ) {
      qWarning( "Can't open '%s' for writing.", qPrintable( file->fileName() ) );
      return;
    }

    QTextStream fileWriterStream( file );
    fileWriterStream.setCodec( QTextCodec::codecForName("UTF-8") );
    fileWriterStream << outText;

    file->close();
  } else {
    qDebug("Skip generating %s because its content did not change", qPrintable( file->fileName() ));
  }
}

#if 0 // TODO: port to cmake
void Printer::printAutoMakefile( const AutoMakefile &am )
{
  QString filename = "Makefile.am";

  if ( !d->mOutputDirectory.isEmpty() )
    filename.prepend( d->mOutputDirectory + '/' );

//  KSaveFile::simpleBackupFile( filename, QString(), ".backup" );

  QFile file( filename );
  if ( !file.open( QIODevice::WriteOnly ) ) {
    qWarning( "Can't open '%s' for writing.", qPrintable( filename ) );
    return;
  }

  QTextStream ts( &file );

  ts << am.text();
}

#endif
