#ifndef _PATRICK_AND_DANNY_STRING_H_
#define _PATRICK_AND_DANNY_STRING_H_
#include <vector>
#include <iostream>
#ifdef __APPLE__

#include <bits/c++config.h>
#include <bits/stringfwd.h>
#include <bits/char_traits.h>
#include <memory> 	// For allocator.
#include <bits/type_traits.h>
#include <iosfwd> 	// For operators >>, <<, and getline decls.
#include <bits/stl_iterator.h>
#include <bits/stl_function.h>  // For less
#include <bits/basic_string.h>
#else
#include <std/bastring.h>
#endif
#define olde_string basic_strung

namespace std {
typedef vector <char,malloc_alloc> pandd_vector;
class string : public pandd_vector {
  void allocstr (const char *str, size_type len) {
    if (len==npos)
      len=strlen(str);
    for (unsigned int i=0;i<len;i++) {
      pandd_vector::push_back(str[i]);
    }
    pandd_vector::push_back('\0');
  }
  void reallocstr (const char *str, const size_type len) {
    pandd_vector::clear();
    allocstr(str,len);
  }
 public:
  typedef size_t size_type;
  const static size_type npos=static_cast<size_type> (-1);

  void clear () {
    *this = "";
  }
  string (const string &a, size_type pos, size_type n=npos) {
    assign (a,pos,n);
  }
  string (const char *str, const unsigned int len) {
    assign(str,len);
  }
  string (const char *str) {
    assign (str);
  }
  string () {
    pandd_vector::push_back('\0');
  }
  string (const string &str) {
    reallocstr(str.c_str(),str.size());
  }
  string& assign (const string &a, size_type pos, size_type n=npos) {
    if (n==npos)
      n=a.size()-pos;
    if (n+pos>a.size())
      n=a.size()-pos;
    reallocstr (a.c_str()+pos,n);
    return *this;
  }
  string&assign (const char *str, const size_type len) {
    reallocstr(str,len);
    return (*this);
  }
  string& assign (const char *str) {
    reallocstr(str,strlen(str));
    return (*this);
  }
  string & assign (const string &str) {
    reallocstr(str.c_str(),str.size());
    return *this;
  }
  string& replace (size_type pos1, size_type n1, const string& str, size_type pos2 = 0, size_type n2 = npos) {
    olde_string <char> bastr (this->c_str());
    olde_string <char> strstr (str.c_str());
    bastr.replace(pos1,n1,strstr,pos2,n2);
    return ((*this)=bastr.c_str());
  }    
  string& replace (size_type pos, size_type n1, const char* s, size_type n2) {
    olde_string <char> bastr (this->c_str());
    bastr.replace(pos,n1,s,n2);
    return ((*this)=bastr.c_str());
  }
  string& replace (size_type pos, size_type n1, const char* s) {
    olde_string <char> bastr (this->c_str());
    bastr.replace(pos,n1,s);
    return ((*this)=bastr.c_str());
  }
  string& replace (size_type pos, size_type n1, size_type n2, char c) {
    olde_string <char> bastr (this->c_str());
    bastr.replace(pos,n1,n2,c);
    return ((*this)=bastr.c_str());
  }
  string& replace (size_type pos, size_type n, char c) {
    olde_string <char> bastr (this->c_str());
    bastr.replace(pos,n,c);
    return ((*this)=bastr.c_str());
  }
  string& replace (iterator i1, iterator i2, const string& str) {
    olde_string <char> bastr (this->c_str());
    olde_string <char> strstr (str.c_str());
    bastr.replace(i1,i2,strstr);
    return ((*this)=bastr.c_str());
  }
  string& replace (iterator i1, iterator i2, const char* s, size_type n) {
    olde_string <char> bastr (this->c_str());
    bastr.replace(i1,i2,s,n);
    return ((*this)=bastr.c_str());
  }
  string& replace (iterator i1, iterator i2, const char* s) {
    olde_string <char> bastr (this->c_str());
    bastr.replace(i1,i2,s);
    return ((*this)=bastr.c_str());
  }
  string& replace (iterator i1, iterator i2, size_type n, char c) {
    olde_string <char> bastr (this->c_str());
    bastr.replace(i1,i2,n,c);
    return ((*this)=bastr.c_str());
  }
  size_type copy (char *buf, size_type n, size_type pos=0) const {
    olde_string <char> bastr ((*this).c_str());
    return bastr.copy(buf,n,pos);
  }
  ~string () {
  }
  const char *c_str() const {
    return &(*this->begin());
  }
  const char *data () const {
    return c_str();
  }
  string &operator= (const char *str) {
    reallocstr(str,strlen(str));
    return *this;
  }
  string operator+ (const char *str)const {
    string retval (*this);
    retval.pop_back();
    retval.allocstr(str,strlen(str));
    return retval;
  }
  string operator+ (const string &str)const {
    string retval (*this);
    retval.pop_back();
    retval.allocstr(str.c_str(),str.size());
    return retval;
  }
  string operator+ (char c)const {	
    char temp[2]={c,0};
    return *this+temp;
  }
  string &operator += (const string & a) {
    string tmp(*this + a);
    *this=tmp;
    return *this;
  }
  string &operator += (char a) {
	char bleh[2]={a,0};
	return (*this)+=bleh;
  }
  /*  string & operator append (const string & a) {
    return *this +=a;
    }*/
  string & append (const string & str, size_type pos =0, size_type n=npos) {
    (*this)+= string(str,pos,n);
    return *this;
  }
  string & append (const char * c, size_type siz) {
    return *this += string(c,siz);
  }
  string & append (const char * c) {
    return *this += string (c);
  }
  iterator end() {return pandd_vector::end()-1;}
  const_iterator end()const {return pandd_vector::end()-1;}
  size_type size () const {
    return pandd_vector::size()-1;
  }
  size_type length () const {
    return pandd_vector::size()-1;
  }
  bool empty () const {
    return !length();
  }
  size_type find (const string& findstr, size_type loc=0)const  {
    const olde_string <char> strstr (this->c_str());
    const olde_string <char> fstrstr (findstr.c_str());
    return strstr.find(fstrstr,loc);
  }
  string substr (size_type pos = 0, size_type n = npos) const{ 
    return string(*this,pos,n);
  }
  size_type find (const char* s, size_type pos, size_type n) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.find(s,pos,n);
  }
  size_type find (const char* s, size_type pos = 0) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.find(s,pos);
  }
  size_type find (char c, size_type pos = 0) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.find(c,pos);
  }
  size_type rfind (const string& str, size_type pos = npos) const{
    const olde_string <char> strstr (this->c_str());
    const olde_string <char> fstrstr (str.c_str());
    return strstr.find(fstrstr,pos);
  }
  size_type rfind (const char* s, size_type pos, size_type n) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.rfind(s,pos,n);
  }
  size_type rfind (const char* s, size_type pos = npos) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.rfind(s,pos);
  }
  size_type rfind (char c, size_type pos = npos) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.rfind(c,pos);
  }
  size_type find_first_of (const string& findstr, size_type loc=0)const  {
    const olde_string <char> strstr (this->c_str());
    const olde_string <char> fstrstr (findstr.c_str());
    return strstr.find_first_of(fstrstr,loc);
  }
  size_type find_first_of (const char* s, size_type pos, size_type n) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.find_first_of(s,pos,n);
  }
  size_type find_first_of (const char* s, size_type pos = 0) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.find_first_of(s,pos);
  }
  size_type find_first_of (char c, size_type pos = 0) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.find_first_of(c,pos);
  }
  size_type find_last_of (const string& findstr, size_type loc=npos)const  {
    const olde_string <char> strstr (this->c_str());
    const olde_string <char> fstrstr (findstr.c_str());
    return strstr.find_last_of(fstrstr,loc);
  }
  size_type find_last_of (const char* s, size_type pos, size_type n) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.find_last_of(s,pos,n);
  }
  size_type find_last_of (const char* s, size_type pos = npos) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.find_last_of(s,pos);
  }
  size_type find_last_of (char c, size_type pos = npos) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.find_last_of(c,pos);
  }
  size_type find_first_not_of (const string& findstr, size_type loc=0)const  {
    const olde_string <char> strstr (this->c_str());
    const olde_string <char> fstrstr (findstr.c_str());
    return strstr.find_first_not_of(fstrstr,loc);
  }
  size_type find_first_not_of (const char* s, size_type pos, size_type n) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.find_first_not_of(s,pos,n);
  }
  size_type find_first_not_of (const char* s, size_type pos = 0) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.find_first_not_of(s,pos);
  }
  size_type find_first_not_of (char c, size_type pos = 0) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.find_first_not_of(c,pos);
  }
  size_type find_last_not_of (const string& findstr, size_type loc=npos)const  {
    const olde_string <char> strstr (this->c_str());
    const olde_string <char> fstrstr (findstr.c_str());
    return strstr.find_last_not_of(fstrstr,loc);
  }
  size_type find_last_not_of (const char* s, size_type pos, size_type n) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.find_last_not_of(s,pos,n);
  }
  size_type find_last_not_of (const char* s, size_type pos = npos) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.find_last_not_of(s,pos);
  }
  size_type find_last_not_of (char c, size_type pos = npos) const {
    const olde_string <char> strstr (this->c_str());
    return strstr.find_last_not_of(c,pos);
  }

  /*
  void swap (string &s) {
    string oldthis (*this);
    (*this)=s;
    s=oldthis;
  }
  */
  void push_back (char c) {
    pandd_vector::operator[](pandd_vector::size()-1)=c;
    pandd_vector::push_back('\0');
  }
  char &at (size_type pos) {
    return (*this)[pos];
  }
  int compare (const string& oth) const {
    return strcmp(c_str(), oth.c_str());
  }
};
 inline  istream&
   operator>> (istream& i, string&s) {olde_string<char> b; i >> b; s = string (b.c_str());return i;  }
 inline ostream&
   operator<< (ostream& o, const string &s) {olde_string<char>b(s.c_str());o<<b;return o;}
 inline   istream&
   getline (istream& i, string& s, char delim = '\n'){
   olde_string <char> b; getline (i,b,delim);s=b.c_str();
   return i;
 }
inline bool
operator== (const string& lhs,
	    const string & rhs)
{
  return (lhs.compare (rhs) == 0);
}

inline bool
operator== (const char* lhs, const string& rhs)
{
  return (rhs.compare (lhs) == 0);
}


inline bool
operator== (const string& lhs, const char* rhs)
{
  return (lhs.compare (rhs) == 0);
}


inline bool
operator!= (const char* lhs, const string& rhs)
{
  return (rhs.compare (lhs) != 0);
}
inline bool
operator!= (const string& lhs, const char* rhs)
{
  return (lhs.compare (rhs) != 0);
}
inline bool
operator!= (const string& lhs,
	    const string& rhs)
{
  return (lhs.compare (rhs) != 0);
}


inline bool
operator< (const string& lhs,
	    const string& rhs)
{
  return (lhs.compare (rhs) < 0);
}

inline bool
operator< (const char* lhs, const string& rhs)
{
  return (rhs.compare (lhs) > 0);
}

inline bool
operator< (const string& lhs, const char* rhs)
{
  return (lhs.compare (rhs) < 0);
}

inline bool
operator> (const char* lhs, const string& rhs)
{
  return (rhs.compare (lhs) < 0);
}

inline bool
operator> (const string& lhs, const char* rhs)
{
  return (lhs.compare (rhs) > 0);
}

inline bool
operator<= (const char* lhs, const string& rhs)
{
  return (rhs.compare (lhs) >= 0);
}

inline bool
operator<= (const string& lhs, const char* rhs)
{
  return (lhs.compare (rhs) <= 0);
}

inline bool
operator>= (const char* lhs, const string& rhs)
{
  return (rhs.compare (lhs) <= 0);
}

inline bool
operator>= (const string& lhs, const char* rhs)
{
  return (lhs.compare (rhs) >= 0);
}

inline bool
operator> (const string& lhs,
	   const string& rhs)
{
  return (lhs.compare (rhs) > 0);
}

inline bool
operator<= (const string& lhs,
	    const string& rhs)
{
  return (lhs.compare (rhs) <= 0);
}


inline string
operator+ (const char * lhs,
	    const string& rhs)
{
  return string(lhs)+rhs;
}
/*
inline string
operator+ (const string & lhs,
	    const char * rhs)
{
  return lhs+string(rhs);
}*/

inline bool
operator>= (const string& lhs,
	    const string& rhs)
{
  return (lhs.compare (rhs) >= 0);
}

}
#endif
