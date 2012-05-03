#ifndef CONVERT_H_INCLUDED
#define CONVERT_H_INCLUDED

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

class Convert
{
public:

    static bool IsDouble( const char* pStr )
        {
            char* pEnd;
            double result = std::strtod( pStr, &pEnd );

            if ( pEnd != pStr )
                while ( std::isspace(*pEnd) )
                    ++pEnd;

            return pStr != pEnd && *pEnd == '\0';
        }

    static bool IsDecimal( const char* pStr )
        {
            char* pEnd;
            long int result = std::strtol( pStr, &pEnd, 10 );

            if ( pEnd != pStr )
                while ( std::isspace(*pEnd) )
                    ++pEnd;

            return pStr != pEnd && *pEnd == '\0';
        }


    static double ToDouble( const char* pStr )
        {
            char* pEnd;
            double result = std::strtod( pStr, &pEnd );

            assert( pStr != pEnd );
            return result;
        }

    static long int ToDecimal( const char* pStr )
        {
            char* pEnd;
            long int result = std::strtol( pStr, &pEnd, 10 );

            assert( pStr != pEnd );
            return result;
        }
};

/*
  Ref.: http://stackoverflow.com/questions/332852/check-variable-type-in-c
*/

#endif // CONVERT_H_INCLUDED
