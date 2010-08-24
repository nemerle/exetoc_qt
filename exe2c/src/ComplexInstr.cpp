// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

//#include "stdafx.h"
//	ComplexInstr.cpp

#include	"CISC.h"
bool g_f_Step_by_Step = false;
bool g_any1_return_TRUE = false;
bool Step_by_Step();


const char finger_for[] 	= "0_jmp1_from2_0_from1_0_jxx3_0_jmp2_from3_";
const char finger_long_if [] 	= "0_jxx1_jmp2_from1_0_from2_";
const char finger_if [] 	= "0_jxx1_0_from1_";
const char finger_if_else[] = "0_jxx1_0_jmp2_from1_0_from2_";
const char finger_while[] 	= "from1_0_jxx2_0_jmp1_from2_";
const char finger_dowhile[] = "from1_0_jxx1_";
const char finger_dowhile_2[] = "from1_0_jxx1_from2_";
//If there is one break, it will happen, i do not find a better way
// for(1;3;2)
// {
//	4
// }

int InstrList_Finger::search_and_add(uint32_t* buf,uint32_t val,int* pn)
{//static function
    int n = *pn;
    for (int i=0;i<n;i++)
    {
        if (buf[i] == val)
            return i+1;
    }
    buf[n] = val;
    *pn = n+1;
    return n+1;
}
bool	InstrList_Finger::finger_compare(char * f1,const char* f2)
{//static function
        for (;;)
        {
                if (*f1 == *f2)
                {
                        if (*f1 == 0)
                                return true;
                        f1++;
                        f2++;
                        continue;
                }
                if (*f2 == '0' && f2[1] == '_')
                {
                        f2 += 2;
                        continue;
                }
                return false;
        }
}
bool	InstrList::if_Ly_In(PINSTR p, POSITION firstpos, POSITION endpos)
{
    //If the 'last' is the 'label', it also counts
    assert(endpos!=m_list.end());
    POSITION pos = firstpos;
    while (pos!=m_list.end())
    {
        PINSTR pinstr = *pos;//list->;
        ++pos;
        if (pinstr == p)
            return true;
        if (pos == endpos)
        {
            if (p->type != i_Label)
                return false;
            pinstr = *pos;//list->;
            ++pos;
            if (p == pinstr)
                return true;
            return false;
        }
    }
    return false;
}
bool	InstrList::ifOneStatement(PINSTR pNode, POSITION firstpos, POSITION endpos)
{	//	do not include 'end'
    //	if the first is a label，allow call
    //	if the last is a label，allow call

    assert(firstpos!=m_list.end());
    assert(endpos!=m_list.end());
    bool ffirst = true;
    POSITION pos = firstpos;
    while (pos!=m_list.end() && pos != endpos)
    {
        PINSTR p = *pos;//list->;
        ++pos;
        if (ffirst && p->type == i_Label)
        {
            if (pos == endpos)
                return false;	//	There is only one label, also to make up the numbers
            ffirst = false;
            continue;
        }

        ffirst = false;

        if (p->type == i_Label)
        {	//	make sure all ref of this label ly in
            PINSTR pr = p->label.ref_instr;
            while (pr)
            {	//	check all ref list
                if (! if_Ly_In(pr, firstpos, endpos) )
                    return false;
                pr = pr->jmp.next_ref_of_this_label;
            }
        }
        if (p->type == i_Jump)
        {	//	make sure it jmp inside
            if (p->jmp.jmp_type == JMP_jmp)
            {
                //if (p->jmp.the_label != pNode->begin.m_conti)
                //	alert("why?");
                if (p->jmp.the_label == pNode->begin.m_break)
                    continue;
                if (p->jmp.the_label == pNode->begin.m_conti
                        && p != pNode->begin.m_not_conti)
                    continue;
            }
            if (! if_Ly_In(p->jmp.the_label, firstpos, endpos) )
                return false;
        }
    }
    if (pos == endpos)
        return true;
    return false;
}

PINSTR	instr_next(const INSTR_LIST& list,const INSTR * p)
{
    INSTR_LIST::const_iterator pos = std::find(list.begin(),list.end(),p);
    if (pos == list.end())
        return NULL;
    ++pos;
    if (pos == list.end())
        return NULL;
    return *pos;
}
PINSTR	instr_prev(const INSTR_LIST& list, const INSTR * p)
{
    INSTR_LIST::const_iterator pos = std::find(list.begin(),list.end(),p);
    if (pos == list.end())
        return NULL;
    --pos;
    if (pos == list.end())
        return NULL;
    return *pos;
}
void	InstrList_Finger::prt_partern(PINSTR phead,char * partern_buf)
{

    if (phead->type != i_CplxBegin)
        return;

    PINSTR p = instr_next(m_list,phead);

    int	t = 0;
    uint32_t buf[20];
    int n = 0;

    while (p != NULL && p != phead->begin.m_end)
    {
        if (p->type == i_Jump)
        {
            intptr_t lab_ptr=(intptr_t )p->jmp.the_label;
            int i = search_and_add(buf,(uint32_t)lab_ptr,&n);
            if (p->jmp.jmp_type == JMP_jmp)
            {
                t += sprintf(partern_buf+t,"jmp%d_",i);
            }
            else
                t += sprintf(partern_buf+t,"jxx%d_",i);

        }
        else if (p->type == i_Label)
        {
            intptr_t lab_ptr=(intptr_t )p;
            int i = search_and_add(buf,(uint32_t)lab_ptr,&n);
            t += sprintf(partern_buf+t,"from%d_",i);

        }
        else if (p->type == i_Begin)
        {
            t += sprintf(partern_buf+t,"0_");
            p = p->begin.m_end;
        }
        else
            ;//why here
        p = instr_next(m_list,p);

        if (t > 120)
        {
            t += sprintf(partern_buf+t,"...");
            t;	//	avoid warning
            break;
        }
    }
}

bool	InstrList_Finger::Finger_check_partern_for1(PINSTR p)
{
    CFunc_InstrList instrl(this->m_list);

#if 0
    int i = 0;  //p1
    do
    {
        i++;    //p3
    }
    while (i < 100);    //p2
#endif
    PINSTR p1 = instr_prev(m_list,p);
    if (p1->var_r1.type != v_Immed)
        return false;
    if (p1->var_r2.type != v_Invalid)
        return false;
    //These two conditions means that only the receiver is in front of the regarded as i = n

    PINSTR p2;
    {
        PINSTR ptem = instr_next(m_list,p);
        if (ptem->type != i_Label)
            return false;
        p2 = ptem->label.ref_instr;
    }
    if (p1 == NULL || p2 == NULL)
        return false;

    if (VAR::IsSame(&p1->var_w, &p2->var_r1)
        || VAR::IsSame(&p1->var_w, &p2->var_r2))
    {
    }
    else
        return false;

    {
        INSTR_LIST::iterator to_remove = std::find(m_list.begin(),m_list.end(),p1);
        m_list.erase(to_remove);
        INSTR_LIST::iterator pos = std::find(m_list.begin(),m_list.end(),p);
        //	insert after i_CplxBegin

        PINSTR begin = new INSTR;
        PINSTR end = new INSTR;
        begin->type = i_Begin;
        end->type = i_End;
        begin->begin.m_end = end;
        ++pos; //because insert after
        m_list.insert(pos,end);
        m_list.insert(pos,p1);
        m_list.insert(pos,begin);
    }

    {
        //p2是条件跳，它前面应该是i_end
        PINSTR pend = instrl.instr_prev_in_func(p2);
        assert(pend->type == i_End);

        PINSTR begin = new INSTR;
        PINSTR end = new INSTR;
        begin->type = i_Begin;
        end->type = i_End;
        begin->begin.m_end = end;
        INSTR_LIST::iterator insert_after = std::find(m_list.begin(),m_list.end(),pend);
        ++insert_after;
        m_list.insert(insert_after, end);
        insert_after = std::find(m_list.begin(),m_list.end(),pend);
        ++insert_after;
        m_list.insert(insert_after, begin);

        for (;;)
        {
            PINSTR p3 = instrl.instr_prev_in_func(pend);
            if (p3->type != i_Add && p3->type != i_Sub)
                break;
            INSTR_LIST::iterator remove_iter = std::find(m_list.begin(),m_list.end(),p3);
            m_list.erase(remove_iter);
            INSTR_LIST::iterator insert_after = std::find(m_list.begin(),m_list.end(),begin);
            ++insert_after;
            m_list.insert(insert_after, p3);
        }
    }

    return true;
}
bool	InstrList_Finger::Finger_check_partern(PINSTR p)
{
    // check the pattern p
    char buf[140];
    this->prt_partern(p,buf);

    if (finger_compare(buf, finger_if))
        p->begin.type = COMP_if;
    else if (finger_compare(buf, finger_long_if))	//	must be placed before if_else
        p->begin.type = COMP_long_if;
    else if (finger_compare(buf, finger_if_else))
        p->begin.type = COMP_if_else;
    else if (finger_compare(buf, finger_while))
        p->begin.type = COMP_while;
    else if (finger_compare(buf, finger_dowhile))
    {
        p->begin.type = COMP_do_while;
        // Special do-while written for the best
        if (this->Finger_check_partern_for1(p))
        {
            log_prtl("find for1");
            p->begin.type = COMP_for1;
        }
    }
    else if (finger_compare(buf, finger_dowhile_2))
        p->begin.type = COMP_do_while;
    else if (finger_compare(buf, finger_for))
    {	//const char finger_for[] 	= "0_jmp1_from2_0_from1_0_jxx3_0_jmp2_from3_";
        p->begin.type = COMP_for;
        //Next, let us make some adjustments in 'for'
        PINSTR p1 = instr_next(m_list,p);	//	p1 points to a label
        assert(p1->type == i_Jump);
        p1 = instr_next(m_list,p1->jmp.the_label);	//p1 points to condition
        if (p1->type == i_Jump
                && p1->jmp.jmp_type != JMP_jmp
                && p1->var_r1.type != 0)
        {
            VAR* pvar = &p1->var_r1;
            PINSTR p2 = instr_prev(m_list,p);
            if (VAR::IsSame(&p2->var_w,pvar))
            {
                INSTR_LIST::iterator erase_iter = std::find(m_list.begin(),m_list.end(),p2);
                m_list.erase(erase_iter);
                INSTR_LIST::iterator insert_iter = std::find(m_list.begin(),m_list.end(),p);
                //POSITION pos = list->Find(p);	//	insert after i_CplxBegin

                PINSTR begin = new INSTR;
                PINSTR end = new INSTR;
                begin->type = i_Begin;
                end->type = i_End;
                begin->begin.m_end = end;
                ++insert_iter;
                m_list.insert(insert_iter,end);
                m_list.insert(insert_iter,p2);
                m_list.insert(insert_iter,begin);
            }
        }
    }
    else
        return false;
    return true;
}
void	Func::Finger_it()
{
    InstrList_Finger the(m_instr_list);
    POSITION pos = m_instr_list.begin();
    for (;pos!=m_instr_list.end();++pos)
    {
        PINSTR p = *pos;//list->;
        if (p->type == i_CplxBegin)
        {
            the.Finger_check_partern(p);
        }
    }
}
bool	InstrList::Flow_c(PINSTR phead)
{
    //	Statement to try to find out the child, including live and then call Flow_a
    assert(phead->type == i_CplxBegin);
    INSTR_LIST::iterator s_pos = std::find(m_list.begin(),m_list.end(),phead);
    INSTR_LIST::iterator e_pos = std::find(m_list.begin(),m_list.end(),phead->begin.m_end);
    assert(s_pos!=m_list.end());
    assert(e_pos!=m_list.end());
    if (phead->begin.type == COMP_switch_case)
    {
        //On swith_case, some special. Starting from the first label to
        INSTR_LIST::iterator pos = s_pos;
        for (;pos!=m_list.end();++pos)
        {
            PINSTR p = *pos;//list->;
            if (p->type == i_Label)
                break;
        }
        return Flow_cc(phead,pos,e_pos);
    }
    if (phead->begin.type == COMP_switch_case_multcomp)
    {
        bool f = false;
        INSTR_LIST::iterator pos = s_pos;
        while (pos!=m_list.end())
        {
            INSTR_LIST::iterator savpos = pos;
            PINSTR p = *pos;//list->;
            ++pos;
            if (p->type == i_Label)
                break;
            if (p->type == i_Jump && p->jmp.jmp_type == JMP_jz)
                f=true;
            if (f)
            {
                if (p->type == i_Begin)
                {
                    pos = std::find(m_list.begin(),m_list.end(),p->begin.m_end);
                    ++pos;
                    continue;
                }
                if (p->type != i_Jump && p->type != i_Begin)
                {
                    //break;  //there is no jump in default
                    return Flow_cc(phead, savpos,e_pos);
                }
            }
        }
        return Flow_cc(phead,pos,e_pos);
    }

    ++s_pos;	//	skip the i_CplxBegin
    PINSTR p = *s_pos;	//	skip first 1
    ++s_pos;
    if (p->type == i_Begin)
    {
        //Such as 'for', allowing the head to have a small statement
        if (this->Flow_a(p))
            return true;
        s_pos = std::find(m_list.begin(),m_list.end(),p->begin.m_end);
        ++s_pos;	//	skip i_End
        ++s_pos;	//	skip first 1
    }
    return Flow_cc(phead,s_pos,e_pos);
}

bool	InstrList::Flow_cc(PINSTR pNode, POSITION firstpos, POSITION endpos)
{	//This is used to cplx in another trying to find a few small begin_end to


    // Of three, has a head no tail, it is necessary to find the largest one contains the first statement, and use i_Begin, i_End enclosed
    // And then continue from i_End to end
    // Flow_c (INSTR_LIST * list, POSITION firstpos, POSITION endpos);
    // Of the found i_Begin, i_End, continue with Flow_a
    // PNode is to check the section belongs begin_end, just to provide m_Break
    assert(firstpos!=m_list.end());
    assert(endpos!=m_list.end());
    if (firstpos == endpos)
        return false;

    PINSTR phead = *firstpos;
    if (phead->type == i_Label)
    {
        PINSTR p = instr_next(m_list,phead);
        if (p->type == i_Begin)	//	Because i_label will back out of the dead followed by i_begin cycle
        {
            phead = p;	//	process the next one
        }
    }
    if (phead->type == i_Begin)
    {
        if (this->Flow_a(phead))
            return true;
        POSITION pos1 = std::find(m_list.begin(),m_list.end(),phead->begin.m_end);
        ++pos1;	//	skip i_End
        return Flow_cc(pNode,pos1,endpos);
    }

    POSITION okpos;
    POSITION pos1;
    pos1 = okpos = firstpos;

    do
    {
        ++pos1;
        if (ifOneStatement(pNode, firstpos,pos1))
            okpos = pos1;	// record the last success
    } while (pos1 != endpos);

    if (okpos == firstpos)	// not find anything
    {
        pos1 = firstpos;
        ++pos1;
        if (pos1!=m_list.end() && pos1 != endpos)
        {
            return Flow_cc(pNode, pos1, endpos);		//	start from next
        }
        return false;
    }

    {
        PINSTR begin = new INSTR;
        PINSTR end = new INSTR;
        begin->type = i_Begin;
        end->type = i_End;
        begin->begin.m_end = end;

        Add_Begin_End(firstpos, okpos, begin, end);

        begin->begin.m_break = pNode->begin.m_break;	//	carry on
        begin->begin.m_conti = pNode->begin.m_conti;	//	carry on

        this->Flow_a(begin);
    }

    if (Step_by_Step())
        return true;

    return Flow_cc(pNode, okpos, endpos);	//next part
}
void InstrList::Add_Begin_End(POSITION firstpos, POSITION &endpos, PINSTR begin, PINSTR end)
{
    this->Add_Begin_End_1(firstpos,endpos,begin,end);
    POSITION pos = m_list.begin();
    while (pos!=m_list.end())
    {
        POSITION savepos=pos;
        PINSTR p = *pos;//list->;
        pos++;
        if (p->type == i_Nop)
        {
            if(savepos==endpos)
            {
                endpos=pos;
            }
            m_list.erase(savepos);
        }
    }
}
void InstrList::Add_Begin_End_1(POSITION firstpos, POSITION endpos, PINSTR begin, PINSTR end)
{
    // If first == i_Label, to allow others to call
    // If last == i_Label, to allow others to call
    // So, when you insert i_Begin and i_End, they should be "cloned"
    // Because I want to borrow twice the same function, not want to write it alone, so write a for (;;) calculations

    //INSTR *endinstr=*endpos;
    //HLType type=endinstr->type;
    for (;;)
    {
        if (begin)
        {
            INSTR *p = *firstpos;
            if (p->type != i_Label)
            {
                m_list.insert(firstpos,begin);
                begin = NULL;	//	cleared away will no longer be dealt with
            }
        }
        if (end)
        {
            PINSTR p = *endpos;
            //assert(p==endinstr);
            if (p->type != i_Label)
            {
                m_list.insert(endpos,end);
                end = NULL;
            }

        }
        if (begin == NULL && end == NULL)
            return;	//	Have to get rid of, and nothing made a


        PINSTR pnew = new INSTR;
        pnew->type = i_Label;
        //pnew->label.label_off = p->label.label_off;
        pnew->label.ref_instr = 0;

        PINSTR p;
        if (begin)	//	Get it first as a
        {
            p = *firstpos;
            POSITION after_firstpos=firstpos;
            ++after_firstpos;
            m_list.insert(after_firstpos,pnew);
            m_list.insert(after_firstpos,begin);
            begin = NULL;	//	cleared away will no longer be dealt with
        }
        else
        {
            p = *endpos;

            m_list.insert(endpos,pnew);
            m_list.insert(endpos,end);
            end = NULL;		//	cleared away will no longer be dealt with
        }
        pnew->label.label_off = p->label.label_off;


        PINSTR p_in = 0;
        PINSTR p_out = 0;
        PINSTR p1 = p->label.ref_instr;
        while (p1)
        {
            PINSTR pnext = p1->jmp.next_ref_of_this_label;
            if (if_Ly_In(p1,firstpos,endpos))
            {
                p1->jmp.the_label = pnew;
                p1->jmp.next_ref_of_this_label = p_in;
                p_in = p1;
            }
            else
            {
                //p1->jmp.the_label = p;	//need not
                p1->jmp.next_ref_of_this_label = p_out;
                p_out = p1;
            }
            p1 = pnext;
        }
        if (p_in)	//	it must be
            pnew->label.ref_instr = p_in;
        else
            pnew->type = i_Nop;		//	Because the original is the label, would not have a clear var
        if (p_out)
            p->label.ref_instr = p_out;
        else
            p->type = i_Nop;
        //}
    }
}

bool InstrList::IsSwitchCase_multcomp(PINSTR begin)
{
    //	Comparison is not the kind switch_case multiple comparisons

    assert(begin->type == i_CplxBegin);
    POSITION pos = std::find(m_list.begin(),m_list.end(),begin);
    M_t* v;

    int first = 0;
    while (pos!=m_list.end())
    {
        POSITION savpos = pos;
        PINSTR p = *pos;//list->;
        ++pos;
        first++;
        if (first == 1)
            continue;	//	The first is certainly i_CplxBegin, do not need to read
        if (first == 2)
        {				//	The first instruction must be jz sth == n
            if (p->type != i_Jump || p->jmp.jmp_type != JMP_jz
                    || p->var_r1.type == 0)
                return false;
            v = p->var_r1.thevar;	//	Write it down, keep up the same as before the next line
            continue;
        }
        if (p->type == i_Jump)
        {
            if (p->jmp.jmp_type == JMP_jz)
            {
                if (v != p->var_r1.thevar)
                    return false;
            }
            else if (p->jmp.jmp_type == JMP_jmp)	//	find default
            {
                if (first < 4)		//	if less then that many, not really a switch case
                    return false;
                return true;
            }
            else
                return false;

            continue;
        }
        //There was no Jump
        if (first < 4)		//	not enough comparisons, not really a switch case
            return false;
        return true;
    }
    return false;
}
bool InstrList::IsSwitchCase(PINSTR begin)
{
        assert(begin->type == i_CplxBegin);
        POSITION pos = std::find(m_list.begin(),m_list.end(),begin);

        bool first = true;
        for (;pos!=m_list.end();++pos)
        {
                PINSTR p = *pos;//list->;
                if (p->type == i_JmpAddr)
                {
                        return true;
                }
                if (p->type == i_Return)
                        return false;
                if (p->type == i_Label)
                        return false;
                if (p->type == i_Jump)
                {	//	allows only one jump
                        if (first)
                                first = false;
                        else
                                return false;
                }
        }
        return false;
}
void	InstrList::Flow_b(PINSTR pParentNode, POSITION firstpos, POSITION endpos)
{	//	compact analysis


        //	last not include
        if (firstpos == endpos)
        {
                //alert_prtf("why firstpos == endpos");
                return;
        }

        PINSTR begin = new INSTR;
        PINSTR end = new INSTR;
        begin->type = i_CplxBegin;
        end->type = i_CplxEnd;
        begin->begin.m_end = end;
        POSITION afterendpos=endpos;
        POSITION beforeendpos=endpos;
        Add_Begin_End(firstpos, endpos, begin, end);
        // endpos might have been removed
        POSITION pos = endpos;
        --pos;		//now it point to i_CplxEnd
        assert(*pos == end);
        --pos;		//now it point to last instr in body
        PINSTR plast = *pos;

        PINSTR plast2 = instr_prev(m_list,plast);	//前一条指令

        PINSTR pNode = begin;
        pNode->begin.m_break = pParentNode->begin.m_break;	//	继承
        pNode->begin.m_conti = pParentNode->begin.m_conti;	//	继承

        PINSTR pfirst = instr_next(m_list,begin);
        PINSTR psecond = instr_next(m_list,pfirst);

        if (pfirst->type == i_Label
                || (pfirst->type == i_Jump && pfirst->jmp.jmp_type == JMP_jmp && psecond->type == i_Label)
                )
        {	//	这是我认为 break 的条件 ！
                PINSTR pconti;
                if (pfirst->type == i_Label)	//	如果是第一种情况
                        pconti = pfirst;
                else
                        pconti = psecond;
                pconti->label.f_conti = true;

                if (plast->type == i_Label)	//	如果最后一条指令是个label，那它肯定是break
                        pNode->begin.m_break = plast;
                else
                        pNode->begin.m_break = 0;

                pNode->begin.m_conti = pconti;

                if (plast->type == i_Label)
                {
                        if (plast2->type == i_Jump
                                && plast2->jmp.jmp_type == JMP_jmp
                                && plast2->jmp.the_label == pconti)
                        {	//	这是 while !
                                pNode->begin.m_not_conti = plast2;
                        }
                }
                else if (plast->type == i_Jump
                                 && plast->jmp.jmp_type == JMP_jmp
                                 && plast->jmp.the_label == pconti)
                {
                        pNode->begin.m_not_conti = plast;
                }
                else
                {
                        //do_while的continue 以后再处理
                }
        }
    else if (plast->type == i_Label && IsSwitchCase(begin))
        {
                pNode->begin.m_break = plast;
                //conti not change
                //	这是switch_case 的特点，就是只用break,却不用continue,非常特殊。
                pNode->begin.type = COMP_switch_case;
                //alert("case find 00");
        }
        else if (plast->type == i_Label && IsSwitchCase_multcomp(begin))
        {
                pNode->begin.m_break = plast;
                //conti not change
                //	这是switch_case 的特点，就是只用break,却不用continue,非常特殊。
                pNode->begin.type = COMP_switch_case_multcomp;
                //alert("case find 01");
        }
        if (Step_by_Step())
                return;		//	我们已经加了个i_CplxBegin_End就算干过活了

        Flow_c(begin);	//	对这个i_CplxBegin再作些处理
}

bool	InstrList::Flow_a(PINSTR pNode)
        //	流程分析第一步
        //	对这个区间进行分析。以后可以递归
{
    assert(pNode->type == i_Begin);

    POSITION endpos = std::find(m_list.begin(),m_list.end(),pNode->begin.m_end);

    POSITION firstpos = std::find(m_list.begin(),m_list.end(),pNode);
    ++firstpos;	//	从第二条指令开始
    assert(firstpos!=m_list.end());
    if (firstpos == endpos)
        return false;

    return Flow_aa(pNode,firstpos,endpos);

    //	Flow_a 是指向一个begin_end
    //	而Flow_aa是指向一个区间，并不一定是begin_end
}

bool Step_by_Step()
{	//	这是为了在非step_by_step状态时，能提高点速度
        //	而在step_by_step状态时，又能使每一步都分得很细
        if (g_f_Step_by_Step)
                return true;
        g_any1_return_TRUE = true;
        return false;
}
bool	InstrList::Flow_aa(PINSTR pBlockHeadNode, POSITION firstpos, POSITION endpos)
{
    // Loose analysis
    // Return true that there has been progress analysis
    // PBlockHeadNode is a block header in the begin, break and continue address for query site

    // Analysis objectives: a discrete label, jmp
    // If it is found i_CplxBegin, if it begin.type == 0 is not recognized, then try to identify
    // If you have already identified, check the stuff of which i_Begin_i_End
    assert(firstpos!=m_list.end());
    assert(endpos!=m_list.end());

    if (firstpos == endpos)
        return false;

    POSITION pos = firstpos;
    PINSTR p = *pos;//list->;
    ++pos;
    assert(pos!=m_list.end());
    if (p->type == i_Begin)
    {
        if (this->Flow_a(p))
            return true;
        POSITION pos1 = std::find(m_list.begin(),m_list.end(),p->begin.m_end);
        assert(pos1!=m_list.end());
        return Flow_aa(pBlockHeadNode,pos1,endpos);
    }
    if (p->type == i_CplxBegin)
    {
        POSITION pos1 = std::find(m_list.begin(),m_list.end(),p->begin.m_end);
        assert(pos1!=m_list.end());
        if (Flow_c(p))
        {
            return true;
        }

        if (p->begin.type == COMP_unknown)
        {	//	at this time it should attempt to identify
            InstrList_Finger the(m_list);
            if (the.Finger_check_partern(p))
            {
                //alert("return true");
                return true;
            }
            //alert("return false");
        }

        return Flow_aa(pBlockHeadNode, pos1, endpos);
    }

    //	the first step, find the first compund instruction
    if (p->type != i_Jump && p->type != i_Label)
    {
        assert(pos!=m_list.end());
        assert(pos!=firstpos);
        return Flow_aa(pBlockHeadNode, pos, endpos);
    }
    if (p->type == i_Jump)
    {	//	检查是不是break或continue
        if (p->jmp.jmp_type == JMP_jmp)
        {
            if (p->jmp.the_label == pBlockHeadNode->begin.m_break)
            {
                return Flow_aa(pBlockHeadNode, pos, endpos);
            }
            if (p->jmp.the_label == pBlockHeadNode->begin.m_conti
                && p != pBlockHeadNode->begin.m_not_conti)
            {
                return Flow_aa(pBlockHeadNode, pos, endpos);
            }
        }
    }
    //	找到了，begin是一个i_Jump 或 i_Label
    if (p->type == i_Jump)
    {	//it must be jump to follow
        PINSTR p1 = p->jmp.the_label;	//it jump here
        POSITION pos1 = std::find(m_list.begin(),m_list.end(),p1);
        assert(pos1!=m_list.end());
        while (pos1!=m_list.end())
        {
            if (pos1 == endpos)
                break;
            if (pos1 == m_list.end())
            {
                log_prtl("label = %x",p->jmp.jmpto_off);
            }
            else if (ifOneStatement(pBlockHeadNode, firstpos,pos1))
            {	//	找到一个紧凑结构
                Flow_b(pBlockHeadNode, firstpos,pos1);
                if (Step_by_Step())
                    return true;	//	可以在这里直接return TRUE了
                assert(pos1!=m_list.end());
                return Flow_aa(pBlockHeadNode, pos1,endpos);
            }
            //assert(pos1);
            ++pos1;
        }
    }
    else if (p->type == i_Label)
    {
                PINSTR p1 = p->label.ref_instr;
                POSITION pos1 = std::find(m_list.begin(),m_list.end(),p1);
                while (pos1!=m_list.end() && pos1 != endpos)
                {
                        ++pos1;
                        assert(pos1!=m_list.end());
                        if (ifOneStatement(pBlockHeadNode, firstpos,pos1))
                        {
                                Flow_b(pBlockHeadNode, firstpos,pos1);
                                if (Step_by_Step())
                                        return true;	//	可以在这里直接return TRUE了
                                assert(pos1!=m_list.end());
                                return Flow_aa(pBlockHeadNode, pos1,endpos);
                        }
                        //assert(pos1);
                }
        }
        return false;
}
/*
        有三种分析，一定要分清楚。

        一种是，有头有尾，已知这是一个松散的statement,要在其中找到一个个的complex statement
        Flow_a(PINSTR phead, INSTR_LIST* list);
        对找到的complex,用Flow_b处理

        二种，有头有尾，且已知这是一个单一的complex statement，需要自己括自己
        Flow_b(INSTR_LIST* list, POSITION firstpos,POSITION endpos);
        括住自己后，call一个Flow_c

        三种，输入一个单一的complex statement,设法要从中找到包含头的最大的一个statement，并用i_Begin,i_End括起来
        再从i_End到尾继续
        Flow_c(PINSTR phead, INSTR_LIST* list);
        对找到的i_Begin,i_End，用Flow_a继续

*/

// --------------------------------------------------------------
void	CFunc_Prt::add_default_entry(CasePrt_List* list, PINSTR thelabel)
{	//	delete all same as default
    //static function

    if (thelabel->type == i_Label)
    {
        CasePrt_List::iterator pos = list->begin();
        while (pos!=list->end())
        {
            CasePrt_List::iterator savpos = pos;
            OneCase* p = *pos;//list->;
            ++pos;
            if (p->thelabel->label.label_off == thelabel->label.label_off)
            {
                //alert_prtf("delete case %d",p->case_n);
                list->erase(savpos);
            }
        }
    }
        OneCase* pnew = new OneCase;
        pnew->case_n = 0xffffffff;		//	不能只检查这一个条件，最后一个才是最重要的
        pnew->thelabel = thelabel;
        list->push_back(pnew);
}
void	CFunc_Prt::Add_case_entry(CasePrt_List* list, int case_n, PINSTR thelabel)
{	//	必须先把switch case的所有项保存起来，最后一起打印
    //alert("add case entry");
    //static function

    CasePrt_List::iterator pos = list->begin();
    while (pos!=list->end())
    {
        CasePrt_List::iterator savpos = pos;
        OneCase* p = *pos;//list->;
        ++pos;
        if (p->case_n == case_n)
            error("why same case");
        if (p->thelabel->label.label_off >= thelabel->label.label_off)
        {
            pos = savpos;
            break;
        }
    }

    OneCase* pnew = new OneCase;
    pnew->case_n = case_n;
    pnew->thelabel = thelabel;
    if (pos!=list->end())
        list->insert(pos, pnew);
    else
        list->push_back(pnew);
    return;
}
