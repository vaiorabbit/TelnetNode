#include "TelnetNode.h"

int main( int argc, char** argv )
{
    TelnetNode::Initialize();

    TelnetNode* pClient = TelnetNode::CreateClient();
    std::puts( "Client started.");

    int nTiming = 0, nSent = 0 ;
    while ( pClient )
    {
        TNMessagePtr pMsg = pClient->PopReceivedText();
        if ( pMsg.get() != NULL )
        {
            std::printf( "Client got message. : %s", pMsg->Text.get() );
            if ( !std::strcmp(pMsg->Text.get(), "bye\n") )
                break;
        }

        ++nTiming;
        if ( nTiming > 2000000 )
        {
            if ( nSent < 10 )
                pClient->SendText( "Hello, Mr.Server.\n" );
            else
                pClient->SendText( "bye\n" );
            nTiming = 0;
            ++nSent;
        }
    }

    TelnetNode::ReleaseNode( pClient );
    std::puts( "Client terminated." );

    TelnetNode::Finalize();

    return 0;
}
