//a class to hold attributes for other classes..
class Attributes {
        public:
        Attributes() { hidden = false; webbcmd = false; immcmd = false;};
        bool hidden; //hidden
        bool webbcmd; //web command
        bool immcmd; //immortal command
        int type;
};

class TFunctor
{
	public:
		Attributes attribs; //public attribute object
			//can hold bools and other various info to be set
			//WHEN the command is created, BEFORE it's added
			//to the command processor, to give it different
			//behaviors.
		virtual ~TFunctor(){};
		virtual void Call()=0;
		virtual void Call(std::string &)=0;        // call using function
		virtual void Call(const char *array[])=0;
		virtual void Call(const char *)=0;
		virtual void Call(const char *, const char *)=0;
		virtual void Call(bool *)=0;
		virtual void Call(int)=0;
		virtual void Call(char)=0;
		virtual void Call(std::vector<std::string *> *d)=0;
		virtual void Call(std::vector<std::string *> *d, int &sock_in)=0;
		virtual void Call(std::string &, int&)=0;
};

template <class TClass> class Functor : public TFunctor 
{
        private:
                void (TClass::*fpt1)();
                void (TClass::*fpt2)(std::string &);
                void (TClass::*fpt3)(const char *);
                void (TClass::*fpt4)(const char *array[]);
                void (TClass::*fpt5)(const char *, const char *);
                void (TClass::*fpt6)(bool *);
                void (TClass::*fpt7)(int);
                void (TClass::*fpt8)(char);
		void (TClass::*fpt9)(std::vector<std::string *> *d);
		void (TClass::*fpt10)(std::vector<std::string *> *d, int &sock_in);
		void (TClass::*fpt11)(std::string &, int&);		
                TClass* pt2Object; // pointer to object

        public:
//no arguements at all.
                Functor(TClass* _pt2Object, void(TClass::*_fpt)())
                { pt2Object = _pt2Object;  fpt1=_fpt;};
//1 std::string
                Functor(TClass* _pt2Object, void(TClass::*_fpt)(std::string &))
                { pt2Object = _pt2Object;  fpt2=_fpt; };
//1 c string
		Functor(TClass* _pt2Object, void(TClass::*_fpt)(const char *))
		{ pt2Object = _pt2Object;  fpt3=_fpt;};
                Functor(TClass* _pt2Object, void(TClass::*_fpt)(const char *array[]))
                { pt2Object = _pt2Object; fpt4=_fpt;};
//2 c strings
                Functor(TClass* _Obj, void(TClass::*_fpt)(const char *, const char *))
                { pt2Object = _Obj; fpt5=_fpt; };
//1 bool
                Functor(TClass* _Obj, void(TClass::*_fpt)(bool *))
                { pt2Object = _Obj; fpt6=_fpt; };

                Functor(TClass* _Obj, void(TClass::*_fpt)(int))
                { pt2Object = _Obj; fpt7=_fpt; };

// 
                Functor(TClass* _Obj, void(TClass::*_fpt)(char))
                { pt2Object = _Obj; fpt8=_fpt; };

		Functor(TClass* _Obj, void(TClass::*_fpt)(std::vector<std::string *> *d))
		{ pt2Object = _Obj, fpt9=_fpt; };		
// 

                Functor(TClass* _Obj, void(TClass::*_fpt)(std::vector<std::string *> *d, int &))
                { pt2Object = _Obj, fpt10=_fpt; }

                Functor(TClass* _pt2Object, void(TClass::*_fpt)(std::string &, int&))
                { pt2Object = _pt2Object;  fpt11=_fpt; };

		virtual ~Functor(){};

                virtual void Call()
                {
                        (*pt2Object.*fpt1)();
                };
                virtual void Call(std::string &string)
                {
                        (*pt2Object.*fpt2)(string);
                };  // execute member function

               virtual void Call(const char *string)
                {
                        (*pt2Object.*fpt3)(string);
                };
                virtual void Call(const char *string[])
                {
                        (*pt2Object.*fpt4)(string);
                };
                virtual void Call(const char *s1, const char *s2)
                {
                        (*pt2Object.*fpt5)(s1, s2);
                };
                virtual void Call(bool *b1)
                {
                        (*pt2Object.*fpt6)(b1);
                };
                virtual void Call(int b1)
                {
                        (*pt2Object.*fpt7)(b1);
                };

                virtual void Call(char b1)
                {
                        (*pt2Object.*fpt8)(b1);
                };
		virtual void Call(std::vector<std::string *> *d)
		{
			(*pt2Object.*fpt9)(d);
		}

                virtual void Call(std::vector<std::string *> *d, int &sock_in)
                {
                        (*pt2Object.*fpt10)(d, sock_in);
                }
                virtual void Call(std::string &string, int &sock_in)
                {
                        (*pt2Object.*fpt11)(string, sock_in);
                };  // execute member function


};

