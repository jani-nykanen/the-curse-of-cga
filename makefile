
.PHONY: png2cga
png2cga:
	gcc ./tools/src/png2cga.c -o ./tools/$@ -lm

.PHONY: bitmaps
bitmaps:
	mkdir -p ASSETS
	./tools/png2cga dev/bitmaps/font.png ASSETS/FONT.BIN
