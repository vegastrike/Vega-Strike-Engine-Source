#include <assert.h>
#include <boost/version.hpp>
#include <expat.h>

#include "star_system_generic.h"
#include "gfx/vec.h"
#include "cmd/planet_generic.h"
#include "cmd/unit_generic.h"
#include "cmd/unit_collide.h"
#include "cmd/collection.h"
#include "gfx/cockpit_generic.h"
#include "audiolib.h"
#include "lin_time.h"
#include "cmd/beam.h"
#include "cmd/bolt.h"
#include "cmd/music.h"
#include "configxml.h"
#include "vs_globals.h"
#include "vegastrike.h"
#include "universe_generic.h"
#include "cmd/nebula_generic.h"
#include "galaxy_gen.h"
#include "cmd/script/mission.h"
#include "in_kb.h"
#include "cmd/script/flightgroup.h"
#include "load_mission.h"
#include "cmd/unit_util.h"
#include "cmd/unit_factory.h"
#include "cmd/unit_collide.h"
#include "vs_random.h"
#include "savegame.h"
#include "networking/netclient.h"
#include "in_kb_data.h"
#include "universe_util.h"               //get galaxy faction, dude
#include "options.h"

#if defined (_MSC_VER) && _MSC_VER <= 1200

#define Vector Vactor
#endif

#include "cs_boostpython.h"

#if defined (_MSC_VER) && _MSC_VER <= 1200

#undef Vector
#endif



using namespace XMLSupport;
vector< Vector >perplines;
extern std::vector< unorigdest* >pendingjump;

void TentativeJumpTo( StarSystem *ss, Unit *un, Unit *jumppoint, const std::string &system )
{
    for (unsigned int i = 0; i < pendingjump.size(); ++i)
        if (pendingjump[i]->un.GetUnit() == un)
            return;
    ss->JumpTo( un, jumppoint, system );
}

float ScaleJumpRadius( float radius )
{
    //need this because sys scale doesn't affect j-point size
    radius *= game_options.jump_radius_scale*game_options.game_speed;
    return radius;
}

StarSystem::StarSystem()
{
    stars = NULL;
    bolts = NULL;
    collidetable = NULL;
    collidemap[Unit::UNIT_ONLY] = new CollideMap( Unit::UNIT_ONLY );
    collidemap[Unit::UNIT_BOLT] = new CollideMap( Unit::UNIT_BOLT );

    no_collision_time = 0;               //(int)(1+2.000/SIMULATION_ATOM);
    ///adds to jumping table;
    name = NULL;
    current_stage = MISSION_SIMULATION;
    time    = 0;
    zone    = 0;
    sigIter = drawList.createIterator();
    this->current_sim_location = 0;
}

StarSystem::StarSystem( const char *filename, const Vector &centr, const float timeofyear )
{
    no_collision_time = 0;               //(int)(1+2.000/SIMULATION_ATOM);
    collidemap[Unit::UNIT_ONLY] = new CollideMap( Unit::UNIT_ONLY );
    collidemap[Unit::UNIT_BOLT] = new CollideMap( Unit::UNIT_BOLT );

    this->current_sim_location  = 0;
    ///adds to jumping table;
    name  = NULL;
    zone  = 0;
    _Universe->pushActiveStarSystem( this );
    bolts = new bolt_draw;
    collidetable   = new CollideTable( this );
    current_stage  = MISSION_SIMULATION;
    this->filename = filename;
    LoadXML( filename, centr, timeofyear );
    if (!name)
        name = strdup( filename );
    sigIter = drawList.createIterator();
    AddStarsystemToUniverse( filename );

    time = 0;
    _Universe->popActiveStarSystem();
}

extern void ClientServerSetLightContext( int lightcontext );
StarSystem::~StarSystem()
{
    if ( _Universe->getNumActiveStarSystem() )
        _Universe->activeStarSystem()->SwapOut();
    _Universe->pushActiveStarSystem( this );
    ClientServerSetLightContext( lightcontext );
    delete[] name;
    for (un_iter iter = drawList.createIterator();!iter.isDone(); ++iter)
        (*iter)->Kill( false );
    //if the next line goes ANYWHERE else Vega Strike will CRASH!!!!!
    //DO NOT MOVE THIS LINE! IT MUST STAY
    if (collidetable) delete collidetable;
    _Universe->popActiveStarSystem();
    vector< StarSystem* >activ;
    while ( _Universe->getNumActiveStarSystem() ) {
        if (_Universe->activeStarSystem() != this)
            activ.push_back( _Universe->activeStarSystem() );
        else
            fprintf( stderr, "Avoided fatal error in deleting star system %s\n", getFileName().c_str() );
        _Universe->popActiveStarSystem();
    }
    while ( activ.size() ) {
        _Universe->pushActiveStarSystem( activ.back() );
        activ.pop_back();
    }
    if ( _Universe->getNumActiveStarSystem() )
        _Universe->activeStarSystem()->SwapIn();
    RemoveStarsystemFromUniverse();
    delete collidemap[Unit::UNIT_ONLY];
    delete collidemap[Unit::UNIT_BOLT];
}

/********* FROM STAR SYSTEM XML *********/
void setStaticFlightgroup( vector< Flightgroup* > &fg, const std::string &nam, int faction )
{
    while ( faction >= (int) fg.size() ) {
        fg.push_back( new Flightgroup() );
        fg.back()->nr_ships = 0;
    }
    if (fg[faction]->nr_ships == 0) {
        fg[faction]->flightgroup_nr = faction;
        fg[faction]->pos.i    = fg[faction]->pos.j = fg[faction]->pos.k = 0;
        fg[faction]->nr_ships = 0;
        fg[faction]->ainame   = "default";
        fg[faction]->faction  = FactionUtil::GetFaction( faction );
        fg[faction]->type     = "Base";
        fg[faction]->nr_waves_left = 0;
        fg[faction]->nr_ships_left = 0;
        fg[faction]->name     = nam;
    }
    ++fg[faction]->nr_ships;
    ++fg[faction]->nr_ships_left;
}

Flightgroup * getStaticBaseFlightgroup( int faction )
{
    //warning mem leak...not big O(num factions)
    static vector< Flightgroup* >fg;
    setStaticFlightgroup( fg, "Base", faction );
    return fg[faction];
}

Flightgroup * getStaticStarFlightgroup( int faction )
{
    //warning mem leak...not big O(num factions)
    static vector< Flightgroup* >fg;
    setStaticFlightgroup( fg, "Base", faction );
    return fg[faction];
}

Flightgroup * getStaticNebulaFlightgroup( int faction )
{
    static vector< Flightgroup* >fg;
    setStaticFlightgroup( fg, "Nebula", faction );
    return fg[faction];
}

Flightgroup * getStaticAsteroidFlightgroup( int faction )
{
    static vector< Flightgroup* >fg;
    setStaticFlightgroup( fg, "Asteroid", faction );
    return fg[faction];
}

Flightgroup * getStaticUnknownFlightgroup( int faction )
{
    static vector< Flightgroup* >fg;
    setStaticFlightgroup( fg, "Unknown", faction );
    return fg[faction];
}

void StarSystem::beginElement( void *userData, const XML_Char *name, const XML_Char **atts )
{
    ( (StarSystem*) userData )->beginElement( name, AttributeList( atts ) );
}

void StarSystem::endElement( void *userData, const XML_Char *name )
{
    ( (StarSystem*) userData )->endElement( name );
}

extern string RemoveDotSystem( const char *input );
string StarSystem::getFileName() const
{
    return getStarSystemSector( filename )+string( "/" )+RemoveDotSystem( getStarSystemName( filename ).c_str() );
}

string StarSystem::getName()
{
    return string( name );
}

void StarSystem::AddUnit( Unit *unit )
{
    if ( stats.system_faction == FactionUtil::GetNeutralFaction() )
        stats.CheckVitals( this );
    if ( unit->isPlanet() || unit->isJumppoint() || unit->isUnit() == ASTEROIDPTR) {
        if (!gravitationalUnits().contains(unit))
            gravitationalUnits().prepend( unit );
    }
    drawList.prepend( unit );
    unit->activeStarSystem = this;     //otherwise set at next physics frame...
    UnitFactory::broadcastUnit( unit, GetZone() );
    unsigned int priority = UnitUtil::getPhysicsPriority( unit );
    //Do we need the +1 here or not - need to look at when current_sim_location is changed relative to this function
    //and relative to this function, when the bucket is processed...
    unsigned int tmp = 1+( (unsigned int) vsrandom.genrand_int32() )%priority;
    this->physics_buffer[(this->current_sim_location+tmp)%SIM_QUEUE_SIZE].prepend( unit );
    stats.AddUnit( unit );
}

bool StarSystem::RemoveUnit( Unit *un )
{
    for (unsigned int locind = 0; locind < Unit::NUM_COLLIDE_MAPS; ++locind)
        if ( !is_null( un->location[locind] ) ) {
            collidemap[locind]->erase( un->location[locind] );
            set_null( un->location[locind] );
        }

    if (drawList.remove(un) ) {
        // regardless of being drawn, it should be in physics list
        for (unsigned int i = 0; i <= SIM_QUEUE_SIZE; ++i) {    
            if(physics_buffer[i].remove(un)){
                i = SIM_QUEUE_SIZE +1;
            }
        }
        stats.RemoveUnit( un );
        return(true);
    }
    return (false);
}

void StarSystem::ExecuteUnitAI()
{
    try {
        Unit *unit = NULL;
        for (un_iter iter = getUnitList().createIterator(); (unit = *iter); ++iter) {
            unit->ExecuteAI();
            unit->ResetThreatLevel();
        }
    }
    catch (const boost::python::error_already_set) {
        if ( PyErr_Occurred() ) {
            PyErr_Print();
            PyErr_Clear();
            fflush( stderr );
            fflush( stdout );
        } throw;
    }
}

extern Unit *TheTopLevelUnit;
//sorry boyz...I'm just a tourist with a frag nav console--could you tell me where I am?
Unit * getTopLevelOwner() 
{
    return (TheTopLevelUnit) ;  // Now we return a pointer to a new game unit created in main(), outside of any lists 
}

void CarSimUpdate( Unit *un, float height )
{
    un->SetVelocity( Vector( un->GetVelocity().i, 0, un->GetVelocity().k ) );
    un->curr_physical_state.position = QVector( un->curr_physical_state.position.i,
                                                height,
                                                un->curr_physical_state.position.k );
}

StarSystem::Statistics::Statistics()
{
    system_faction   = FactionUtil::GetNeutralFaction();
    newfriendlycount = 0;
    newenemycount    = 0;
    newcitizencount  = 0;
    newneutralcount  = 0;
    friendlycount    = 0;
    enemycount = 0;
    neutralcount     = 0;
    citizencount     = 0;
    checkIter = 0;
    navCheckIter     = 0;
}

void StarSystem::Statistics::CheckVitals( StarSystem *ss )
{
    int faction = FactionUtil::GetFactionIndex( UniverseUtil::GetGalaxyFaction( ss->getFileName() ) );
    if (faction != system_faction) {
        *this = Statistics();                    //invoke copy constructor to clear it
        this->system_faction = faction;
        for (un_iter ui = ss->getUnitList().createIterator();!ui.isDone();++ui){
            this->AddUnit( *ui );                 //siege will take some time
        }
        return;                                          //no need to check vitals now, they're all set
    }
    size_t iter = navCheckIter;
    int    k    = 0;
    if ( iter >= navs[0].size() ) {
        iter -= navs[0].size();
        k     = 1;
    }
    if ( iter >= navs[1].size() ) {
        iter -= navs[1].size();
        k     = 2;
    }
    size_t totalnavchecking = 25;
    size_t totalsyschecking = 25;
    while ( iter < totalnavchecking && iter < navs[k].size() ) {
        if (navs[k][iter].GetUnit() == NULL) {
            navs[k].erase( navs[k].begin()+iter );
            break;
        } else {
            ++iter;
            ++navCheckIter;
        }
    }
    if ( k == 2 && iter >= navs[k].size() )
        navCheckIter = 0;                        //start over next time
    size_t sortedsize = ss->collidemap[Unit::UNIT_ONLY]->sorted.size();
    int    sysfac     = system_faction;
    size_t counter    = checkIter+totalsyschecking;
    for (; checkIter < counter && checkIter < sortedsize; ++checkIter) {
        Collidable *collide = &ss->collidemap[Unit::UNIT_ONLY]->sorted[checkIter];
        if (collide->radius > 0) {
            Unit *un  = collide->ref.unit;
            float rel = UnitUtil::getRelationFromFaction( un, sysfac );
            if ( FactionUtil::isCitizenInt( un->faction ) ) {
                ++newcitizencount;
            } else {
                if (rel > 0.05)
                    ++newfriendlycount;
                else if (rel < 0.)
                    ++newenemycount;
                else
                    ++newneutralcount;
            }
        }
    }
    if (checkIter >= sortedsize && sortedsize
        > (unsigned int) (enemycount+neutralcount+friendlycount
                          +citizencount)/4 /*suppose at least 1/4 survive a given frame*/) {
        citizencount     = newcitizencount;
        newcitizencount  = 0;
        enemycount = newenemycount;
        newenemycount    = 0;
        neutralcount     = newneutralcount;
        newneutralcount  = 0;
        friendlycount    = newfriendlycount;
        newfriendlycount = 0;
        checkIter = 0;                           //start over with list
    }
}

void StarSystem::Statistics::AddUnit( Unit *un )
{
    float rel = UnitUtil::getRelationFromFaction( un, system_faction );
    if ( FactionUtil::isCitizenInt( un->faction ) ) {
        ++citizencount;
    } else {
        if (rel > 0.05)
            ++friendlycount;
        else if (rel < 0.)
            ++enemycount;
        else
            ++neutralcount;
    }
    if ( un->GetDestinations().size() )
        jumpPoints[un->GetDestinations()[0]].SetUnit( un );
    if ( UnitUtil::isSignificant( un ) ) {
        int k = 0;
        if (rel > 0) k = 1;                      //base
        if ( un->isPlanet() && !un->isJumppoint() )
            k = 1;                                       //friendly planet
        //asteroid field/debris field
        if ( UnitUtil::isAsteroid( un ) ) k = 2;
        navs[k].push_back( UnitContainer( un ) );
    }
}

void StarSystem::Statistics::RemoveUnit( Unit *un )
{
    float rel = UnitUtil::getRelationFromFaction( un, system_faction );
    if ( FactionUtil::isCitizenInt( un->faction ) ) {
        --citizencount;
    } else {
        if (rel > 0.05)
            --friendlycount;
        else if (rel < 0.)
            --enemycount;
        else
            --neutralcount;
    }
    if ( un->GetDestinations().size() ) {
        //make sure it is there
        jumpPoints[(un->GetDestinations()[0])].SetUnit( NULL );
        //kill it--stupid I know--but hardly time critical
        jumpPoints.erase( jumpPoints.find( un->GetDestinations()[0] ) );
    }
    bool temp_erased = false;
    if ( UnitUtil::isSignificant( un ) ) {
        for (int k = 0; k < 3 && !temp_erased; ++k)
            for (size_t i = 0; i < navs[k].size();++i) {
                if (navs[k][i].GetUnit() == un){
                    //slow but who cares
                    navs[k].erase( navs[k].begin()+i );
                    temp_erased = true;
                    break; // would we exist in this array more than once?
                }
            }
    }
}

//Variables for debugging purposes only - eliminate later
unsigned int physicsframecounter = 1;
unsigned int theunitcounter = 0;
unsigned int totalprocessed = 0;
unsigned int movingavgarray[128] = {0};
unsigned int movingtotal = 0;
double aggfire      = 0;
int    numprocessed = 0;
double targetpick   = 0;

void StarSystem::RequestPhysics( Unit *un, unsigned int queue )
{
    Unit   *unit = NULL;
    un_iter iter = this->physics_buffer[queue].createIterator();
    while ( (unit = *iter) && *iter != un )
        ++iter;
    if (unit == un) {
        un->predicted_priority = 0;
        unsigned int newloc = (current_sim_location+1)%SIM_QUEUE_SIZE;
        if (newloc != queue)
            iter.moveBefore( this->physics_buffer[newloc] );
    }
}

void StarSystem::UpdateUnitPhysics( bool firstframe )
{
    static bool phytoggle  = true;
    static int  batchcount = SIM_QUEUE_SIZE-1;
    double aitime = 0;
    double phytime = 0;
    double collidetime     = 0;
    double bolttime = 0;
    targetpick   = 0;
    aggfire      = 0;
    numprocessed = 0;
    stats.CheckVitals( this );
    if (phytoggle) {
        for (++batchcount; batchcount > 0; --batchcount) {
            //BELOW COMMENTS ARE NO LONGER IN SYNCH
            //NOTE: Randomization is necessary to preserve scattering - otherwise, whenever a
            //unit goes from low-priority to high-priority and back to low-priority, they
            //get synchronized and start producing peaks.
            //NOTE2: But... randomization must come only on priority changes. Otherwise, it may
            //interfere with subunit scheduling. Luckily, all units that make use of subunit
            //scheduling also require a constant base priority, since otherwise priority changes
            //will wreak havoc with subunit interpolation. Luckily again, we only need
            //randomization on priority changes, so we're fine.
            try {
                Unit *unit = NULL;
                for (un_iter iter = physics_buffer[current_sim_location].createIterator(); (unit = *iter); ++iter) {
                    int priority  = UnitUtil::getPhysicsPriority( unit );
                    //Doing spreading here and only on priority changes, so as to make AI easier
                    int predprior = unit->predicted_priority;
                    //If the priority has really changed (not an initial scattering, because prediction doesn't match)
                    if (priority != predprior) {
                        if (predprior == 0)
                            //Validate snapshot of current interpolated state (this is a reschedule)
                            unit->curr_physical_state = unit->cumulative_transformation;
                        //Save priority value as prediction for next scheduling, but don't overwrite yet.
                        predprior = priority;
                        //Scatter, so as to achieve uniform distribution
                        priority  = 1+( ( (unsigned int) vsrandom.genrand_int32() )%priority );
                    }
                    float backup = SIMULATION_ATOM;
                    theunitcounter   = theunitcounter+1;
                    SIMULATION_ATOM *= priority;
                    unit->sim_atom_multiplier = priority;
                    double aa = queryTime();
                    unit->ExecuteAI();
                    double bb = queryTime();
                    unit->ResetThreatLevel();
                    //FIXME "firstframe"-- assume no more than 2 physics updates per frame.
                    unit->UpdatePhysics( identity_transformation, identity_matrix, Vector( 0,
                                                                                           0,
                                                                                           0 ), priority
                                         == 1 ? firstframe : true, &this->gravitationalUnits(), unit );
                    double cc = queryTime();
                    aitime  += bb-aa;
                    phytime += cc-bb;
                    SIMULATION_ATOM = backup;
                    unit->predicted_priority = predprior;
                }
            }
            catch (const boost::python::error_already_set) {
                if ( PyErr_Occurred() ) {
                    PyErr_Print();
                    PyErr_Clear();
                    fflush( stderr );
                    fflush( stdout );
                } throw;
            }
            double c0  = queryTime();
            Bolt::UpdatePhysics( this );
            double cc  = queryTime();
            last_collisions.clear();
            collidemap[Unit::UNIT_BOLT]->flatten();
            if (Unit::NUM_COLLIDE_MAPS > 1)
                collidemap[Unit::UNIT_ONLY]->flatten( *collidemap[Unit::UNIT_BOLT] );
            Unit *unit;
            for (un_iter iter = physics_buffer[current_sim_location].createIterator(); (unit = *iter);) {
                int   priority = unit->sim_atom_multiplier;
                float backup   = SIMULATION_ATOM;
                SIMULATION_ATOM *= priority;
                unsigned int newloc = (current_sim_location+priority)%SIM_QUEUE_SIZE;
                unit->CollideAll();
                SIMULATION_ATOM = backup;
                if (newloc == current_sim_location)
                    ++iter;
                else
                    iter.moveBefore( physics_buffer[newloc] );
            }
            double dd = queryTime();
            collidetime += dd-cc;
            bolttime    += cc-c0;
            current_sim_location = (current_sim_location+1)%SIM_QUEUE_SIZE;
            ++physicsframecounter;
            totalprocessed += theunitcounter;
            theunitcounter  = 0;
        }
    } else {
        Unit *unit = NULL;
        for (un_iter iter = getUnitList().createIterator(); (unit = *iter); ++iter) {
            unit->ExecuteAI();
            last_collisions.clear();
            unit->UpdatePhysics( identity_transformation, identity_matrix, Vector( 0,
                                                                                   0,
                                                                                   0 ), firstframe,
                                 &this->gravitationalUnits(), unit );
            unit->CollideAll();
        }
    }
}

extern void TerrainCollide();
extern void UpdateAnimatedTexture();
extern void UpdateCameraSnds();

extern float getTimeCompression();

//server
void ExecuteDirector()
{
    unsigned int curcockpit = _Universe->CurrentCockpit();
    {
        for (unsigned int i = 0; i < active_missions.size(); ++i)
            if (active_missions[i]) {
                _Universe->SetActiveCockpit( active_missions[i]->player_num );
                StarSystem *ss = _Universe->AccessCockpit()->activeStarSystem;
                if (ss) _Universe->pushActiveStarSystem( ss );
                mission = active_missions[i];
                active_missions[i]->DirectorLoop();
                if (ss) _Universe->popActiveStarSystem();
            }
    }
    _Universe->SetActiveCockpit( curcockpit );
    mission = active_missions[0];
    processDelayedMissions();
    {
        for (unsigned int i = 1; i < active_missions.size();) {
            if (active_missions[i]) {
                if (active_missions[i]->runtime.pymissions) {
                    ++i;
                } else {
                    unsigned int w = active_missions.size();
                    active_missions[i]->terminateMission();
                    if ( w == active_missions.size() ) {
                        printf( "MISSION NOT ERASED\n" );
                        break;
                    }
                }
            } else {
                active_missions.Get()->erase( active_missions.Get()->begin()+i );
            }
        }
    }
}

Unit* StarSystem::nextSignificantUnit()
{
    return(*sigIter);
}

void StarSystem::Update( float priority )
{
    Unit *unit;
    bool  firstframe = true;
    //No time compression here
    float normal_simulation_atom = SIMULATION_ATOM;
    time += GetElapsedTime();
    _Universe->pushActiveStarSystem( this );
    if ( time > SIMULATION_ATOM*2 ) {
        while ( time > SIMULATION_ATOM ) {
            //Chew up all SIMULATION_ATOMs that have elapsed since last update
            ExecuteDirector();
            TerrainCollide();
            Unit::ProcessDeleteQueue();
            current_stage = MISSION_SIMULATION;
            collidetable->Update();
            for (un_iter iter = drawList.createIterator(); (unit = *iter); ++iter)
                unit->SetNebula( NULL );
            UpdateMissiles();                    //do explosions
            UpdateUnitPhysics( firstframe );

            firstframe = false;
        }
        time -= SIMULATION_ATOM;
    }
    SIMULATION_ATOM = normal_simulation_atom;
    _Universe->popActiveStarSystem();
}

//client
void StarSystem::Update( float priority, bool executeDirector )
{
    bool   firstframe = true;
    ///this makes it so systems without players may be simulated less accurately
    for (unsigned int k = 0; k < _Universe->numPlayers(); ++k)
        if (_Universe->AccessCockpit( k )->activeStarSystem == this)
            priority = 1;
    float normal_simulation_atom = SIMULATION_ATOM;
    SIMULATION_ATOM /= ( priority/getTimeCompression() );
    ///just be sure to restore this at the end
    time += GetElapsedTime();
    _Universe->pushActiveStarSystem( this );
    double bolttime = 0;
      if (time > SIMULATION_ATOM ){
        //Chew up all SIMULATION_ATOMs that have elapsed since last update
          while ( time > SIMULATION_ATOM ) {
            if (current_stage == MISSION_SIMULATION) {
                TerrainCollide();
                UpdateAnimatedTexture();
                Unit::ProcessDeleteQueue();
                if ( (run_only_player_starsystem
                      && _Universe->getActiveStarSystem( 0 ) == this) || !run_only_player_starsystem )
                    if (executeDirector)
                        ExecuteDirector();
                static int dothis = 0;
                if ( this == _Universe->getActiveStarSystem( 0 ) )
                    if ( (++dothis)%2 == 0 )
                        AUDRefreshSounds();
                for (unsigned int i = 0; i < active_missions.size(); ++i)
                    //waste of frakkin time
                    active_missions[i]->BriefingUpdate();
                current_stage = PROCESS_UNIT;
            } else if (current_stage == PROCESS_UNIT) {
                UpdateUnitPhysics( firstframe );
                UpdateMissiles(); //do explosions
                collidetable->Update();
                if ( this == _Universe->getActiveStarSystem( 0 ) )
                    UpdateCameraSnds();
                bolttime      = queryTime();
                bolttime      = queryTime()-bolttime;
                current_stage = MISSION_SIMULATION;
                firstframe    = false;
            }
            time -= SIMULATION_ATOM;
        }
        unsigned int i = _Universe->CurrentCockpit();
        for (unsigned int j = 0; j < _Universe->numPlayers(); ++j)
            if (_Universe->AccessCockpit( j )->activeStarSystem == this) {
                _Universe->SetActiveCockpit( j );
                _Universe->AccessCockpit( j )->updateAttackers();
                if ( _Universe->AccessCockpit( j )->Update() ) {
                    SIMULATION_ATOM = normal_simulation_atom;
                    _Universe->SetActiveCockpit( i );
                    _Universe->popActiveStarSystem();
                    return;
                }
            }
        _Universe->SetActiveCockpit( i );
    }
    if ( sigIter.isDone() )
        sigIter = drawList.createIterator();
    else
        ++sigIter;
    while ( !sigIter.isDone() && !UnitUtil::isSignificant( *sigIter) )
        ++sigIter;
    //If it is done, leave it NULL for this frame then.
    //WARNING cockpit does not get here...
    SIMULATION_ATOM = normal_simulation_atom;
    //WARNING cockpit does not get here...
    _Universe->popActiveStarSystem();
}

/*
 **************************************************************************************
 *** STAR SYSTEM JUMP STUFF                                                          **
 **************************************************************************************
 */

Hashtable< std::string, StarSystem, 127 >star_system_table;

void StarSystem::AddStarsystemToUniverse( const string &mname )
{
    star_system_table.Put( mname, this );
}

void StarSystem::RemoveStarsystemFromUniverse()
{
    if ( star_system_table.Get( filename ) )
        star_system_table.Delete( filename );
}

StarSystem * GetLoadedStarSystem( const char *system )
{
    StarSystem *ss = star_system_table.Get( string( system ) );
    std::string ssys( string( system )+string( ".system" ) );
    if (!ss)
        ss = star_system_table.Get( ssys );
    return ss;
}

std::vector< unorigdest* >pendingjump;

bool PendingJumpsEmpty()
{
    return pendingjump.empty();
}

extern void SetShieldZero( Unit* );

void StarSystem::ProcessPendingJumps()
{
    for (unsigned int kk = 0; kk < pendingjump.size(); ++kk) {
        Unit *un = pendingjump[kk]->un.GetUnit();
        if (pendingjump[kk]->delay >= 0) {
            Unit *jp = pendingjump[kk]->jumppoint.GetUnit();
            if (un && jp) {
                QVector delta = ( jp->LocalPosition()-un->LocalPosition() );
                float   dist  = delta.Magnitude();
                if (pendingjump[kk]->delay > 0) {
                    float speed  = dist/pendingjump[kk]->delay;
                    bool  player = (_Universe->isPlayerStarship( un ) != NULL);
                    if (dist > 10 && player) {
                        if (un->activeStarSystem == pendingjump[kk]->orig)
                            un->SetCurPosition( un->LocalPosition()+SIMULATION_ATOM*delta*(speed/dist) );
                    } else if (!player) {
                        un->SetVelocity( Vector( 0, 0, 0 ) );
                    }
                    if (game_options.jump_disables_shields)
                        SetShieldZero( un );
                }
            }
            double time = GetElapsedTime();
            if (time > 1)
                time = 1;
            pendingjump[kk]->delay -= time;
            continue;
        } else {
#ifdef JUMP_DEBUG
            VSFileSystem::vs_fprintf( stderr, "Volitalizing pending jump animation.\n" );
#endif
            _Universe->activeStarSystem()->VolitalizeJumpAnimation( pendingjump[kk]->animation );
        }
        int playernum = _Universe->whichPlayerStarship( un );
        //In non-networking mode or in networking mode or a netplayer wants to jump and is ready or a non-player jump
        if ( Network == NULL || playernum < 0 || ( Network != NULL && playernum >= 0 && Network[playernum].readyToJump() ) ) {
            Unit *un = pendingjump[kk]->un.GetUnit();
            StarSystem *savedStarSystem = _Universe->activeStarSystem();
            //Download client descriptions of the new zone (has to be blocking)
            if (Network != NULL)
                Network[playernum].downloadZoneInfo();
            if ( un == NULL || !_Universe->StillExists( pendingjump[kk]->dest )
                || !_Universe->StillExists( pendingjump[kk]->orig ) ) {
#ifdef JUMP_DEBUG
                VSFileSystem::vs_fprintf( stderr, "Adez Mon! Unit destroyed during jump!\n" );
#endif
                delete pendingjump[kk];
                pendingjump.erase( pendingjump.begin()+kk );
                --kk;
                continue;
            }
            bool dosightandsound = ( (pendingjump[kk]->dest == savedStarSystem) || _Universe->isPlayerStarship( un ) );
            _Universe->setActiveStarSystem( pendingjump[kk]->orig );
            if ( un->TransferUnitToSystem( kk, savedStarSystem, dosightandsound ) )
                un->DecreaseWarpEnergy( false, 1.0f );
            if (dosightandsound)
                _Universe->activeStarSystem()->DoJumpingComeSightAndSound( un );
	    _Universe->AccessCockpit()->OnJumpEnd(un);
            delete pendingjump[kk];
            pendingjump.erase( pendingjump.begin()+kk );
            --kk;
            _Universe->setActiveStarSystem( savedStarSystem );
            //In networking mode we tell the server we want to go back in game
            if (Network != NULL) {
                //Find the corresponding networked player
                if (playernum >= 0) {
                    Network[playernum].inGame();
                    Network[playernum].unreadyToJump();
                }
            }
        }
    }
}

double calc_blend_factor( double frac, int priority, unsigned int when_it_will_be_simulated, int cur_simulation_frame )
{
    if (when_it_will_be_simulated == SIM_QUEUE_SIZE) {
        return 1;
    } else {
        int relwas = when_it_will_be_simulated-priority;
        if (relwas < 0) relwas += SIM_QUEUE_SIZE;
        int relcur = cur_simulation_frame-relwas-1;
        if (relcur < 0) relcur += SIM_QUEUE_SIZE;
        return (relcur+frac)/(double) priority;
    }
}

void ActivateAnimation( Unit *jumppoint )
{
    jumppoint->graphicOptions.Animating = 1;
    for (un_iter i = jumppoint->getSubUnits(); !i.isDone(); ++i)
        ActivateAnimation( *i );
}

static bool isJumping( const vector< unorigdest* > &pending, Unit *un )
{
    for (size_t i = 0; i < pending.size(); ++i)
        if (pending[i]->un == un)
            return true;
    return false;
}

QVector SystemLocation( std::string system );
double howFarToJump();
QVector ComputeJumpPointArrival( QVector pos, std::string origin, std::string destination )
{
    QVector finish = SystemLocation( destination );
    QVector start  = SystemLocation( origin );
    QVector dir    = finish-start;
    if ( dir.MagnitudeSquared() ) {
        dir.Normalize();
        dir = -dir;
        pos = -pos;
        pos.Normalize();
        if ( pos.MagnitudeSquared() ) pos.Normalize();
        return (dir*.5+pos*.125)*howFarToJump();
    }
    return QVector( 0, 0, 0 );
}

bool StarSystem::JumpTo( Unit *un, Unit *jumppoint, const std::string &system, bool force, bool save_coordinates )
{
    if ( ( un->DockedOrDocking()&(~Unit::DOCKING_UNITS) ) != 0 )
        return false;
    if (Network == NULL || force) {
        if (un->jump.drive >= 0)
            un->jump.drive = -1;
#ifdef JUMP_DEBUG
        VSFileSystem::vs_fprintf( stderr, "jumping to %s.  ", system.c_str() );
#endif
        StarSystem *ss = star_system_table.Get( system );
        std::string ssys( system+".system" );
        if (!ss)
            ss = star_system_table.Get( ssys );
        bool justloaded = false;
        if (!ss) {
            justloaded = true;
            ss = _Universe->GenerateStarSystem( ssys.c_str(), filename.c_str(), Vector( 0, 0, 0 ) );
            //NETFIXME: Do we want to generate the system if an AI unit jumps?
        }
        if ( ss && !isJumping( pendingjump, un ) ) {
#ifdef JUMP_DEBUG
            VSFileSystem::vs_fprintf( stderr, "Pushing back to pending queue!\n" );
#endif
            bool dosightandsound = ( ( this == _Universe->getActiveStarSystem( 0 ) ) || _Universe->isPlayerStarship( un ) );
            int  ani = -1;
            if (dosightandsound)
                ani = _Universe->activeStarSystem()->DoJumpingLeaveSightAndSound( un );
	    _Universe->AccessCockpit()->OnJumpBegin(un);
            pendingjump.push_back( new unorigdest( un, jumppoint, this, ss, un->GetJumpStatus().delay, ani, justloaded,
                                                  save_coordinates ? ComputeJumpPointArrival( un->Position(), this->getFileName(),
                                                                                              system ) : QVector( 0, 0, 0 ) ) );
        } else {
#ifdef JUMP_DEBUG
            VSFileSystem::vs_fprintf( stderr, "Failed to retrieve!\n" );
#endif
            return false;
        }
        if (jumppoint)
            ActivateAnimation( jumppoint );
    } else
    //Networking mode
    if (jumppoint) {
        Network->jumpRequest( system, jumppoint->GetSerial() );
    }
    return true;
}

