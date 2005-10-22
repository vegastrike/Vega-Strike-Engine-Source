#ifdef MMORPGC_INC
#else
#define MMORPGC_INC 1
//linux version is int
//extern commandI CommandInterpretor;
class POSpack;
class mmoc : public commandI {
//		sockaddr_in m_addr;
		int socket;
		std::string start; //holds strings waiting to be processed
		std::string tempstr; //holds temporary strings
		bool status;
		int binarysize;
		bool binmode; //bypass string processing
		bool POSmode; //expect position packet
	public:
		bool getStatus(int); //also sets it, mutex'd
		mmoc();
		int getSocket() {return socket;};
		void connectTo(const char *host, const char *port);
		void negotiate(std::vector<std::string *> *d);
		bool listenThread();
		void createThread();
		void send(char *buffer, int size);
		void send(std::string &string);
		void ParseBinaryData(std::vector<char *> &in);
		void ParseMovement(POSpack &in);
		void mmoc::ParseRemoteInput(char *buf);
		virtual void conoutf(std::string &, int x = 0, int y = 0, int z = 0);
		void close();
		Functor<mmoc> *cmd;
		Functor<mmoc> *cl; //disconnect/close
		Functor<mmoc> *csay;
};

int startThread(void *mmoc2use);
#endif
