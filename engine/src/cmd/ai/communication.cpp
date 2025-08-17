/*
 * communication.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "communication.h"
#include "root_generic/vs_globals.h"
#include "src/vs_logging.h"
#include "src/config_xml.h"
#include <assert.h>
#include "src/audiolib.h"
#include "root_generic/options.h"
#include "src/universe.h"

using namespace XMLSupport;

FSM::FSM(const std::string &filename) {
    //loads a conversation finite state machine with deltaRelation weight transition from an XML?
    if (filename.empty()) {
        nodes.push_back(Node::MakeNode("welcome to cachunkcachunk.com", 0));
        nodes.push_back(Node::MakeNode("I love you!", .1));
        nodes.push_back(Node::MakeNode("J00 0wnz m3", .08));
        nodes.push_back(Node::MakeNode("You are cool!", .06));
        nodes.push_back(Node::MakeNode("You are nice!", .05));
        nodes.push_back(Node::MakeNode("Ya you're naled! NALED PAL!", -.02));
        nodes.push_back(Node::MakeNode("i 0wnz j00", -.08));
        nodes.push_back(Node::MakeNode("I hate you!", -.1));

        nodes.push_back(Node::MakeNode("Docking operation complete.", 0));
        nodes.push_back(Node::MakeNode("Please move into a green docking box and press d.", 0));
        nodes.push_back(Node::MakeNode("Docking operation begun.", 0));
        nodes.push_back(Node::MakeNode("Clearance denied.", 0));
        nodes.push_back(Node::MakeNode("Clearance granted.", 0));
        nodes.push_back(Node::MakeNode("No.", 0));
        nodes.push_back(Node::MakeNode("Yes.", 0));
        nodes.push_back(Node::MakeNode("Prepare To Be Searched. Maintain Speed and Course.", 0));
        nodes.push_back(Node::MakeNode("No contraband detected: You may proceed.", 0));
        nodes.push_back(Node::MakeNode("Contraband detected! All units close and engage!", 0));
        nodes.push_back(Node::MakeNode("Your Course is deviating! Maintain Course!", 0));
        nodes.push_back(Node::MakeNode("Request Clearance To Land.", 0));
        nodes.push_back(Node::MakeNode("*hit*", -.2));
        vector<unsigned int> edges;
        unsigned int i;
        for (i = 0; i < nodes.size() - 13; i++) {
            edges.push_back(i);
        }
        for (i = 0; i < nodes.size(); i++) {
            nodes[i].edges = edges;
        }
    } else {
        LoadXML(filename.c_str());
    }
}

int FSM::GetUnDockNode() const {
    return nodes.size() - 16;
}

int FSM::GetFailDockNode() const {
    return nodes.size() - 15;
}

int FSM::GetDockNode() const {
    return nodes.size() - 14;
}

int FSM::GetUnAbleToDockNode() const {
    return nodes.size() - 13;
}

int FSM::GetAbleToDockNode() const {
    return nodes.size() - 12;
}

int FSM::GetNoNode() const {
    return nodes.size() - 11;
}

int FSM::GetYesNode() const {
    return nodes.size() - 10;
}

int FSM::GetContrabandInitiateNode() const {
    return nodes.size() - 9;
}

int FSM::GetContrabandUnDetectedNode() const {
    return nodes.size() - 8;
}

int FSM::GetContrabandDetectedNode() const {
    return nodes.size() - 7;
}

int FSM::GetContrabandWobblyNode() const {
    return nodes.size() - 6;
}

int FSM::GetRequestLandNode() const {
    return nodes.size() - 5;
}

int FSM::GetHitNode() const {
    return nodes.size() - 1;
}

int FSM::GetDamagedNode() const {
    return nodes.size() - 2;
}

int FSM::GetDealtDamageNode() const {
    return nodes.size() - 3;
}

int FSM::GetScoreKillNode() const {
    return nodes.size() - 4;
}

static float sq(float i) {
    return i * i;
}

bool nonneg(float i) {
    return i >= 0;
}

std::string FSM::Node::GetMessage(unsigned int &multiple) const {
    multiple = rand() % messages.size();
    return messages[multiple];
}

// createSound, implemented in unit_functions.cpp / libaudioserver.cpp
// FIXME: this variability makes it hard to use proper include files
extern int createSound(std::string file, bool val);

int FSM::Node::GetSound(unsigned char sex, unsigned int multiple, float &gain) {
    unsigned int index = multiple + ((unsigned int) sex) * messages.size();
    if (index < sounds.size()) {
        gain = gains[index];
        if (sounds[index] < 0) {
            sounds[index] = createSound(soundfiles[index], false);
            AUDSoundGain(sounds[index], gains[index], false);
            return sounds[index];
        } else {
            return sounds[index];
        }
    } else {
        gain = 1.0f;
        return -1;
    }
}

bool FSM::Node::StopSound(unsigned char sex) {
    unsigned int index = ((unsigned int) sex) * messages.size();
    bool ret = false;
    for (unsigned int i = index; i < index + messages.size() && i < sounds.size(); ++i) {
        if (sounds[i] > 0 && AUDIsPlaying(sounds[i])) {
            AUDStopPlaying(sounds[i]);
            ret = true;
        }
    }
    return ret;
}

bool FSM::StopAllSounds(unsigned char sex) {
    bool ret = false;
    for (unsigned int i = 0; i < nodes.size(); ++i) {
        if (nodes[i].StopSound(sex)) {
            ret = true;
        }
    }
    return ret;
}

void FSM::Node::AddSound(std::string soundfile, unsigned char sex, float gain) {
    static std::string emptystr;

    for (int multiple = 0;; ++multiple) {
        unsigned int index = ((unsigned int) sex) * messages.size() + multiple;
        while (index >= sounds.size()) {
            sounds.push_back(-1);
            soundfiles.push_back(emptystr);
            gains.push_back(1.0f);
        }

        if (soundfiles[index].empty()) {
            soundfiles[index] = soundfile;
            gains[index] = gain;

            // Preload sound if configured to do so
            if (game_options()->comm_preload) {
                GetSound(sex, multiple, gain);
            }

            break;
        }
    }
}

int FSM::getCommMessageMood(int curstate, float mood, float randomresponse, float relationship) const {
    const FSM::Node *n = (unsigned int) curstate
            < nodes.size() ? (&nodes[curstate]) : (&nodes[getDefaultState(relationship)]);
    mood += -randomresponse + 2 * randomresponse * ((float) rand()) / RAND_MAX;

    int choice = 0;
#if 0
    float bestchoice = 4;
    bool  fitmood    = false;
    for (unsigned i = 0; i < n->edges.size(); i++) {
        float md = nodes[n->edges[i]].messagedelta;
        bool  newfitmood = nonneg( mood ) == nonneg( md );
        if ( (!fitmood) || newfitmood ) {
            float newbestchoice = sq( md-mood );
            if ( (newbestchoice <= bestchoice) || (fitmood == false && newfitmood == true) )
                if ( (newbestchoice == bestchoice && rand()%2) || newbestchoice < bestchoice ) {
                    //to make sure some variety happens
                    fitmood    = newfitmood;
                    choice     = i;
                    bestchoice = newbestchoice;
                }
        }
    }
#endif
    vector<unsigned int> g;
    vector<unsigned int> b;
    static float pos_limit = XMLSupport::parse_float(vs_config->getVariable("AI",
            "LowestPositiveCommChoice",
            "0"));
    static float neg_limit = XMLSupport::parse_float(vs_config->getVariable("AI",
            "LowestNegativeCommChoice",
            "-.00001"));
    for (unsigned int i = 0; i < n->edges.size(); i++) {
        float md = nodes[n->edges[i]].messagedelta;
        if (md >= pos_limit) {
            g.push_back(i);
        }
        if (md <= neg_limit) {
            b.push_back(i);
        }
    }
    if (g.size() != 0 && (relationship > 0 || (b.size() == 0))) {
        choice = g[(rand() % g.size())];
    } else if (b.size()) {
        choice = b[rand() % b.size()];
    }
    return choice;
}

int FSM::getDefaultState(float relationship) const {
    if (relationship < -1) {
        relationship = -1;
    }
    if (relationship > 1) {
        relationship = 1;
    }       //clamp it
    float mood = relationship;
    float randomresponse = .01;
    int curstate = 0;

    const FSM::Node *n = &nodes[curstate];
    mood += -randomresponse + 2 * randomresponse * ((float) rand()) / RAND_MAX;

    int choice = 0;
    float bestchoice = 16;
    bool fitmood = false;
    for (unsigned i = 0; i < n->edges.size(); i++) {
        float md = nodes[n->edges[i]].messagedelta;
        bool newfitmood = nonneg(mood) == nonneg(md);
        if ((!fitmood) || newfitmood) {
            float newbestchoice = sq(md - mood);
            if ((newbestchoice <= bestchoice) || (fitmood == false && newfitmood == true)) {
                if ((newbestchoice == bestchoice && rand() % 2) || newbestchoice < bestchoice) {
                    //to make sure some variety happens
                    fitmood = newfitmood;
                    choice = i;
                    bestchoice = newbestchoice;
                }
            }
        }
    }     //(0,relationship,.01)
    return nodes[0].edges[choice];
}

std::string FSM::GetEdgesString(unsigned int curstate) {
    std::string retval = "\n";
    if (nodes.size() <= curstate) {
        VS_LOG(error,
                (boost::format("Error with faction relationship due to %1$d not being in range of faction")
                        % curstate));
        return "\n1. Transmit Error\n2. Transmit Error\n3. Transmit Error\n";
    }
    for (unsigned int i = 0; i < nodes[curstate].edges.size(); i++) {
        retval += tostring((int) ((i + 1) % 10)) + "." + nodes[nodes[curstate].edges[i]].messages[0] + "\n";
    }
    if (configuration().graphics.hud.print_request_docking) {
        retval += "0. Request Docking Clearance";
    }
    return retval;
}

float FSM::getDeltaRelation(int prevstate, unsigned int current_state) const {
    if (nodes.size() <= current_state) {
        VS_LOG(error,
                (boost::format("Error with faction relationship due to %1$d not being in range of faction")
                        % current_state));
        return 0;
    }
    return nodes[current_state].messagedelta;
}

void CommunicationMessage::Init(Unit *send, Unit *recv) {
    if (send == NULL) {
        return;
    }
    fsm = FactionUtil::GetConversation(send->faction, recv->faction);
    sender.SetUnit(send);
    this->prevstate = this->curstate = fsm->getDefaultState(send->getRelation(recv));
    this->edgenum = -1;
}

float myround(float i) {
    float j = floor(i);
    if (i - j >= .5) {
        return j + 1;
    }
    return j;
}

float myroundclamp(float i) {
    float j = myround(i);
    if (j < 0) {
        j = 0;
    }
    return j;
}

void CommunicationMessage::SetAnimation(std::vector<Animation *> *ani, unsigned char sex) {
    this->sex = sex;     //for audio
    if (ani) {
        if (ani->size() > 0) {
            float mood = fsm->getDeltaRelation(this->prevstate, this->curstate);
            mood += .1;
            mood *= (ani->size()) / .2;
            unsigned int index = (unsigned int) myroundclamp(floor(mood));
            if (index >= ani->size()) {
                index = ani->size() - 1;
            }
            this->ani = (*ani)[index];
        } else {
            this->ani = NULL;
        }
    } else {
        this->ani = NULL;
    }
}

void CommunicationMessage::SetCurrentState(int msg, std::vector<Animation *> *ani, unsigned char sex) {
    curstate = msg;
    SetAnimation(ani, sex);
    assert(this->curstate >= 0);
}

CommunicationMessage::CommunicationMessage(Unit *send,
        Unit *recv,
        int messagechoice,
        std::vector<Animation *> *ani,
        unsigned char sex) {
    Init(send, recv);
    prevstate = fsm->getDefaultState(send->getRelation(recv));
    if (fsm->nodes[prevstate].edges.size()) {
        this->edgenum = messagechoice % fsm->nodes[prevstate].edges.size();
        curstate = fsm->nodes[prevstate].edges[this->edgenum];
    }
    SetAnimation(ani, sex);
    assert(this->curstate >= 0);
}

CommunicationMessage::CommunicationMessage(Unit *send,
        Unit *recv,
        int laststate,
        int thisstate,
        std::vector<Animation *> *ani,
        unsigned char sex) {
    Init(send, recv);
    prevstate = laststate;
    curstate = thisstate;
    SetAnimation(ani, sex);
    assert(this->curstate >= 0);
}

CommunicationMessage::CommunicationMessage(Unit *send, Unit *recv, std::vector<Animation *> *ani, unsigned char sex) {
    Init(send, recv);
    SetAnimation(ani, sex);
    assert(this->curstate >= 0);
}

CommunicationMessage::CommunicationMessage(Unit *send,
        Unit *recv,
        const CommunicationMessage &prevstate,
        int curstate,
        std::vector<Animation *> *ani,
        unsigned char sex) {
    Init(send, recv);
    this->prevstate = prevstate.curstate;
    if (fsm->nodes[this->prevstate].edges.size()) {
        this->edgenum = curstate % fsm->nodes[this->prevstate].edges.size();
        this->curstate = fsm->nodes[this->prevstate].edges[edgenum];
    }
    SetAnimation(ani, sex);
    assert(this->curstate >= 0);
}

char tohexdigit(int x) {
    if (x <= 9 && x >= 0) {
        return (char) (x + '0');
    } else {
        return (char) (x - 10 + 'A');
    }
}

RGBstring colToString(GFXColor col) {
    unsigned char r = (unsigned char) (col.r * 255);
    unsigned char g = (unsigned char) (col.g * 255);
    unsigned char b = (unsigned char) (col.b * 255);
    RGBstring ret;
    ret.str[0] = '#';
    ret.str[7] = '\0';
    ret.str[1] = tohexdigit(r / 16);
    ret.str[2] = tohexdigit(r % 16);
    ret.str[3] = tohexdigit(g / 16);
    ret.str[4] = tohexdigit(g % 16);
    ret.str[5] = tohexdigit(b / 16);
    ret.str[6] = tohexdigit(b % 16);
    return ret;
}

RGBstring GetRelationshipRGBstring(float rel) {
    static GFXColor col_enemy = vs_config->getColor("relation_enemy", vs_config->getColor("enemy",
            GFXColor(1.0,
                    0.0,
                    0.0,
                    1.0))); // red   - like target
    static GFXColor col_friend = vs_config->getColor("relation_friend", vs_config->getColor("friend",
            GFXColor(0.0,
                    1.0,
                    0.0,
                    1.0))); // green - like target
    static GFXColor col_neutral = vs_config->getColor("relation_neutral", vs_config->getColor("black_and_white",
            GFXColor(1.0,
                    1.0,
                    1.0,
                    1.0))); // white - NOT like target
    GFXColor col;
    if (rel == 0) {
        col = col_neutral;
    } else {
        if (rel < 0) {
            rel = -rel;
            col = col_enemy;
        } else {
            col = col_friend;
        }
        if (rel < 1.0) {
            col = colLerp(col_neutral, col, rel);
        }
    }
    return colToString(col);
}

unsigned int DoSpeech(Unit *un, Unit *player_un, const FSM::Node &node) {
    const float scale_rel_color = configuration().graphics.hud.scale_relationship_color;
    static std::string
            ownname_RGBstr = colToString(vs_config->getColor("player_name", GFXColor(0.0, 0.2, 1.0))).str; // bluish
    unsigned int dummy = 0;
    string speech = node.GetMessage(dummy);
    string myname("[Static]");
    if (un != NULL) {
        myname = un->getUnitType() == Vega_UnitType::planet ? un->name : un->getFullname();
        Flightgroup *fg = un->getFlightgroup();
        if (fg && fg->name != "base" && fg->name != "Base") {
            myname = fg->name + " " + XMLSupport::tostring(un->getFgSubnumber()) + ", " + un->getFullname();
        } else if (myname.length() == 0) {
            myname = un->name;
        }
        if (player_un != NULL) {
            if (player_un == un) {
                myname = ownname_RGBstr + myname + "#000000";
            } else {
                float rel = un->getRelation(player_un);
                myname = GetRelationshipColorString(rel) + myname + "#000000";
            }
        }
    }
    mission->msgcenter->add(myname, "all",
            GetRelationshipColorString(node.messagedelta * scale_rel_color) + speech
                    + "#000000");     //multiply by 2 so colors are easier to tell
    return dummy;
}

void LeadMe(Unit *un, string directive, string speech, bool changetarget) {
    if (un != NULL) {
        for (unsigned int i = 0; i < _Universe->numPlayers(); i++) {
            Unit *pun = _Universe->AccessCockpit(i)->GetParent();
            if (pun) {
                if (pun->getFlightgroup() == un->getFlightgroup()) {
                    DoSpeech(un, pun, FSM::Node::MakeNode(speech, .1));
                }
            }
        }
        Flightgroup *fg = un->getFlightgroup();
        if (fg) {
            if (fg->leader.GetUnit() != un) {
                if ((!_Universe->isPlayerStarship(fg->leader.GetUnit())) || _Universe->isPlayerStarship(un)) {
                    fg->leader.SetUnit(un);
                }
            }
            fg->directive = directive;
            if (changetarget) {
                fg->target.SetUnit(un->Target());
            }
            if ((directive == "")) {
                fg->target.SetUnit(NULL);
            }
        }
    }
}
