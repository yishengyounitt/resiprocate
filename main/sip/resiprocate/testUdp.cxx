#include <sipstack/Fifo.hxx>
#include <sipstack/UdpTransport.hxx>
#include <sipstack/Logger.hxx>

using namespace Vocal2;

#define VOCAL_SUBSYSTEM Subsystem::TEST

int
main(int argc, char* argv[])
{
   Log::initialize(Log::COUT, Log::DEBUG, argv[0]);
   Fifo<Message> fifo;
   
   DebugLog (<< "testing");
   
   UdpTransport t(5060, fifo);
   t.run();
}
