// Copyright(C) 1999-2005 LiuTaoTao，bookaa@rorsoft.com

#include <QDebug>
#include <algorithm>
#include	"CISC.h"
bool g_f_Step_by_Step = false;
bool g_any1_return_TRUE = false;
bool Step_by_Step(void);


const char finger_for[] 	= "0_jmp1_from2_0_from1_0_jxx3_0_jmp2_from3_";
const char finger_long_if []= "0_jxx1_jmp2_from1_0_from2_";
const char finger_if [] 	= "0_jxx1_0_from1_"; // begin conditional_jmp1   begin label_from1
const char finger_if_else[] = "0_jxx1_0_jmp2_from1_0_from2_";
const char finger_while[] 	= "from1_0_jxx2_0_jmp1_from2_";
const char finger_dowhile[] = "from1_0_jxx1_";
const char finger_dowhile_2[] = "from1_0_jxx1_from2_";
//If there is one break, it will happen, i do not find a better way
// for(1;3;2)
// {
//	4
// }

int InstrList_Finger::search_and_add(intptr_t* buf,intptr_t val,int &pn)
{//static function
    for (int i=0;i<pn;i++)
    {
        if (buf[i] == val)
            return i+1;
    }
    buf[pn++] = val;
    return pn;
}
bool	InstrList_Finger::finger_compare(const char * f1,const char* f2)
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
        if (*f2 == '0' && f2[1] == '_') // 0_ skipped ?
        {
            f2 += 2;
            continue;
        }
        return false;
    }
}
bool	InstrList::if_Ly_In(const Instruction * p, POSITION firstpos, POSITION endpos)
{
    //If the 'last' is the 'label', it also counts
    assert(endpos!=m_list.end());
    POSITION pos = firstpos;
    while (pos!=m_list.end())
    {
        Instruction * pinstr = *(pos++);
        if (p == pinstr)
            return true;
        if (pos == endpos)
        {
            if (p->type != i_Label)
                return false;
            pinstr = *pos;
            if (p == pinstr)
                return true;
            return false;
        }
    }
    return false;
}
bool	InstrList::ifOneStatement(const Instruction * pNode, const POSITION &firstpos, const POSITION &endpos)
{
    //	do not include 'end'
    //	if the first is a label，allow call
    //	if the last is a label，allow call

    assert(firstpos!=m_list.end());
    assert(endpos!=m_list.end());
    bool ffirst = true;
    POSITION pos = firstpos;
    while (pos!=m_list.end() && pos != endpos)
    {
        Instruction * p = *(pos++);
        if (ffirst && p->type == i_Label)
        {
            if (pos == endpos)
                return false;	//	There is only one label, also to make up the numbers
            ffirst = false;
            continue;
        }

        ffirst = false;

        if (p->type == i_Label)
        {	//	make sure all ref of this label lie in
            Instruction * pr = p->label.ref_instr;
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
                if (p->jmp.target_label == pNode->begin.m_break)
                    continue;
                if (p->jmp.target_label == pNode->begin.m_conti && p != pNode->begin.m_not_conti)
                    continue;
            }
            if (! if_Ly_In(p->jmp.target_label, firstpos, endpos) )
                return false;
        }
    }
    if (pos == endpos)
        return true;
    return false;
}

Instruction *	instr_next(const INSTR_LIST& list,const Instruction * p)
{
    INSTR_LIST::const_iterator pos = std::find(list.begin(),list.end(),p);
    if (pos == list.end())
        return NULL;
    ++pos;
    if (pos == list.end())
        return NULL;
    return *pos;
}
Instruction *	instr_prev(const INSTR_LIST& list, const Instruction * p)
{
    INSTR_LIST::const_iterator pos = std::find(list.begin(),list.end(),p);
    if (pos == list.end())
        return NULL;
    --pos;
    if (pos == list.end())
        return NULL;
    return *pos;
}
QString	InstrList_Finger::prt_partern(const Instruction * phead) const
{

    if (phead->type != i_CplxBegin)
        return "";
    QString collector="";
    Instruction * p = instr_next(m_list,phead);

    intptr_t buf[20];
    memset(buf,0,20*sizeof(intptr_t));
    int n = 0,label_num;
    int collected_entries=0;
    while (p != NULL && p != phead->begin.m_end)
    {
        collected_entries++;
        switch(p->type)
        {
            case i_Jump:
                label_num = search_and_add(buf,intptr_t(p->jmp.target_label),n);
                if (p->jmp.jmp_type == JMP_jmp)
                    collector+=QString("jmp%1_").arg(label_num);
                else
                    collector+=QString("jxx%1_").arg(label_num);
                break;
            case i_Label:
                label_num = search_and_add(buf,intptr_t(p),n);
                collector+=QString("from%1_").arg(label_num);
                break;
            case i_Begin:
                collector+="0_";
                p = p->begin.m_end; // skipping to end of p's BB
                break;
            default:
                collected_entries--; // no entry was added in this case, also "why here ?"

        }
        p = instr_next(m_list,p);
        if (collected_entries > 12) // maximum collected pattern length
        {
            collector+="...";
            break;
        }
    }
    qDebug()<<collector;
    return collector;
}

static void insert_after(INSTR_LIST &lst,INSTR_LIST::iterator iter,Instruction *insn)
{
    lst.insert(++iter,insn);
}
bool	InstrList_Finger::Finger_check_partern_for1(Instruction * p)
{
    CFunc_InstrList instrl(this->m_list);
    Instruction * p1 = instr_prev(m_list,p);
    if (p1->var_r1.type != v_Immed)
        return false;
    if (p1->var_r2.type != v_Invalid)
        return false;
    //These two conditions means that only the receiver is in front of the tested as i = n

    Instruction * p2;
    {
        Instruction * ptem = instr_next(m_list,p);
        if (ptem->type != i_Label)
            return false;
        p2 = ptem->label.ref_instr;
    }
    if (p1 == NULL || p2 == NULL)
        return false;

    if (VAR::IsSame(&p1->var_w, &p2->var_r1) || VAR::IsSame(&p1->var_w, &p2->var_r2))
    {
    }
    else
        return false;

    {
        m_list.erase(std::find(m_list.begin(),m_list.end(),p1));
        INSTR_LIST::iterator pos = std::find(m_list.begin(),m_list.end(),p); //	insert after i_CplxBegin

        Instruction * begin = new Instruction(i_Begin);
        Instruction * end = new Instruction(i_End);
        begin->begin.m_end = end;
        insert_after(m_list,pos,end);
        insert_after(m_list,pos,p1);
        insert_after(m_list,pos,begin);
    }

    {
        //p2 is a conditional jump in front of it should be i_end
        Instruction * pend = instrl.instr_prev_in_func(p2);
        assert(pend->type == i_End);

        Instruction * begin = new Instruction(i_Begin);
        Instruction * end = new Instruction(i_End);
        begin->begin.m_end = end;
        insert_after(m_list,std::find(m_list.begin(),m_list.end(),pend),end);
        insert_after(m_list,std::find(m_list.begin(),m_list.end(),pend),begin);

        for (;;)
        {
            Instruction * p3 = instrl.instr_prev_in_func(pend);
            if (p3->type != i_Add && p3->type != i_Sub)
                break;
            m_list.erase(std::find(m_list.begin(),m_list.end(),p3));
            insert_after(m_list,std::find(m_list.begin(),m_list.end(),begin),p3);
        }
    }

    return true;
}
bool	InstrList_Finger::Finger_check_partern(Instruction * p)
{
    // check the pattern p
    char buf[140]={0};
    strcpy(buf,this->prt_partern(p).toStdString().c_str());

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
    {
        //const char finger_for[] 	= "0_jmp1_from2_0_from1_0_jxx3_0_jmp2_from3_";
        p->begin.type = COMP_for;
        //Next, let us make some adjustments in 'for'
        Instruction * p1 = instr_next(m_list,p);	//	p1 points to a label
        assert(p1->type == i_Jump);
        p1 = instr_next(m_list,p1->jmp.target_label);	//p1 points to condition
        if (p1->type == i_Jump
                && p1->jmp.jmp_type != JMP_jmp
                && p1->var_r1.type != 0)
        {
            VAR* pvar = &p1->var_r1;
            Instruction * p2 = instr_prev(m_list,p);
            if (VAR::IsSame(&p2->var_w,pvar))
            {
                m_list.erase(std::find(m_list.begin(),m_list.end(),p2));
                INSTR_LIST::iterator insert_iter = std::find(m_list.begin(),m_list.end(),p); //	insert after i_CplxBegin

                Instruction * begin = new Instruction(i_Begin);
                Instruction * end = new Instruction(i_End);
                begin->begin.m_end = end;
                insert_after(m_list,insert_iter,end);
                insert_after(m_list,insert_iter,p2);
                insert_after(m_list,insert_iter,begin);
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
        Instruction * p = *pos;//list->;
        if (p->type == i_CplxBegin)
        {
            the.Finger_check_partern(p);
        }
    }
}
bool	InstrList::Flow_c(Instruction * phead)
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
            if ((*pos)->type == i_Label)
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
            Instruction * p = *(pos++);
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
    Instruction * p = *(s_pos++);	//	skip first 1
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

bool	InstrList::Flow_cc(Instruction * pNode, POSITION firstpos, POSITION endpos)
{
    //This is used to cplx in another trying to find a few small begin_end to
    // Of three, has a head no tail, it is necessary to find the largest one contains the first statement, and use i_Begin, i_End enclosed
    // And then continue from i_End to end
    // Flow_c (INSTR_LIST * list, POSITION firstpos, POSITION endpos);
    // Of the found i_Begin, i_End, continue with Flow_a
    // PNode is to check the section belongs begin_end, just to provide m_Break
    POSITION okpos;
    POSITION pos1;
    assert(firstpos!=m_list.end());
    assert(endpos!=m_list.end());

    if (firstpos == endpos)
        return false;
    Instruction * phead = *firstpos;

    while(phead->type == i_Label || phead->type == i_Begin)
    {
        if (firstpos == endpos)
            return false;
        phead = *firstpos;
        if (phead->type == i_Label)
        {
            Instruction * p = instr_next(m_list,phead);
            if (p->type == i_Begin)	//	Because i_label will back out of the dead followed by i_begin cycle
                phead = p;	//	process the next one
        }
        if (phead->type == i_Begin)
        {
            if (this->Flow_a(phead))
                return true;
            pos1 = std::find(m_list.begin(),m_list.end(),phead->begin.m_end);
            firstpos=++pos1; //	skip i_End
        }
        else
            break;
    }


    pos1 = okpos = firstpos;

    do
    {
        ++pos1;
        if (ifOneStatement(pNode, firstpos,pos1))
            okpos = pos1;	// record the last success
    } while (pos1 != endpos);

    if (okpos == firstpos)	// not found anything
    {
        pos1 = firstpos;
        ++pos1;
        if (pos1==m_list.end() || pos1 == endpos)
            return false;
        return Flow_cc(pNode, pos1, endpos);		//	start from next
    }

    {
        Instruction * begin = new Instruction(i_Begin);
        Instruction * end = new Instruction(i_End);
        begin->begin.m_end = end;
        POSITION afterokpos=okpos;
        ++afterokpos;
        Add_Begin_End(firstpos, okpos, begin, end);
        okpos=--afterokpos;

        begin->begin.m_break = pNode->begin.m_break;	//	inherit
        begin->begin.m_conti = pNode->begin.m_conti;	//	inherit

        this->Flow_a(begin);
    }

    if (Step_by_Step())
        return true;

    return Flow_cc(pNode, okpos, endpos);	//next part
}
void InstrList::RemoveNops()
{
    POSITION pos = m_list.begin();
    while (pos!=m_list.end())
    {
        if((*pos)->type==i_Nop)
            pos=m_list.erase(pos);
        else
            ++pos;
    }
}
void InstrList::Add_Begin_End(POSITION firstpos, POSITION endpos, Instruction * begin, Instruction * end)
{
    this->Add_Begin_End_1(firstpos,endpos,begin,end);
    // remove nopped instructions
    RemoveNops();
}
void InstrList::Add_Begin_End_1(POSITION firstpos, POSITION endpos, Instruction * begin, Instruction * end)
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
            Instruction *p = *firstpos;
            if (p->type != i_Label)
            {
                m_list.insert(firstpos,begin);
                begin = NULL;	//	cleared away will no longer be dealt with
            }
        }
        if (end)
        {
            Instruction * p = *endpos;
            //assert(p==endinstr);
            if (p->type != i_Label)
            {
                m_list.insert(endpos,end);
                end = NULL;
            }

        }
        if (begin == NULL && end == NULL)
            return;	//	Have to get rid of, and nothing made a


        Instruction * pnew = new Instruction(i_Label);
        //pnew->label.label_off = p->label.label_off;
        pnew->label.ref_instr = 0;

        Instruction * p;
        if (begin)	//	Get it first as a
        {
            p = *firstpos;
            insert_after(m_list,firstpos,pnew);
            insert_after(m_list,firstpos,begin);
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


        Instruction * p_in = 0;
        Instruction * p_out = 0;
        Instruction * p1 = p->label.ref_instr;
        while (p1)
        {
            Instruction * pnext = p1->jmp.next_ref_of_this_label;
            if (if_Ly_In(p1,firstpos,endpos))
            {
                p1->jmp.target_label = pnew;
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

bool InstrList::IsSwitchCase_multcomp(Instruction * begin)
{
    //	Comparison is not the kind switch_case multiple comparisons

    assert(begin->type == i_CplxBegin);
    POSITION iter = std::find(m_list.begin(),m_list.end(),begin);
    M_t* v;

    int first = 1;
    ++iter; //	The first is certainly i_CplxBegin, do not need to read
    for(;iter!=m_list.end();++iter)
    {
        Instruction * p = *iter;
        first++;
        if (first == 2)
        {				//	The first instruction must be jz sth == n
            if (p->type != i_Jump || p->jmp.jmp_type != JMP_jz || p->var_r1.type == 0)
                return false;
            v = p->var_r1.thevar;	//	Write it down, keep up the same as before the next line
            continue;
        }
        if (p->type != i_Jump)
        {
            //There was no Jump
            if (first < 4)		//	not enough comparisons, not really a switch case
                return false;
            return true;
        }
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
    }
    return false;
}
bool InstrList::IsSwitchCase(Instruction * begin)
{
    assert(begin->type == i_CplxBegin);
    POSITION pos = std::find(m_list.begin(),m_list.end(),begin);

    bool first = true;
    for (;pos!=m_list.end();++pos)
    {
        Instruction * p = *pos;//list->;
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
            if (false==first)
                return false;
            first = false;
        }
    }
    return false;
}
void	InstrList::Flow_b(const Instruction * pParentNode, POSITION firstpos, POSITION endpos)
{	//	compact analysis


    //	last not include
    if (firstpos == endpos)
    {
        //alert_prtf("why firstpos == endpos");
        return;
    }

    Instruction * begin = new Instruction(i_CplxBegin);
    Instruction * end = new Instruction(i_CplxEnd);
    begin->begin.m_end = end;
    Add_Begin_End(firstpos, endpos, begin, end);
    POSITION pos = std::find(m_list.begin(),m_list.end(),end);
    assert(pos != m_list.end());
    --pos;		//now it points to last instr in body
    Instruction * plast = *pos;

    Instruction * plast2 = instr_prev(m_list,plast);	//Previous instruction

    Instruction * pNode = begin;
    pNode->begin.m_break = pParentNode->begin.m_break;	//	inherit
    pNode->begin.m_conti = pParentNode->begin.m_conti;	//	inherit

    Instruction * pfirst = instr_next(m_list,begin);
    Instruction * psecond = instr_next(m_list,pfirst);

    if (pfirst->type == i_Label
            || (pfirst->type == i_Jump && pfirst->jmp.jmp_type == JMP_jmp && psecond->type == i_Label)
            )
    {	//	This is my opinion, the conditions for break!
        Instruction * pconti;
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
                    && plast2->jmp.target_label == pconti)
            {	//	这是 while !
                pNode->begin.m_not_conti = plast2;
            }
        }
        else if (plast->type == i_Jump
                 && plast->jmp.jmp_type == JMP_jmp
                 && plast->jmp.target_label == pconti)
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
        //This is switch_case characteristic is the only break,
        //but do not continue, very special.
        pNode->begin.type = COMP_switch_case_multcomp;
        //alert("case find 01");
    }
    if (Step_by_Step())
        return;		//	We have added a i_CplxBegin_End even done live

    Flow_c(begin);	//	Some of this further processing i_CplxBegin
}

bool	InstrList::Flow_a(Instruction * pNode)
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
bool	InstrList::Flow_aa(Instruction * pBlockHeadNode, POSITION firstpos, const POSITION &endpos)
{
    // Loose analysis
    // Return true that there has been progress analysis
    // PBlockHeadNode is a block header in the begin, break and continue address for query site

    // Analysis objectives: a discrete label, jmp
    // If it is found i_CplxBegin, if it begin.type == 0 is not recognized, then try to identify
    // If you have already identified, check the stuff of which i_Begin_i_End
    POSITION pos = firstpos;
    POSITION pos1;
    Instruction * p = 0;
    Instruction * p1= 0;

    assert(firstpos!=m_list.end());
    assert(endpos!=m_list.end());

    if (firstpos == endpos)
        return false;
    //	the first step, find the first compund instruction
    while(pos!=endpos && pos!=m_list.end())
    {
        firstpos=pos;
        p = *(pos++);
        if ( (p->type == i_Jump || p->type == i_Label || p->type == i_Begin || p->type == i_CplxBegin) )
        {
            //	Check is not a break or continue
            if (p->type == i_Jump && p->jmp.jmp_type == JMP_jmp)
            {
                if (p->jmp.target_label == pBlockHeadNode->begin.m_break)
                {
                    qDebug()<<"Skipping 'break'\n";
                    continue;
                }
                if (p->jmp.target_label == pBlockHeadNode->begin.m_conti   && p != pBlockHeadNode->begin.m_not_conti)
                {
                    qDebug()<<"Skipping 'continue'\n";
                    continue;
                }
            }

            break;
        }
    }
    if(pos==endpos || pos==m_list.end())
        return false;
    assert(pos!=m_list.end());
    assert(0!=p);
    switch(p->type)
    {
        case i_Begin:
            qDebug()<<"Trying 'i_Begin'\n";
            if (this->Flow_a(p))
                return true;
            pos1 = std::find(m_list.begin(),m_list.end(),p->begin.m_end);
            assert(pos1!=m_list.end());
            return Flow_aa(pBlockHeadNode,pos1,endpos);

        case i_CplxBegin:
            qDebug()<<"Trying 'i_CplxBegin'\n";
            pos1 = std::find(m_list.begin(),m_list.end(),p->begin.m_end);
            assert(pos1!=m_list.end());
            if (Flow_c(p))
                return true;

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

        case i_Jump:
            qDebug()<<"Trying 'i_Jump'\n";
            //it must be jump to follow
            p1 = p->jmp.target_label;	//it jump here
            pos1 = std::find(m_list.begin(),m_list.end(),p1);
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
                {	//	Found a compact structure
                    Flow_b(pBlockHeadNode, firstpos,pos1);
                    if (Step_by_Step())
                        return true;	//	Where you can directly return TRUE
                    assert(pos1!=m_list.end());
                    return Flow_aa(pBlockHeadNode, pos1,endpos);
                }
                //assert(pos1);
                ++pos1;
            }
            break;
        case i_Label:
            qDebug()<<"Trying 'i_Label'\n";
            p1 = p->label.ref_instr;
            pos1 = std::find(m_list.begin(),m_list.end(),p1);
            while (pos1!=m_list.end() && pos1 != endpos)
            {
                ++pos1;
                assert(pos1!=m_list.end());
                if (ifOneStatement(pBlockHeadNode, firstpos,pos1))
                {
                    Flow_b(pBlockHeadNode, firstpos,pos1);
                    if (Step_by_Step())
                        return true;	//	Where you can directly return TRUE
                    assert(pos1!=m_list.end());
                    return Flow_aa(pBlockHeadNode, pos1,endpos);
                }
                //assert(pos1);
            }
        default:
            qDebug()<<"flow_aa: skipping opcode " << p->type;
    }
    return false;
}
/*
There are three analysis, have to make clear.

        One is a head and a tail, known this is a loose statement,
            find the one in which the complex statement
        Flow_a (PINSTR phead, INSTR_LIST * list);
        Of finding the complex, dealing with Flow_b

        Two, from beginning to end, and is known this is a single complex statement,
             including the need to own their own
        Flow_b (INSTR_LIST * list, POSITION firstpos, POSITION endpos);
        After their own enclosed, call a Flow_c

        Three, enter a single complex statement,
             try to find the include the header from one of the biggest statement,
             and use i_Begin, i_End quotes
        Continue to the end and from i_End
        Flow_c (PINSTR phead, INSTR_LIST * list);
        Of finding the i_Begin, i_End, continue with Flow_a
*/

// --------------------------------------------------------------
void	CFunc_Prt::add_default_entry(CasePrt_List* list, Instruction * thelabel)
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
    //	Can not just check this one condition, the last one is the most important
    list->push_back(new OneCase(0xffffffff,thelabel));
}
void	CFunc_Prt::Add_case_entry(CasePrt_List* list, int case_n, Instruction * thelabel)
{
    //	Switch case must first save all the items, and finally with the print
    //alert("add case entry");
    //static function

    CasePrt_List::iterator iter = list->begin();
    for (;iter!=list->end(); ++iter)
    {
        OneCase* p = *iter;
        if (p->case_n == case_n)
            error("why same case");
        if (p->thelabel->label.label_off >= thelabel->label.label_off)
        {
            --iter; // move pos to the last proper element
            break;
        }
    }
    // std::list handles inserting at end() iterator
    list->insert(iter, new OneCase(case_n,thelabel));
    return;
}
