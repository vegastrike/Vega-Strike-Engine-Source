/*
 * communication.h
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_CMD_AI_COMMUNICATION_H
#define VEGA_STRIKE_ENGINE_CMD_AI_COMMUNICATION_H

#include "cmd/unit_generic.h"
#include "gfxlib_struct.h"
#include "vs_logging.h"

class FSM {
protected:
public:
    struct Node {
        vector<std::string> messages;
        vector<std::string> soundfiles; //messages.size() sound file path for each sex
        vector<int> sounds; //messages.size() sound for each sex
        vector<float> gains; //messages.size()
        float messagedelta;
        vector<unsigned int> edges;
        int GetSound(unsigned char sex, unsigned int multiple, float &gain);
        bool StopSound(unsigned char sex);
        std::string GetMessage(unsigned int &multiple) const;
        void AddSound(std::string soundfile, unsigned char sex, float gain = 1.0f);

        Node(const vector<std::string> &message, float messagedel) : messages(message), messagedelta(messagedel) {
            if (messages.size() == 0) {
                messages.push_back("<static>");
            }
        }

        static Node MakeNode(std::string message, float messagedel) {
            vector<string> tmp;
            tmp.push_back(message);
            return Node(tmp, messagedel);
        }
    };
    vector<Node> nodes;
    bool StopAllSounds(unsigned char sex);
    FSM(const std::string &filename);
    void LoadXML(const char *factionfile);
    void beginElement(const string &name, const AttributeList attributes);
    static void beginElement(void *userData, const XML_Char *name, const XML_Char **atts);
    static void endElement(void *userData, const XML_Char *name);
    std::string GetEdgesString(unsigned int curstate);
    float getDeltaRelation(int prevstate, unsigned int curstate) const;
    int getCommMessageMood(int curstate, float mood, float randomresponsefactor, float relationship) const;
    int getDefaultState(float relationship) const;
    int GetUnDockNode() const;
    int GetFailDockNode() const;
    int GetDockNode() const;
    int GetAbleToDockNode() const;
    int GetUnAbleToDockNode() const;
    int GetYesNode() const;
    int GetNoNode() const;
    int GetHitNode() const;
    int GetDamagedNode() const;
    int GetDealtDamageNode() const;
    int GetScoreKillNode() const;
    int GetRequestLandNode() const;
    int GetContrabandInitiateNode() const;
    int GetContrabandUnDetectedNode() const;
    int GetContrabandDetectedNode() const;
    int GetContrabandWobblyNode() const;
};

class CommunicationMessage {
    void Init(Unit *send, Unit *recv);
    void SetAnimation(std::vector<class Animation *> *ani, unsigned char sex);
public:
    FSM *fsm; //the finite state that this communcation stage is in
    class Animation *ani;
    unsigned char sex; //which sound should play
    int prevstate;
    int curstate;
    int edgenum; //useful for server validation, -1 = did not move via an edge.
    UnitContainer sender;
    CommunicationMessage(Unit *send, Unit *recv, std::vector<class Animation *> *ani, unsigned char sex);
    CommunicationMessage(Unit *send, Unit *recv, int curstate, std::vector<class Animation *> *ani, unsigned char sex);
    CommunicationMessage(Unit *send,
            Unit *recv,
            int prevvstate,
            int curstate,
            std::vector<class Animation *> *ani,
            unsigned char sex);
    CommunicationMessage(Unit *send, Unit *recv, const CommunicationMessage &prevsvtate, int curstate,
            std::vector<class Animation *> *ani, unsigned char sex);
    void SetCurrentState(int message, std::vector<class Animation *> *ani, unsigned char sex);

    FSM::Node *getCurrentState() const {
        if (curstate < (int) fsm->nodes.size()) {
            return &fsm->nodes[curstate];
        } else {
            int cs = fsm->getDefaultState(0);
            if (cs < (int) fsm->nodes.size()) {
                return &fsm->nodes[cs];
            }
            VS_LOG(error, "Critical error: fsm has less than 3 nodes");
            return &fsm->nodes[0];
        }
    }

    const vector<FSM::Node> &GetPossibleState() const;

    float getDeltaRelation() const {
        return fsm->getDeltaRelation(prevstate, curstate);
    }
};

struct RGBstring {
    char str[8];
};

RGBstring colToString(GFXColor col);
RGBstring GetRelationshipRGBstring(float rel);

inline std::string GetRelationshipColorString(float rel) {
    return GetRelationshipRGBstring(rel).str;
}

unsigned int DoSpeech(Unit *un, Unit *player_un, const FSM::Node &node);

#endif //VEGA_STRIKE_ENGINE_CMD_AI_COMMUNICATION_H
