#include "TelnetNode.h"

int main( int argc, char** argv )
{
    TelnetNode::Initialize();

    TelnetNode* pServer = TelnetNode::CreateServer();
    std::puts("Server started.");

    while ( pServer )
    {
        TNMessagePtr pMsg = pServer->PopReceivedText();
        if ( pMsg.get() != NULL )
        {
            std::printf("Server got message. : %s", pMsg->Text.get());
            if ( !std::strcmp(pMsg->Text.get(), "bye\n") )
            {
                pServer->SendText( "bye\n", 0 );
                break;
            }
        }
    }

    TelnetNode::ReleaseNode( pServer );
    std::puts("Server terminated.");

    TelnetNode::Finalize();

    return 0;
}
