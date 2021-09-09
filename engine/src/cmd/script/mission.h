/*
 * mission.h
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) Alexander Rawass <alexannika@users.sourceforge.net>
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike contributors
 * Copyright (C) 2021 Stephen G. Tuggy
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


/*
 *  xml Mission written by Alexander Rawass <alexannika@users.sourceforge.net>
 */
/// Enumerates functions for python modules
///

#ifndef _MISSION_H_
#define _MISSION_H_
#include <gnuhash.h>

#include <expat.h>
#include <string>
#include <fstream>

//#include "xml_support.h"
#include "easydom.h"

#ifndef VS_MIS_SEL
#include "vegastrike.h"
#include "vs_logging.h"
#include "vs_globals.h"
#include "msgcenter.h"
#include "cmd/container.h"
class Unit;
class Order;
class MessageCenter;
#endif

#include "star_system.h"

#include <assert.h>
// using std::string;

// using XMLSupport::AttributeList;

#define qu( x ) ("\""+x+"\"")

/* *********************************************************** */

#ifdef VS_MIS_SEL
#define missionNode easyDomNode
#endif

class varInst;

typedef std::vector< varInst* >        olist_t;
typedef vsUMap< std::string, varInst* >omap_t;
std::string varToString( varInst *s );
class Flightgroup;
#ifndef VS_MIS_SEL

/* *********************************************************** */

#define SCRIPT_PARSE 0
#define SCRIPT_RUN 1
#define SCRIPT_PARSE_DECL 2

enum parsemode_type {PARSE_FULL, PARSE_DECL};

enum callback_module_type
{
    CMT_UNKNOWN=0,
    CMT_IO, CMT_STD, CMT_STRING, CMT_OLIST, CMT_OMAP, CMT_ORDER, CMT_UNIT, CMT_BRIEFING
};

enum callback_module_std_type
{
    CMT_STD_UNKNOWN=0,
    CMT_STD_Rnd,
    CMT_STD_getGameTime,
    CMT_STD_ResetTimeCompression,
    CMT_STD_getSystemFile,     //useful when comparing _jumps_
    CMT_STD_getNumAdjacentSystems,
    CMT_STD_getAdjacentSystem,
    CMT_STD_getCurrentAIUnit,
    CMT_STD_getCurrentAIOrder,
    CMT_STD_isNull,
    CMT_STD_setNull,
    CMT_STD_equal,
    CMT_STD_Int,
    CMT_STD_Float,
    CMT_STD_setDifficulty,
    CMT_STD_getDifficulty,
    CMT_STD_playSound,
    CMT_STD_playSoundCockpit,
    CMT_STD_playAnimation,
    CMT_STD_getGalaxyProperty,
    CMT_STD_musicAddList,
    CMT_STD_musicPlaySong,
    CMT_STD_musicPlayList,
    CMT_STD_terminateMission
};
enum callback_module_briefing_type
{
    CMT_BRIEFING_UNKNOWN=0,
    CMT_BRIEFING_addShip,
    CMT_BRIEFING_removeShip,
    CMT_BRIEFING_enqueueOrder,
    CMT_BRIEFING_replaceOrder,
    CMT_BRIEFING_setShipPosition,
    CMT_BRIEFING_getShipPosition,
    CMT_BRIEFING_terminate,
    CMT_BRIEFING_setCamPosition,
    CMT_BRIEFING_setCamOrientation,
    CMT_BRIEFING_setCloak
};
enum callback_module_olist_type
{
    CMT_OLIST_UNKNOWN=0,
    CMT_OLIST_new,
    CMT_OLIST_delete,
    CMT_OLIST_push_back,
    CMT_OLIST_pop_back,
    CMT_OLIST_back,
    CMT_OLIST_at,
    CMT_OLIST_erase,
    CMT_OLIST_set,
    CMT_OLIST_toxml,
    CMT_OLIST_size
};

enum callback_module_omap_type
{
    CMT_OMAP_UNKNOWN=0,
    CMT_OMAP_new,
    CMT_OMAP_delete,
    CMT_OMAP_set,
    CMT_OMAP_get,
    CMT_OMAP_toxml,
    CMT_OMAP_size
};

enum callback_module_order_type
{
    CMT_ORDER_UNKNOWN=0,
    CMT_ORDER_newAggressiveAI,
    CMT_ORDER_newMoveTo,
    CMT_ORDER_newChangeHeading,
    CMT_ORDER_newFaceTarget,
    CMT_ORDER_newFireAt,
    CMT_ORDER_newExecuteFor,
    CMT_ORDER_newCloakFor,
    CMT_ORDER_newMatchVelocity,
    CMT_ORDER_newMatchAngularVelocity,
    CMT_ORDER_newMatchLinearVelocity,
    CMT_ORDER_newFlyToWaypoint,
    CMT_ORDER_newFlyToWaypointDefend,
    CMT_ORDER_newFlyToJumppoint,
    CMT_ORDER_newPatrol,
    CMT_ORDER_newOrderList,
    CMT_ORDER_newSuperiority,

    CMT_ORDER_enqueueOrder,
    CMT_ORDER_enqueueOrderFirst,
    CMT_ORDER_eraseOrder,
    CMT_ORDER_findOrder,
    CMT_ORDER_SteerUp,
    CMT_ORDER_SteerRight,
    CMT_ORDER_SteerRollRight,
    CMT_ORDER_SteerStop,
    CMT_ORDER_SteerAccel,
    CMT_ORDER_SteerAfterburn,
    CMT_ORDER_SteerSheltonSlide,
    CMT_ORDER_print,
    CMT_ORDER_setActionString
};

enum callback_module_string_type
{
    CMT_STRING_UNKNOWN=0,
    CMT_STRING_new,
    CMT_STRING_delete,
    CMT_STRING_print,
    CMT_STRING_equal,
    CMT_STRING_begins
};

enum callback_module_unit_type
{
    CMT_UNIT_UNKNOWN=0,
    CMT_UNIT_equal,     //checks for sameness
    CMT_UNIT_getContainer,
    CMT_UNIT_getUnitFromContainer,
    CMT_UNIT_deleteContainer,
    CMT_UNIT_getUnit,
    CMT_UNIT_getTurret,
    CMT_UNIT_getPlayer,
    CMT_UNIT_getPlayerX,
    CMT_UNIT_getRandCargo,
    CMT_UNIT_getCredits,
    CMT_UNIT_getSaveData,     //gets saved data for this unit
    CMT_UNIT_addCredits,
    CMT_UNIT_launch,
    CMT_UNIT_launchNebula,
    CMT_UNIT_launchPlanet,
    CMT_UNIT_launchJumppoint,
    CMT_UNIT_getPosition,
    CMT_UNIT_getFaction,
    CMT_UNIT_getVelocity,
    CMT_UNIT_getTarget,
    CMT_UNIT_getName,
    CMT_UNIT_setName,
    CMT_UNIT_getThreat,
    CMT_UNIT_setTarget,
    CMT_UNIT_setPosition,
    CMT_UNIT_getDistance,
    CMT_UNIT_getMinDis,
    CMT_UNIT_getAngle,
    CMT_UNIT_getAngleToPos,
    CMT_UNIT_getFShieldData,
    CMT_UNIT_getRShieldData,
    CMT_UNIT_getLShieldData,
    CMT_UNIT_getBShieldData,
    CMT_UNIT_getEnergyData,
    CMT_UNIT_getHullData,
    CMT_UNIT_getRSize,
    CMT_UNIT_isStarShip,
    CMT_UNIT_isPlanet,
    CMT_UNIT_isSun,
    CMT_UNIT_isSignificant,
    CMT_UNIT_isJumppoint,
    CMT_UNIT_getRelation,
    CMT_UNIT_Jump,
    CMT_UNIT_getOrientationP,
    CMT_UNIT_getOrder,
    CMT_UNIT_removeFromGame,
    CMT_UNIT_getFgLeader,
    CMT_UNIT_setFgLeader,
    CMT_UNIT_getFgDirective,
    CMT_UNIT_setFgDirective,
    CMT_UNIT_getFgID,
    CMT_UNIT_getFgName,
    CMT_UNIT_getFgSubnumber,
    CMT_UNIT_addCargo,
    CMT_UNIT_removeCargo,
    CMT_UNIT_upgrade,
    CMT_UNIT_incrementCargo,
    CMT_UNIT_decrementCargo,
    CMT_UNIT_scanSystem,
    CMT_UNIT_scannerNearestEnemy,
    CMT_UNIT_scannerNearestFriend,
    CMT_UNIT_scannerNearestShip,
    CMT_UNIT_scannerLeader,
    CMT_UNIT_scannerNearestEnemyDist,
    CMT_UNIT_scannerNearestFriendDist,
    CMT_UNIT_scannerNearestShipDist,
    CMT_UNIT_correctStarSystem,     //useful when comparing _jumps_
    CMT_UNIT_getFgId,
    CMT_UNIT_frameOfReference,
    CMT_UNIT_communicateTo,
    CMT_UNIT_commAnimation,
    CMT_UNIT_switchFg,
    CMT_UNIT_toxml
};

enum tag_type
{
    DTAG_UNKNOWN,
    DTAG_MISSION,
    DTAG_SETTINGS, DTAG_ORIGIN, DTAG_VARIABLES, DTAG_FLIGHTGROUPS,
    DTAG_ROT, DTAG_POS, DTAG_ORDER,
    DTAG_MODULE, DTAG_SCRIPT, DTAG_IF, DTAG_BLOCK,
    DTAG_SETVAR, DTAG_EXEC, DTAG_CALL, DTAG_WHILE,
    DTAG_AND_EXPR, DTAG_OR_EXPR, DTAG_NOT_EXPR, DTAG_TEST_EXPR,
    DTAG_FMATH, DTAG_VMATH,
    DTAG_VAR_EXPR, DTAG_DEFVAR,
    DTAG_CONST,
    DTAG_ARGUMENTS,
    DTAG_GLOBALS,
    DTAG_RETURN,
    DTAG_IMPORT
};
enum var_type {VAR_FAILURE, VAR_BOOL, VAR_FLOAT, VAR_INT, VAR_OBJECT, VAR_VOID, VAR_ANY};

enum tester_type {TEST_GT, TEST_LT, TEST_EQ, TEST_NE, TEST_GE, TEST_LE};

/* *********************************************************** */

class missionNode;

enum scope_type {VI_GLOBAL, VI_MODULE, VI_LOCAL, VI_TEMP, VI_IN_OBJECT, VI_ERROR, VI_CONST, VI_CLASSVAR};

class varInst
{
public: varInst( scope_type sctype )
    {
        scopetype  = sctype;
        objectname = std::string();
        object     = NULL;
    }
    varInst()
    {
        VS_LOG_AND_FLUSH(fatal, "varInst() obsolete");
        assert( 0 );
    }

    std::string       name;
    var_type     type;

    scope_type   scopetype;
    double       float_val;
    bool bool_val;
    int int_val;
    std::string       string_val;

    std::string       objectname;
    void *object;

    missionNode *defvar_node;
    missionNode *block_node;

    unsigned int varId;
};

/* *********************************************************** */

class varInstVec : public vector< varInst* >
{
public:
    unsigned int addVar( varInst *vi )
    {
        push_back( vi );
        int index = size()-1;
        vi->varId = index;
        return index;
    }
};
class varInstMap : public vsUMap< std::string, varInst* >
{
public:
    varInstVec varVec;
};

/* *********************************************************** */

class scriptContext
{
public:
    varInstMap  *varinsts;
    missionNode *block_node;

    scriptContext()
    {
        varinsts   = NULL;
        block_node = NULL;
    }
};

/* *********************************************************** */

class contextStack
{
public:
    std::vector< scriptContext* >contexts;
    varInst *return_value;
};

/* *********************************************************** */

class missionNode;

class NopeNadaNothingYoureLettingYourImaginationRunWild
{
public:
    int i;
};
class missionThread
{
protected:
public:
    virtual ~missionThread() {}
    std::vector< contextStack* >exec_stack;
    std::vector< missionNode* > module_stack;
    std::vector< unsigned int > classid_stack;
};

/* *********************************************************** */

class missionNode : public tagDomNode
{
public:
    struct script_t
    {
        std::string        name; //script,defvar,module
        varInstMap    variables; //script,module
        std::vector< varInstMap* >classvars;  //module
        varInst      *varinst; //defvar,const
        missionNode  *if_block[3]; //if
        missionNode  *while_arg[2]; //while
        int tester; //test
        missionNode  *test_arg[2]; //test
        enum var_type vartype; //defvar,script
        std::string        initval;
        missionNode  *context_block_node; //defvar
        vsUMap< std::string, missionNode* >scripts; //module
        missionNode  *exec_node; //exec, return
        int nr_arguments; //script
        missionNode  *argument_node; //script
        missionNode  *module_node; //exec
        unsigned int  classinst_counter;
        int context_id;
        int varId;
        callback_module_type callback_module_id;
        int method_id;
    }
    script;
};

/* *********************************************************** */

class pythonMission;
class PythonMissionBaseClass;
#endif //VS_MIS_SEL

class Mission
{
public:
    enum MISSION_AUTO {AUTO_OFF=-1, AUTO_NORMAL=0, AUTO_ON=1};
    unsigned int player_num;
    MISSION_AUTO player_autopilot;
    MISSION_AUTO global_autopilot;
    struct Objective
    {
        float  completeness;
        std::string objective;
        Unit * getOwner();
        void   setOwner( Unit *u )
        {
            Owner.SetUnit( u );
        }
        UnitContainer Owner;
        Objective()
        {
            completeness = 0;
        }
        Objective( float complete, std::string obj )
        {
            completeness = complete;
            objective    = obj;
        }
    };
    std::vector< Objective >objectives;
    void SetUnpickleData( std::string dat )
    {
        unpickleData = dat;
    }
    class Briefing*briefing;
    static double gametime;
    std::string mission_name;
    void terminateMission();
    Unit  *call_unit_launch( class CreateFlightgroup*fg, int type /*_UnitType type*/, const std::string&destinations );

    Mission( const char *configfile, bool loadscripts = true );
    Mission( const char *filename, const std::string &pythonscript, bool loadscripts = true );
    std::string Pickle(); //returns filename\npickleddata
    void UnPickle( std::string pickled ); //takes in pickeddata
    void AddFlightgroup( Flightgroup *fg );
    void initMission( bool loadscripts = true );

    int getPlayerMissionNumber(); //-1 if not found or invalid player_num.
    static Mission * getNthPlayerMission( int cp, int num );

///alex Please help me make this function...this is called between mission loops
    ~Mission();
//deletes missions pushed back onto a delete queue at a *safe time*
    static void wipeDeletedMissions();
    static int number_of_flightgroups;
    static int number_of_ships;

    static std::vector< Flightgroup* >flightgroups;

    Flightgroup * findFlightgroup( const std::string &fg_name, const std::string &faction );

    std::string getVariable( std::string name, std::string defaultval );

#ifndef VS_MIS_SEL
    void GetOrigin( QVector &pos, string &planetname );

    void DirectorLoop();
    void DirectorStart( missionNode *node );
    void DirectorStartStarSystem( StarSystem *ss );
    void DirectorInitgame();
    void DirectorEnd();
    void DirectorBenchmark();
    void DirectorShipDestroyed( Unit *unit );
    void BriefingStart();
//clobbers the cam view & renders btw 0,0, and 1,1
    class TextPlane * BriefingRender();
    void BriefingLoop();
    void BriefingUpdate();
    void BriefingEnd();
    bool BriefingInProgress();
    double getGametime();

    void loadMissionModules();
    void loadModule( std::string modulename );
    void addModule( std::string modulename );
    bool runScript( std::string modulename, const std::string &scriptname, unsigned int classid = 0 );
    bool runScript( missionNode*, const std::string &scriptname, unsigned int classid = 0 );
    void RunDirectorScript( const std::string& );
    unsigned int createClassInstance( std::string modulename );
    void setCurrentAIUnit( Unit *unit )
    {
        current_ai_unit = unit;
    }
    void setCurrentAIOrder( Order *order )
    {
        current_ai_order = order;
    }
    varInst * lookupClassVariable( std::string modulename, std::string varname, unsigned int classid );
    void destroyClassInstance( std::string modulename, unsigned int classid );

    static MessageCenter *msgcenter;

    void call_vector_into_olist( varInst *vec_vi, QVector vec3 );
    void deleteVarInst( varInst *vi, bool del_local = false );
#endif //VS_MIS_SEL

private:
//std::string getVariable(easyDomNode *section, std::string name, std::string defaultval);
    void ConstructMission( const char *configfile, const std::string &pythonscript, bool loadscripts = true );
    missionNode *top;

    easyDomNode *variables;
    easyDomNode *origin_node;

#ifndef VS_MIS_SEL
    Unit      *current_ai_unit;
    Order     *current_ai_order;

    int        debuglevel;
    bool       start_game;
    bool       do_trace;
    int        tracelevel; //unusued

    static int total_nr_frames;

//ofstream var_out;

    parsemode_type parsemode;

    missionNode   *director;
    easyDomFactory< missionNode > *importf;

    tagMap tagmap;
    char  *nextpythonmission;
    std::string unpickleData;
public:
    struct Runtime
    {
        std::vector< missionThread* >threads;
        PythonMissionBaseClass *pymissions;
        vsUMap< std::string, missionNode* >modules;
        int thread_nr;
        missionThread *cur_thread;
        vsUMap< std::string, missionNode* >global_variables;
        varInstVec global_varvec;
        //std::vector<const void *()> callbacks;
    }
    runtime;
private:
    friend void UnpickleMission( std::string pickled );
//used only for parsing
    std::vector< missionNode* >scope_stack;
    missionNode    *current_module;
    missionNode    *current_script;

    std::vector< std::string >import_stack;
    vsUMap< std::string, callback_module_string_type >  module_string_map;
    vsUMap< std::string, callback_module_omap_type >    module_omap_map;
    vsUMap< std::string, callback_module_olist_type >   module_olist_map;
    vsUMap< std::string, callback_module_order_type >   module_order_map;
    vsUMap< std::string, callback_module_unit_type >    module_unit_map;
    vsUMap< std::string, callback_module_std_type >     module_std_map;
    vsUMap< std::string, callback_module_briefing_type >module_briefing_map;
    vsUMap< std::string, callback_module_type >module_map;

    void saveVariables( const std::ostream &out );
    void initTagMap();

    void initCallbackMaps();
#endif //VS_MIS_SEL

    bool checkMission( easyDomNode *node, bool loadscripts );
    void doVariables( easyDomNode *node );
    void checkVar( easyDomNode *node );
    void doFlightgroups( easyDomNode *node );
    void doOrder( easyDomNode *node, Flightgroup *fg );
    void checkFlightgroup( easyDomNode *node );
    bool doPosition( easyDomNode*node, double pos[3], class CreateFlightgroup* );
//    bool doRotation( easyDomNode*node, float rot[3], class CreateFlightgroup* );
    void doOrigin( easyDomNode *node );
    void doSettings( easyDomNode *node );

#ifndef VS_MIS_SEL
    void doModule( missionNode *node, int mode );

    scriptContext * addContext( missionNode *node );
    void removeContext();
    void removeContextStack();
    void addContextStack( missionNode *node );

    void trace( missionNode *node, int mode );

    varInst * doScript( missionNode *node, int mode, varInstMap *varmap = NULL );
    void doBlock( missionNode *node, int mode );
    bool doBooleanVar( missionNode *node, int mode );
    varInst * lookupLocalVariable( missionNode *asknode );
    varInst * lookupModuleVariable( std::string mname, missionNode *asknode );
    varInst * lookupModuleVariable( missionNode *asknode );
    varInst * lookupClassVariable( missionNode *asknode );
    varInst * lookupGlobalVariable( missionNode *asknode );
    varInst * doVariable( missionNode *node, int mode );
    void checkStatement( missionNode *node, int mode );
    void doIf( missionNode *node, int mode );
    void doWhile( missionNode *node, int mode );
    bool checkBoolExpr( missionNode *node, int mode );
    bool doAndOr( missionNode *node, int mode );
    bool doNot( missionNode *node, int mode );
    bool doTest( missionNode *node, int mode );
    void doDefVar( missionNode *node, int mode, bool global_var = false );
    void doSetVar( missionNode *node, int mode );
    varInst * doCall( missionNode *node, int mode );
    varInst * doExec( missionNode *node, int mode );
    varInst * doConst( missionNode *node, int mode );

    int doIntVar( missionNode *node, int mode );
    int checkIntExpr( missionNode *node, int mode );
    int doIMath( missionNode *node, int mode );
    varInst * doMath( missionNode *node, int mode );
    int intMath( std::string mathname, int res1, int res2 );
    double floatMath( std::string mathname, double res1, double res2 );
    varInst * checkExpression( missionNode *node, int mode );

    void assignVariable( varInst *v1, varInst *v2 );

    scriptContext * makeContext( missionNode *node );
    bool checkVarType( varInst *var, enum var_type check_type );

    double checkFloatExpr( missionNode *node, int mode );
    double doFloatVar( missionNode *node, int mode );
    double doFMath( missionNode *node, int mode );

    void doArguments( missionNode *node, int mode, varInstMap *varmap = NULL );
    void doReturn( missionNode *node, int mode );
    void doGlobals( missionNode *node, int mode );
    void doImport( missionNode *node, int mode );

    bool have_return( int mode );
    missionNode * lookupScript( std::string scriptname, std::string modulename );

    var_type vartypeFromString( std::string type );

    varInst * doObjectVar( missionNode *node, int mode );
    varInst * checkObjectExpr( missionNode *node, int mode );

    void fatalError( missionNode *node, int mode, std::string message );
    void runtimeFatal( std::string message );
    void warning( std::string message );
    void debug( missionNode *node, int mode, std::string message );
    void debug( int level, missionNode *node, int mode, std::string message );

    void printNode( missionNode *node, int mode );

    void printRuntime();
    void printThread( missionThread *thread );
    void printVarmap( const varInstMap &vmap );
    void printVarInst( varInst *vi );
    void saveVarInst( varInst *vi, std::ostream &out );
    void printVarInst( int dbg_level, varInst *vi );
    void printGlobals( int dbg_level );
    void printModules();

    std::string modestring( int mode );

    varInst * searchScopestack( std::string name );

    varInst * callRnd( missionNode *node, int mode );
    varInst * callPrintFloats( missionNode *node, int mode );
    varInst * callGetGameTime( missionNode *node, int mode );
    varInst * callResetTimeCompression( missionNode *node, int mode );
    varInst * callGetSystemName( missionNode *node, int mode );
    varInst * callGetSystemFile( missionNode *node, int mode, StarSystem *ss = NULL );
    varInst * callGetNumAdjacentSystems( missionNode *node, int mode );
    varInst * callGetGalaxyProperty( missionNode *node, int mode );
    varInst * callGetAdjacentSystem( missionNode *node, int mode );
    varInst * call_isNull( missionNode *node, int mode );
    varInst * call_setNull( missionNode *node, int mode );
    varInst * call_isequal( missionNode *node, int mode );
    varInst * callGetCurrentAIUnit( missionNode *node, int mode );
    varInst * callGetCurrentAIOrder( missionNode *node, int mode );
    varInst * call_musicAddList( missionNode *node, int mode );
    varInst * call_musicPlaySong( missionNode *node, int mode );
    varInst * call_musicPlayList( missionNode *node, int mode );

    varInst * call_int_cast( missionNode *node, int mode );
    varInst * call_float_cast( missionNode *node, int mode );

    varInst * call_io_sprintf( missionNode *node, int mode );
    varInst * call_io_printf( missionNode *node, int mode );
    std::string replaceNewline( std::string origstr );

    varInst * call_io_message( missionNode *node, int mode );
    varInst * call_io_printmsglist( missionNode *node, int mode );

    std::string method_str( missionNode *node );
    missionNode * getArgument( missionNode *node, int mode, int arg_nr );

    varInst * call_olist( missionNode *node, int mode );
    olist_t * getOListObject( missionNode *node, int mode, varInst *ovi );
    varInst * call_olist_new( missionNode *node, int mode );
    void call_olist_pop_back( missionNode *node, int mode, varInst *ovi );
    void call_olist_push_back( missionNode *node, int mode, varInst *ovi, varInst *push );
    varInst * call_olist_at( missionNode *node, int mode, varInst *ovi, int index );
    varInst * call_olist_back( missionNode *node, int mode, varInst *ovi );
    void call_olist_toxml( missionNode *node, int mode, varInst *ovi );
    QVector call_olist_tovector( missionNode *node, int mode, varInst *ovi );
    void call_olist_set( missionNode *node, int mode, varInst *ovi, int index, varInst *new_vi );

    varInst * call_omap( missionNode *node, int mode );
    omap_t * getOMapObject( missionNode *node, int mode, varInst *ovi );
    varInst * call_omap_new( missionNode *node, int mode );

    varInst * getObjectArg( missionNode *node, int mode );

    varInst * call_unit( missionNode *node, int mode );
    varInst * call_briefing( missionNode *node, int mode );
    Unit * getUnitObject( missionNode *node, int mode, varInst *ovi );

//void call_unit_launch(missionNode *node, int mode, std::string name, std::string faction, std::string type, std::string ainame, int nr_ships, Vector &pos);

    void call_unit_toxml( missionNode *node, int mode, varInst *ovi );

    varInst * call_string( missionNode *node, int mode );
    void call_string_print( missionNode *node, int mode, varInst *ovi );
    varInst * call_string_new( missionNode *node, int mode, std::string initstring );
    std::string * getStringObject( missionNode *node, int mode, varInst *ovi );
    std::string call_string_getstring( missionNode *node, int mode, varInst *ovi );
    std::string getStringArgument( missionNode *node, int mode, int arg_nr );

    void findNextEnemyTarget( Unit *my_unit );

    varInst * doCall( missionNode *node, int mode, std::string module, std::string method );
    void doCall_toxml( std::string module, varInst *ovi );

    varInst * newVarInst( scope_type scopetype );
    int vi_counter, old_vi_counter;
    int olist_counter, old_olist_counter;
    int string_counter, old_string_counter;

    void deleteVarMap( varInstMap *vmap );
//pushes this mission onto a destruct queue for future destruction and removes from activeMission
    varInst * call_terminateMission( missionNode *node, int mode );

    varInst * call_order( missionNode *node, int mode );
    Order * getOrderObject( missionNode *node, int mode, varInst *ovi );

    QVector getVec3Arg( missionNode *node, int mode, int arg_nr );
    double getFloatArg( missionNode *node, int mode, int arg_nr );
    int getIntArg( missionNode *node, int mode, int arg_nr );
    bool getBoolArg( missionNode *node, int mode, int arg_nr );
    Unit * getUnitArg( missionNode *node, int mode, int arg_nr );
#endif //VS_MIS_SEL
};

#endif //_MISSION_H_

