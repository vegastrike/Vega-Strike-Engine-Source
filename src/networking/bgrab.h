/*

    Include file: Defines libbgrab library - bgrab.h

    The contents of this file are subject to the Mozilla Public
    License Version 1.1 (the "License"); you may not use this file
    except in compliance with the License. You may obtain a copy of
    the License at http://www.mozilla.org/MPL/
    
    Software distributed under the License is distributed on an "AS
    IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
    implied. See the License for the specific language governing
    rights and limitations under the License.
   
    The Original Code is the libbgrab framegrabber library.
    
    The Initial Developer of the Original Code is A. Schiffler.  
    Portions created by A. Schiffler are Copyright (C) 1999-2000
    A. Schiffler.  All Rights Reserved.
    
    Contributor(s): Andreas Schiffler <aschiffler@home.com>
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


/* Framegrabber device structure */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/types.h>

#include <linux/videodev.h>

#include <pthread.h>

/* ------ Defines used for framegrabber setup */

/* SETTING_ used for fg_get_setting() and fg_set_setting() */

#define SETTING_BRIGHTNESS	0
#define SETTING_HUE		1
#define SETTING_COLOUR		2
#define SETTING_CONTRAST	3

/* CHANNEL_ used for fg_set_channel() */

#define CHANNEL_TUNER		0
#define CHANNEL_COMPOSITE	1
#define CHANNEL_SVIDEO		2

/* VIDEOMODE_ used for fg_set_channel() */

#define VIDEOMODE_PAL		VIDEO_MODE_PAL			
#define VIDEOMODE_NTSC		VIDEO_MODE_NTSC
#define VIDEOMODE_SECAM		VIDEO_MODE_SECAM

/* FORMAT_ used by fg_start_grab_image() */

#define FORMAT_GREY		VIDEO_PALETTE_GREY
#define FORMAT_RGB565		VIDEO_PALETTE_RGB565
#define FORMAT_RGB24		VIDEO_PALETTE_RGB24
#define FORMAT_RGB32		VIDEO_PALETTE_RGB32
#define FORMAT_YUV422P		VIDEO_PALETTE_YUV422P
#define FORMAT_YUV420P		VIDEO_PALETTE_YUV420P

/* REGION_ used by fg_set_frequency() */

#define REGION_NTSC_BROADCAST		0
#define REGION_NTSC_CABLE		1
#define REGION_NTSC_CABLE_HRC		2
#define REGION_NTSC_BROADCAST_JAPAN	3
#define REGION_NTSC_CABLE_JAPAN		4
#define REGION_PAL_EUROPE		5
#define REGION_PAL_EUROPE_EAST		6
#define REGION_PAL_ITALY		7
#define REGION_PAL_NEWZEALAND		8
#define REGION_PAL_AUSTRALIA		9
#define REGION_PAL_IRELAND		10

/* ------- Defines for internal use  */

#define IMAGE_BUFFER_EMPTY	0
#define IMAGE_BUFFER_FULL	1
#define IMAGE_BUFFER_INUSE	2

static const int error_exit_status = -1; 

/* Channel frequency tables */

#define NUM_CHANNEL_LISTS	11

struct CHANLIST {
    char *name;
    int   freq;
};

struct CHANLISTS {
    char             *name;
    struct CHANLIST  *list;
    int               count;
};

#define CHAN_COUNT(x) (sizeof(x)/sizeof(struct CHANLIST))


/* ------- Framegrabber device structure */

struct fgdevice {
			int video_dev;

			int width;
			int height;
			int input;
			int format;

			struct video_mmap vid_mmap[2];
			int current_grab_number;
			struct video_mbuf vid_mbuf;
			char *video_map;
			int grabbing_active;
			int have_new_frame;
			void *current_image;
			pthread_mutex_t buffer_mutex;
			pthread_t grab_thread;
			pthread_cond_t buffer_cond;
			int totalframecount;
			int image_size;
			int image_pixels;
			int framecount;
			int fps_update_interval;
			double fps;
			double lasttime;
			unsigned short *y8_to_rgb565;
			unsigned char  *rgb565_to_y8;
		};

/* --------- Function prototypes */

#ifdef __cplusplus
extern "C"
{
#endif

int fg_open_device (struct fgdevice *fg, const char *devicename);
int fg_print_info(struct fgdevice *fg); 
int fg_close_device (struct fgdevice *fg);

void fg_set_fps_interval(struct fgdevice *fg, int interval);
double fg_get_fps(struct fgdevice *fg);

int fg_set_channel(struct fgdevice *fg, int channel, int videomode);
int fg_set_frequency(struct fgdevice *fg, int region, int index);

int fg_get_setting(struct fgdevice *fg, int which_setting);
int fg_set_setting(struct fgdevice *fg, int which_setting, int value);

int fg_start_grab_image (struct fgdevice *fg, int width, int height, int format);
int fg_stop_grab_image (struct fgdevice *fg);
void * fg_get_next_image(struct fgdevice *fg);

double timeGet(void);

int getFrequency(int region, int channel);

#ifdef __cplusplus
}
#endif
