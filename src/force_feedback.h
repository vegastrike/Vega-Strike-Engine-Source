
#ifndef _FORCE_FEEDBACK_H_
#define _FORCE_FEEDBACK_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdio.h>

#if HAVE_FORCE_FEEDBACK
#include <linux/input.h>

#define BITS_PER_LONG (sizeof(long) * 8)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)    ((array[LONG(bit)] >> OFF(bit)) & 1)

#define N_EFFECTS 4

#endif // have_force_feedback

class ForceFeedback {
public:
  ForceFeedback();

  bool haveFF();

  void playDurationEffect(uint eff_nr,bool activate);
  void playShortEffect(uint eff_nr);

 private:
  bool have_ff;

#if HAVE_FORCE_FEEDBACK
 private:
  void init();

  void playEffect(uint eff_nr);
  void stopEffect(uint eff_nr);

  int device_nr;

  struct ff_effect effects[N_EFFECTS];
  struct input_event play, stop;
  int ff_fd;
  unsigned long features[4];
  int n_effects;	/* Number of effects the device can play at the same time */

  double eff_last_time[N_EFFECTS];

  double min_effect_time;

  bool is_played[N_EFFECTS];

#endif // have_force_feedback
};

#endif // _FORCE_FEEDBACK_H_
