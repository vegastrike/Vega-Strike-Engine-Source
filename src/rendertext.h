
class RText {
	private:
		//text rendering
		struct cline { 
			cline() { outtime = 0; };
			cline(const cline &in) {
			if(in.cref.size() > 0)
				cref.append(in.cref);
			outtime = in.outtime;
			}
			std::string cref;
			int outtime; 
		};
		std::vector<cline> conlines;
		std::vector<std::string> vhistory;
		int ndraw;
		int WORDWRAP;
		int conskip;
		int histpos;
		std::string commandbuf;
	public:
		RText();
		virtual ~RText();
		std::string getcurcommand();
		int RText::text_width(char *str); //set the text width?
		void RText::draw_textf(std::string &fstr, int left, int top, int gl_num, int arg); //bad bad don't use
		void RText::draw_text(std::string &str, float left, float top, int gl_num);
//draws str to the screen, don't forget to glOrtho first.
		void RText::draw_envbox_aux(float s0, float t0, int x0, int y0, int z0,
			float s1, float t1, int x1, int y1, int z1,
			float s2, float t2, int x2, int y2, int z2,
			float s3, float t3, int x3, int y3, int z3,
			int texture); //enviorment box?
		void RText::draw_envbox(int t, int w);//..
		void renderconsole(); //renders the text in the console
		void conline(std::string &sf, bool highlight); //add a line to
			//the console
		void conoutf(std::string &s, int a = 0, int b = 0, int c = 0);
		//add a line to the console(Use this one.)
		void saycommand(char *init); //actually does the appending of
			//the string to the commandbuf, and seperates entries
		void RText::ConsoleKeyboardI(int code, bool isdown, int cooked);
			//interpret keyboard input to the console
//		char *RText::getComBuf();
		void RText::conoutn(std::string &in, int a, int b, int c);
//Menus
//		bool rendermenu();
//		void newmenu(char *name);
//		void menumanual(int m, int n, char *text);
//		void menuitem(char *text, char *action);
//		bool menukey(int code, bool isdown);
//		char *getMenuAction();
//		void RText::blendbox(int x1, int y1, int x2, int y2, bool border);

			//returns the current command being typed in, for the
			//prompt eg: > /addtri

};

