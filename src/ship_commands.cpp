#include "cmd/unit_generic.h"
#include "config_xml.h"
#include "xml_support.h"
#include "vs_globals.h"
#include "universe_util.h"

static inline float fmin(float a, float b) { return (a<b)?a:b; };
static inline float fmax(float a, float b) { return (a>b)?a:b; };

class ShipCommands {
	public:
		ShipCommands() { 
			csetkps = new Functor<ShipCommands>(this, &ShipCommands::setkps);
			CommandInterpretor.addCommand(csetkps, "setspeed", ARG_1CSTR); //1 c++ string argument,
		}
        ~ShipCommands() {
            CommandInterpretor.remCommand("setspeed"); if (csetkps) delete csetkps, csetkps=NULL;
        }
		void setkps(char *in);

    private:
        Functor<ShipCommands> *csetkps;
};


static ShipCommands *ship_commands=NULL;

void ShipCommands::setkps(char *in)
{
    float kps = XMLSupport::parse_float(std::string(in));
    Unit *player = UniverseUtil::getPlayer();
    if (player) {
        static float game_speed = XMLSupport::parse_float (vs_config->getVariable("physics","game_speed","1"));
        static bool display_in_meters = XMLSupport::parse_bool (vs_config->getVariable("physics","display_in_meters","true"));
        static bool lie = XMLSupport::parse_bool (vs_config->getVariable("physics","game_speed_lying","true"));
	    if (lie) 
	        kps *= game_speed; else
            kps /= display_in_meters?1.0f:3.6f;

        player->GetComputerData().set_speed = fmin(player->GetComputerData().max_speed(),kps);
    }
}

void InitShipCommands()
{
    if (ship_commands) delete ship_commands;
    ship_commands = new ShipCommands;
}

void UninitShipCommands()
{
    if (ship_commands) delete ship_commands;
    ship_commands = NULL;
}
