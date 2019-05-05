// in_mouse.c -- dos mouse code

#include "quakedef.h"
#include "dosisms.h"

cvar_t	m_filter = {"m_filter","1"};

qboolean	mouse_avail;
int		mouse_buttons;
int		mouse_oldbuttonstate;
int		mouse_buttonstate;
float	mouse_x, mouse_y;
float	old_mouse_x, old_mouse_y;


cvar_t	in_joystick = {"joystick","1"};
cvar_t	joy_numbuttons = {"joybuttons","4", true};

qboolean	joy_avail;
int		joy_oldbuttonstate;
int		joy_buttonstate;

int     joyxl, joyxh, joyyl, joyyh; 
int		joystickx, joysticky;

qboolean		need_center;


void IN_StartupJoystick (void);
qboolean IN_ReadJoystick (void);

/*
===========
IN_StartupMouse
===========
*/
void IN_StartupMouse (void)
{
	if ( COM_CheckParm ("-nomouse") ) 
		return; 
 
// check for mouse
	regs.x.ax = 0;
	dos_int86(0x33);
	mouse_avail = regs.x.ax;
	if (!mouse_avail)
	{
		Con_Printf ("No mouse found\n");
		return;
	}
	
	mouse_buttons = regs.x.bx;
	if (mouse_buttons > 3)
		mouse_buttons = 3;
	Con_Printf("%d-button mouse available\n", mouse_buttons);
}

/*
===========
IN_Init
===========
*/
void IN_Init (void)
{
	Cvar_RegisterVariable (&m_filter);
	Cvar_RegisterVariable (&in_joystick);
	Cvar_RegisterVariable (&joy_numbuttons);

	IN_StartupMouse ();
	IN_StartupJoystick ();
}

/*
===========
IN_Shutdown
===========
*/
void IN_Shutdown (void)
{

}


/*
===========
IN_Commands
===========
*/
void IN_Commands (void)
{
	int		i;

	if (mouse_avail)
	{
		regs.x.ax = 3;		// read buttons
		dos_int86(0x33);
		mouse_buttonstate = regs.x.bx;
	
	// perform button actions
		for (i=0 ; i<mouse_buttons ; i++)
		{
			if ( (mouse_buttonstate & (1<<i)) &&
			!(mouse_oldbuttonstate & (1<<i)) )
			{
				Key_Event (K_MOUSE1 + i, true);
			}
			if ( !(mouse_buttonstate & (1<<i)) &&
			(mouse_oldbuttonstate & (1<<i)) )
			{
				Key_Event (K_MOUSE1 + i, false);
			}
		}	
		
		mouse_oldbuttonstate = mouse_buttonstate;
	}
	
	if (joy_avail)
	{
		joy_buttonstate = ((dos_inportb(0x201) >> 4)&15)^15;
	// perform button actions
		for (i=0 ; i<joy_numbuttons.value ; i++)
		{
			if ( (joy_buttonstate & (1<<i)) &&
			!(joy_oldbuttonstate & (1<<i)) )
			{
				Key_Event (K_JOY1 + i, true);
			}
			if ( !(joy_buttonstate & (1<<i)) &&
			(joy_oldbuttonstate & (1<<i)) )
			{
				Key_Event (K_JOY1 + i, false);
			}
		}
		
		joy_oldbuttonstate = joy_buttonstate;
	}
}


/*
===========
IN_Move
===========
*/
void IN_MouseMove (usercmd_t *cmd)
{
	int		mx, my;

	if (!mouse_avail)
		return;

	regs.x.ax = 11;		// read move
	dos_int86(0x33);
	mx = (short)regs.x.cx;
	my = (short)regs.x.dx;
	
	if (m_filter.value)
	{
		mouse_x = (mx + old_mouse_x) * 0.5;
		mouse_y = (my + old_mouse_y) * 0.5;
	}
	else
	{
		mouse_x = mx;
		mouse_y = my;
	}
	old_mouse_x = mx;
	old_mouse_y = my;

	mouse_x *= sensitivity.value;
	mouse_y *= sensitivity.value;

// add mouse X/Y movement to cmd
	if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
		cmd->sidemove += m_side.value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw.value * mouse_x;
	
	if (in_mlook.state & 1)
		V_StopPitchDrift ();
		
	if ( (in_mlook.state & 1) && !(in_strafe.state & 1))
	{
		cl.viewangles[PITCH] += m_pitch.value * mouse_y;
		if (cl.viewangles[PITCH] > 80)
			cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70)
			cl.viewangles[PITCH] = -70;
	}
	else
	{
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward.value * mouse_y;
		else
			cmd->forwardmove -= m_forward.value * mouse_y;
	}
}

/*
===========
IN_JoyMove
===========
*/
void IN_JoyMove (usercmd_t *cmd)
{
	float	speed, aspeed;

	if (!joy_avail || !in_joystick.value) 
		return; 
 
	IN_ReadJoystick (); 
	if (joysticky > joyyh*2 || joystickx > joyxh*2)
		return;		// assume something jumped in and messed up the joystick
					// reading time (win 95)

	if (in_speed.state & 1)
		speed = cl_movespeedkey.value;
	else
		speed = 1;
	aspeed = speed*host_frametime;

	if (in_strafe.state & 1)
	{
		if (joystickx < joyxl)
			cmd->sidemove -= speed*cl_sidespeed.value; 
		else if (joystickx > joyxh) 
			cmd->sidemove += speed*cl_sidespeed.value; 
	}
	else
	{
		if (joystickx < joyxl)
			cl.viewangles[YAW] += aspeed*cl_yawspeed.value;
		else if (joystickx > joyxh) 
			cl.viewangles[YAW] -= aspeed*cl_yawspeed.value;
		cl.viewangles[YAW] = anglemod(cl.viewangles[YAW]);
	}

	if (in_mlook.state & 1)
	{
		if (m_pitch.value < 0)
			speed *= -1;
		
		if (joysticky < joyyl) 
			cl.viewangles[PITCH] += aspeed*cl_pitchspeed.value;
		else if (joysticky > joyyh) 
			cl.viewangles[PITCH] -= aspeed*cl_pitchspeed.value;
	}
	else
	{
		if (joysticky < joyyl) 
			cmd->forwardmove += speed*cl_forwardspeed.value; 
		else if (joysticky > joyyh) 
			cmd->forwardmove -= speed*cl_backspeed.value;  
	}
}

/*
===========
IN_Move
===========
*/
void IN_Move (usercmd_t *cmd)
{
	IN_MouseMove (cmd);
	IN_JoyMove (cmd);
}

/* 
============================================================================ 
 
					JOYSTICK 
 
============================================================================ 
*/



qboolean IN_ReadJoystick (void)
{
	int		b;
	int		count;

	joystickx = 0;
	joysticky = 0;

	count = 0;

	b = dos_inportb(0x201);
	dos_outportb(0x201, b);

// clear counters
	while (++count < 10000)
	{
		b = dos_inportb(0x201);

		joystickx += b&1;
		joysticky += (b&2)>>1;
		if ( !(b&3) )
			return true;
	}
	
	Con_Printf ("IN_ReadJoystick: no response\n");
	joy_avail = false;
	return false;
}

/*
=============
WaitJoyButton
=============
*/
qboolean WaitJoyButton (void) 
{ 
	int             oldbuttons, buttons; 
 
	oldbuttons = 0; 
	do 
	{
		key_count = -1;
		Sys_SendKeyEvents ();
		key_count = 0;
		if (key_lastpress == K_ESCAPE)
		{
			Con_Printf ("aborted.\n");
			return false;
		}
		key_lastpress = 0;
		SCR_UpdateScreen ();
		buttons =  ((dos_inportb(0x201) >> 4)&1)^1; 
		if (buttons != oldbuttons) 
		{ 
			oldbuttons = buttons; 
			continue; 
		}
	} while ( !buttons); 
 
	do 
	{ 
		key_count = -1;
		Sys_SendKeyEvents ();
		key_count = 0;
		if (key_lastpress == K_ESCAPE)
		{
			Con_Printf ("aborted.\n");
			return false;
		}
		key_lastpress = 0;
		SCR_UpdateScreen ();
		buttons =  ((dos_inportb(0x201) >> 4)&1)^1; 
		if (buttons != oldbuttons) 
		{ 
			oldbuttons = buttons; 
			continue; 
		} 
	} while ( buttons); 
 
	return true; 
} 
 
 
 
/* 
=============== 
IN_StartupJoystick 
=============== 
*/  
void IN_StartupJoystick (void) 
{ 
	int     centerx, centery; 
 
 	Con_Printf ("\n");

	joy_avail = false; 
	if ( COM_CheckParm ("-nojoy") ) 
		return; 
 
	if (!IN_ReadJoystick ()) 
	{ 
		joy_avail = false; 
		Con_Printf ("joystick not found\n"); 
		return; 
	} 

	Con_Printf ("joystick found\n"); 
 
	Con_Printf ("CENTER the joystick\nand press button 1 (ESC to skip):\n"); 
	if (!WaitJoyButton ()) 
		return; 
	IN_ReadJoystick (); 
	centerx = joystickx; 
	centery = joysticky; 
 
	Con_Printf ("Push the joystick to the UPPER LEFT\nand press button 1 (ESC to skip):\n"); 
	if (!WaitJoyButton ()) 
		return; 
	IN_ReadJoystick (); 
	joyxl = (centerx + joystickx)/2; 
	joyyl = (centerx + joysticky)/2; 
 
	Con_Printf ("Push the joystick to the LOWER RIGHT\nand press button 1 (ESC to skip):\n"); 
	if (!WaitJoyButton ()) 
		return; 
	IN_ReadJoystick (); 
	joyxh = (centerx + joystickx)/2; 
	joyyh = (centery + joysticky)/2; 

	joy_avail = true; 
	Con_Printf ("joystick configured.\n"); 

 	Con_Printf ("\n");
} 
 
 
 

