#include "vegastrike.h"
#include "vs_path.h"
#include "vs_globals.h"

#include "config_xml.h"
#include "cmd/script/mission.h"

#include "force_feedback.h"

#define FF_DOIT 1


ForceFeedback::ForceFeedback(){
#if HAVE_FORCE_FEEDBACK
  init();
#else
  have_ff=false;
  printf("Force feedback support disabled when compiled\n");
#endif
};

bool ForceFeedback::haveFF(){
  return have_ff;
}

void ForceFeedback::playDurationEffect(uint eff_nr,bool activate){
  if(!have_ff){
    return;
  }
#if HAVE_FORCE_FEEDBACK
  if(activate==true && is_played[eff_nr]==false){
    playEffect(eff_nr);
    is_played[eff_nr]=true;
  }
  else if(activate==false && is_played[eff_nr]==true){
    stopEffect(eff_nr);
    is_played[eff_nr]=false;
  }
#endif
}

void ForceFeedback::playShortEffect(uint eff_nr){
  if(!have_ff){
    return;
  }

#if HAVE_FORCE_FEEDBACK
  double nowtime=mission->getGametime();
  
  if(nowtime<eff_last_time[eff_nr]+min_effect_time){
    // to make sure that effects aren't done too fast after another
    return;
  }

  playEffect(eff_nr);

  eff_last_time[eff_nr]=nowtime;
#endif
}

#if HAVE_FORCE_FEEDBACK

void ForceFeedback::playEffect(uint eff_nr){
#if FF_DOIT
  play.type = EV_FF;
  play.code = effects[eff_nr].id;
  play.value = 1;
  
  if (write(ff_fd, (const void*) &play, sizeof(play)) == -1) {
    perror("ff: Play effect");
    have_ff=false;
    close(ff_fd);
    return;
  }
#endif

  printf("played effect nr %d\n",eff_nr);
}

void ForceFeedback::stopEffect(uint eff_nr){
#if FF_DOIT
  stop.type = EV_FF;
  stop.code = effects[eff_nr].id;
  stop.value = 0;
  
  if (write(ff_fd, (const void*) &stop, sizeof(stop)) == -1) {
    perror("ff: stop effect");
    have_ff=false;
    close(ff_fd);
    return;
  }
#endif

  printf("stopped effect nr %d\n",eff_nr);
}


void ForceFeedback::init(){
  have_ff=XMLSupport::parse_bool(vs_config->getVariable("joystick","force_feedback","false"));

  if(!have_ff){
    printf("force feedback disabled in config file\n");
    return;
  }

  device_nr=atoi(vs_config->getVariable("joystick","ff_device","0").c_str());

  char devname[200];
  sprintf(devname,"/dev/input/event%d",device_nr);

  ff_fd=open(devname,O_RDWR);
  if(ff_fd==-1){
    perror("error while opening force feedback device");
    have_ff=false;
    return;
  }
  
  printf("Device %s opened\n", devname);
	
	/* Query device */
	if (ioctl(ff_fd, EVIOCGBIT(EV_FF, sizeof(unsigned long) * 4), features) == -1) {
		perror("ff:Ioctl query");
		have_ff=false;
		close(ff_fd);
		return;
	}

	printf("Axes query: ");

	if (test_bit(ABS_X, features)) printf("Axis X ");
	if (test_bit(ABS_Y, features)) printf("Axis Y ");
	if (test_bit(ABS_WHEEL, features)) printf("Wheel ");

	printf("\nEffects: ");

	if (test_bit(FF_CONSTANT, features)) printf("Constant ");
	if (test_bit(FF_PERIODIC, features)) printf("Periodic ");
	if (test_bit(FF_SPRING, features)) printf("Spring ");
	if (test_bit(FF_FRICTION, features)) printf("Friction ");
	if (test_bit(FF_RUMBLE, features)) printf("Rumble ");

	printf("\nNumber of simultaneous effects: ");

	if (ioctl(ff_fd, EVIOCGEFFECTS, &n_effects) == -1) {
		perror("Ioctl number of effects");
		have_ff=false;
		close(ff_fd);
		return;
	}

	printf("nr_effects: %d\n", n_effects);

	if(n_effects<N_EFFECTS){
	  printf("not enough effects in device - ff disabled\n");
	  close(ff_fd);
	  have_ff=false;
	  return;
	}
	/* download a constant effect */
	effects[1].type = FF_CONSTANT;
	effects[1].id = -1;
	effects[1].u.constant.level = 0x2000;	/* Strength : 25 % */
	effects[1].direction = 0x6000;	/* 135 degrees */
	effects[1].u.constant.envelope.attack_length = 0x100;
	effects[1].u.constant.envelope.attack_level = 0;
	effects[1].u.constant.envelope.fade_length = 0x100;
	effects[1].u.constant.envelope.fade_level = 0;
	effects[1].trigger.button = 0;
	effects[1].trigger.interval = 0;
	effects[1].replay.length = 2000;  /* 2 seconds */
	effects[1].replay.delay = 0;

	/* download a periodic sinusoidal effect */
	effects[0].type = FF_PERIODIC;
	effects[0].id = -1;
	effects[0].u.periodic.waveform = FF_SINE;
	effects[0].u.periodic.period = 0.4*0x100;	/* 0.1 second */
	effects[0].u.periodic.magnitude = 0x6000;	/* 0.5 * Maximum magnitude */
	effects[0].u.periodic.offset = 0;
	effects[0].u.periodic.phase = 0;
	effects[0].direction = 0x000;	/* Along X axis */
	effects[0].u.periodic.envelope.attack_length = 0x100;
	effects[0].u.periodic.envelope.attack_level = 0;
	effects[0].u.periodic.envelope.fade_length = 0x100;
	effects[0].u.periodic.envelope.fade_level = 0;
	effects[0].trigger.button = 0;
	effects[0].trigger.interval = 0;
	effects[0].replay.length = 500;  /* 1 seconds */
	effects[0].replay.delay = 0;

	/* download an condition spring effect */
	effects[2].type = FF_SPRING;
	effects[2].id = -1;
	effects[2].u.condition[0].right_saturation = 0x7fff;
	effects[2].u.condition[0].left_saturation = 0x7fff;
	effects[2].u.condition[0].right_coeff = 0x2000;
	effects[2].u.condition[0].left_coeff = 0x2000;
	effects[2].u.condition[0].deadband = 0x0;
	effects[2].u.condition[0].center = 0x0;
	effects[2].u.condition[1] = effects[2].u.condition[0];
	effects[2].trigger.button = 0;
	effects[2].trigger.interval = 0;
	effects[2].replay.length = 0xFFFF;
	effects[2].replay.delay = 0;


	/* download an condition damper effect */
	effects[3].type = FF_DAMPER;
	effects[3].id = -1;
	effects[3].u.condition[0].right_saturation = 0x7fff;
	effects[3].u.condition[0].left_saturation = 0x7fff;
	effects[3].u.condition[0].right_coeff = 0x2000;
	effects[3].u.condition[0].left_coeff = 0x2000;
	effects[3].u.condition[0].deadband = 0x0;
	effects[3].u.condition[0].center = 0x0;
	effects[3].u.condition[1] = effects[3].u.condition[0];
	effects[3].trigger.button = 0;
	effects[3].trigger.interval = 0;
	effects[3].replay.length = 2000;  /* 2 seconds */
	effects[3].replay.delay = 0;

	for(int i=0;i<N_EFFECTS;i++){
	  
	  if (ioctl(ff_fd, EVIOCSFF, &effects[i]) == -1) {
	    perror("error while uploading effect");
	    have_ff=false;
	    close(ff_fd);
	    return;
	  }
	  eff_last_time[i]=0;
	}

	min_effect_time=0.3;
}


#endif
