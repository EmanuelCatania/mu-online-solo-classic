// Compiles the existing protocol headers. The repository's Protocol_Check.cpp
// also includes stdafx.h, so that translation unit is probed separately and
// does not compile for the browser yet.
#include "Net/Protocol/ProtocolBase.h"
#include "Net/Protocol/GameServerProtocol.h"
#include "Net/Protocol/ConnectServerProtocol.h"

int main(void) {
    return sizeof(Proto::PBMSG_HEAD) == 3 ? 0 : 1;
}
