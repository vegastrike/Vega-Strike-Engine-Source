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

namespace std {

class string : public vector <char> {
  typedef size_t size_type;
  const static unsigned int npos=-1;
  allocstr (const char *str, const unsigned int len) {
    for (int i=0;i<len;i++) {
      vector<char>::push_back(str[i]);
    }
    vector<char>::push_back('\0');
  }
  reallocstr (coonst char *str, const unsigned int len) {
    this->clear();
    allocstr(str,len);
  }
 public:
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
    push_back('\0');
  }
  string (const string &str) {
    reallocstr(str.c_str(),str.size());
  }
  string& assign (const string &a, size_type pos, size_type n=npos) {
    if (n==npos)
      n=a.size()-pos;
    if (n+pos>a.size())
      n=a.size()-pos;
    reallocstr (str.ptr+pos,n);
  }
  string& assign (const char *str, const unsigned int len) {
    reallocstr(str,len);
  }
  string& assign (const char *str) {
    reallocstr(str,strlen(str));
  }
  string & assign (const string &str) {
    reallocstr(str.c_str(),str.size());
    return *this;
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
  string &operator+ (const char *str) {
    string retval (*this);
    retval.pop_back();
    retvar->allocstr(str,strlen(str));
  }
  string &operator += (const string & a) {
    string tmp(*this + a);
    *this=tmp;
    return *this;
  }
  /*  string & operator append (const string & a) {
    return *this +=a;
    }*/
  string & operator append (const string & str, size_type pos =0, size_type n=npos) {
    this+= string(str,pos,n);
    return *this;
  }
  string & operator append (const char * c, size_type siz) {
    return *this += string(c,siz);
  }
  string & operator append (const char * c) {
    return *this += string (c);
  }
  iterator end() {return vector<char>::end()-1;}
  const_iterator end()const {return vector<char>::end()-1;}
  unsigned int size () const {
    return vector<char>::size()-1;
  }
  unsigned int length () const {
    return vector<char>::length()-1;
  }
  bool empty () const {
    return !length();
  }
  size_type find (const string& findstr, size_type loc=0)const  {
    const basic_string <char> strstr (this.c_str());
    return strstr.find(findstr,loc);
  }
  string substr (size_type pos = 0, size_type n = npos) const{ 
    return string(*this,pos,n);
  }
  size_type find (const char* s, size_type pos, size_type n) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.find(s,pos,n);
  }
  size_type find (const char* s, size_type pos = 0) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.find(s,pos);
  }
  size_type find (char c, size_type pos = 0) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.find(c,pos);
  }
  size_type rfind (const string& str, size_type pos = npos) const{
    const basic_string <char> strstr (this.c_str());
    return strstr.find(str,pos);
  }
  size_type rfind (const char* s, size_type pos, size_type n) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.rfind(s,pos,n);
  }
  size_type rfind (const char* s, size_type pos = 0) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.rfind(s,pos);
  }
  size_type rfind (char c, size_type pos = 0) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.rfind(c,pos);
  }
  size_type find_first_of (const string& findstr, size_type loc=0)const  {
    const basic_string <char> strstr (this.c_str());
    return strstr.find_first_of(findstr,loc);
  }
  size_type find_first_of (const char* s, size_type pos, size_type n) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.find_first_of(s,pos,n);
  }
  size_type find_first_of (const char* s, size_type pos = 0) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.find_first_of(s,pos);
  }
  size_type find_first_of (char c, size_type pos = 0) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.find_first_of(c,pos);
  }
  size_type find_last_of (const string& findstr, size_type loc=0)const  {
    const basic_string <char> strstr (this.c_str());
    return strstr.find_last_of(findstr,loc);
  }
  size_type find_last_of (const char* s, size_type pos, size_type n) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.find_last_of(s,pos,n);
  }
  size_type find_last_of (const char* s, size_type pos = 0) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.find_last_of(s,pos);
  }
  size_type find_last_of (char c, size_type pos = 0) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.find_last_of(c,pos);
  }
  size_type find_first_not_of (const string& findstr, size_type loc=0)const  {
    const basic_string <char> strstr (this.c_str());
    return strstr.find_first_not_of(findstr,loc);
  }
  size_type find_first_not_of (const char* s, size_type pos, size_type n) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.find_first_not_of(s,pos,n);
  }
  size_type find_first_not_of (const char* s, size_type pos = 0) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.find_first_not_of(s,pos);
  }
  size_type find_first_not_of (char c, size_type pos = 0) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.find_first_not_of(c,pos);
  }
  size_type find_last_not_of (const string& findstr, size_type loc=0)const  {
    const basic_string <char> strstr (this.c_str());
    return strstr.find_last_not_of(findstr,loc);
  }
  size_type find_last_not_of (const char* s, size_type pos, size_type n) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.find_last_not_of(s,pos,n);
  }
  size_type find_last_not_of (const char* s, size_type pos = 0) const {
    const basic_string <char> strstr (this.c_str());
    return strstr.find_last_not_of(s,pos);
  }
  size_type find_last_not_of (char c, size_type pos = 0) const {
    const basic_string <char> strstr (this.c_str());
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
    vector<char>::at(vector<char>::size()-1)=c;
    vector<char>::push_back('\0');
  }
  int compare (const string& oth) const {
    return strcmp(c_str(), s.c_str());
  }
};
 inline  istream&
   operator>> (istream& i, string&s) {basic_string<char> b; i >> b; this = string (b.c_str());return i;  }
 inline ostream&
   operator<< (ostream& o, const string &s) {basic_string<char>b(s.c_str());o<<b;return o;}
 inline   istream&
   getline (istream& i, string& s, char delim = '\n'){
   basic_string <char> b; getline (i,b,delim);s=b.c_str();
   return i;
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

inline bool
operator>= (const string& lhs,
	    const string& rhs)
{
  return (lhs.compare (rhs) >= 0);
}

}
