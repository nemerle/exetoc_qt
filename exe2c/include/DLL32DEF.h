// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#ifndef	DLL32DEF_H
#define	DLL32DEF_H

#include <string>
#include "CCbuf.h"
#include "types.h"
CCbuf* ReadDefFile(const std::string & fname);
void onExit_DLL32DEF();
std::string DLLDEF_Get_ApiName_from_ord(char * pDLLname, WORD ord);

#endif	//DLL32DEF_H
