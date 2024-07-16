CC := gcc

gif:
	wget https://theuselessweb.site/chihuahuaspin/spin.gif -O chihuahuaspin.temp.gif
	ffmpeg -y -i chihuahuaspin.temp.gif -vf palettegen chihuahuaspin.palette.png
	ffmpeg -y -i chihuahuaspin.temp.gif -i chihuahuaspin.palette.png -filter_complex paletteuse -r 10 chihuahuaspin.gif
	xxd -i chihuahuaspin.gif > spin.gif.h

linux:
	$(CC) main.c -Os -lm -lX11 -o chihuahuaspin.linux

macos:
	$(CC) main.c -Os -framework Cocoa -o chihuahuaspin.mach

windows:
	$(CC) main.c -Os -lgdi32 -lwinmm -o chihuahuaspin.exe
