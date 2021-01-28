
.PHONY: png2cga
png2cga:
	gcc ./tools/src/png2cga.c -o ./tools/$@ -lm

.PHONY: bitmaps
bitmaps:
	mkdir -p ASSETS
	./tools/png2cga dev/bitmaps/font.png ASSETS/FONT.BIN -mask
	./tools/png2cga dev/bitmaps/figure.png ASSETS/FIGURE.BIN -mask
	./tools/png2cga dev/bitmaps/tileset.png ASSETS/TILESET.BIN
	./tools/png2cga dev/bitmaps/hud.png ASSETS/HUD.BIN

.PHONY: tilemaps
tilemaps:
	mkdir -p ASSETS
	./tools/tmx2bin dev/tilemaps/basemap.tmx ASSETS/MAP.BIN

assets: bitmaps tilemaps

.PHONY: tmx2bin
tmx2bin:
	(cd tools/src/tmx2bin; go build -o ../../$@)