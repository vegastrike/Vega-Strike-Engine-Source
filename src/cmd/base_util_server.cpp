#include <string>
#include <stdlib.h>
#include "base_util.h"
namespace BaseUtil {
	int Room (std::string text) {
	}
	void Texture(int room, std::string index, std::string file, float x, float y) {
	}
	void Ship (int room, std::string index,QVector pos,Vector Q, Vector R) {
	}
	static void BaseLink (BaseInterface::Room *room,float x, float y, float wid, float hei, std::string text) {
	}
	void Link (int room, std::string index, float x, float y, float wid, float hei, std::string text, int to) {
		LinkPython (room, index, "",x, y,wid, hei, text, to);
	}
	void LinkPython (int room, std::string index,std::string pythonfile, float x, float y, float wid, float hei, std::string text, int to) {
	}
	void Launch (int room, std::string index, float x, float y, float wid, float hei, std::string text) {
		LaunchPython (room, index,"", x, y, wid, hei, text);
	}
	void LaunchPython (int room, std::string index,std::string pythonfile, float x, float y, float wid, float hei, std::string text) {
	}
	void EjectPython (int room, std::string index,std::string pythonfile, float x, float y, float wid, float hei, std::string text) {
	}
	void Comp(int room, std::string index, float x, float y, float wid, float hei, std::string text, std::string modes) {
	  CompPython(room, index,"", x, y, wid, hei, text,modes) ;
 
	}
	void CompPython(int room, std::string index,std::string pythonfile, float x, float y, float wid, float hei, std::string text, std::string modes) { 
	}
	void Python(int room, std::string index, float x, float y, float wid, float hei, std::string text, std::string pythonfile) {
	}
	void Message(std::string text) {
	}
	void EnqueueMessage(std::string text) {
	}
	void EraseLink (int room, std::string index) {
	}
	void EraseObj (int room, std::string index) {
	}
	int GetCurRoom () {
	}
	int GetNumRoom () {
	}
}
