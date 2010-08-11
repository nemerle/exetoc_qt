// Copyright(C) 1999-2005 LiuTaoTaobookaa@rorsoft.com


//	exe2c project
#ifndef	STRPARSE_H
#define	STRPARSE_H

void get_1part(char * buf,const char * &p);
void skip_space(const char * &p);
void skip_eos(const char * &p);
bool if_split_char(char c);
uint32_t Str2Num(const char * p);
signed int Str2Int(const char * p);


#endif	//STRPARSE_H
