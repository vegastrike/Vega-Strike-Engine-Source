#include "cockpit_gfx.h"

#include "configuration/game_config.h"
#include "gfxlib.h"
#include "unit_generic.h"
#include "universe.h"
#include "planet.h"
#include "unit_util.h"
#include "weapon_info.h"
#include "gfx/vdu.h"
#include "gfx/camera.h"
#include "gfx/nav/navcomputer.h"
#include "gfx/gauge.h"
#include "gfx/cockpit_gfx_utils.h"

#include <algorithm>

extern float rand01();
#define SWITCH_CONST (.9)

// got to move this to some more generic place
#define SCATTER_CUBE \
    QVector( rand()/RAND_MAX -.5, rand()/RAND_MAX -.5, rand()/RAND_MAX -.5 )

float LookupTargetStat( int stat, Unit *target );

inline void DrawOneTargetBox( const QVector &Loc,
                              float rSize,
                              const Vector &CamP,
                              const Vector &CamQ,
                              const Vector &CamR,
                              float lock_percent,
                              bool ComputerLockon,
                              bool Diamond  )
{
    SetThickness(ShapeType::Box);
    static float rat = XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "min_target_box_size", ".01" ) );
    float len = (Loc).Magnitude();
    float curratio   = rSize/len;
    if (curratio < rat)
        rSize = len*rat;
    GFXEnable( SMOOTH );
    GFXBlendMode( SRCALPHA, INVSRCALPHA );
    static VertexBuilder<> verts;
    verts.clear();
    if (Diamond) {
        GetDiamond(Loc, CamP, CamQ, rSize);
        GFXDraw( GFXLINESTRIP, verts );
    } else if (ComputerLockon) {
        verts = GetRectangle(Loc, CamP, CamQ, rSize);
        GFXDraw( GFXLINESTRIP, verts );
    } else {
        verts = GetOpenRectangle(Loc, CamP, CamQ, rSize);
        GFXDraw( GFXLINE, verts );
    }

    // Still locking on
    lock_percent = std::min(0.0f, lock_percent);
    if (lock_percent < .99) {
        //eallySwitch=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","switchToTargetModeOnKey","true"));
        static float diamondthick =
            XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "DiamondLineThickness", "1" ) );                          //1.05;
        glLineWidth( diamondthick );
        static bool  center       =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "LockCenterCrosshair", "false" ) );

        if (center) {
            verts = GetLockingIcon(Loc, CamP, CamQ, rSize, lock_percent);
            GFXDraw( GFXLINE, verts );
        } else {
            if (lock_percent == 0) {
                verts = GetAnimatedLockingIcon(Loc, CamP, CamQ, CamR, rSize, lock_percent);

                static const unsigned char indices[] = {
                    0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 7, 8,
                    8, 9, 9, 10, 10, 11, 11, 12, 12, 11, 11, 13, 13, 14,
                    14, 15, 15, 5, 16, 7, 7, 17, 17, 18, 18, 19, 19, 1
                };

                GFXDrawElements( GFXLINE, verts, indices, sizeof(indices) / sizeof(*indices) );
            } else {
                verts = GetAnimatedLockingIcon(Loc, CamP, CamQ, CamR, rSize, lock_percent);

                static const unsigned char indices[] = {
                    0, 1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 9, 10, 10, 11
                };

                GFXDrawElements( GFXLINE, verts, indices, sizeof(indices) / sizeof(*indices) );
            }
        }
    }
    SetThickness( ShapeType::Default );
    GFXDisable( SMOOTH );
}


inline void DrawDockingBoxes( Unit *un, const Unit *target, const Vector &CamP, const Vector &CamQ, const Vector &CamR )
{
    if ( target->IsCleared( un ) ) {
        GFXBlendMode( SRCALPHA, INVSRCALPHA );
        static GFXColor dockboxstop = vs_config->getColor( "docking_box_halt", GFXColor(1,0,0,1) );
        static GFXColor dockboxgo   = vs_config->getColor( "docking_box_proceed", GFXColor(0,1,.5,1) );
        static GFXColor waypointcolor = vs_config->getColor( "docking_box_waypoint", GFXColor(0, 1, 1, 0.3) );
        const vector< DockingPorts >d = target->DockingPortLocations();
        for (unsigned int i = 0; i < d.size(); i++)
        {
            float rad = d[i].GetRadius() / sqrt( 2.0 );
            QVector dockpos = Transform(
                    target->GetTransformation(),
                    d[i].GetPosition().Cast()
                ) - _Universe->AccessCamera()->GetPosition();

            if (!d[i].IsDockable())
            {
                if (waypointcolor.a > 0.01) {
                    GFXColorf( waypointcolor );
                    DrawOneTargetBox( dockpos, rad, CamP, CamQ, CamR, 1,
                                      true, true );
                }
                continue;
            }
            GFXDisable( DEPTHTEST );
            GFXDisable( DEPTHWRITE );
            GFXColorf( dockboxstop );
            DrawOneTargetBox( dockpos, rad, CamP, CamQ, CamR, 1,
                              true, true );
            GFXEnable( DEPTHTEST );
            GFXEnable( DEPTHWRITE );
            GFXColorf( dockboxgo );
            DrawOneTargetBox( dockpos, rad, CamP, CamQ, CamR, 1,
                              true, true );
        }
        GFXDisable( DEPTHTEST );
        GFXDisable( DEPTHWRITE );
        GFXColor4f( 1, 1, 1, 1 );
    }
}


inline void DrawITTSLine( QVector fromLoc, QVector aimLoc, GFXColor linecolor=GFXColor( 1, 1, 1, 1 ) )
{
    GFXColorf( linecolor );
    GFXEnable( SMOOTH );
    GFXBlendMode( SRCALPHA, INVSRCALPHA );
    const float verts[2 * 3] = {
        static_cast<float>(fromLoc.x),
        static_cast<float>(fromLoc.y),
        static_cast<float>(fromLoc.z),
        static_cast<float>(aimLoc.x),
        static_cast<float>(aimLoc.y),
        static_cast<float>(aimLoc.z),
    };
    GFXDraw( GFXLINE, verts, 2 );
    GFXDisable( SMOOTH );
}


inline void DrawITTSMark( float Size, QVector p, QVector q, QVector aimLoc, GFXColor markcolor=GFXColor( 1, 1, 1, 1 ) )
{
    GFXColorf( markcolor );
    GFXEnable( SMOOTH );
    GFXBlendMode( SRCALPHA, INVSRCALPHA );
    static VertexBuilder<> verts;
    verts.clear();
    verts.insert( aimLoc + p*Size );
    verts.insert( aimLoc - q*Size );
    verts.insert( aimLoc - p*Size );
    verts.insert( aimLoc + q*Size );
    verts.insert( aimLoc + p*Size );
    GFXDraw( GFXLINESTRIP, verts );
    GFXDisable( SMOOTH );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Draw the arrow pointing to the target.
 */
//THETA : angle between the arrow head and the two branches (divided by 2) (20 degrees here).
#define TARGET_ARROW_COS_THETA (0.93969262078590838405410927732473)
#define TARGET_ARROW_SIN_THETA (0.34202014332566873304409961468226)
#define TARGET_ARROW_SIZE (0.05)

void DrawArrowToTarget(const Radar::Sensor& sensor, Unit *target,
                       float  projection_limit_x, float projection_limit_y,
                       float inv_screen_aspect_ratio)
{
    Unit *player = sensor.GetPlayer();
    if (player && target) {
        Radar::Track track = sensor.CreateTrack(target);
        GFXColorf(sensor.GetColor(track));
        DrawArrowToTarget(sensor, track.GetPosition(), projection_limit_x, projection_limit_y,
                          inv_screen_aspect_ratio);
    }
}

void DrawArrowToTarget(const Radar::Sensor& sensor, Vector localcoord,
                       float  projection_limit_x, float projection_limit_y,
                       float inv_screen_aspect_ratio)
{
    float  s, t, s_normalized, t_normalized, inv_len;
    Vector p1, p2, p_n;

    //Project target position on k.
    inv_len = 1/fabs( localcoord.k );
    s = -localcoord.i*inv_len;
    t = localcoord.j*inv_len;
    if (localcoord.k > 0) {
        //The unit is in front of us.
        //Check if the unit is in the screen.
        if ( (fabs( s ) < projection_limit_x) && (fabs( t ) < projection_limit_y) )
            return;              //The unit is in the screen, do not display the arrow.
    }
    inv_len = 1/sqrt( s*s+t*t );
    s_normalized = s*inv_len;                 //Save the normalized projected coordinates.
    t_normalized = t*inv_len;

    //Apply screen aspect ratio correction.
    s *= inv_screen_aspect_ratio;
    if ( fabs( t ) > fabs( s ) ) {
        //Normalize t.
        if (t > 0) {
            s /= t;
            t  = 1;
        } else if (t < 0) {
            s /= -t;
            t  = -1;
        }                       //case t == 0, do nothing everything is ok.
    } else {
        //Normalize s.
        if (s > 0) {
            t /= s;
            s  = 1;
        } else if (s < 0) {
            t /= -s;
            s  = -1;
        }                      //case s == 0, do nothing everything is ok.
    }
    //Compute points p1 and p2 composing the arrow. Hard code a 2D rotation.
    //p1 = p - TARGET_ARROW_SIZE * p.normalize().rot(THETA), p being the arrow head position (s,t).
    //p2 = p - TARGET_ARROW_SIZE * p.normalize().rot(-THETA)
    p_n.i = -TARGET_ARROW_SIZE*s_normalized;       //Vector p will be used to compute the two branches of the arrow.
    p_n.j = -TARGET_ARROW_SIZE*t_normalized;
    p1.i  = p_n.i*TARGET_ARROW_COS_THETA-p_n.j*TARGET_ARROW_SIN_THETA;      //p1 = p.rot(THETA)
    p1.j  = p_n.j*TARGET_ARROW_COS_THETA+p_n.i*TARGET_ARROW_SIN_THETA;
    p2.i  = p_n.i*TARGET_ARROW_COS_THETA-p_n.j*(-TARGET_ARROW_SIN_THETA);       //p2 = p.rot(-THETA)
    p2.j  = p_n.j*TARGET_ARROW_COS_THETA+p_n.i*(-TARGET_ARROW_SIN_THETA);
    p1.i += s;
    p1.j *= g_game.aspect;
    p1.j += t;
    p2.i += s;
    p2.j *= g_game.aspect;
    p2.j += t;
    p2.k  = p1.k = 0;

    GFXEnable( SMOOTH );
    GFXDisable( TEXTURE0 );
    GFXDisable( TEXTURE1 );
    GFXBlendMode( SRCALPHA, INVSRCALPHA );

    const float verts[4 * 3] = {
        s,    t,    0,
        p1.x, p1.y, p1.z,
        p2.x, p2.y, p2.z,
        s,    t,    0,
    };
    GFXDraw( GFXLINESTRIP, verts, 4 );

    GFXColor4f( 1, 1, 1, 1 );
    GFXDisable( SMOOTH );
}


void DrawCommunicatingBoxes(std::vector< VDU* >vdu)
{
    Vector CamP, CamQ, CamR;
    _Universe->AccessCamera()->GetPQR( CamP, CamQ, CamR );
    //Vector Loc (un->ToLocalCoordinates(target->Position()-un->Position()));
    for (unsigned int i = 0; i < vdu.size(); ++i) {
        Unit *target = vdu[i]->GetCommunicating();
        if (target) {
            static GFXColor black_and_white = vs_config->getColor( "communicating" );
            QVector Loc( target->Position()-_Universe->AccessCamera()->GetPosition() );
            GFXDisable( TEXTURE0 );
            GFXDisable( TEXTURE1 );
            GFXDisable( DEPTHTEST );
            GFXDisable( DEPTHWRITE );
            GFXBlendMode( SRCALPHA, INVSRCALPHA );
            GFXDisable( LIGHTING );
            GFXColorf( black_and_white );

            DrawOneTargetBox( Loc, target->rSize()*1.05, CamP, CamQ, CamR, 1, 0 );
        }
    }
}


void DrawGauges( Cockpit *cockpit, Unit *un, Gauge *gauges[],
                 float gauge_time[], float cockpit_time, TextPlane *text,
                 GFXColor textcol) {
    int i;
    for (i = 0; i < UnitImages< void >::TARGETSHIELDF; i++) {
        if (gauges[i]) {
            gauges[i]->Draw( cockpit->LookupUnitStat( i, un ) );
            float damage =
                un->GetImageInformation().cockpit_damage[(1+MAXVDUS+i)%(MAXVDUS+1+UnitImages < void > ::NUMGAUGES)];
            if (gauge_time[i] >= 0) {
                if ( damage > .0001 && ( cockpit_time > ( gauge_time[i]+(1-damage) ) ) )
                    if (rand01() > SWITCH_CONST)
                        gauge_time[i] = -cockpit_time;
                /*else {
                 *  static string gauge_static = vs_config->getVariable("graphics","gauge_static","static.ani");
                 *  static Animation vdu_ani(gauge_static.c_str(),true,.1,BILINEAR);
                 *  vdu_ani.DrawAsVSSprite(gauges[i]);
                 *  }*/
            } else if ( cockpit_time > ( ( ( 1-(-gauge_time[i]) )+damage ) ) ) {
                if (rand01() > SWITCH_CONST)
                    gauge_time[i] = cockpit_time;
            }
        }
    }
    if (!text)
        return;
    GFXColorf( textcol );
    GFXColor     origbgcol = text->bgcol;
    static float background_alpha    =
        XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "text_background_alpha", "0.0625" ) );
    bool automatte = (0 == origbgcol.a);
    if (automatte) text->bgcol = GFXColor( 0, 0, 0, background_alpha );
    for (i = UnitImages< void >::KPS; i < UnitImages< void >::AUTOPILOT_MODAL; i++) {
        if (gauges[i]) {
            float sx, sy, px, py;
            gauges[i]->GetSize( sx, sy );
            gauges[i]->GetPosition( px, py );
            text->SetCharSize( sx, sy );
            text->SetPos( px, py );
            float tmp  = cockpit->LookupUnitStat( i, un );
            float tmp2 = 0;
            char  ourchar[64];
            sprintf( ourchar, "%.0f", tmp );
            if (i == UnitImages< void >::KPS) {
                float c = 300000000.0f;
                if (tmp > c/10) {
                    tmp2 = tmp/c;
                    sprintf( ourchar, "%.2f C", tmp2 );
                }
            }
            if (i == UnitImages< void >::MASSEFFECT)
                sprintf( ourchar, "MASS:%.0f%% (base)", tmp );
            GFXColorf( textcol );
            text->SetSize( 2, -2 );
            text->Draw( string( ourchar ), 0, false, false, automatte );
        }
    }
    for (i = UnitImages< void >::AUTOPILOT_MODAL; i < UnitImages< void >::NUMGAUGES; i++) {
        if (gauges[i]) {
            float sx, sy, px, py;
            gauges[i]->GetSize( sx, sy );
            gauges[i]->GetPosition( px, py );
            text->SetCharSize( sx, sy );
            text->SetPos( px, py );
            float tmp    = cockpit->LookupUnitStat( i, un );
            int   ivalue = (int) tmp;
            std::string modename;
            std::string modevalue;
            switch (i)
            {
            case UnitImages< void >::AUTOPILOT_MODAL:
                modename = "AUTO:";
                break;
            case UnitImages< void >::SPEC_MODAL:
                modename = "SPEC:";
                break;
            case UnitImages< void >::FLIGHTCOMPUTER_MODAL:
                modename = "FCMP:";
                break;
            case UnitImages< void >::TURRETCONTROL_MODAL:
                modename = "TCNT:";
                break;
            case UnitImages< void >::ECM_MODAL:
                modename = "ECM :";
                break;
            case UnitImages< void >::CLOAK_MODAL:
                modename = "CLK :";
                break;
            case UnitImages< void >::TRAVELMODE_MODAL:
                modename = "GCNT:";
                break;
            case UnitImages< void >::RECIEVINGFIRE_MODAL:
                modename = "RFIR:";
                break;
            case UnitImages< void >::RECEIVINGMISSILES_MODAL:
                modename = "RMIS:";
                break;
            case UnitImages< void >::RECEIVINGMISSILELOCK_MODAL:
                modename = "RML :";
                break;
            case UnitImages< void >::RECEIVINGTARGETLOCK_MODAL:
                modename = "RTL :";
                break;
            case UnitImages< void >::COLLISIONWARNING_MODAL:
                modename = "COL :";
                break;
            case UnitImages< void >::CANJUMP_MODAL:
                modename = "JUMP:";
                break;
            case UnitImages< void >::CANDOCK_MODAL:
                modename = "DOCK:";
                break;
            default:
                modename = "UNK :";
            }
            switch (ivalue)
            {
            case UnitImages< void >::OFF:
                modevalue = "OFF";
                break;
            case UnitImages< void >::ON:
                modevalue = "ON";
                break;
            case UnitImages< void >::SWITCHING:
                modevalue = "<>";
                break;
            case UnitImages< void >::ACTIVE:
                modevalue = "ACTIVE";
                break;
            case UnitImages< void >::FAW:
                modevalue = "FAW";
                break;
            case UnitImages< void >::MANEUVER:
                modevalue = "MANEUVER";
                break;
            case UnitImages< void >::TRAVEL:
                modevalue = "TRAVEL";
                break;
            case UnitImages< void >::NOTAPPLICABLE:
                modevalue = "N / A";
                break;
            case UnitImages< void >::READY:
                modevalue = "READY";
                break;
            case UnitImages< void >::NODRIVE:
                modevalue = "NO DRIVE";
                break;
            case UnitImages< void >::TOOFAR:
                modevalue = "TOO FAR";
                break;
            case UnitImages< void >::NOTENOUGHENERGY:
                modevalue = "LOW ENERGY";
                break;
            case UnitImages< void >::WARNING:
                modevalue = "WARNING!";
                break;
            case UnitImages< void >::NOMINAL:
                modevalue = " - ";
                break;
            case UnitImages< void >::AUTOREADY:
                modevalue = "AUTO READY";
                break;
            default:
                modevalue = "MALFUNCTION!";
            }
            GFXColorf( textcol );
            //text->SetSize(px+textwidthapproxHACK*(modename.size()+modevalue.size()), -2);
            text->SetSize( 2, -2 );
            text->Draw( modename+modevalue, 0, false, false, automatte );
        }
    }
    text->bgcol = origbgcol;
    GFXColor4f( 1, 1, 1, 1 );
}

// This draws a cross
void DrawNavigationSymbol( const Vector &Loc, const Vector &P, const Vector &Q, float size )
{


    SetThickness(ShapeType::Cross);
    size = .125*size;
    GFXBlendMode( SRCALPHA, INVSRCALPHA );
    GFXEnable( SMOOTH );

    static VertexBuilder<> verts = GetCross(Loc, P, Q, size);
    GFXDraw( GFXLINE, verts );

    GFXDisable( SMOOTH );
    SetThickness(ShapeType::Default);
}


bool DrawNavSystem(NavigationSystem* nav_system, Camera* camera, float cockpit_offset)
{
    if(!nav_system->CheckDraw()) {
        return false;
    }

    camera->SetFov( camera->GetFov() );
    camera->setCockpitOffset( cockpit_offset );
    camera->UpdateGFX( GFXFALSE, GFXFALSE, GFXTRUE );
    nav_system->Draw();

    return true;
}


void DrawRadar(const Radar::Sensor& sensor, float  cockpit_time, float radar_time,
               VSSprite *radarSprites[2],
               Radar::Display* radarDisplay)
{
    assert(radarDisplay);

    if (radarSprites[0] || radarSprites[1])
    {
        GFXDisable(TEXTURE0);
        GFXDisable(LIGHTING);


        radarDisplay->Draw(sensor, radarSprites[0], radarSprites[1]);

        GFXEnable(TEXTURE0);

        // Draw radar damage
        float damage = (sensor.GetPlayer()->GetImageInformation().cockpit_damage[0]);
        if (sensor.GetMaxRange() < 1.0)
            damage = std::min(damage, 0.25f);
        if (damage < .985) {
            if (radar_time >= 0) {
                if ( damage > .001 && ( cockpit_time > radar_time+(1-damage) ) ) {
                    if (rand01() > SWITCH_CONST)
                        radar_time = -cockpit_time;
                } else {
                    static Animation radar_ani( "static_round.ani", true, .1, BILINEAR );
                    radar_ani.DrawAsVSSprite( radarSprites[0] );
                    radar_ani.DrawAsVSSprite( radarSprites[1] );
                }
            } else if ( cockpit_time > ( ( 1-(-radar_time) )+damage ) ) {
                if (rand01() > SWITCH_CONST)
                    radar_time = cockpit_time;
            }
        }
    }
}


void DrawTacticalTargetBox(const Radar::Sensor& sensor)
{
    static bool drawtactarg =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "DrawTacticalTarget", "false" ) );
    if (!drawtactarg)
        return;
    if (sensor.GetPlayer()->getFlightgroup() == NULL)
        return;
    Unit *target = sensor.GetPlayer()->getFlightgroup()->target.GetUnit();
    if (target) {
        Vector  CamP, CamQ, CamR;
        _Universe->AccessCamera()->GetPQR( CamP, CamQ, CamR );
        //Vector Loc (un->ToLocalCoordinates(target->Position()-un->Position()));
        QVector Loc( target->Position()-_Universe->AccessCamera()->GetPosition() );
        GFXDisable( TEXTURE0 );
        GFXDisable( TEXTURE1 );
        GFXDisable( DEPTHTEST );
        GFXDisable( DEPTHWRITE );
        GFXBlendMode( SRCALPHA, INVSRCALPHA );
        GFXDisable( LIGHTING );

        static float thethick = XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "TacTargetThickness", "1.0" ) );
        static float fudge    = XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "TacTargetLength", "0.1" ) );
        static float foci     = XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "TacTargetFoci", "0.5" ) );
        glLineWidth( (int) thethick );         //temp
        Radar::Track track = sensor.CreateTrack(target, Loc);
        GFXColorf(sensor.GetColor(track));

        //DrawOneTargetBox (Loc, target->rSize(), CamP, CamQ, CamR,un->computeLockingPercent(),un->TargetLocked());

        //** jay
        float rSize = track.GetSize();

        static VertexBuilder<> verts;
        verts.clear();

        verts.insert( Loc+( (-CamP).Cast()+(-CamQ).Cast() )*rSize*(foci+fudge) );
        verts.insert( Loc+( (-CamP).Cast()+(-CamQ).Cast() )*rSize*(foci-fudge) );

        verts.insert( Loc+( (-CamP).Cast()+(CamQ).Cast() )*rSize*(foci+fudge) );
        verts.insert( Loc+( (-CamP).Cast()+(CamQ).Cast() )*rSize*(foci-fudge) );

        verts.insert( Loc+( (CamP).Cast()+(-CamQ).Cast() )*rSize*(foci+fudge) );
        verts.insert( Loc+( (CamP).Cast()+(-CamQ).Cast() )*rSize*(foci-fudge) );

        verts.insert( Loc+( (CamP).Cast()+(CamQ).Cast() )*rSize*(foci+fudge) );
        verts.insert( Loc+( (CamP).Cast()+(CamQ).Cast() )*rSize*(foci-fudge) );

        GFXDraw( GFXLINE, verts );

        glLineWidth( (int) 1 );         //temp
    }
}

void DrawTargetBoxes(const Radar::Sensor& sensor)
{
    if (sensor.InsideNebula())
        return;

    StarSystem     *ssystem  = _Universe->activeStarSystem();
    UnitCollection *unitlist = &ssystem->getUnitList();
    //UnitCollection::UnitIterator *uiter=unitlist->createIterator();

    Vector CamP, CamQ, CamR;
    _Universe->AccessCamera()->GetPQR( CamP, CamQ, CamR );

    GFXDisable( TEXTURE0 );
    GFXDisable( TEXTURE1 );
    GFXDisable( DEPTHTEST );
    GFXDisable( DEPTHWRITE );
    GFXBlendMode( SRCALPHA, INVSRCALPHA );
    GFXDisable( LIGHTING );
    const Unit *target;
    Unit *player = sensor.GetPlayer();
    assert(player);
    for (un_kiter uiter = unitlist->constIterator(); (target=*uiter)!=NULL; ++uiter) {
        if (target != player) {
            QVector  Loc( target->Position() );
            Radar::Track track = sensor.CreateTrack(target, Loc);

            GFXColorf(sensor.GetColor(track));
            switch (track.GetType())
            {
            case Radar::Track::Type::Base:
            case Radar::Track::Type::CapitalShip:
            case Radar::Track::Type::Ship:
                if (sensor.IsTracking(track))
                {
                    static bool draw_dock_box =
                        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_docking_boxes", "true" ) );
                    if (draw_dock_box)
                        DrawDockingBoxes( player, target, CamP, CamQ, CamR );
                    DrawOneTargetBox( Loc, target->rSize(), CamP, CamQ, CamR, player->computeLockingPercent(), true );
                } else {
                    DrawOneTargetBox( Loc, target->rSize(), CamP, CamQ, CamR, player->computeLockingPercent(), false );
                }
                break;

            default:
                break;
            }
        }
    }
}


void DrawTargetBox(const Radar::Sensor& sensor, bool draw_line_to_target, bool draw_line_to_targets_target,
                   bool always_itts, float locking_percent, bool draw_line_to_itts, bool steady_itts)
{
    if (sensor.InsideNebula())
        return;
    Unit *player = sensor.GetPlayer();
    assert(player);
    Unit *target = player->Target();
    if (!target)
        return;

    float speed, range;
    int   neutral = FactionUtil::GetNeutralFaction();
    Vector  CamP, CamQ, CamR;
    _Universe->AccessCamera()->GetPQR( CamP, CamQ, CamR );
    //Vector Loc (un->ToLocalCoordinates(target->Position()-un->Position()));
    QVector Loc( target->Position()-_Universe->AccessCamera()->GetPosition() );

    GFXDisable( TEXTURE0 );
    GFXDisable( TEXTURE1 );
    GFXDisable( DEPTHTEST );
    GFXDisable( DEPTHWRITE );
    GFXBlendMode( SRCALPHA, INVSRCALPHA );
    GFXDisable( LIGHTING );
    static bool draw_nav_symbol = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "drawNavSymbol", "false" ) );
    if (draw_nav_symbol) {
        DrawNavigationSymbol(player->GetComputerData().NavPoint, CamP, CamQ,
                             CamR.Cast().Dot( (player->GetComputerData().NavPoint).Cast()-_Universe->AccessCamera()->GetPosition() ) );
    }
    Radar::Track track = sensor.CreateTrack(target, Loc);
    GFXColor trackcolor=sensor.GetColor(track);
    GFXColorf(trackcolor);
    if (draw_line_to_target) {
        GFXBlendMode( SRCALPHA, INVSRCALPHA );
        GFXEnable( SMOOTH );
        QVector myLoc( _Universe->AccessCamera()->GetPosition() );

        Unit *targets_target = target->Target();
        if (draw_line_to_targets_target && targets_target != NULL) {
            QVector ttLoc = targets_target->Position();
            const float verts[3 * 3] = {
                static_cast<float>(myLoc.x),
                static_cast<float>(myLoc.y),
                static_cast<float>(myLoc.z),

                static_cast<float>(Loc.x),
                static_cast<float>(Loc.y),
                static_cast<float>(Loc.z),

                static_cast<float>(ttLoc.x),
                static_cast<float>(ttLoc.x),
                static_cast<float>(ttLoc.x),
            };
            GFXDraw( GFXLINESTRIP, verts, 3 );
        } else {
            const float verts[2 * 3] = {
                static_cast<float>(myLoc.x),
                static_cast<float>(myLoc.y),
                static_cast<float>(myLoc.z),

                static_cast<float>(Loc.x),
                static_cast<float>(Loc.y),
                static_cast<float>(Loc.z),
            };
            GFXDraw( GFXLINESTRIP, verts, 2 );
        }
        GFXDisable( SMOOTH );
    }
    static bool draw_target_nav_symbol =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_target_nav_symbol", "true" ) );
    static bool draw_jump_nav_symbol   =
        XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_jump_target_nav_symbol", "true" ) );
    bool nav_symbol = false;
    // FIXME: Replace with UnitUtil::isDockableUnit?
    if ( draw_target_nav_symbol
        && ( (target->faction == neutral
              && target->isUnit() == _UnitType::unit) || target->isUnit() == _UnitType::asteroid
            || ( target->isPlanet() && ( (Planet*) target )->isAtmospheric()
                && ( draw_jump_nav_symbol
                     || target->GetDestinations().empty() ) ) || !sensor.InRange(track)) ) {
        static float nav_symbol_size = XMLSupport::parse_float( vs_config->getVariable( "graphics", "nav_symbol_size", ".25" ) );
        GFXColor4f( 1, 1, 1, 1 );
        DrawNavigationSymbol( Loc, CamP, CamQ, Loc.Magnitude()*nav_symbol_size );
        nav_symbol = true;
    } else {
        static bool lock_nav_symbol =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "lock_significant_target_box", "true" ) );
        DrawOneTargetBox( Loc, target->rSize(), CamP, CamQ, CamR, locking_percent, player->TargetLocked()
                         && ( lock_nav_symbol || !UnitUtil::isSignificant( target ) ) );
    }

    static bool draw_dock_box = XMLSupport::parse_bool( vs_config->getVariable( "graphics", "draw_docking_boxes", "true" ) );
    if (draw_dock_box)
        DrawDockingBoxes(player, target, CamP, CamQ, CamR);
    if ( (always_itts || player->GetComputerData().itts) && !nav_symbol ) {
        float   mrange;
        float   err  =  .01*( 1 - player->CloakVisible() );
        float   scatter  = .25*player->rSize();
        static bool ITTS_averageguns =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "ITTSUseAverageGunSpeed", "true" ) );
        static bool ITTS_for_locks =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "ITTSForLockable", "false" ) );
        static bool ITTS_for_beams =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "ITTSForBeams", "false" ) );
        static bool line_to_itts_alpha =
            XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "ITTSLineToMarkAlpha", "0.1" ) );
        QVector p = CamP.Cast();
        QVector q = CamQ.Cast();
        QVector offs = _Universe->AccessCamera()->GetPosition() - SCATTER_CUBE*scatter*10*err;
        QVector iLoc;
        Vector PlayerPosition = player->Position();
        Vector PlayerVelocity = player->GetVelocity();
        GFXColor mntcolor;
        if (ITTS_averageguns) {
            player->getAverageGunSpeed( speed, range, mrange );
            iLoc = target->PositionITTS( PlayerPosition, PlayerVelocity, speed, steady_itts ) - offs;
            if (draw_line_to_itts)
                DrawITTSLine(Loc, iLoc, trackcolor);
            DrawITTSMark(scatter, p, q, iLoc, trackcolor);
        }
        else {	// per-mount ITTS
            int nummounts = player->getNumMounts();
            if (draw_line_to_itts) {
                for (int i = 0; i < nummounts; i++) {
                    if ( (player->mounts[i].status == Mount::ACTIVE)
                        && (ITTS_for_beams || (player->mounts[i].type->type != WEAPON_TYPE::BEAM))
                        && (ITTS_for_locks || (player->mounts[i].type->lock_time == 0)) )
                    {
                        iLoc = target->PositionITTS( PlayerPosition, PlayerVelocity, player->mounts[i].type->speed, steady_itts ) - offs;
                        DrawITTSLine( Loc, iLoc, GFXColor(trackcolor.r, trackcolor.g, trackcolor.b, line_to_itts_alpha) );
                    }
                }
            }
            for (int i = 0; i < nummounts; i++) {
                if ( (player->mounts[i].status == Mount::ACTIVE)
                    && (ITTS_for_beams || (player->mounts[i].type->type != WEAPON_TYPE::BEAM))
                    && (ITTS_for_locks || (player->mounts[i].type->lock_time == 0)) )
                {
                    mntcolor = MountColor( &player->mounts[i] );
                    iLoc = target->PositionITTS( PlayerPosition, PlayerVelocity, player->mounts[i].type->speed, steady_itts ) - offs;
                    DrawITTSMark(scatter, p, q, iLoc, mntcolor);
                }
            }
        }       // per mount
    }       // draw ITTS
}


void DrawTurretTargetBoxes(const Radar::Sensor& sensor)
{
    if (sensor.InsideNebula())
        return;


    GFXDisable( TEXTURE0 );
    GFXDisable( TEXTURE1 );
    GFXDisable( DEPTHTEST );
    GFXDisable( DEPTHWRITE );
    GFXDisable( LIGHTING );

    static VertexBuilder<> verts;

    //This avoids rendering the same target box more than once
    Unit *subunit;
    std::set<Unit *> drawn_targets;
    for (un_iter iter = sensor.GetPlayer()->getSubUnits(); (subunit=*iter)!=NULL; ++iter) {
        if (!subunit)
            return;
        Unit *target = subunit->Target();
        if (!target || (drawn_targets.find(target) != drawn_targets.end()))
            continue;
        drawn_targets.insert(target);

        Vector CamP, CamQ, CamR;
        _Universe->AccessCamera()->GetPQR( CamP, CamQ, CamR );
        //Vector Loc (un->ToLocalCoordinates(target->Position()-un->Position()));
        QVector     Loc( target->Position()-_Universe->AccessCamera()->GetPosition() );
        Radar::Track track = sensor.CreateTrack(target, Loc);
        static bool draw_nav_symbol =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "drawNavSymbol", "false" ) );
        if (draw_nav_symbol) {
            GFXColor4f( 1, 1, 1, 1 );
            DrawNavigationSymbol( subunit->GetComputerData().NavPoint, CamP, CamQ,
                                 CamR.Cast().Dot( (subunit->GetComputerData().NavPoint).Cast()
                                                 -_Universe->AccessCamera()->GetPosition() ) );
        }
        GFXColorf(sensor.GetColor(track));

        //DrawOneTargetBox (Loc, target->rSize(), CamP, CamQ, CamR,un->computeLockingPercent(),un->TargetLocked());

        //** jay
        float rSize = track.GetSize();

        GFXEnable( SMOOTH );
        GFXBlendMode( SRCALPHA, INVSRCALPHA );

        verts.clear();

        verts.insert( Loc+(CamP).Cast()*rSize*1.3 );
        verts.insert( Loc+(CamP).Cast()*rSize*.8 );

        verts.insert( Loc+(-CamP).Cast()*rSize*1.3 );
        verts.insert( Loc+(-CamP).Cast()*rSize*.8 );

        verts.insert( Loc+(CamQ).Cast()*rSize*1.3 );
        verts.insert( Loc+(CamQ).Cast()*rSize*.8 );

        verts.insert( Loc+(-CamQ).Cast()*rSize*1.3 );
        verts.insert( Loc+(-CamQ).Cast()*rSize*.8 );

        GFXDraw( GFXLINESTRIP, verts );

        GFXDisable( SMOOTH );
    }
}


void DrawTargetGauges( Unit *target, Gauge *gauges[] )
{
    int i;
    for (i = UnitImages< void >::TARGETSHIELDF; i < UnitImages< void >::KPS; i++) {
        if (gauges[i]) {
            gauges[i]->Draw( LookupTargetStat(i, target) );
        }
    }
}


float LookupTargetStat( int stat, Unit *target )
{
    switch (stat)
    {
    case UnitImages< void >::TARGETSHIELDF:
        return target->FShieldData();

    case UnitImages< void >::TARGETSHIELDR:
        return target->RShieldData();

    case UnitImages< void >::TARGETSHIELDL:
        return target->LShieldData();

    case UnitImages< void >::TARGETSHIELDB:
        return target->BShieldData();
    }
    return 1;
}
