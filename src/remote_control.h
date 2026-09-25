#ifndef __REMOTE_CONTROL_H__
#define __REMOTE_CONTROL_H__

// Optional scripted control of the game for automated end-to-end tests.
//
// When the environment variable VS_REMOTE_CONTROL names a Python file, the
// file is imported once Python is up and its tick(docked) function is called
// at the start of every frame (in space and in bases).  The VSRemote module
// gives it what a human has but the regular VS/Base modules do not: the
// links of the current base room, clicking them, and pressing the key
// commands bound in vegastrike.config (DockKey, JumpKey, ...).
namespace RemoteControl {
	void Tick(bool docked);
}

#endif
