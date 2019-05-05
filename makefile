CC=gcc
#CFLAGS=-Zomf -g -Wall
CFLAGS=-Zomf -O2 -Wall -fomit-frame-pointer -fno-strength-reduce -funroll-loops
#CFLAGS=-O2 -Wall -fno-strength-reduce
#LDFLAGS=-static
#LDFLAGS=-Zomf -Zsysv-signals
LDFLAGS=-Zomf -Zsys -s
#LIBS=-lm -lsocket -lmmpm2
LIBS=-lm -lmmpm2 -ltcp32dll -lso32dll
O=linux
DMASIMLIBS=

OUTPUT=	xquake.exe

all:	$(OUTPUT)

clean:
	rm -f *.obj $(OUTPUT)

%.obj : %.s
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $<

%.obj : %.c
	$(CC) $(CFLAGS) -c $<

%.obj : %.asm
	masm $<

HEADERS = asm_draw.h client.h cmd.h common.h cvar.h d_iface.h\
         d_local.h mathlib.h model.h modelgen.h net.h quakeasm.h\
         quakedef.h spritegn.h zone.h crc.h net_vcr.h wad.h sbar.h\
         world.h

PORTABLE_OBJS= \
 console.obj \
 r_light.obj \
 r_misc.obj \
 pr_cmds.obj \
 cd_null.obj \
 menu.obj \
 draw.obj \
 cl_demo.obj \
 cl_input.obj \
 keys.obj \
 cl_main.obj \
 cl_parse.obj \
 cl_tent.obj \
 cmd.obj \
 common.obj \
 crc.obj \
 cvar.obj \
 d_edge.obj \
 d_fill.obj \
 d_init.obj \
 d_modech.obj \
 d_part.obj \
 d_polyse.obj \
 d_scan.obj \
 d_sky.obj \
 d_sprite.obj \
 d_surf.obj \
 d_zpoint.obj \
 host.obj \
 host_cmd.obj \
 mathlib.obj \
 model.obj \
 net_main.obj \
 net_vcr.obj \
 pr_edict.obj \
 pr_exec.obj \
 r_aclip.obj \
 r_efrag.obj \
 r_alias.obj \
 r_bsp.obj \
 r_draw.obj \
 r_edge.obj \
 r_main.obj \
 r_part.obj \
 r_sky.obj \
 r_sprite.obj \
 r_surf.obj \
 r_vars.obj \
 sbar.obj \
 screen.obj \
 sv_main.obj \
 sv_move.obj \
 sv_phys.obj \
 sv_user.obj \
 view.obj \
 wad.obj \
 world.obj \
 zone.obj

LINUX_OBJS= \
 d_vars.obj \
 sys_os2.obj \
 net_bsd.obj \
 net_dgrm.obj \
 net_loop.obj \
 net_udp.obj \
 nonintel.obj \
 snd_dma.obj \
 snd_mem.obj \
 snd_dart.obj \
 snd_mix.obj \
 d_draw.obj \
 d_draw16.obj \
 d_parta.obj \
 d_polysa.obj \
 d_scana.obj\
 d_spr8.obj \
 math.obj \
 r_aliasa.obj \
 r_drawa.obj \
 r_edgea.obj \
 surf16.obj \
 surf8.obj \
 worlda.obj \
 r_aclipa.obj \
 snd_mixa.obj \
 sys_dosa.obj

SLOWER_C_OBJS= \
 d_vars.obj \
 nonintel.obj

OBJS=$(PORTABLE_OBJS) $(LINUX_OBJS)

sym_$(O):	xquake
	nm xquake > sym_$(O)

start_squake: ../start_squake
	cp ../start_squake $(LOCAL)

xquake.exe:	$(OBJS) vid_dive.obj in_null.obj
	$(CC) $(LDFLAGS) $(OBJS) vid_dive.obj in_null.obj show.def show.res -o xquake.exe $(LIBS)

IOPL_OBJS= \
 iopl.obj

quakefs.exe:	$(OBJS) vid_vio.obj vid_vga.obj vregset.obj $(IOPL_OBJS)
	$(CC) $(LDFLAGS) $(OBJS) vid_vio.obj vid_vga.obj vregset.obj $(IOPL_OBJS) quakefs.def -o quakefs.exe $(LIBS)

$(OBJS) $(FAST_I386_OBJS) $(SLOWER_C_OBJS) dmasim:	$(HEADERS)

