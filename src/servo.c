
/* PiKrellCam
|
|  Copyright (C) 2015-2016 Bill Wilson    billw@gkrellm.net
|
|  PiKrellCam is free software: you can redistribute it and/or modify it
|  under the terms of the GNU General Public License as published by
|  the Free Software Foundation, either version 3 of the License, or
|  (at your option) any later version.
|
|  PiKrellCam is distributed in the hope that it will be useful, but WITHOUT
|  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
|  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
|  License for more details.
|
|  You should have received a copy of the GNU General Public License
|  along with this program. If not, see http://www.gnu.org/licenses/
|
|  This file is part of PiKrellCam.
*/

/* BCM2835-ARM-Peripherals.pdf document:
|    https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/BCM2835-ARM-Peripherals.pdf
|  Addendum covers clock manager:
|    http://www.scribd.com/doc/127599939/BCM2835-Audio-clocks
*/

#include "pikrellcam.h"
#include <stdint.h>
#include <sys/mman.h>

#define PI_1_PERIPHERAL_BASE	0x20000000
#define PI_2_PERIPHERAL_BASE	0x3F000000

#define GPIO_BASE    0x200000
#define PWM_BASE     0x20C000
#define CLOCK_BASE   0x101000


/* xxx_REG defines are uint32_t pointer offsets to registers in the gpio,
|  pwm or clock address space and are BCM2835-ARM-Peripherals.pdf or scribd
|  register byte addresses / 4
*/
#define	PWM_CTL_REG              (0x0 / 4)
#define     CTL_REG_RESET_STATE  0
#define     CTL_REG_PWM1_ENABLE  1
#define     CTL_REG_MSEN1        0x80
#define     CTL_REG_PWM1_MS_MODE (CTL_REG_PWM1_ENABLE | CTL_REG_MSEN1)
#define     CTL_REG_PWM2_ENABLE  0x100
#define     CTL_REG_MSEN2        0x8000
#define     CTL_REG_PWM2_MS_MODE (CTL_REG_PWM2_ENABLE | CTL_REG_MSEN2)

#define	PWM_RNG1_REG    (0x10 / 4)
#define	PWM_DAT1_REG    (0x14 / 4)
#define	PWM_RNG2_REG    (0x20 / 4)
#define	PWM_DAT2_REG    (0x24 / 4)

#define	GPSET_REG     (0x1c / 4)
#define	GPCLR_REG     (0x28 / 4)
#define	GPLEV_REG     (0x34 / 4)
#define	GPPUD_REG     (0x94 / 4)
#define	  PUD_DOWN    1
#define	  PUD_UP      2
#define	GPPUDCLK_REG  (0x98 / 4)


/* PWM clock manager registers CM_PWMDIV & CM_PWMCTL from the scribd addendum:
*/
#define CM_PASSWORD  0x5A000000
#define	CM_PWMCTL_REG      (0xa0 / 4)
#define    PWMCTL_BUSY     0x80		// Read only
#define    PWMCTL_KILL     0x20
#define    PWMCTL_ENABLE   0x10
#define    PWMCTL_SRC_OSC  0x1
#define	CM_PWMDIV_REG      (0xa4 / 4)
#define    PWMDIV_DIVI(divi) (divi << 12)	// bits 23-12, max 4095


#define PWM_CLOCK_HZ    19200000.0
#define PWM_RESOLUTION  0.000005		// 5 usec resolution
#define	PWM_MSEC_TO_COUNT(ms)	((ms) / PWM_RESOLUTION / 1000.0)
#define PULSE_WIDTH_RESOLUTION	.00001	// .01 msec resolution

#define	SERVO_MODE_MOVE_ONE    0
#define	SERVO_MODE_MOVE_STEPS  1
#define	SERVO_MODE_MOVE_LIMIT  2

#define SERVO_IDLE       0
#define SERVO_NEW_MOVE   1
#define SERVO_MOVING     2
#define SERVO_STOP       3

typedef struct
	{
	pthread_mutex_t mutex;
	int     status;
	int     pan,
	        tilt,
	        delay;
	}
	ServoControl;

static ServoControl  servo_control;

/* Pointers to mapped peripheral registers.
*/
static volatile uint32_t *gpio_mmap;
static volatile uint32_t *pwm_mmap;
static volatile uint32_t *clock_mmap;


static pthread_t  servo_thread_ref;

static void	(*pwm_width_func)(int channel, int width, boolean invert);

static FILE *fblaster;

static float	pan_cur = 150.0,
				tilt_cur = 150.0;

static int		pan_channel,
				tilt_channel;

  /* Servo pulse width units are .01 msec (so width = 150 is 1.5 msec)
  */
void
pwm_width_hardware(int channel, int width, boolean invert)
	{
	uint32_t count;
	int      reg;

	if (channel == 1)
		reg = PWM_DAT1_REG;
	else if (channel == 2)
		reg = PWM_DAT2_REG;
	else
		return;

	if (invert)
		width = 300 - width;	// 150 msec is center
	if (width < SERVO_MIN_WIDTH)
		width = SERVO_MIN_WIDTH;
	if (width > SERVO_MAX_WIDTH)
		width = SERVO_MAX_WIDTH;
	count = (uint32_t) (PULSE_WIDTH_RESOLUTION / PWM_RESOLUTION) * width;
	*(pwm_mmap + reg) = count;
	}

void
pwm_width_servoblaster(int channel, int width, boolean invert)
	{
	char           buf[64];
	static boolean logged = FALSE;

	if (channel < 0)
		return;
	if (!fblaster)
		fblaster = fopen("/dev/servoblaster", "w");
	if (!fblaster)
		{
		if (!logged)
			log_printf_no_timestamp("Failed to open /dev/servoblaster: %m\n");
		logged = TRUE;
		return;
		}
	if (invert)
		width = 300 - width;
	if (width < SERVO_MIN_WIDTH)
		width = SERVO_MIN_WIDTH;
	if (width > SERVO_MAX_WIDTH)
		width = SERVO_MAX_WIDTH;
	snprintf(buf, sizeof(buf), "%d=%d\n", channel, width);
	fwrite(buf, strlen(buf), 1, fblaster);
	fflush(fblaster);
	}

void
gpio_to_channel(int gpio, int *channel, int *altfn)
	{
	int	chan = -1, alt = -1;

	if (gpio == 12 || gpio == 18)
		{
		chan = 1;
		alt = (gpio == 18) ? 5 : 0;
		}
	if (gpio == 13 || gpio == 19)
		{
		chan = 2;
		alt = (gpio == 19) ? 5 : 0;
		}
	if (channel)
		*channel = chan;
	if (altfn)
		*altfn = alt;
	}

void
servo_get_position(int *pan, int *tilt)
	{
	if (pan)
		*pan = (int) pan_cur;
	if (tilt)
		*tilt = (int) tilt_cur;
	}

  /* GPFSELn registers start at offset zero from gpio_mmap.
  |  BCM2835-ARM-Peripherals.pdf pg 91, 10 gpios per GPFSELn with mode bits:
  */
static unsigned int gpfsel_mode_table[] =
	{
	/* in    out   alt0   alt1   alt2   alt3   alt4   alt5 */
	0b000, 0b001, 0b100, 0b101, 0b110, 0b111, 0b011, 0b010
	};

void
gpio_alt_function(int pin, int altfn)
	{
	int  reg   = pin / 10,
	     shift = (pin % 10) * 3;

	if (altfn >= 0 && altfn <= 5)
		*(gpio_mmap + reg) = (*(gpio_mmap + reg) & ~(0x7 << shift))
							| (gpfsel_mode_table[altfn + 2] << shift);
	}

void
gpio_set_mode(int pin, int mode)	/* mode 0:input 1:output */
	{
	int  reg   = pin / 10,
	     shift = (pin % 10) * 3;

	if (mode == 0 || mode == 1)
		*(gpio_mmap + reg) = (*(gpio_mmap + reg) & ~(0x7 << shift))
							| (gpfsel_mode_table[mode] << shift);
	}


  /* BCM2835-ARM-Peripherals.pdf pg 101 - GPIO Pull-up/down sequence
  */
void
gpio_set_pud(int pin, int pud)
	{
	int  reg = GPPUDCLK_REG + ((pin > 31) ? 1 : 0);

	if (pud != PUD_DOWN && pud != PUD_UP)
		return;
	*(gpio_mmap + GPPUD_REG) = pud;
	usleep(2);			// min wait of 150 cycles
	*(gpio_mmap + reg) = 1 << (pin & 0x1f);
	usleep(2);
	*(gpio_mmap + GPPUD_REG) = 0;
	*(gpio_mmap + reg) = 0;
	}

int
gpio_read(int pin)
	{
	int  reg = GPLEV_REG + ((pin > 31) ? 1 : 0);

	return (*(gpio_mmap + reg) & (1 << (pin & 0x1f)) ? 1 : 0 ); 
	}

void
gpio_write(int pin, int level)
	{
	int  reg = ((level == 0) ? GPCLR_REG : GPSET_REG) + ((pin > 31) ? 1 : 0);

	*(gpio_mmap + reg) = 1 << (pin & 0x1f);
	}

int
pi_model(void)
	{
	FILE       *f;
	static int model;
	char       buf[200], arm[32];

	if (model == 0)
		{
		if ((f = fopen("/proc/cpuinfo", "r")) != NULL)
			{
			while (fgets(buf, sizeof(buf), f) != NULL)
				{
				if (sscanf(buf, "model name %*s %31s", arm) > 0)
					{
					if (!strcmp(arm, "ARMv7"))
						model = 2;
					else
						model = 1;
					break;
					}
				}
			fclose(f);
			}
		}
	return model;
	}

static void
_servo_move(int pan, int tilt, int delay)
	{
	float	pan_inc, tilt_inc;
	int		pan_delta, tilt_delta, max_delta, i;

	if (pan_channel < 0 && tilt_channel < 0)
		return;

	servo_control.status = SERVO_MOVING;
	pikrellcam.servo_moving = TRUE;

	if (pan_cur == 0)
		pan_cur = (float) pan;
	if (tilt_cur == 0)
		tilt_cur = (float) tilt;

	pan_delta  = pan - pan_cur;
	tilt_delta = tilt - tilt_cur;
	max_delta = MAX(abs(pan_delta), abs(tilt_delta));

	pan_inc = (abs(pan_delta) > 1.0)
				? (float) pan_delta / (float) max_delta : 0;
	tilt_inc = (abs(tilt_delta) > 1.0)
				? (float) tilt_delta / (float) max_delta : 0;

//printf("pan: %d pan_cur:%.0f pan_delta:%d pan_inc:%.2f\n",
//		pan, pan_cur, pan_delta, pan_inc);
//printf("tilt:%d tilt_cur:%.0f tilt_delta:%d tilt_inc:%.2f\n",
//		tilt, tilt_cur, tilt_delta, tilt_inc);

	for (i = 1; i < max_delta && delay > 0; ++i)
		{
		pan_cur += pan_inc;
		tilt_cur += tilt_inc;
		pwm_width_func(pan_channel, (int) pan_cur, pikrellcam.servo_pan_invert);
		pwm_width_func(tilt_channel, (int) tilt_cur, pikrellcam.servo_tilt_invert);
		usleep(delay * 1000);
		pthread_mutex_lock(&servo_control.mutex);
		if (servo_control.status != SERVO_MOVING)
			{
			pan_cur = floorf(pan_cur);
			tilt_cur = floorf(tilt_cur);
			if (servo_control.status != SERVO_NEW_MOVE)
				{
				preset_on_check((int) pan_cur, (int) tilt_cur);
				pthread_mutex_unlock(&servo_control.mutex);
				usleep(pikrellcam.servo_settle_msec * 1000);
				pikrellcam.servo_moving = FALSE;
				pikrellcam.state_modified = TRUE;
				}
			else
				pthread_mutex_unlock(&servo_control.mutex);
			return;
			}
		pthread_mutex_unlock(&servo_control.mutex);
		}
	pan_cur = (float) pan;
	tilt_cur = (float) tilt;
	pwm_width_func(pan_channel, pan, pikrellcam.servo_pan_invert);
	pwm_width_func(tilt_channel, tilt, pikrellcam.servo_tilt_invert);
	pthread_mutex_lock(&servo_control.mutex);
	if (servo_control.status != SERVO_NEW_MOVE)
		{
		servo_control.status = SERVO_IDLE;
		preset_on_check(pan, tilt);
		pthread_mutex_unlock(&servo_control.mutex);
		usleep(pikrellcam.servo_settle_msec * 1000);
		pikrellcam.servo_moving = FALSE;
		pikrellcam.state_modified = TRUE;
		}
	else
		pthread_mutex_unlock(&servo_control.mutex);
	}

static void *
servo_thread(void *ptr)
	{
	static boolean	first_move_done;

	while (1)
		{
		if (servo_control.status != SERVO_NEW_MOVE)
			{
			usleep(50000);
			continue;
			}

		if (!first_move_done)	/* Minimize possible current spike */
			{
			pikrellcam.servo_moving = TRUE;
			pthread_mutex_lock(&servo_control.mutex);
			pwm_width_func(pan_channel, servo_control.pan, pikrellcam.servo_pan_invert);
			usleep(300000);
			pwm_width_func(tilt_channel, servo_control.tilt, pikrellcam.servo_tilt_invert);
			usleep(300000);
			pan_cur = servo_control.pan;
			tilt_cur = servo_control.tilt;
			servo_control.status = SERVO_IDLE;
			preset_on_check((int) pan_cur, (int) tilt_cur);
			pthread_mutex_unlock(&servo_control.mutex);
			pikrellcam.servo_moving = FALSE;
			pikrellcam.state_modified = TRUE;
			}
		else
			_servo_move(servo_control.pan, servo_control.tilt, servo_control.delay);
		first_move_done = TRUE;
		}
	return NULL;
	}

void
servo_move(int pan, int tilt, int delay)
	{
	if (!pikrellcam.have_servos)
		return;
	pthread_mutex_lock(&servo_control.mutex);
	servo_control.pan = pan;
	servo_control.tilt = tilt;
	servo_control.delay = delay;
	servo_control.status = SERVO_NEW_MOVE;
	pikrellcam.on_preset = FALSE;
	pthread_mutex_unlock(&servo_control.mutex);
	}

void
servo_init(void)
	{
	int      fd, peripheral_base, pan_alt, tilt_alt;
	uint32_t divi, t1, t2;

	pan_cur = tilt_cur = 150.0;
	if (!pikrellcam.have_servos)
		return;
	if (pikrellcam.servo_use_servoblaster)
		{
		pwm_width_func = pwm_width_servoblaster;
		pan_channel = pikrellcam.servo_pan_gpio;
		tilt_channel = pikrellcam.servo_tilt_gpio;
		pthread_create (&servo_thread_ref, NULL, servo_thread, NULL);
		log_printf_no_timestamp("======= Servo using ServoBlaster (%d %d)\n",
						pan_channel, tilt_channel);
		return;
		}
	pwm_width_func = pwm_width_hardware;
	gpio_to_channel(pikrellcam.servo_pan_gpio, &pan_channel, &pan_alt);
	gpio_to_channel(pikrellcam.servo_tilt_gpio, &tilt_channel, &tilt_alt);
	if (   (pan_channel < 0 && tilt_channel < 0)
	    || (pan_channel == tilt_channel)
	   )
		{
		log_printf_no_timestamp("======= Servo init failed, bad gpio numbers: %s\n",
			(pan_channel == tilt_channel && pan_channel != -1) ?
				"PWM channel collision" : "gpio number not a hardware PWM");
		pan_channel = tilt_channel = -1;
		return;
		}
	peripheral_base = (pi_model() == 2) ? PI_2_PERIPHERAL_BASE : PI_1_PERIPHERAL_BASE;

	if ((fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0)
		{
		pan_channel = tilt_channel = -1;
		log_printf_no_timestamp("======= Servo init failed: /dev/mem open failed.");
		return;
		}
	gpio_mmap = (uint32_t *) mmap(NULL, 0x100, PROT_READ|PROT_WRITE, MAP_SHARED,
						fd, peripheral_base + GPIO_BASE);
	pwm_mmap  = (uint32_t *) mmap(NULL, 0x100, PROT_READ|PROT_WRITE, MAP_SHARED,
						fd, peripheral_base + PWM_BASE);
	clock_mmap = (uint32_t *) mmap(NULL, 0x100, PROT_READ|PROT_WRITE, MAP_SHARED,
						fd, peripheral_base + CLOCK_BASE);
	close(fd);

	if (pwm_mmap == MAP_FAILED || clock_mmap == MAP_FAILED)
		{
		pan_channel = tilt_channel = -1;
		log_printf_no_timestamp("======= Servo init failed: PWM gpio mmap() failed");
		return;
		}
	if (pan_channel > 0)
		gpio_alt_function(pikrellcam.servo_pan_gpio, pan_alt);
	if (tilt_channel > 0)
		gpio_alt_function(pikrellcam.servo_tilt_gpio, tilt_alt);

	*(clock_mmap + CM_PWMCTL_REG) = CM_PASSWORD | PWMCTL_KILL;
	usleep(10);  

	divi = (uint32_t) (PWM_CLOCK_HZ * PWM_RESOLUTION);
	*(clock_mmap + CM_PWMDIV_REG) = CM_PASSWORD | PWMDIV_DIVI(divi);
	*(clock_mmap + CM_PWMCTL_REG) = CM_PASSWORD | PWMCTL_ENABLE | PWMCTL_SRC_OSC;

	/* Turn off PWM, set range registers and enable clocks so PWM channels run
	|  in M/S mode where data count gives pulse width range count is period.
	*/
	*(pwm_mmap + PWM_CTL_REG) = CTL_REG_RESET_STATE;
	usleep(50);
	*(pwm_mmap + PWM_RNG1_REG) = (uint32_t) PWM_MSEC_TO_COUNT(20);
	*(pwm_mmap + PWM_RNG2_REG) = (uint32_t) PWM_MSEC_TO_COUNT(20);
	*(pwm_mmap + PWM_CTL_REG) = CTL_REG_PWM1_MS_MODE | CTL_REG_PWM2_MS_MODE;

	/* t1 & t2 is pulse width time in .01 msec units.
	*/
	t1 = *(pwm_mmap + PWM_DAT1_REG);
	t1 = (uint32_t) ((float) t1 * PWM_RESOLUTION / PULSE_WIDTH_RESOLUTION);

	t2 = *(pwm_mmap + PWM_DAT2_REG);
	t2 = (uint32_t) ((float) t2 * PWM_RESOLUTION / PULSE_WIDTH_RESOLUTION);

	pan_cur  = (float) ((pan_channel  == 1) ? t1 : t2);
	if (pikrellcam.servo_pan_invert)
		pan_cur = 300.0 - pan_cur;
	if (pan_cur < pikrellcam.servo_pan_min || pan_cur > pikrellcam.servo_pan_max)
		pan_cur = 150;

	tilt_cur = (float) ((tilt_channel == 1) ? t1 : t2);
	if (pikrellcam.servo_tilt_invert)
		tilt_cur = 300.0 - tilt_cur;
	if (tilt_cur < pikrellcam.servo_tilt_min || tilt_cur > pikrellcam.servo_tilt_max)
		tilt_cur = 150;
	log_printf_no_timestamp("======= Servo using hardware PWM (%d %d)\n",
					pikrellcam.servo_pan_gpio, pikrellcam.servo_tilt_gpio);

	pthread_create (&servo_thread_ref, NULL, servo_thread, NULL);
	}


#define	PAN_LEFT	0
#define	PAN_RIGHT	1
#define	TILT_UP		2
#define	TILT_DOWN	3

typedef struct
	{
	char *name;
	int	 id,
	     n_args;
	}
ServoCommand;

static ServoCommand servo_commands[] =
	{
		{ "pan_left",  PAN_LEFT,  1 },
		{ "pan_right", PAN_RIGHT, 1 },
		{ "tilt_up",   TILT_UP,	  1 },
		{ "tilt_down", TILT_DOWN, 1 },
	};

#define N_SERVO_COMMANDS    (sizeof(servo_commands) / sizeof(ServoCommand))

static int
servo_move_position(int cur, int dir, int mode, int limit)
	{
	if (mode == SERVO_MODE_MOVE_ONE)
		cur += dir;
	else if (mode == SERVO_MODE_MOVE_STEPS)
		cur += dir * pikrellcam.servo_move_steps;
	else if (mode == SERVO_MODE_MOVE_LIMIT)
		cur = limit;
	return cur;
	}

void
servo_command(char *cmd_line)
	{
	ServoCommand *scmd;
	int          i, n, id = -1;
	int          pan, tilt, mode;
	char         buf[64], arg1[32];
	static int   prev_id;

	if (!pikrellcam.have_servos)
		return;

	arg1[0] = '\0';
	n = sscanf(cmd_line, "%63s %31s", buf, arg1);
	if (n < 1)
		return;

	for (i = 0; i < N_SERVO_COMMANDS; ++i)
		{
		scmd = &servo_commands[i];
		if (!strcmp(scmd->name, buf))
			{
			if (scmd->n_args <= n - 1)
				id = scmd->id;
			break;
			}
		}
	if (id == -1)
		{
//      inform_message("Bad motion command.");
		return;
		}
	pan = (int) pan_cur;
	tilt = (int) tilt_cur;
	mode = atoi(arg1);

	if (   mode == SERVO_MODE_MOVE_LIMIT
	    && servo_control.status == SERVO_MOVING
	    && id == prev_id
	   )
		{
		servo_control.status = SERVO_IDLE;	/* Force to idle */
		return;
		}
	prev_id = id;

	switch (id)
		{
		case PAN_LEFT:
			pan = servo_move_position(pan, -1, mode, pikrellcam.servo_pan_min);
			if (pan < pikrellcam.servo_pan_min)
				pan = pikrellcam.servo_pan_min;
			servo_move(pan, tilt, pikrellcam.servo_move_step_msec);
			break;

		case PAN_RIGHT:
			pan = servo_move_position(pan, 1, mode, pikrellcam.servo_pan_max);
			if (pan > pikrellcam.servo_pan_max)
				pan = pikrellcam.servo_pan_max;
			servo_move(pan, tilt, pikrellcam.servo_move_step_msec);
			break;

		case TILT_UP:
			tilt = servo_move_position(tilt, 1, mode, pikrellcam.servo_tilt_max);
			if (tilt > pikrellcam.servo_tilt_max)
				tilt = pikrellcam.servo_tilt_max;
			servo_move(pan, tilt, pikrellcam.servo_move_step_msec);
			break;

		case TILT_DOWN:
			tilt = servo_move_position(tilt, -1, mode, pikrellcam.servo_tilt_min);
			if (tilt < pikrellcam.servo_tilt_min)
				tilt = pikrellcam.servo_tilt_min;
			servo_move(pan, tilt, pikrellcam.servo_move_step_msec);
			break;
		}
	}
