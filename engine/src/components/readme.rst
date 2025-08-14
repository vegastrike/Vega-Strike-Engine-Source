============
LibComponent
============
************
Introduction
************
LibComponent is a Vegastrike library dealing with ship components or modules. While it provides some functionality dealing with upgrade/downgrade and damage/repair, it is mostly a place to store the attributes of various components. As such, it does not deal with things such as GUI, 3d graphics, audio, etc.

It is intended to be reasonably encapsulated in order to support unit testing.

LibComponent does rely on several lower level libraries including:

* LibResource - primarily for the Resource class.
* Logging - for logging
* LibDamage - for Hull, Armor and Shield classes.

*********
Component
*********
Component is the base class of all components. It provides the various shared attributes (mass, volume, price, name, etc.) and facilities (load, save, damage, repair, etc.) All components derive from it.

***************
EnergyConsumer
***************
An EnergyConsumer is any component that consumes any form of energy (fuel , ftl or energy). It is fairly straightforward but it's important to note it can be either *partial* or not. 

A partial consumer can consume less than the required quantity. In practice, this is almost all, as we don't really care if the drive has enough fuel. If we're out of fuel, next cycle (0.1 seconds later) we'll be out for good.

We use non-partial consumers in cases like jump drives, where the consumption is not stretched over time and we do care if the ship has enough of the consumed fuel.

There is support for infinite and no-source. Infinite always works. NoSource is not yet used.

***************
EnergyContainer
***************
An EnergyContainer stores fuel for the EnergyConsumer. There are three types - Fuel, Energy and FTL Energy.

The game's initial container is fuel, This is used by the reactor, drive and afterburner.

*******
Reactor
*******
The Reactor converts Fuel to Energy and FTL Energy. 

*****************
ComponentsManager
*****************
This class represents a ship. It has one of each component (hull, reactor, drive, etc.). It is also a stand-in for the subclass Unit. 

It also stored the money, as a static class variable. For now, we only have one player and that's the only person with money we care about. 

If we move to a Skyrim style vendors with limited money, we'd need to do something about it.

*********
LibDamage
*********
Hull, Shield and Armor also subclass DamageableLayer from LibDamage.

*********
CargoHold
*********
There are three cargo holds in the game (soon) - the actual cargo hold, the upgrade space (for components) and the hidden cargo hold (for contraband). The original game had one hold, which necessitated code to check if something is cargo or a component. 
