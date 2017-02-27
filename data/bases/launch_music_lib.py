import VS
import vsrandom
GlobalMusic = VS.musicAddList ('launch.m3u')
def PlayLaunch ():
	if (vsrandom.random()<.02):
		VS.musicPlaySong('../music/launchjoke.ogg')
#		VS.micro_sleep(45000000)
	else:
		if (GlobalMusic!= -1):
			VS.musicPlayList (GlobalMusic)
#			VS.micro_sleep(6000000)
