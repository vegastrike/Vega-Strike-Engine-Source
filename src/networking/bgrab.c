/*

    Library: Threaded BTTV Grabbing with triple buffering

    The contents of this file are subject to the Mozilla Public
    License Version 1.1 (the "License"); you may not use this file
    except in compliance with the License. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/
    
    Software distributed under the License is distributed on an "AS
    IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
    implied. See the License for the specific language governing
    rights and limitations under the License.
   
    The Original Code is the libbgrab framegrabbing library.
    
    The Initial Developer of the Original Code is A. Schiffler.  
    Portions created by A. Schiffler are Copyright (C) 1999-2000
    A. Schiffler.  All Rights Reserved.
    
    Contributor(s): A. Schiffler <aschiffler@home.com>
                                                                                                                dnl 
    Alternatively, the contents of this file may be used under the
    terms of the GNU General Public License Version 2 or later (the
    "GPL"), in which case the provisions of the GPL are applicable 
    instead of those above.  If you wish to allow use of your 
    version of this file only under the terms of the GPL and not to
    allow others to use your version of this file under the MPL,
    indicate your decision by deleting the provisions above and
    replace them with the notice and other provisions required by
    the GPL.  If you do not delete the provisions above, a recipient
    may use your version of this file under either the MPL or the
    GPL.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    To contact the author try:
    |  Andreas Schiffler                    aschiffler@home.com  |
    |  Senior Systems Engineer  -  Deskplayer.Com Inc., Buffalo  |
    |  4707 Eastwood Cres., Niagara Falls, Ont  L2E 1B4, Canada  |
    |  +1-905-371-3652 (private)  -  +1-905-371-8834 (work/fax)  |

*/

#include "bgrab.h"

/* NOTE: Requires BTTV compatible framegrabber card for full functionality. */
/*       Might work with other cards/input devices.                         */

/* Channel Frequencies */

#include "frequencies.c"

/* Routine to calculate the frequency for a channel */

int getFrequency(int region, int index)
{
 if ((region>=0) && (region<NUM_CHANNEL_LISTS)) {
  if ((index>=0) && (index<chanlists[region].count)) {  
   fprintf(stderr,"Region %s : Channel %s\n",chanlists[region].name,chanlists[region].list[index].name);
   return(chanlists[region].list[index].freq);
  }
 }  
 return(0);
}

/* Routine to get the time as float */
double timeGet(void)
{
  struct timeval tv;
  double curtime;
  
  /* get wallclock time */
  gettimeofday(&tv, NULL);
  curtime=(double)tv.tv_sec+1.0e-6*(double)tv.tv_usec;
  return (curtime); 
}

/* Call for each frame to get FPS calculation */
void calc_fps(struct fgdevice *fg)
{
 double curtime;
 
 /* Check interval */
 if (fg->fps_update_interval<1) {
  fg->fps=0.0;
 } else {
  if ( (fg->framecount<0) || (fg->framecount>=fg->fps_update_interval)) {
   /* Initialize counter */
   fg->framecount=0;
   fg->lasttime=timeGet();
   fg->fps=0.0;
  } else {
   fg->framecount++;
   if (fg->framecount==fg->fps_update_interval) {
    curtime=timeGet();
    fg->fps=(double)fg->framecount/(curtime-fg->lasttime);
    fg->lasttime=curtime;
    fg->framecount=0;
   }
  }
 }  
}

/* Set the interval (in frames) after which the fps counter is updated */
void fg_set_fps_interval(struct fgdevice *fg, int interval)
{
 fg->fps_update_interval=interval;
 fg->framecount=-1;
}

/* Return fps */
double fg_get_fps(struct fgdevice *fg)
{
 return(fg->fps);
}
 
/* Prints device specific information to stderr */
int fg_print_info(struct fgdevice *fg)
{
 struct video_capability video_caps;
 struct video_channel video_chnl;
 struct video_audio video_aud;
 char BooleanText[2][4]={"YES\0","NO \0"};
 int max_tuner;
 int i;
 
 if (ioctl (fg->video_dev, VIDIOCGCAP, &video_caps) == -1) {
  perror ("ioctl (VIDIOCGCAP)");
  return(-1);
 } else {
  
  /* List capabilities */
  
  fprintf (stderr,"Device Info: ");
  fprintf (stderr,"%s\n",video_caps.name);

  fprintf (stderr," Can capture ... : %s  ",BooleanText[((video_caps.type & VID_TYPE_CAPTURE) == 0)]);
  fprintf (stderr," Can clip ...... : %s  ",BooleanText[((video_caps.type & VID_TYPE_CLIPPING) == 0)]);
  fprintf (stderr," Channels ...... : %i\n",video_caps.channels);

  fprintf (stderr," Has tuner ..... : %s  ",BooleanText[((video_caps.type & VID_TYPE_TUNER) == 0)]);
  fprintf (stderr," Ovl overwrites  : %s  ",BooleanText[((video_caps.type & VID_TYPE_FRAMERAM) == 0)]);
  fprintf (stderr," Audio devices . : %i\n",video_caps.audios);

  fprintf (stderr," Has teletext .. : %s  ",BooleanText[((video_caps.type & VID_TYPE_TELETEXT) == 0)]);
  fprintf (stderr," Can scale ..... : %s  ",BooleanText[((video_caps.type & VID_TYPE_SCALES) == 0)]);
  fprintf (stderr," Width min-max . : %i-%i\n",video_caps.minwidth,video_caps.maxwidth);

  fprintf (stderr," Can overlay ... : %s  ",BooleanText[((video_caps.type & VID_TYPE_OVERLAY) == 0)]);
  fprintf (stderr," Monochrome .... : %s  ",BooleanText[((video_caps.type & VID_TYPE_MONOCHROME) == 0)]);
  fprintf (stderr," Height min-max  : %i-%i\n",video_caps.minheight,video_caps.maxheight);
  
  fprintf (stderr," Can chromakey . : %s  ",BooleanText[((video_caps.type & VID_TYPE_CHROMAKEY) == 0)]);
  fprintf (stderr," Can subcapture  : %s\n",BooleanText[((video_caps.type & VID_TYPE_SUBCAPTURE) == 0)]);
  
  /* List input channels */
  
  max_tuner=0;
  for (i=0; i<video_caps.channels; i++) {
   video_chnl.channel = i;
   if (ioctl (fg->video_dev, VIDIOCGCHAN, &video_chnl) == -1) {
    perror ("ioctl (VIDIOCGCHAN)");
    return(-1);
   } else {
    fprintf (stderr," Channel %i: %s ",i,video_chnl.name);
    if ((video_chnl.type & VIDEO_TYPE_TV)==0) {
     fprintf (stderr,"(camera input)\n");
    } else {
     fprintf (stderr,"(TV input)\n");
    } 
    fprintf (stderr,"  Tuners : %i  ",video_chnl.tuners);
    if (video_chnl.tuners>max_tuner) max_tuner=video_chnl.tuners; 
    fprintf (stderr,"  Has audio : %s\n",BooleanText[((video_chnl.flags & VIDEO_VC_AUDIO) == 0)]);
   }
  }
  
  /* Audio channels */
  for (i=0; i<video_caps.audios; i++) {
   video_aud.audio=i;
   if (ioctl (fg->video_dev, VIDIOCGAUDIO, &video_aud) == -1) {
    perror ("ioctl (VIDIOCGAUDIO)");
    return(-1);
   } else {
    fprintf (stderr," Audio %i: %s\n",i,video_aud.name);
    fprintf (stderr,"  Controllable: ");
    if ((video_aud.flags & VIDEO_AUDIO_MUTABLE) != 0) fprintf (stderr,"Muting ");
    if ((video_aud.flags & VIDEO_AUDIO_VOLUME) != 0) fprintf (stderr,"Volume ");
    if ((video_aud.flags & VIDEO_AUDIO_BASS) != 0) fprintf (stderr,"Bass ");
    if ((video_aud.flags & VIDEO_AUDIO_TREBLE) != 0) fprintf (stderr,"Treble ");
    fprintf (stderr,"\n");
   } 
  }

  /* Retrieve sizes and offsets */
  if (ioctl (fg->video_dev, VIDIOCGMBUF, &fg->vid_mbuf) == -1) {
   perror ("ioctl (VIDIOCGMBUF)");
   return(-1);
  }
  /* Print memory info */
  fprintf (stderr,"Memory Map of %i frames: %i bytes\n",fg->vid_mbuf.frames,fg->vid_mbuf.size);
  for (i=0; i<fg->vid_mbuf.frames; i++) {
   fprintf (stderr," Offset of frame %i: %i\n",i,fg->vid_mbuf.offsets[i]);
  }
 }
 
 return(0);
}

/* Read card settings */
int fg_get_setting(struct fgdevice *fg, int which_setting)
{
 struct video_picture video_pict;
 
 if (ioctl (fg->video_dev, VIDIOCGPICT, &video_pict) == -1) {
  perror ("ioctl (VIDIOCGPICT)");
  return(-1);
 } else {
  switch (which_setting) {
   case SETTING_BRIGHTNESS:
    return(video_pict.brightness);
    break;
   case SETTING_HUE:
    return(video_pict.hue);
    break;
   case SETTING_COLOUR:
    return(video_pict.colour);
    break;
   case SETTING_CONTRAST:
    return(video_pict.contrast);
    break;
  } 
 }
 return(-1);
}

/* Adjust card settings */
int fg_set_setting(struct fgdevice *fg, int which_setting, int value)
{
 struct video_picture video_pict;
 
 if (ioctl (fg->video_dev, VIDIOCGPICT, &video_pict) == -1) {
  perror ("ioctl (VIDIOCGPICT)");
  return(-1);
 } else {
  switch (which_setting) {
   case SETTING_BRIGHTNESS:
    video_pict.brightness=value;
    break;
   case SETTING_HUE:
    video_pict.hue=value;
    break;
   case SETTING_COLOUR:
    video_pict.colour=value;
    break;
   case SETTING_CONTRAST:
    video_pict.contrast=value;
    break;
  } 
  if (ioctl (fg->video_dev, VIDIOCSPICT, &video_pict) == -1) {
   perror ("ioctl (VIDIOCSPICT)");
   return(-1);
  } 
 }
 return(0);
}

/* Set the input channel and videomode on card */
int fg_set_channel(struct fgdevice *fg, int channel, int videomode)
{
 struct video_capability video_cap;
 struct video_channel video_chan;
 struct video_tuner tuner;
  
 /* Query for number of channels */
 if (ioctl (fg->video_dev, VIDIOCGCAP, &video_cap) == -1) {
  perror ("ioctl (VIDIOCGCAP)");
  return(-1);
 } 

 /* Clip channel to allowed range */
 if (channel<0) {
  channel=0;
 } else {
  video_cap.channels--;
  if (channel>video_cap.channels) channel=video_cap.channels;
 }

 /* Set channel number */
 video_chan.channel=0; 
 if (ioctl (fg->video_dev, VIDIOCGCHAN, &video_chan) == -1) {
  perror ("ioctl (VIDIOCGCHAN)");
  return(-1);
 } else {
  video_chan.channel=channel; 
  if (ioctl (fg->video_dev, VIDIOCSCHAN, &video_chan) == -1) {
   perror ("ioctl (VIDIOCSCHAN)");
   fprintf (stderr,"Channel was %i\n",channel);
   return(-1);
  } 
 }

 /* Set videomode */
 tuner.tuner=0;
 if (channel==CHANNEL_TUNER) {
  if (ioctl (fg->video_dev, VIDIOCGTUNER, &tuner) == -1) {
   perror ("ioctl (VIDIOCGTUNER)");
   return(-1);
  }
 }
 tuner.mode=videomode;
 if (ioctl (fg->video_dev, VIDIOCSTUNER, &tuner) == -1) {
  perror ("ioctl (VIDIOCSTUNER)");
  return(-1);
 }

 return(0);
}

/* Set the tuner frequency */
int fg_set_frequency(struct fgdevice *fg, int region, int index)
{
 int frequency;
 unsigned long adjusted_frequency;
 
 frequency=getFrequency (region, index);
 if (frequency>0) {
  adjusted_frequency=frequency*16/1000;
  if (ioctl (fg->video_dev, VIDIOCSFREQ, &adjusted_frequency) == -1) {
   perror ("ioctl (VIDIOCSFREQ)");
   return(-1);
  }
 }
 
 return(0); 
}

/* Self running thread that grabbs all images */
void * grab_images_thread (struct fgdevice *fg)
{
 int in_loop;
 
 /* Loop ... grabbing images */
 in_loop=1;
 while (in_loop) {

  /* Lock buffers and state */
  pthread_mutex_lock(&fg->buffer_mutex);

  /* Advance grab-frame number */
  fg->current_grab_number=((fg->current_grab_number + 1) % 2);

  /* Unlock buffers and state */
  pthread_mutex_unlock(&fg->buffer_mutex);

  /* Wait for next image in the sequence to complete grabbing */
  if (ioctl (fg->video_dev, VIDIOCSYNC, &fg->vid_mmap[fg->current_grab_number]) == -1) {
   perror ("VIDIOCSYNC");
   return((void *)&error_exit_status);
  }

  /* Issue new grab command for this buffer */
  if (ioctl (fg->video_dev, VIDIOCMCAPTURE, &fg->vid_mmap[fg->current_grab_number]) == -1) {
   perror ("VIDIOCMCAPTURE");
   return((void *)&error_exit_status);
  }

  /* Lock buffers and state */
  pthread_mutex_lock(&fg->buffer_mutex);

  /* Announce that a new frame is available */
  fg->have_new_frame=1;

  /* Get loop state */
  in_loop=fg->grabbing_active;

  /* Unlock buffers and state */
  pthread_mutex_unlock(&fg->buffer_mutex);

  /* Signal potentially waiting main thread */
  pthread_cond_signal(&fg->buffer_cond);
   
 }

 
 pthread_exit(NULL);
}

/* Initialize grabber and start grabbing-thread */
int fg_start_grab_image (struct fgdevice *fg, int width, int height, int format)
{
 int i;
 int depth;
 
 /* Store variables in structure */
 fg->width=width;
 fg->height=height;
 fg->format=format;
 
 /* Retrieve buffer size and offsets */
 if (ioctl (fg->video_dev, VIDIOCGMBUF, &fg->vid_mbuf) == -1) {
  perror ("ioctl (VIDIOCGMBUF)");
  return(-1);
 }
 
 /* Map grabber memory into user space */
 fg->video_map = mmap (0, fg->vid_mbuf.size, PROT_READ|PROT_WRITE,MAP_SHARED,fg->video_dev,0);
 if ((unsigned char *)-1 == (unsigned char *)fg->video_map) {
  perror ("mmap()");
  return(-1);
 }

 /* Determine depth from format */
 depth=2;
 switch (format) {
  case VIDEO_PALETTE_GREY:
   depth=1;
   break;
  case VIDEO_PALETTE_RGB565:
  case VIDEO_PALETTE_YUV422P:
  case VIDEO_PALETTE_YUV420P:
   depth=2;
   break;
  case VIDEO_PALETTE_RGB24:
   depth=3;
   break;
  case VIDEO_PALETTE_RGB32:
   depth=4;
   break;
  default:
   fprintf (stderr,"Unknown format.\n");
   return(-1);
   break; 
 }

 /* Generate mmap records */
 for (i=0; i<2; i++) {
  fg->vid_mmap[i].format = format;
  fg->vid_mmap[i].frame  = i;
  fg->vid_mmap[i].width  = width;
  fg->vid_mmap[i].height = height;
 }
  
 /* Calculate framebuffer size and allocate memory for user frames */
 fg->image_pixels=width*height;
 fg->image_size=fg->image_pixels*depth;
 if ((fg->current_image=malloc(fg->image_size))==NULL) {
  perror ("malloc()");
  return(-1);
 }
   
 /* Initiate capture for frames */
 for (i=0; i<2; i++) {
  /* Sync old images in the sequence to complete grabbing */
  ioctl (fg->video_dev, VIDIOCSYNC, &fg->vid_mmap[i]);
  /* Start capture */
  if (ioctl (fg->video_dev, VIDIOCMCAPTURE, &fg->vid_mmap[i]) == -1) {
   perror ("VIDIOCMCAPTURE");
   return(-1);
  }
 }
 
 /* Reset grab counter variables */
 fg->current_grab_number=0;
 fg->totalframecount=0;
 fg->have_new_frame=0;
  
 /* Initialize mutex */
 if (pthread_mutex_init(&fg->buffer_mutex, NULL)==-1) {
  perror("pthread_mutex_init()");
  return(-1);
 } 

 /* Initialize conditional */
 if (pthread_cond_init(&fg->buffer_cond, NULL)==-1) {
  perror("pthread_cond_init()");
  return(-1);
 } 

 /* Set thread loop flag */
 fg->grabbing_active=1;

 /* Start grabbing thread */
 if (pthread_create(&fg->grab_thread, NULL,(void *)grab_images_thread,(void *)fg)==-1) {
  perror ("pthread_create()");
  return(-1);
 } 

 
 return(0);
}

/* Stop grabbing thread */
int fg_stop_grab_image(struct fgdevice *fg)
{
 /* Set flag for thread to finish */
 fg->grabbing_active=0;

 /* Wait for grabbing thread to exit */
 if (pthread_join(fg->grab_thread,NULL)==-1) {
  perror("pthread_join()");
  return(-1);
 }

 /* Free image buffers */
 if (fg->current_image) free(fg->current_image);
 fg->current_image=NULL;

 /* Unmap memory */
 if (munmap (fg->video_map, fg->vid_mbuf.size) == -1) {
  perror ("munmap()");
  return(-1);
 }

 return(0);
}

/* Get last image, lock to be processed */ 
void * fg_get_next_image(struct fgdevice *fg)
{
 /* Check if we are grabbing */
 if (fg->grabbing_active) {

  /* Increase counter */
  fg->totalframecount++;
   
  /* Lock buffers and state */
  pthread_mutex_lock(&fg->buffer_mutex);
  
  /* Check if a new frame is available */
  if (!fg->have_new_frame) {
   /* Wait for signal - unlocking mutex in the process */
   pthread_cond_wait(&fg->buffer_cond,&fg->buffer_mutex);
  }

  /* Copy frame from other-than-grab framebuffer */
  memcpy (fg->current_image,&fg->video_map[fg->vid_mbuf.offsets[(fg->current_grab_number+1) % 2]],fg->image_size);

  /* Mark frame as used */
  fg->have_new_frame=0;
  
  /* Unlock buffers and state */
  pthread_mutex_unlock(&fg->buffer_mutex);
 
  /* FPS calculation */
  calc_fps(fg);
  
 } else {
  /* No grabbing */
  fg->current_image=NULL;
 }
  
 /* Return pointer */
 return (fg->current_image);
}

/* Open framegrabber device */
int fg_open_device (struct fgdevice *fg, const char *devicename)
{                                                                 
 int i,r,g,b;
 unsigned short *curypos;
 unsigned char *currgbpos;
 unsigned short us;

 /* Open the video4linux device */
 fg->video_dev = open (devicename, O_RDWR);
 if (fg->video_dev == -1) {
  perror("open()");
  return (-1);
 }
 
 /* Reset variables */
 fg->grabbing_active=0;
 fg->width=-1;
 fg->height=-1;
 fg->format=-1;
 fg->input=-1;
 fg->image_size=-1;
 fg->image_pixels=-1;
 fg->fps_update_interval=-1;
 
 /* Initialize color space conversion tables */

 /* Y 8bit to RGB 16bit */
 if ((fg->y8_to_rgb565=malloc(256*sizeof(short)))==NULL) {
  perror("malloc");
 }
 curypos=fg->y8_to_rgb565;
 for (i=0; i<256; i++) {  
  *curypos=((i >> 3) << 11 ) | ((i >> 2) << 5) | (i >> 3);
  curypos++; 
 } 

 /* RGB 16 bit to Y 8bit */
 if ( (fg->rgb565_to_y8=malloc(64*1024*sizeof(char)))==NULL) {
  perror("malloc");
 }
 currgbpos=fg->rgb565_to_y8;  
 for (i=0; i<(64*1024); i++) {
  us=(unsigned short)i;
  r=(us >> 11);
  g=((us >> 5) & 63);
  b=(us & 31);
  *currgbpos=(r*2450+g*2404+b*934)/1024;          
  currgbpos++;
 }

 return(0);
}

/* Close framegrabber device */
int fg_close_device (struct fgdevice *fg)
{
 /* Free allocated memory */
 if (fg->current_image) free(fg->current_image);
 if (fg->y8_to_rgb565) free(fg->y8_to_rgb565);
 if (fg->rgb565_to_y8) free(fg->rgb565_to_y8);

 /* Close video4linux device */
 return(close(fg->video_dev));
}
