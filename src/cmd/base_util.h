#include <string>

namespace BaseUtil {
	int Room (std::string text);
	void Texture(int room, std::string index, std::string file, float x, float y);
	void Ship (int room, std::string index,QVector pos,Vector R, Vector Q);
	void LinkPython (int room, std::string index,std::string pythonfile, float x, float y, float wid, float hei, std::string text, int to);
	void LaunchPython (int room, std::string index,std::string pythonfile, float x, float y, float wid, float hei, std::string text);
	void CompPython(int room, std::string index,std::string pythonfile, float x, float y, float wid, float hei, std::string text, std::string modes);

	void Link (int room, std::string index, float x, float y, float wid, float hei, std::string text, int to);
	void Launch (int room, std::string index, float x, float y, float wid, float hei, std::string text);
	void Comp(int room, std::string index, float x, float y, float wid, float hei, std::string text, std::string modes);
	void Python(int room, std::string index, float x, float y, float wid, float hei, std::string text, std::string pythonfile,bool front=false);
	void MessageToRoom(int room, std::string text);
	void EnqueueMessageToRoom(int room, std::string text);
	void Message(std::string text);
	void EnqueueMessage(std::string text);
	void EraseLink (int room, std::string index);
	void EraseObj (int room, std::string index);
	int GetCurRoom ();
	int GetNumRoom ();
}
