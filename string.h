#include <vector>
#include <bastring>
#include <string.h>

namespace std {

class string : public vector <char> {
  typedef size_t size_type;
  const static unsigned int npos=-1;
  allocstr (const char *str, const unsigned int len) {
    for (int i=0;i<len;i++) {
      push_back(str[i]);
    }
    push_back('\0');
  }
  reallocstr (coonst char *str, const unsigned int len) {
    this->clear();
    allocstr(str,len);
  }
 public:
  string (const char *str, const unsigned int len) {
    reallocstr(str,len);
  }
  string (const char *str) {
    reallocstr(str,strlen(str));
  }
  string () {
    push_back('\0');
  }
  string (const string &str) {
    reallocstr(str.c_str(),str.size());
  }
  ~string () {
  }
  const char *c_str() const {
    return &(*this->begin());
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
  unsigned int find (const string& findstr, const unsigned int loc)const  {
    const basic_string <char> strstr (this.c_str());
    return strstr.find(findstr,loc);
  }
};
}
    
