/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#include <QtCore/QTextStream>
#include <QDebug>

#include "printer.h"

using namespace KODE;

class Printer::Private
{
  public:
    Private( Printer *parent )
      : mParent( parent ), mCreationWarning( false ), mGenerator( "libkode" )
    {
    }

    QString classHeader( const Class &classObject, bool publicMembers, bool nestedClass = false );
    QString classImplementation( const Class &classObject, bool nestedClass = false );
    Code functionHeaders( const Function::List &functions,
                          const QString &className,
                          int access );

    Printer *mParent;
    Style mStyle;
    bool mCreationWarning;
    QString mGenerator;
    QString mOutputDirectory;
    QString mSourceFile;
};

QString Printer::Private::classHeader( const Class &classObject, bool publicMembers, bool nestedClass )
{
  Code code;

  if ( nestedClass )
    code.indent();

  if ( !classObject.docs().isEmpty() ) {
    code += "/**";
    code.indent();
    code.addFormattedText( classObject.docs() );
    code.unindent();
    code += " */";
  }

  QString txt = "class " + mStyle.className( classObject.name() );

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
    code.indent();
  }

  if ( classObject.isQObject() ) {
    code += "Q_OBJECT";
    code.newLine();
  }

  Class::List nestedClasses = classObject.nestedClasses();
  // Generate nestedclasses
  if ( !classObject.nestedClasses().isEmpty() ) {
    code += QLatin1String("public:");

    Class::List::ConstIterator it, itEnd = nestedClasses.constEnd();
    for ( it = nestedClasses.constBegin(); it != itEnd; ++it ) {
      code += classHeader( (*it), false, true );
    }

    code.newLine();
  }

  Typedef::List typedefs = classObject.typedefs();
  if ( typedefs.count() > 0 ) {
    code += "public:";
    code.indent();

    Typedef::List::ConstIterator it;
    for ( it = typedefs.constBegin(); it != typedefs.constEnd(); ++it )
      code += (*it).declaration();

    code.unindent();
    code.newLine();
  }

  Enum::List enums = classObject.enums();
  if ( enums.count() > 0 ) {
    code += "public:";
    code.indent();

    Enum::List::ConstIterator it;
    for ( it = enums.constBegin(); it != enums.constEnd(); ++it )
      code += (*it).declaration();

    code.unindent();
    code.newLine();
  }

  Function::List functions = classObject.functions();

  code.addBlock( functionHeaders( functions, classObject.name(), Function::Public ) );

#ifdef KDAB_TEMP // this is only wanted for value classes...
  if ( classObject.useDPointer() && !classObject.memberVariables().isEmpty() ) {
    Function cc( classObject.name() );
    cc.addArgument( "const " + classObject.name() + '&' );
    Function op( "operator=", classObject.name() + "& " );
    op.addArgument( "const " + classObject.name() + '&' );
    Function::List list;
    list << cc << op;
    code.addBlock( functionHeaders( list, classObject.name(), Function::Public ) );
  }
#endif

  code.addBlock( functionHeaders( functions, classObject.name(), Function::Public | Function::Slot ) );
  code.addBlock( functionHeaders( functions, classObject.name(), Function::Signal ) );
  code.addBlock( functionHeaders( functions, classObject.name(), Function::Protected ) );
  code.addBlock( functionHeaders( functions, classObject.name(), Function::Protected | Function::Slot ) );
  code.addBlock( functionHeaders( functions, classObject.name(), Function::Private ) );
  code.addBlock( functionHeaders( functions, classObject.name(), Function::Private | Function::Slot ) );

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
      code += "public:";
    else if ( !hasPrivateFunc || hasPrivateSlot )
      code += "private:";

    code.indent();

    if ( classObject.useDPointer() && !classObject.memberVariables().isEmpty() ) {
      code += "class PrivateDPtr;";
      code += "PrivateDPtr *d;";
    } else {
      MemberVariable::List variables = classObject.memberVariables();
      MemberVariable::List::ConstIterator it2;
      for ( it2 = variables.constBegin(); it2 != variables.constEnd(); ++it2 ) {
        MemberVariable v = *it2;

        QString decl;
        if ( v.isStatic() )
          decl += "static ";

        decl += v.type();

        //if ( v.type().right( 1 ) != "*" && v.type().right( 1 ) != "&" )
          decl += ' ';

        decl += v.name() + ';';

        code += decl;
      }
    }
  }

  if( !nestedClass )
    code.setIndent( 0 );
  else
    code.unindent();

  code += "};";

  return code.text();
}

QString Printer::Private::classImplementation( const Class &classObject, bool nestedClass)
{
  Code code;

  bool needNewLine = false;

  QString functionClassName = nestedClass ? classObject.parentClassName() + QLatin1String("::") + classObject.name() : classObject.name();

  if ( classObject.useDPointer() && !classObject.memberVariables().isEmpty() ) {
    Class privateClass( functionClassName + "::PrivateDPtr" );
    MemberVariable::List vars = classObject.memberVariables();
    MemberVariable::List::ConstIterator it;
    Function ctor("PrivateDPtr");
    bool hasInitializers = false;
    for ( it = vars.constBegin(); it != vars.constEnd(); ++it ) {
        MemberVariable v = *it;
        privateClass.addMemberVariable( v );
        if ( !v.initializer().isEmpty() ) {
            ctor.addInitializer( v.name() + '(' + v.initializer() + ')' );
            hasInitializers = true;
        }
    }
    if (hasInitializers)
        privateClass.addFunction(ctor);
    code += classHeader( privateClass, true /*publicMembers*/ );
    if (hasInitializers)
        code += classImplementation( privateClass );
  }

  MemberVariable::List vars = classObject.memberVariables();
  MemberVariable::List::ConstIterator itV;
  for ( itV = vars.constBegin(); itV != vars.constEnd(); ++itV ) {
    MemberVariable v = *itV;
    if ( !v.isStatic() )
      continue;

    code += v.type() + functionClassName + "::" + v.name() + " = " + v.initializer() +
            ';';
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

    code += mParent->functionSignature( f, functionClassName, true );

    if ( !f.initializers().isEmpty() ) {
      code.indent();
      code += ": " + f.initializers().join( ", " );
      code.unindent();
    }

    code += '{';
    if ( classObject.useDPointer() && !classObject.memberVariables().isEmpty() &&
         f.name() == classObject.name() ) {
      code.indent();
      code += "d = new PrivateDPtr;";
      code.unindent();
      code.newLine();
    }

    code.addBlock( f.body(), 2 );

    if ( classObject.useDPointer() && !classObject.memberVariables().isEmpty() &&
        f.name() == '~' + classObject.name() ) {
      code.newLine();
      code.indent();
      code += "delete d;";
      code += "d = 0;";
      code.unindent();
    }
    code += '}';
    code.newLine();
  }

  if ( classObject.useDPointer() && !classObject.memberVariables().isEmpty() ) {

#ifdef KDAB_TEMP // only for value classes...
    // print copy constructor
    Function cc( classObject.name() );
    cc.addArgument( "const " + functionClassName + "& other" );

    Code body;
    body += "d = new PrivateDPtr;";
    body += "*d = *other.d;";
    cc.setBody( body );

    code += mParent->functionSignature( cc, functionClassName, true );

    // call copy constructor of base classes
    Class::List baseClasses = classObject.baseClasses();
    if ( !baseClasses.isEmpty() ) {
      QStringList list;
      for ( int i = 0; i < baseClasses.count(); ++i ) {
        list.append( baseClasses[ i ].name() + "( other )" );
      }

      code.indent();
      code += ": " + list.join( ", " );
      code.unindent();
    }

    code += '{';
    code.addBlock( cc.body(), 2 );
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
    body += "*d = *other.d;";
    body.newLine();
    body += "return *this;";
    op.setBody( body );

    code += mParent->functionSignature( op, functionClassName, true );
    code += '{';
    code.addBlock( op.body(), 2 );
    code += '}';
    code.newLine();
#endif
  }

  // Generate nested class functions
  if( !classObject.nestedClasses().isEmpty() ) {
    foreach ( Class nestedClass, classObject.nestedClasses() ) {
      code += classImplementation( nestedClass, true );
    }
  }

  return code.text();
}

Code Printer::Private::functionHeaders( const Function::List &functions,
                                        const QString &className,
                                        int access )
{
  bool needNewLine = false;
  bool hasAccess = false;

  Code code;

  Function::List::ConstIterator it;
  for ( it = functions.constBegin(); it != functions.constEnd(); ++it ) {
    Function f = *it;
    if ( f.access() == access ) {
      if ( !hasAccess ) {
        code += f.accessAsString() + ':';
        hasAccess = true;
      }
      code.indent();
      if ( !(*it).docs().isEmpty() ) {
        code += "/**";
        code.indent();
        code.addFormattedText( (*it).docs() );
        code.unindent();
        code += " */";
      }
      code += mParent->functionSignature( *it, className ) + ';';
      code.unindent();
      needNewLine = true;
    }
  }

  if ( needNewLine )
    code.newLine();

  return code;
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

QString Printer::functionSignature( const Function &function,
                                    const QString &className,
                                    bool forImplementation )
{
  QString s;

  if ( function.isStatic() && !forImplementation ) {
    s += "static ";
  }

  QString ret = function.returnType();
  if ( !ret.isEmpty() ) {
    s += ret;
    //if ( ret.right( 1 ) != "*" && ret.right( 1 ) != "&" ) {
      s += ' ';
    //}
  }

  if ( forImplementation )
    s += d->mStyle.className( className ) + "::";

  if ( className == function.name() ) {
    // Constructor
    s += d->mStyle.className( function.name() );
  } else {
    s += function.name();
  }

  s += '(';
  if ( function.hasArguments() )
    s += ' ' + function.arguments( forImplementation ).join( ", " ) + ' ';
  s += ')';

  if ( function.isConst() )
    s += " const";

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

void Printer::printHeader( const File &file )
{
  Code out;

  if ( d->mCreationWarning )
    out += creationWarning();

  out.addBlock( licenseHeader( file ) );

  // Create include guard
  QString className = file.filenameHeader();
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
  QStringList processed;
  Class::List classes = file.classes();
  Class::List::ConstIterator it;
  for ( it = classes.constBegin(); it != classes.constEnd(); ++it ) {
    Q_ASSERT(!(*it).name().isEmpty());
    QStringList includes = (*it).headerIncludes();
    //qDebug() << "includes=" << includes;
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

  // Create enums
  Enum::List enums = file.fileEnums();
  Enum::List::ConstIterator enumIt;
  for ( enumIt = enums.constBegin(); enumIt != enums.constEnd(); ++enumIt ) {
    out += (*enumIt).declaration();
    out.newLine();
  }

  // Create forward declarations
  processed.clear();
  for ( it = classes.constBegin(); it != classes.constEnd(); ++it ) {
    QStringList decls = (*it).forwardDeclarations();
    QStringList::ConstIterator it2;
    for ( it2 = decls.constBegin(); it2 != decls.constEnd(); ++it2 ) {
      if ( !processed.contains( *it2 ) ) {
        out += "class " + *it2 + ';';
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
  if ( !header.open( QIODevice::WriteOnly ) ) {
    qWarning( "Can't open '%s' for writing.", qPrintable( filename ) );
    return;
  }

  QTextStream h( &header );

  h << out.text();

  header.close();
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
    out += "using namespace " + file.nameSpace() + ';';
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
    out.addBlock( f.body(), 2 );
    out += '}';
    out.newLine();
  }

  // Classes
  bool containsQObject = false;
  for ( it = classes.constBegin(); it != classes.constEnd(); ++it ) {
    if ( (*it).isQObject() )
      containsQObject = true;

    QString str = d->classImplementation( *it );
    if ( !str.isEmpty() )
      out += d->classImplementation( *it );
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
  if ( !implementation.open( QIODevice::WriteOnly ) ) {
    qWarning( "Can't open '%s' for writing.", qPrintable( filename ) );
    return;
  }

  QTextStream h( &implementation );

  h << out.text();

  implementation.close();
}
