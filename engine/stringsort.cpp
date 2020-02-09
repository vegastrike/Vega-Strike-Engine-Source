#include <vector>
#include <string>
#include <stdio.h>
#include <algorithm>
using std::string;
using std::vector;
int nextnondigit  (string a) {
	for (unsigned int i=0;i<a.length();++i) {
		if (!isdigit(a[i]))
			return i;
	}
	return a.length();
}
class LexibackwardComparator {
public:
	bool operator () (string a, string b) {
		if (a.empty()||b.empty())
			return false;
		if (isdigit (a[0]) && isdigit(b[0])) {
			int aint=0;
			int bint=0;
			sscanf (a.c_str(),"%d",&aint);
			sscanf (b.c_str(),"%d",&bint);
			if (aint==bint) {
				return (*this)(a.substr(nextnondigit(a)),b.substr(nextnondigit(b)));
			}
			return aint<bint;
		}
		if (a[0]==b[0]) {
			return (*this)(a.substr(1),b.substr(1));
		}
		return a[0]<b[0];
	}

};

int main (int argc, char ** argv) {
	vector <string> l;
	for (int i=1;i<argc;++i) {
		l.push_back(argv[i]);
	}
	std::sort (l.begin(),l.end(),LexibackwardComparator());
	{for (unsigned int i=0;i<l.size();++i) {
		printf ("%s\n",l[i].c_str());
	}}
	
	
}
