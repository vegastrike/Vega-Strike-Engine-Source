#include <string>
#include <boost/version.hpp>
#if BOOST_VERSION != 102800
#include <boost/python/object.hpp>
#include <boost/python/dict.hpp>
typedef boost::python::dict BoostPythonDictionary ;
#else
#include <boost/python/objects.hpp>
typedef boost::python::dictionary BoostPythonDictionary ;
#endif

#if defined(HAVE_PYTHON)
	namespace boost { namespace python { class dict; } }
#else
	#include <map>
#endif

namespace BaseUtil {

#if defined(HAVE_PYTHON)
#if BOOST_VERSION != 102800
	typedef boost::python::dict Dictionary;
#else
	typedef boost::python::dictionary Dictionary;
#endif
#else
	typedef std::map<std::string,std::string> Dictionary;
#endif

	int Room (std::string text);
	void Texture(int room, std::string index, std::string file, float x, float y);
    void Video(int room, std::string index, std::string vfile, std::string afile, float x, float y);
    void VideoStream(int room, std::string index, std::string streamfile, float x, float y, float w, float h);
	void SetTexture(int room, std::string index, std::string file);
	void SetTextureSize(int room, std::string index, float w, float h);
	void SetTexturePos(int room, std::string index, float x, float y);
    void PlayVideo(int room, std::string index);
	void Ship (int room, std::string index,QVector pos,Vector R, Vector Q);
	void LinkPython (int room, std::string index,std::string pythonfile, float x, float y, float wid, float hei, std::string text, int to);
	void LaunchPython (int room, std::string index,std::string pythonfile, float x, float y, float wid, float hei, std::string text);
	void EjectPython (int room, std::string index,std::string pythonfile, float x, float y, float wid, float hei, std::string text);
	void CompPython(int room, std::string index,std::string pythonfile, float x, float y, float wid, float hei, std::string text, std::string modes);
	void GlobalKeyPython(std::string pythonfile);

	void Link (int room, std::string index, float x, float y, float wid, float hei, std::string text, int to);
	void Launch (int room, std::string index, float x, float y, float wid, float hei, std::string text);
	void Comp(int room, std::string index, float x, float y, float wid, float hei, std::string text, std::string modes);
	void Python(int room, std::string index, float x, float y, float wid, float hei, std::string text, std::string pythonfile,bool front=false);
	void MessageToRoom(int room, std::string text);
	void EnqueueMessageToRoom(int room, std::string text);
	void Message(std::string text);
	void EnqueueMessage(std::string text);
	void RunScript (int room, std::string ind, std::string pythonfile, float time);
	void TextBox (int room, std::string ind, std::string text, float x, float y, Vector widheimult, Vector backcol, float backalp, Vector forecol);
	void SetTextBoxText(int room, std::string ind, std::string text);
	void SetLinkArea(int room, std::string index, float x, float y, float wid, float hei);
	void SetLinkText(int room, std::string index, std::string text);
	void SetLinkPython(int room, std::string index, std::string python);
	void SetLinkRoom(int room, std::string index, int to);
	void SetLinkEventMask(int room, std::string index, std::string maskdef); // c=click, u=up, d=down, e=enter, l=leave, m=move
	void EraseLink (int room, std::string index);
	void EraseObj (int room, std::string index);
	int GetCurRoom ();
	void SetCurRoom (int room);
	int GetNumRoom ();
	bool BuyShip(std::string name, bool my_fleet, bool force_base_inventory);
	bool SellShip(std::string name);

	// GUI events
	void SetEventData(Dictionary data);
	void SetMouseEventData(std::string type, float x, float y, int buttonMask); // [type], [mousex], [mousey], [mousebuttons]
	void SetKeyEventData(std::string type, unsigned int keycode, unsigned int modmask=~0);
	void SetKeyStatusEventData(unsigned int modmask=~0);
	const Dictionary& GetEventData();

	// GUI events (engine internals)
	Dictionary& _GetEventData();

	// Auxiliary
	float GetTextHeight(std::string text, Vector widheimult);
	float GetTextWidth(std::string text, Vector widheimult);
	void LoadBaseInterface(std::string name);
	void LoadBaseInterfaceAtDock(std::string name, Unit* dockat, Unit *dockee);
	void refreshBaseComputerUI(const class Cargo *dirtyCarg);
	void ExitGame();
}
