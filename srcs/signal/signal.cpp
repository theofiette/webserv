#include "signal.hpp"

void sigHandlerDefault(int sig)
{
	g_sig = 128 + sig;
	throw (ExceptionSignal());
}
