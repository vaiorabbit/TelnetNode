// -*- mode: C++; coding: utf-8-unix -*-
#ifndef TELNETNODE_H_INCLUDED
#define TELNETNODE_H_INCLUDED

#include <cassert>
#include <cstring>
#include <algorithm>
#include <map>
#include <queue>
#include <vector>

#if defined(__APPLE__) || defined(LINUX) || defined(__CYGWIN__)
#  include <tr1/memory> // std::tr1::shared_ptr
#  include <pthread.h>
// BSD socket
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <arpa/inet.h>
#  include <netinet/in.h>
#  include <netdb.h> // hostent, gethostbyname
#  include <errno.h>
#  include <cstdio>
#  define TNPLATFORM_UNIX
#elif defined(WIN32)
#  include <windows.h>
#  include <memory> // std::tr1::shared_ptr
#  include <stdio.h>
#  define snprintf _snprintf
#  pragma warning(disable: 4996) // suppress security warnings
#  define TNPLATFORM_WINDOWS
#else
#  error "Unsupported Platform"
#endif

#if defined(TNPLATFORM_UNIX)
#  define TNAPI
#elif defined(TNPLATFORM_WINDOWS)
#  define TNAPI WINAPI
#endif


#if defined(TNPLATFORM_UNIX)
#  define TNSocketHandle int
#  define TNSocketHandle_Invalid -1
#  define closesocket(socket_handle) close((socket_handle))
#elif defined(TNPLATFORM_WINDOWS)
#  define TNSocketHandle SOCKET
#  define TNSocketHandle_Invalid INVALID_SOCKET
#  pragma comment(lib, "wsock32.lib")
#endif



// TNMutex : Abstraction layer for Win32 CS and pthread_mutex.
class TNMutex
{
public:
    TNMutex()
        {
#if defined(TNPLATFORM_UNIX)
            pthread_mutex_init( &m_Mutex, 0 );
#elif defined(TNPLATFORM_WINDOWS)
            ::InitializeCriticalSection( &m_Mutex );
#endif
        }

    ~TNMutex()
        {
#if defined(TNPLATFORM_UNIX)
            pthread_mutex_destroy( &m_Mutex );
#elif defined(TNPLATFORM_WINDOWS)
            ::DeleteCriticalSection( &m_Mutex );
#endif
        }

    void Lock()
        {
#if defined(TNPLATFORM_UNIX)
            pthread_mutex_lock( &m_Mutex );
#elif defined(TNPLATFORM_WINDOWS)
            ::EnterCriticalSection( &m_Mutex );
#endif
        }

    void Unlock()
        {
#if defined(TNPLATFORM_UNIX)
            pthread_mutex_unlock( &m_Mutex );
#elif defined(TNPLATFORM_WINDOWS)
            ::LeaveCriticalSection( &m_Mutex );
#endif
        }

private:

#if defined(TNPLATFORM_UNIX)
    pthread_mutex_t m_Mutex;
#elif defined(TNPLATFORM_WINDOWS)
    CRITICAL_SECTION m_Mutex;
#endif
};


// TNThread : Abstraction layer for platform threading APIs.
class TNThread
{
public:

#if defined(TNPLATFORM_UNIX)
    typedef pthread_t Handle;
    static const int InvalidHandle = 0;
    typedef void* RetVal;
    typedef RetVal TNAPI (*EntryFunc)( void* arg );
#elif defined(TNPLATFORM_WINDOWS)
#  define InvalidHandle NULL
    typedef HANDLE Handle;
    typedef DWORD RetVal;
    typedef RetVal (TNAPI *EntryFunc)( LPVOID arg );
#endif

    TNThread()
        :m_hThread(InvalidHandle) {}

    ~TNThread()
        {}

    void Run( EntryFunc pfnFunc, void* pArg )
        {
#if defined(TNPLATFORM_UNIX)
            pthread_create( &m_hThread, NULL, pfnFunc, pArg );
#elif defined(TNPLATFORM_WINDOWS)
            m_hThread = ::CreateThread( 0, 0, pfnFunc, pArg, 0, 0 );
#endif
        }

    void Join()
        {
#if defined(TNPLATFORM_UNIX)
            pthread_join( m_hThread, NULL );
#elif defined(TNPLATFORM_WINDOWS)
            ::WaitForSingleObject( m_hThread, INFINITE );
#endif
            Invalidate();
        }

    void Invalidate()
        { m_hThread = InvalidHandle; }

    bool IsInvalid()
        { return m_hThread == InvalidHandle; }

    static void Exit()
        {
#if defined(TNPLATFORM_UNIX)
            pthread_exit( NULL );
#elif defined(TNPLATFORM_WINDOWS)
            ::ExitThread( 0 );
#endif
        }

private:

    Handle m_hThread;
};


// TNTextPtr : A single line string
template <typename T>
struct TNTextPtrDeleter
{
    void operator()( T* p )
        {
            delete [] p;
        }
};

typedef std::tr1::shared_ptr<char> TNTextPtr;


// TNMessage : Text received from peer node
struct TNMessage
{
    TNTextPtr Text;
    unsigned int ID;

    TNMessage( TNTextPtr pText, unsigned int uID )
        : Text(pText)
        , ID(uID)
        {}
};

typedef std::tr1::shared_ptr<TNMessage> TNMessagePtr;
typedef std::queue<TNMessagePtr> TNMessageQueue;

// TNReceiveBuffer :
// * Stocks 'recv'ed data
// * Splits data into lines of text
class TNReceiveBuffer
{
public:
    typedef std::vector<char> RawBuffer;
    typedef std::queue<TNTextPtr> TextQueue;

    TNReceiveBuffer( unsigned int uInitialSize = 8192 )
        {
            m_Buffer.reserve( uInitialSize );
        }

    ~TNReceiveBuffer()
        {}

    bool Empty()
        {
            return m_Texts.empty();
        }

    TNTextPtr GetText()
        {
            TNTextPtr result;
            if ( !m_Texts.empty() )
            {
                result = m_Texts.front();
                m_Texts.pop();
            }

            return result;
        }

    void Append( const char* pBuffer, unsigned int uBufferSize )
        {
            // stock
            if ( m_Buffer.size() + uBufferSize >= m_Buffer.capacity() )
                m_Buffer.resize( m_Buffer.capacity() + uBufferSize );

            m_Buffer.insert( m_Buffer.end(), pBuffer, pBuffer+uBufferSize );

            // split
            RawBuffer::iterator it_head = m_Buffer.begin();
            RawBuffer::iterator it_tail = std::find( it_head, m_Buffer.end(), '\n' );
            while ( it_tail != m_Buffer.end() )
            {
                unsigned int length = (it_tail - it_head);
                char* pRawNewText = new char[length+2]; // 2 == '\n'+'\0'
                // std::copy( it_head, it_tail+1, pRawNewText ); // +1 == '\n' // VC++2010 warns std::copy is unsafe
                std::memcpy( pRawNewText, &(*it_head), length+1 ); // +1 == '\n'
                pRawNewText[length+1] = '\0';
                m_Texts.push( TNTextPtr(pRawNewText, TNTextPtrDeleter<char>()) );
                it_head = it_tail + 1;
                if ( it_head == m_Buffer.end() )
                    break;
                it_tail = std::find( it_head, m_Buffer.end(), '\n' );
            }

            m_Buffer.clear();
        }

private:

    RawBuffer m_Buffer;
    TextQueue m_Texts;
}; // End : TNReceiveBuffer


// TelnetNode : The public interface
class TelnetNode
{
public:

    static int Initialize();
    static void Finalize();

    static TelnetNode* CreateServer( const char* address = "LOCALHOST", unsigned int port = 23 );
    static TelnetNode* CreateClient( const char* address = "LOCALHOST", unsigned int port = 23 );
    static void ReleaseNode( TelnetNode* pNode )
        { delete pNode; }

    virtual bool IsServer() =0;

    virtual bool SendText( const char* pText, unsigned int uClient = 0 ) =0;

    void PushReceivedText( TNTextPtr pText, unsigned int uClient = 0 )
        {
            m_MessageMutex.Lock();
            TNMessagePtr msg( new TNMessage(pText, uClient) );
            m_Messages.push( msg );
            m_MessageMutex.Unlock();
        }

    TNMessagePtr PopReceivedText()
        {
            TNMessagePtr result;
            m_MessageMutex.Lock();
            if ( !m_Messages.empty() )
            {
                result = m_Messages.front();
                m_Messages.pop();
            }
            m_MessageMutex.Unlock();

            return result;
        }

protected:

    TelnetNode() {}
    virtual ~TelnetNode() {}
    TelnetNode& operator=( const TelnetNode& other );

private:

    TNMutex m_MessageMutex;
    TNMessageQueue m_Messages;
}; // End : TelnetNode


// TelnetClient : Provides the client-specific implementation (Connect, etc.)
class TelnetClient : public TelnetNode
{
public:
    TelnetClient()
        : m_Thread()
        , m_Socket(TNSocketHandle_Invalid)
        , m_SocketMutex()
        {}

    virtual ~TelnetClient()
        {
            Close();
        }

    virtual bool IsServer()
        { return false; }

    virtual bool SendText( const char* pText, unsigned int /*uClient*/ = 0 )
        {
            const unsigned int rawBufSize = 8192;
            char rawBuffer[rawBufSize];
            snprintf( rawBuffer, 8192, "%s", pText );
            std::size_t length = std::strlen( rawBuffer );
            int flags = 0;
            int bytes = send( m_Socket, rawBuffer, length, flags );

            return bytes >= 0;
        }

    bool Connect( const char* address = "LOCALHOST", unsigned int port = 23 )
        {
            bool result = false;

            Close();
            m_Thread.Invalidate();

            m_Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
            if ( m_Socket != TNSocketHandle_Invalid )
            {
                sockaddr_in service = { 0 };

                service.sin_family      = AF_INET;
                service.sin_port        = htons( port );
                service.sin_addr.s_addr = inet_addr( address );

                if ( service.sin_addr.s_addr == INADDR_NONE )
                {
                    hostent* pHost = gethostbyname( address );
                    std::memcpy( &service.sin_addr.s_addr, pHost->h_addr, pHost->h_length );
                    service.sin_family = pHost->h_addrtype;
                }

                int connectResult = connect( m_Socket, (sockaddr*)&service, sizeof(service) );
                if ( connectResult == 0 )
                {
                    m_Thread.Run( ReceiveThreadEntry, this );
                }
                else
                {
                    assert( !"TelnetClient::Connect : connect != 0" );
                }

                if ( !m_Thread.IsInvalid() )
                {
                    result = true;
                }
                else
                {
                    closesocket( m_Socket );
                    m_Socket = TNSocketHandle_Invalid;
                }
            }
            else
            {
                assert( !"TelnetClient::Connect : m_Socket == TNSocketHandle_Invalid" );
            }

            return result;
        }

    void Close()
        {
            if ( m_Socket != TNSocketHandle_Invalid )
            {
                m_SocketMutex.Lock();
                closesocket( m_Socket );
                m_Socket = TNSocketHandle_Invalid;
                m_SocketMutex.Unlock();

                m_Thread.Join();
                m_Thread.Invalidate();
            }
        }

private:

    static TNThread::RetVal TNAPI ReceiveThreadEntry( void* arg )
        {
            ((TelnetClient*)arg)->ReceiveThread();
            TNThread::Exit();

            return 0;
        }

    void ReceiveThread()
        {
            bool done = false;

            m_SocketMutex.Lock();
            TNSocketHandle clientSocket = m_Socket;
            done = (clientSocket == TNSocketHandle_Invalid);
            m_SocketMutex.Unlock();

            const unsigned int rawBufSize = 8192;
            char rawBuffer[rawBufSize];

            TNReceiveBuffer receiveBuffer;

            while ( !done )
            {
                int flags = 0;
                int bytes = recv( clientSocket, rawBuffer, rawBufSize, flags );

                if ( bytes <= 0 )
                {
                    done = true;
                    m_SocketMutex.Lock();
                    closesocket( clientSocket );
                    clientSocket = TNSocketHandle_Invalid;
                    m_Socket = clientSocket;
                    m_SocketMutex.Unlock();
                }
                else
                {
                    receiveBuffer.Append( rawBuffer, bytes );
                }

                while ( !receiveBuffer.Empty() )
                {
                    PushReceivedText( receiveBuffer.GetText() );
                }

                m_SocketMutex.Lock();
                clientSocket = m_Socket;
                done = (clientSocket == TNSocketHandle_Invalid);
                m_SocketMutex.Unlock();
            }
        }

    TNThread       m_Thread;
    TNSocketHandle m_Socket;
    TNMutex        m_SocketMutex;
}; // End : TelnetClient


// TelnetServer : Provides the server-specific implementation (Listen, etc.)
class TelnetServer : public TelnetNode
{
public:
    TelnetServer()
        : m_ListenThread()
        , m_ListenSocket(TNSocketHandle_Invalid)
        , m_ListenSocketMutex()
        , m_uClientCreatedCount(0)
        , m_Clients()
        {}

    virtual ~TelnetServer()
        {
            Close();
        }

    virtual bool IsServer()
        { return true; }

    virtual bool SendText( const char* pText, unsigned int uClient = 0 )
        {
            const unsigned int rawBufSize = 8192;
            char rawBuffer[rawBufSize];
            snprintf( rawBuffer, 8192, "%s", pText );
            std::size_t length = std::strlen( rawBuffer );

            bool result = true;
            if ( uClient == 0 )
            {
                for ( ConnectionMap::iterator it = m_Clients.begin(); it != m_Clients.end(); ++it )
                {
                    ConnectionPtr pClient = (*it).second;
                    bool done = pClient->Send( rawBuffer, length );
                    if ( !done )
                        result = false;
                }
            }
            else
            {
                ConnectionMap::iterator it = m_Clients.find( uClient );
                if ( it != m_Clients.end() )
                {
                    ConnectionPtr pClient = (*it).second;
                    result = pClient->Send( rawBuffer, length );
                }
            }

            return result;
        }

    bool Listen( unsigned int port = 23 )
        {
            bool result = false;

            Close();
            m_ListenThread.Invalidate();

            m_ListenSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
            if ( m_ListenSocket != TNSocketHandle_Invalid )
            {
                sockaddr_in service = { 0 };

                service.sin_family      = AF_INET;
                service.sin_port        = htons( port );
                service.sin_addr.s_addr = htonl( INADDR_ANY );

                int bindResult = bind( m_ListenSocket, (sockaddr*)&service, sizeof(service) );
                if ( bindResult == 0 )
                {
                    int listenResult = listen( m_ListenSocket, SOMAXCONN );
                    if ( listenResult == 0 )
                    {
                        m_ListenThread.Run( ListenThreadEntry, this );
                    }
                    else
                    {
                        assert( !"TelnetServer::Listen : listen != 0" );
                    }
                }
                else
                {
                    assert( !"TelnetServer::Listen : bind != 0" );
                }

                if ( !m_ListenThread.IsInvalid() )
                {
                    result = true;
                }
            }
            else
            {
                assert( !"TelnetServer::Listen : m_ListenSocket == TNSocketHandle_Invalid" );
            }

            return result;
        }

    void Close()
        {
            if ( m_ListenSocket != TNSocketHandle_Invalid )
            {
                m_ListenSocketMutex.Lock();
                closesocket( m_ListenSocket );
                m_ListenSocket = TNSocketHandle_Invalid;
                m_ListenSocketMutex.Unlock();

                m_ListenThread.Join();
                m_ListenThread.Invalidate();
            }
        }

    // TelnetServer::Connection : a connection established with a client
    class Connection
    {
    public:

        Connection( TelnetServer* pServer = NULL, unsigned int uClient = -1, TNSocketHandle hSocket = TNSocketHandle_Invalid )
            : m_pServer(pServer)
            , m_uClientID(uClient)
            , m_SocketMutex()
            , m_Socket(hSocket)
            , m_Thread()
            {}

        ~Connection()
            {}

        bool Send( const char* pText, std::size_t uLength )
            {
                m_SocketMutex.Lock();
                int flags = 0;
                int bytes = send( m_Socket, pText, uLength, flags);
                m_SocketMutex.Unlock();

                return bytes >= 0;
            }

        void Close()
            {
                if ( m_Socket != TNSocketHandle_Invalid )
                {
                    m_SocketMutex.Lock();
                    closesocket( m_Socket );
                    m_Socket = TNSocketHandle_Invalid;
                    m_SocketMutex.Unlock();

                    m_Thread.Join();
                    m_Thread.Invalidate();
                }
            }

        void Run()
            {
                m_Thread.Run( ReceiveThreadEntry, this );
            }

    private:

        static TNThread::RetVal TNAPI ReceiveThreadEntry( void* arg )
            {
                ((Connection*)arg)->ReceiveThread();
                TNThread::Exit();

                return 0;
            }

        void ReceiveThread()
            {
                bool done = false;

                m_SocketMutex.Lock();
                TNSocketHandle clientSocket = m_Socket;
                done = (clientSocket == TNSocketHandle_Invalid);
                m_SocketMutex.Unlock();

                const unsigned int rawBufSize = 8192;
                char rawBuffer[rawBufSize];

                TNReceiveBuffer receiveBuffer;

                while ( !done )
                {
                    int flags = 0;
                    int bytes = recv( clientSocket, rawBuffer, rawBufSize, flags );

                    if ( bytes > 0 )
                    {
                        receiveBuffer.Append( rawBuffer, bytes );
                    }

                    while ( !receiveBuffer.Empty() )
                    {
                        m_pServer->PushReceivedText( receiveBuffer.GetText() );
                    }

                    m_SocketMutex.Lock();
                    clientSocket = m_Socket;
                    done = (clientSocket == TNSocketHandle_Invalid);
                    m_SocketMutex.Unlock();
                }
            }

        TelnetServer*  m_pServer;
        unsigned int   m_uClientID;
        TNMutex        m_SocketMutex;
        TNSocketHandle m_Socket;
        TNThread       m_Thread;
    };

    typedef std::tr1::shared_ptr<Connection> ConnectionPtr;
    typedef std::map<unsigned int, ConnectionPtr> ConnectionMap;

    // End : TelnetServer::Connection

private:

    static TNThread::RetVal TNAPI ListenThreadEntry( void* arg )
        {
            ((TelnetServer*)arg)->ListenThread();
            TNThread::Exit();

            return 0;
        }

    void ListenThread()
        {
            bool done = false;

            m_ListenSocketMutex.Lock();
            TNSocketHandle listenSocket = m_ListenSocket;
            done = (listenSocket == TNSocketHandle_Invalid);
            m_ListenSocketMutex.Unlock();

            while ( !done )
            {
                TNSocketHandle clientSocket = TNSocketHandle_Invalid;
                clientSocket = accept( listenSocket, NULL, NULL );
                if ( clientSocket != TNSocketHandle_Invalid )
                {
                    unsigned int uClientID = ++m_uClientCreatedCount;
                    ConnectionPtr pConnection( new Connection(this, uClientID, clientSocket) );
                    pConnection->Run();
                    m_Clients[uClientID] = pConnection;
                }

                m_ListenSocketMutex.Lock();
                listenSocket = m_ListenSocket;
                done = (listenSocket == TNSocketHandle_Invalid);
                m_ListenSocketMutex.Unlock();
            }

            for ( ConnectionMap::iterator it = m_Clients.begin(); it != m_Clients.end(); ++it )
            {
                ConnectionPtr pConnection = (*it).second;
                if ( pConnection )
                    pConnection->Close();
            }
            m_Clients.clear();
        }

    TNThread       m_ListenThread;
    TNSocketHandle m_ListenSocket;
    TNMutex        m_ListenSocketMutex;
    unsigned int   m_uClientCreatedCount;
    ConnectionMap  m_Clients;
}; // End : TelnetServer



// static
int TelnetNode::Initialize()
{
    int result = 0;
#if defined(TNPLATFORM_WINDOWS)
    WSADATA wsaData;
    WORD wVersion = MAKEWORD( 2, 2 );
    result = WSAStartup( wVersion, &wsaData );
    if ( result != 0 )
    {
        WSACleanup();
    }
#endif // defined(TNPLATFORM_WINDOWS)
    return result;
}

// static
void TelnetNode::Finalize()
{
#if defined(TNPLATFORM_WINDOWS)
    WSACleanup();
#endif // defined(TNPLATFORM_WINDOWS)
}

// static
TelnetNode* TelnetNode::CreateServer( const char* address, unsigned int port )
{
    TelnetServer* pServer = new TelnetServer;

    bool listenSucceeded = pServer->Listen( port );
    if ( !listenSucceeded )
    {
        delete pServer;
        return NULL;
    }

    return pServer;
}

// static
TelnetNode* TelnetNode::CreateClient( const char* address, unsigned int port )
{
    TelnetClient* pClient = new TelnetClient;

    bool connectSucceeded = pClient->Connect( address, port );
    if ( !connectSucceeded )
    {
        delete pClient;
        return NULL;
    }

    return pClient;
}

#endif // TELNETNODE_H_INCLUDED