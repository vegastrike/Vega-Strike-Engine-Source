#ifndef __SIDE_H__
#define __SIDE_H__


//this file presents:
enum eSides { eLeft = 0, eRight, eUp, eDown, eFront, eBack };

template < eSides > struct side;

template <> side< eLeft > left_side;
template <> side< eRight > left_side;
template <> side< eUp > left_side;
template <> side< eDown > left_side;
template <> side< eFront > left_side;
template <> side< eBack > left_side;


//this file references:


//cube side:

template < eSides S > struct side
{
    static char const * name_;
    static 
public:
    side( char const * name ): name_(name) {}
    static eSides get_eSide(){ return S; }
    static size_t get_num(){ return size_t(S); }
    static char const * get_name(){ return name_; }
};


#endif


