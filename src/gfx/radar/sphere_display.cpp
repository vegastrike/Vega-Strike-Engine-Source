// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "lin_time.h" // GetElapsedTime
#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "gfxlib.h"
#include "viewarea.h"
#include "sphere_display.h"

namespace
{

float GetDangerRate(Radar::Sensor::ThreatLevel::Value threat)
{
    using namespace Radar;

    switch (threat)
    {
    case Sensor::ThreatLevel::High:
        return 20.0; // Fast pulsation

    case Sensor::ThreatLevel::Medium:
        return 7.5; // Slow pulsation

    default:
        return 0.0; // No pulsation
    }
}

} // anonymous namespace

namespace Radar
{

SphereDisplay::SphereDisplay()
    : innerSphere(0.98),
      radarTime(0.0)
{
}

void SphereDisplay::Draw(const Sensor& sensor,
                         VSSprite *frontSprite,
                         VSSprite *rearSprite)
{
    assert(frontSprite || rearSprite); // There should be at least one radar display

    radarTime += GetElapsedTime();

    leftRadar.SetSprite(frontSprite);
    rightRadar.SetSprite(rearSprite);

    if (frontSprite)
        frontSprite->Draw();
    if (rearSprite)
        rearSprite->Draw();

    Sensor::TrackCollection tracks = sensor.FindTracksInRange();

    // FIXME: Consider using std::sort instead of the z-buffer
    GFXEnable(DEPTHTEST);
    GFXEnable(DEPTHWRITE);

    DrawBackground(sensor, leftRadar);
    DrawBackground(sensor, rightRadar);

    for (Sensor::TrackCollection::const_iterator it = tracks.begin(); it != tracks.end(); ++it)
    {
        if (it->GetPosition().z < 0)
        {
            // Draw tracks behind the ship
            DrawTrack(sensor, rightRadar, *it);
        }
        else
        {
            // Draw tracks in front of the ship
            DrawTrack(sensor, leftRadar, *it);
        }
    }

    GFXPointSize(1);
    GFXDisable(DEPTHTEST);
    GFXDisable(DEPTHWRITE);
}

void SphereDisplay::DrawTrack(const Sensor& sensor,
                              const ViewArea& radarView,
                              const Track& track)
{
    if (!radarView.IsActive())
        return;

    GFXColor color = sensor.GetColor(track);

    Vector position = track.GetPosition();
    if (position.z < 0){
        static bool  negate_z       =
            XMLSupport::parse_bool( vs_config->getVariable( "graphics", "hud", "show_negative_blips_as_positive", "false" ));
        if (negate_z)
            position.z=-position.z;
        else                                    
            position.z = 0;
    }
    const float trackSize = 2.0;

    // FIXME: Jitter only on boundary, not in center
    if (sensor.InsideNebula())
    {
        Jitter(0.02, 0.04, position);
    }
    else
    {
        const bool isNebula = (track.GetType() == Track::Type::Nebula);
        const bool isEcmActive = track.HasActiveECM();
        if (isNebula || isEcmActive)
        {
            float error = 0.02 * trackSize;
            Jitter(error, error, position);
        }
    }

    // The magnitude is used to calculate the unit vector. With subtle scaling
    // of the magnitude we generate a unit vector whose length will vary from
    // innerSphere to 1.0, depending on the distance to the object. Combined
    // with the OpenGL z-buffering, this will ensure that close tracks are drawn
    // on top of distant tracks.
    float magnitude = position.Magnitude();
    float scaleFactor = 0.0; // [0; 1] where 0 = border, 1 = center
    const float maxRange = sensor.GetMaxRange();
    if (magnitude <= maxRange)
    {
        // [innerSphere; 1]
        scaleFactor = (1.0 - innerSphere) * (maxRange - magnitude) / maxRange;
        magnitude /= (1.0 - scaleFactor);
    }
    Vector scaledPosition = Vector(-position.x, position.y, position.z) / magnitude;

    Vector head = radarView.Scale(scaledPosition);
    
    GFXColor headColor = color;
    if (sensor.UseThreatAssessment())
    {
        float dangerRate = GetDangerRate(sensor.IdentifyThreat(track));
        if (dangerRate > 0.0)
        {
            // Blinking track
            headColor.a *= cosf(dangerRate * radarTime);
        }
    }
    // Fade out dying ships
    if (track.IsExploding())
    {
        headColor.a *= (1.0 - track.ExplodingProgress());
    }

    GFXColorf(headColor);
    if (sensor.IsTracking(track))
    {
        DrawTargetMarker(head, trackSize);
    }
    GFXPointSize(trackSize);
    GFXBegin(GFXPOINT);
    GFXVertexf(head);
    GFXEnd();
}

void SphereDisplay::DrawTargetMarker(const Vector& position, float trackSize)
{
    // Crosshair
    const float crossSize = 8.0;
    const float xcross = crossSize / g_game.x_resolution;
    const float ycross = crossSize / g_game.y_resolution;

    // The crosshair wiggles as it moves around. The wiggling is less noticable
    // when the crosshair is drawn with the smooth option.
    GFXEnable(SMOOTH);
    GFXLineWidth(trackSize);
    GFXBegin(GFXLINE);
    GFXVertex3f(position.x + xcross, position.y, 0.0f);
    GFXVertex3f(position.x - xcross, position.y, 0.0f);
    GFXVertex3f(position.x, position.y - ycross, 0.0f);
    GFXVertex3f(position.x, position.y + ycross, 0.0f);
    GFXEnd();
    GFXDisable(SMOOTH);
}

void SphereDisplay::DrawBackground(const Sensor& sensor, const ViewArea& radarView)
{
    // Split crosshair

    if (!radarView.IsActive())
        return;

    GFXColor groundColor = radarView.GetColor();

    float velocity = sensor.GetPlayer()->GetWarpVelocity().Magnitude();
    float logvelocity = 3.0; // std::log10(1000.0);
    if (velocity > 1000.0)
    {
        // Max logvelocity is log10(speed_of_light) = 10.46
        logvelocity = std::log10(velocity);
    }
    const float size = 3.0 * logvelocity; // [9; 31]
    const float xground = size / g_game.x_resolution;
    const float yground = size / g_game.y_resolution;
    Vector center = radarView.Scale(Vector(0.0, 0.0, 0.0));

    GFXEnable(SMOOTH);
    GFXLineWidth(1);
    GFXColorf(groundColor);
    GFXBegin(GFXLINE);
    GFXVertexf(Vector(center.x - 2.0 * xground, center.y, center.z));
    GFXVertexf(Vector(center.x - xground, center.y, center.z));
    GFXVertexf(Vector(center.x + 2.0 * xground, center.y, center.z));
    GFXVertexf(Vector(center.x + xground, center.y, center.z));
    GFXVertexf(Vector(center.x, center.y - 2.0 * yground, center.z));
    GFXVertexf(Vector(center.x, center.y - yground, center.z));
    GFXVertexf(Vector(center.x, center.y + 2.0 * yground, center.z));
    GFXVertexf(Vector(center.x, center.y + yground, center.z));
    GFXEnd();
    GFXDisable(SMOOTH);
}

} // namespace Radar
