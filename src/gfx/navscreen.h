
class NavigationSystem
{
public:
	NavigationSystem() {draw = 0; Setup();}
	~NavigationSystem() {draw = 0; delete mesh;}

void Draw();
void Setup();
void SetDraw(bool n);
bool CheckDraw();

private:
class Mesh * mesh[10];
int meshes;
bool draw;
};
