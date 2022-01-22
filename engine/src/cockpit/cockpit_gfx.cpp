#include "cockpit_gfx.h"

#include "configuration/game_config.h"
#include "gfxlib.h"
#include "unit_generic.h"
#include "universe.h"
#include "planet.h"
#include "unit_util.h"
#include "weapon_info.h"


// got to move this to some more generic place
#define SCATTER_CUBE \
    QVector( rand()/RAND_MAX -.5, rand()/RAND_MAX -.5, rand()/RAND_MAX -.5 )

inline void DrawOneTargetBox( const QVector &Loc,
                              float rSize,
                              const Vector &CamP,
                              const Vector &CamQ,
                              const Vector &CamR,
                              float lock_percent,
                              bool ComputerLockon,
                              bool Diamond  )
{
    static float boxthick = XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "BoxLineThickness", "1" ) );
    static float rat = XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "min_target_box_size", ".01" ) );
    float len = (Loc).Magnitude();
    float curratio   = rSize/len;
    if (curratio < rat)
        rSize = len*rat;
    GFXLineWidth( boxthick );
    GFXEnable( SMOOTH );
    GFXBlendMode( SRCALPHA, INVSRCALPHA );
    static VertexBuilder<> verts;
    verts.clear();
    if (Diamond) {
        float ModrSize = rSize/1.41;
        verts.insert( Loc+(.75*CamP+CamQ).Cast()*ModrSize );
        verts.insert( Loc+(CamP+.75*CamQ).Cast()*ModrSize );
        verts.insert( Loc+(CamP-.75*CamQ).Cast()*ModrSize );
        verts.insert( Loc+(.75*CamP-CamQ).Cast()*ModrSize );
        verts.insert( Loc+(-.75*CamP-CamQ).Cast()*ModrSize );
        verts.insert( Loc+(-CamP-.75*CamQ).Cast()*ModrSize );
        verts.insert( Loc+(.75*CamQ-CamP).Cast()*ModrSize );
        verts.insert( Loc+(CamQ-.75*CamP).Cast()*ModrSize );
        verts.insert( Loc+(.75*CamP+CamQ).Cast()*ModrSize );
        GFXDraw( GFXLINESTRIP, verts );
    } else if (ComputerLockon) {
        verts.insert( Loc+(CamP+CamQ).Cast()*rSize );
        verts.insert( Loc+(CamP-CamQ).Cast()*rSize );
        verts.insert( Loc+(-CamP-CamQ).Cast()*rSize );
        verts.insert( Loc+(CamQ-CamP).Cast()*rSize );
        verts.insert( Loc+(CamP+CamQ).Cast()*rSize );
        GFXDraw( GFXLINESTRIP, verts );
    } else {
        verts.insert( Loc+(CamP+CamQ).Cast()*rSize );
        verts.insert( Loc+(CamP+.66*CamQ).Cast()*rSize );

        verts.insert( Loc+(CamP-CamQ).Cast()*rSize );
        verts.insert( Loc+(CamP-.66*CamQ).Cast()*rSize );

        verts.insert( Loc+(-CamP-CamQ).Cast()*rSize );
        verts.insert( Loc+(-CamP-.66*CamQ).Cast()*rSize );

        verts.insert( Loc+(CamQ-CamP).Cast()*rSize );
        verts.insert( Loc+(CamQ-.66*CamP).Cast()*rSize );

        verts.insert( Loc+(CamP+CamQ).Cast()*rSize );
        verts.insert( Loc+(CamP+.66*CamQ).Cast()*rSize );

        verts.insert( Loc+(CamP+CamQ).Cast()*rSize );
        verts.insert( Loc+(.66*CamP+CamQ).Cast()*rSize );

        verts.insert( Loc+(CamP-CamQ).Cast()*rSize );
        verts.insert( Loc+(.66*CamP-CamQ).Cast()*rSize );

        verts.insert( Loc+(-CamP-CamQ).Cast()*rSize );
        verts.insert( Loc+(-.66*CamP-CamQ).Cast()*rSize );

        verts.insert( Loc+(CamQ-CamP).Cast()*rSize );
        verts.insert( Loc+(.66*CamQ-CamP).Cast()*rSize );

        verts.insert( Loc+(CamP+CamQ).Cast()*rSize );
        verts.insert( Loc+(.66*CamP+CamQ).Cast()*rSize );

        GFXDraw( GFXLINE, verts );
    }
    if (lock_percent < .99) {
        if (lock_percent < 0)
            lock_percent = 0;
        //eallySwitch=XMLSupport::parse_bool(vs_config->getVariable("graphics","hud","switchToTargetModeOnKey","true"));
        static float diamondthick =
            XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "DiamondLineThickness", "1" ) );                          //1.05;
        glLineWidth( diamondthick );
        static bool  center       =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "LockCenterCrosshair", "false" ) );
        static float absmin       =
            XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "min_lock_box_size", ".001" ) );
        static float endreticle   =
            XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "MinMissileBracketSize", "1.05" ) );                        //1.05;
        static float startreticle =
            XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "MaxMissileBracketSize", "2.05" ) );                          //2.05;
        static float diamondsize  = XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "DiamondSize", "2.05" ) );         //1.05;
        static float bracketsize  =
            XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "RotatingBracketSize", "0.58" ) );                         //1.05;
        static float thetaspeed   =
            XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "DiamondRotationSpeed", "1" ) );                        //1.05;
        static float lockline     =
            XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "LockConfirmLineLength", "1.5" ) );                      //1.05;
        if (center) {
            static float bracketwidth =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "RotatingBracketWidth", "0.1" ) );                              //1.05;
            static bool  bracketinnerouter =
                XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "RotatingBracketInner", "true" ) );                                  //1.05;
            float innerdis   = endreticle+(startreticle-endreticle)*lock_percent;
            float outerdis   = innerdis+bracketsize;
            float bracketdis = (bracketinnerouter ? innerdis : outerdis);
            float r = rSize < absmin ? absmin : rSize;

            verts.clear();

            verts.insert( Loc+CamP*(innerdis*r) );
            verts.insert( Loc+CamP*(outerdis*r) );

            verts.insert( Loc+CamP*(bracketdis*r)+CamQ*(bracketwidth*r) );
            verts.insert( Loc+CamP*(bracketdis*r)-CamQ*(bracketwidth*r) );

            verts.insert( Loc-CamP*(innerdis*r) );
            verts.insert( Loc-CamP*(outerdis*r) );

            verts.insert( Loc-CamP*(bracketdis*r)+CamQ*(bracketwidth*r) );
            verts.insert( Loc-CamP*(bracketdis*r)-CamQ*(bracketwidth*r) );

            verts.insert( Loc+CamQ*(innerdis*r) );
            verts.insert( Loc+CamQ*(outerdis*r) );

            verts.insert( Loc+CamQ*(bracketdis*r)+CamP*(bracketwidth*r) );
            verts.insert( Loc+CamQ*(bracketdis*r)-CamP*(bracketwidth*r) );

            verts.insert( Loc-CamQ*(innerdis*r) );
            verts.insert( Loc-CamQ*(outerdis*r) );

            verts.insert( Loc-CamQ*(bracketdis*r)+CamP*(bracketwidth*r) );
            verts.insert( Loc-CamQ*(bracketdis*r)-CamP*(bracketwidth*r) );

            GFXDraw( GFXLINE, verts );
        } else {
            float  max   = diamondsize;
            float  coord = endreticle+(startreticle-endreticle)*lock_percent;              //rSize/(1-lock_percent);//this is a number between 1 and 100
            double rtot  = 1./sqrtf( 2 );

            //this causes the rotation!
            float  theta = 4*M_PI*lock_percent*thetaspeed;
            Vector LockBox( -cos( theta )*rtot, -rtot, sin( theta )*rtot );
            //Vector LockBox (0*rtot,-rtot,1*rtot);

            static float diamondthick =
                XMLSupport::parse_float( vs_config->getVariable( "graphics", "hud", "DiamondLineThickness", "1" ) );                              //1.05;
            GFXLineWidth( diamondthick );
            QVector TLockBox( rtot*LockBox.i+rtot*LockBox.j, rtot*LockBox.j-rtot*LockBox.i, LockBox.k );
            QVector SLockBox( TLockBox.j, TLockBox.i, TLockBox.k );
            QVector Origin = (CamP+CamQ).Cast()*(rSize*coord);
            QVector Origin1 = (CamP-CamQ).Cast()*(rSize*coord);
            TLockBox = (TLockBox.i*CamP+TLockBox.j*CamQ+TLockBox.k*CamR).Cast();
            SLockBox = (SLockBox.i*CamP+SLockBox.j*CamQ+SLockBox.k*CamR).Cast();
            double  r1Size = rSize*bracketsize;
            if (r1Size < absmin)
                r1Size = absmin;

            TLockBox *= r1Size;
            SLockBox *= r1Size;

            max *= rSize*.75*endreticle;
            verts.clear();
            if (lock_percent == 0) {
                const QVector qCamP(CamP.Cast());
                const QVector qCamQ(CamQ.Cast());

                verts.insert( Loc+qCamQ*max*lockline );
                verts.insert( Loc+qCamQ*max );
                verts.insert( Loc+Origin+TLockBox );
                verts.insert( Loc+Origin );
                verts.insert( Loc+Origin+SLockBox );
                verts.insert( Loc+qCamP*max );
                verts.insert( Loc+qCamP*max*lockline );
                verts.insert( Loc-qCamP*max );
                verts.insert( Loc-Origin-SLockBox );
                verts.insert( Loc-Origin );
                verts.insert( Loc-Origin-TLockBox );
                verts.insert( Loc-qCamQ*max );
                verts.insert( Loc-qCamQ*max*lockline );
                verts.insert( Loc+Origin1+TLockBox );
                verts.insert( Loc+Origin1 );
                verts.insert( Loc+Origin1-SLockBox );
                verts.insert( Loc-qCamP*max*lockline );
                verts.insert( Loc-Origin1+SLockBox );
                verts.insert( Loc-Origin1 );
                verts.insert( Loc-Origin1-TLockBox );

                static const unsigned char indices[] = {
                    0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 7, 8,
                    8, 9, 9, 10, 10, 11, 11, 12, 12, 11, 11, 13, 13, 14,
                    14, 15, 15, 5, 16, 7, 7, 17, 17, 18, 18, 19, 19, 1
                };

                GFXDrawElements( GFXLINE, verts, indices, sizeof(indices) / sizeof(*indices) );
            } else {
                verts.insert( Loc+Origin+TLockBox );
                verts.insert( Loc+Origin );
                verts.insert( Loc+Origin+SLockBox );
                verts.insert( Loc-Origin-SLockBox );
                verts.insert( Loc-Origin );
                verts.insert( Loc-Origin-TLockBox );
                verts.insert( Loc+Origin1+TLockBox );
                verts.insert( Loc+Origin1 );
                verts.insert( Loc+Origin1-SLockBox );
                verts.insert( Loc-Origin1+SLockBox );
                verts.insert( Loc-Origin1 );
                verts.insert( Loc-Origin1-TLockBox );

                static const unsigned char indices[] = {
                    0, 1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 9, 10, 10, 11
                };

                GFXDrawElements( GFXLINE, verts, indices, sizeof(indices) / sizeof(*indices) );
            }
        }
    }
    GFXLineWidth( 1 );
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

void DrawNavigationSymbol( const Vector &Loc, const Vector &P, const Vector &Q, float size )
{
    static float crossthick = GameConfig::GetVariable( "graphics", "hud", "NavCrossLineThickness", 1.0 );                        //1.05;

    GFXLineWidth( crossthick );
    size = .125*size;
    GFXBlendMode( SRCALPHA, INVSRCALPHA );
    GFXEnable( SMOOTH );

    static VertexBuilder<> verts;
    verts.clear();
    verts.insert( Loc+P*size );
    verts.insert( Loc+.125*P*size );
    verts.insert( Loc-P*size );
    verts.insert( Loc-.125*P*size );
    verts.insert( Loc+Q*size );
    verts.insert( Loc+.125*Q*size );
    verts.insert( Loc-Q*size );
    verts.insert( Loc-.125*Q*size );
    verts.insert( Loc+.0625*Q*size );
    verts.insert( Loc+.0625*P*size );
    verts.insert( Loc-.0625*Q*size );
    verts.insert( Loc-.0625*P*size );
    verts.insert( Loc+.9*P*size+.125*Q*size );
    verts.insert( Loc+.9*P*size-.125*Q*size );
    verts.insert( Loc-.9*P*size+.125*Q*size );
    verts.insert( Loc-.9*P*size-.125*Q*size );
    verts.insert( Loc+.9*Q*size+.125*P*size );
    verts.insert( Loc+.9*Q*size-.125*P*size );
    verts.insert( Loc-.9*Q*size+.125*P*size );
    verts.insert( Loc-.9*Q*size-.125*P*size );
    GFXDraw( GFXLINE, verts );

    GFXDisable( SMOOTH );
    GFXLineWidth( 1 );
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


