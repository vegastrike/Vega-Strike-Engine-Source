// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include <cmath>
#include <vector>
#include <boost/assign/std/vector.hpp>
#include "lin_time.h" // GetElapsedTime
#include "cmd/unit_generic.h"
#include "cmd/unit_util.h"
#include "gfxlib.h"
#include "viewarea.h"
#include "bubble_display.h"

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

BubbleDisplay::BubbleDisplay()
    : innerSphere(0.45),
      outerSphere(1.0),
      sphereZoom(1.0),
      radarTime(0.0),
      currentTargetMarkerSize(0.0),
      lastAnimationTime(0.0)
{
    using namespace boost::assign; // vector::operator+=
    explodeSequence += 0.0, 0.0001, 0.0009, 0.0036, 0.0100, 0.0225, 0.0441, 0.0784, 0.1296, 0.2025, 0.3025, 0.4356, 0.6084, 0.8281, 1.0, 0.8713, 0.7836, 0.7465, 0.7703, 0.8657, 1.0, 0.9340, 0.9595, 1.0, 0.9659, 1.0;
    implodeSequence += 1.0, 0.9999, 0.9991, 0.9964, 0.9900, 0.9775, 0.9559, 0.9216, 0.8704, 0.7975, 0.6975, 0.5644, 0.3916, 0.1719, 0.0, 0.1287, 0.2164, 0.2535, 0.2297, 0.1343, 0.0, 0.0660, 0.0405, 0.0, 0.0341, 0.0;
}

void BubbleDisplay::PrepareAnimation(const ZoomSequence& sequence)
{
    AnimationItem firstItem;
    firstItem.duration = 0.0;
    firstItem.sphereZoom = sequence[0];
    animation.push(firstItem);

    float duration = 2.0;
    for (ZoomSequence::size_type i = 1; i < sequence.size(); ++i)
    {
        AnimationItem item;
        item.duration = duration;
        item.sphereZoom = sequence[i];
        animation.push(item);
        duration = 0.05;
    }
}

void BubbleDisplay::OnDockEnd()
{
    PrepareAnimation(explodeSequence);
}

void BubbleDisplay::OnJumpBegin()
{
    PrepareAnimation(implodeSequence);
}

void BubbleDisplay::OnJumpEnd()
{
    PrepareAnimation(explodeSequence);
}

void BubbleDisplay::Animate()
{
    if (!animation.empty())
    {
        AnimationCollection::const_reference item = animation.front();
        if (radarTime > lastAnimationTime + item.duration)
        {
            sphereZoom = item.sphereZoom;
            animation.pop();
            lastAnimationTime = radarTime;
        }
    }
}

void BubbleDisplay::Draw(const Sensor& sensor, VSSprite *front, VSSprite *rear)
{
    radarTime += GetElapsedTime();

    SetViewArea(front, leftRadar);
    SetViewArea(rear, rightRadar);

    front->Draw();
    rear->Draw();

    Sensor::TrackCollection tracks = sensor.FindTracksInRange();

    Animate();

    GFXEnable(DEPTHTEST);
    GFXEnable(DEPTHWRITE);
    GFXEnable(SMOOTH);

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

    DrawBackground(leftRadar, currentTargetMarkerSize);
    DrawBackground(rightRadar, currentTargetMarkerSize);

    GFXPointSize(1);
    GFXDisable(DEPTHTEST);
    GFXDisable(DEPTHWRITE);
    GFXDisable(SMOOTH);
}

void BubbleDisplay::DrawTrack(const Sensor& sensor,
                              const ViewArea& radarView,
                              const Track& track)
{
    GFXColor color = sensor.GetColor(track);

    Vector position = track.GetPosition();
    if (position.z < 0)
        position.z = -position.z;

    float magnitude = position.Magnitude();
    float scaleFactor = 0.0; // [0; 1] where 0 = border, 1 = center
    float maxRange = sensor.GetMaxRange();
    if (magnitude <= maxRange)
    {
        // [innerSphere; outerSphere]
        scaleFactor = (outerSphere - innerSphere) * ((maxRange - magnitude) / maxRange);
        magnitude /= (1.0 - scaleFactor);
    }

    if (sensor.InsideNebula())
    {
        magnitude /= (1.0 - 0.04 * Jitter(0.0, 1.0));
    }
    Vector scaledPosition = sphereZoom * Vector(-position.x, position.y, position.z) / magnitude;

    Vector head = radarView.Scale(scaledPosition);

    GFXColor headColor = color;

    headColor.a *= 0.2 + scaleFactor * (1.0 - 0.2); // [0;1] => [0.1;1]
    if (sensor.UseThreatAssessment())
    {
        float dangerRate = GetDangerRate(sensor.IdentifyThreat(track));
        if (dangerRate > 0.0)
        {
            // Blinking blip
            headColor.a *= cosf(dangerRate * radarTime);
        }
    }

    // Fade out dying ships
    if (track.IsExploding())
    {
        headColor.a *= (1.0 - track.ExplodingProgress());
    }

    float trackSize = std::max(1.0f, std::log10(track.GetSize()));
    if (track.GetType() != Track::Type::Cargo)
        trackSize += 1.0;

    GFXColorf(headColor);
    if (sensor.IsTracking(track))
    {
        currentTargetMarkerSize = trackSize;
        DrawTargetMarker(head, trackSize);
    }

    const bool isNebula = (track.GetType() == Track::Type::Nebula);
    const bool isEcmActive = track.HasActiveECM();
    if (isNebula || isEcmActive)
    {
        // Vary size between 50% and 150%
        trackSize *= Jitter(0.5, 1.0);
    }

    GFXPointSize(trackSize);
    GFXBegin(GFXPOINT);
    GFXVertexf(head);
    GFXEnd();
}

void BubbleDisplay::DrawTargetMarker(const Vector& position, float trackSize)
{
    // Split octagon
    float size = 3.0 * std::max(trackSize, 3.0f);
    float xsize = size / g_game.x_resolution;
    float ysize = size / g_game.y_resolution;
    GFXLineWidth(1);
    GFXBegin(GFXLINESTRIP);
    GFXVertex3f(position.x - xsize / 2, position.y - ysize, position.z);
    GFXVertex3f(position.x - xsize, position.y - ysize / 2, position.z);
    GFXVertex3f(position.x - xsize, position.y + ysize / 2, position.z);
    GFXVertex3f(position.x - xsize / 2, position.y + ysize, position.z);
    GFXEnd();
    GFXBegin(GFXLINESTRIP);
    GFXVertex3f(position.x + xsize / 2, position.y - ysize, position.z);
    GFXVertex3f(position.x + xsize, position.y - ysize / 2, position.z);
    GFXVertex3f(position.x + xsize, position.y + ysize / 2, position.z);
    GFXVertex3f(position.x + xsize / 2, position.y + ysize, position.z);
    GFXEnd();
}

void BubbleDisplay::DrawBackground(const ViewArea& radarView, float trackSize)
{
    GFXColor groundColor = radarView.GetColor();

    // Split octagon
    float size = 3.0 * std::max(trackSize, 3.0f);
    float xground = size / g_game.x_resolution;
    float yground = size / g_game.y_resolution;
    Vector center = radarView.Scale(Vector(0.0, 0.0, 0.0));

    GFXColorf(groundColor);
    GFXLineWidth(1);
    GFXBegin(GFXLINESTRIP);
    GFXVertex3f(center.x - xground, center.y - yground / 2, center.z);
    GFXVertex3f(center.x - xground / 2, center.y - yground, center.z);
    GFXVertex3f(center.x + xground / 2, center.y - yground, center.z);
    GFXVertex3f(center.x + xground, center.y - yground / 2, center.z);
    GFXEnd();
    GFXBegin(GFXLINESTRIP);
    GFXVertex3f(center.x - xground, center.y + yground / 2, center.z);
    GFXVertex3f(center.x - xground / 2, center.y + yground, center.z);
    GFXVertex3f(center.x + xground / 2, center.y + yground, center.z);
    GFXVertex3f(center.x + xground, center.y + yground / 2, center.z);
    GFXEnd();
}

} // namespace Radar
