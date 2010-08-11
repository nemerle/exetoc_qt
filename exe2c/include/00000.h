// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com
#pragma once
#include <stdint.h>
typedef uint32_t ea_t;
typedef	uint32_t	SIZEOF;
typedef uint32_t	VarTypeID;
#include "mylist.h"

//char *	new_str(const char * p);

int		alert_prtf(const char * fmt, ...);
#define log_prtf
#define alert
#define log_prtt
#define log_prtl
void nop();

#define	SIZE_unknown 0xfffffffe
#define BIT32_is_4 4
