#include <QtCore>
#include "stdio.h"

// Taken from http://www.partow.net/programming/hashfunctions/index.html
// Author: Arash Partow - 2002
unsigned int RSHash(const char *str)
{
   unsigned int b    = 378551;
   unsigned int a    = 63689;
   unsigned int hash = 0;

   for(size_t i = 0; i < (size_t)strlen(str); i++)
   {
      hash = hash * a + str[i];
      a    = a * b;
   }

   return hash;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QFile in("steamids.txt");
	QFile out("hashids.txt");

	if ( !in.open(QIODevice::ReadOnly | QIODevice::Text) )
		return 1;

	if ( !out.open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text) )
		return 1;
	
	QTextStream o( &out );

	while ( !in.atEnd() )
	{
		QString line = in.readLine();
		line.remove( '\n' );
		unsigned int uHash = RSHash( line.toAscii().data() );
		o << uHash << "\n";
	}

	in.close();
	out.close();

	return 0;
}
