
prefix = 
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin

#PROFILE=YES
PROFILE=APPLY

CC = /opt/rs90-toolchain/bin/mipsel-linux-gcc
LD = $(CC)
AS = $(CC)

CFLAGS		+= -Ofast -mips32 -fdata-sections -ffunction-sections -mno-fp-exceptions -mno-check-zero-division -mframe-header-opt -fsingle-precision-constant -fno-common -march=mips32 -mtune=mips32 
CFLAGS		+= -fno-common
CFLAGS		+= -mlong32 -mno-micromips -mno-interlink-compressed
CFLAGS		+= -flto -funroll-loops -fsection-anchors
CFLAGS		+= -fno-stack-protector -fomit-frame-pointer -falign-functions=1 -falign-jumps=1 -falign-loops=1

CFLAGS += -D_GNU_SOURCE=1 -DIS_LITTLE_ENDIAN
LDFLAGS = -lc -lgcc -lm -lSDL -lasound -lz -no-pie -Wl,--as-needed -Wl,--gc-sections -flto -s

ifeq ($(PROFILE), YES)
CFLAGS 		+= -fprofile-generate="/media/data/local/home"
LDFLAGS 	+= -lgcov
else ifeq ($(PROFILE), APPLY)
CFLAGS		+= -fprofile-use
endif

ASFLAGS = $(CFLAGS)

TARGETS_RS90 =  sdlgnuboy_rs90.dge
TARGETS_RG99 =  sdlgnuboy_rg99.dge
TARGETS = $(TARGETS_RS90) $(TARGETS_RG99)

ASM_OBJS =

SYS_DEFS = -DIS_LITTLE_ENDIAN  -DIS_LINUX -DNATIVE_AUDIO
SYS_OBJS = sys/nix/nix.o $(ASM_OBJS)
SYS_INCS = -I./sys/nix -Ifont -Isrc/core -Isys

SDL_OBJS = sys/sdl/sdl.o sys/sdl/keymap.o sys/sdl/scaler.o sys/sdl/font_drawing.o
SDL_OBJS_RS90 = sys/alsa/alsa.o
SDL_OBJS_RG99 = sys/sdl/sdl_audio_rg99.c

all: $(TARGETS)

include Rules

$(TARGETS_RS90): $(OBJS) $(SYS_OBJS) $(SDL_OBJS) $(SDL_OBJS_RS90)
	$(LD) $(CFLAGS) $(OBJS) $(SYS_OBJS) $(SDL_OBJS) $(SDL_OBJS_RS90) -o $@ $(LDFLAGS)

$(TARGETS_RG99): $(OBJS) $(SYS_OBJS) $(SDL_OBJS) $(SDL_OBJS_RG99)
	$(LD) $(CFLAGS) $(OBJS) $(SYS_OBJS) $(SDL_OBJS) $(SDL_OBJS_RG99) -o $@ $(LDFLAGS)

clean:
	rm -f *gnuboy gmon.out src/core/*.o sys/*.o sys/*/*.o
	
clean_gcda:
	rm -f src/core/*.gcda sys/*.gcda sys/*/*.gcda





