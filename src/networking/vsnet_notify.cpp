#include <iostream>
#include "vsnet_dloadmgr.h"

using std::cerr;
using std::endl;

void	VSNotify( VsnetDownload::Client::State s, VsnetDownload::Client::Error e)
{
	cerr<<"!!! DOWNLOAD ERROR : State="<<VsnetDownload::Client::getState(s)<<" - Error="<<VsnetDownload::Client::getError(e)<<endl;
}

void	VsnetDownload::Client::VSNotify::setTotalBytes( int sz )
{
}

void	VsnetDownload::Client::VSNotify::addBytes( int sz )
{
}

