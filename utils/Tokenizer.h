#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

#include <cstring>
#include <vector>

class Tokenizer
{
public:

    Tokenizer( const char* pStr, const char* pDelims )
        : m_pStr(NULL)
        , m_pDelims(NULL)
        , m_apTokens()
        {
            SetString( pStr );
            SetDelimiters( pDelims );
            Tokenize();
        }

    Tokenizer()
        : m_pStr(NULL)
        , m_pDelims(NULL)
        , m_apTokens()
        {}

    ~Tokenizer()
        {
            if ( m_pStr )
                delete [] m_pStr;

            if ( m_pDelims )
                delete [] m_pDelims;
        }

    void SetString( const char* pStr )
        {
            if ( m_pStr )
            {
                delete m_pStr;
                m_apTokens.clear();
            }

            std::size_t len = std::strlen( pStr );
            m_pStr = new char[len+1];
            std::strncpy( m_pStr, pStr, len );
            m_pStr[len] = '\0';
        }

    void SetDelimiters( const char* pDelims )
        {
            if ( m_pDelims )
            {
                delete m_pDelims;
                m_apTokens.clear();
            }

            std::size_t len = std::strlen( pDelims );
            m_pDelims = new char[len+1];
            std::strncpy( m_pDelims, pDelims, len );
            m_pDelims[len] = '\0';
        }

    void Tokenize()
        {
            if ( !m_pStr || !m_pDelims )
                return;

            char* cp = std::strtok( m_pStr, m_pDelims );
            while ( cp )
            {
                m_apTokens.push_back( cp );
                cp = std::strtok( NULL, m_pDelims );
            }
        }

    unsigned int GetTokensCount()
        { return m_apTokens.size(); }

    char* GetToken( int nIndex )
        { return GetTokensCount() ? m_apTokens[nIndex] : NULL; }

private:

    char* m_pStr;
    char* m_pDelims;
    std::vector<char*> m_apTokens;
};

#endif // TOKENIZER_H_INCLUDED
