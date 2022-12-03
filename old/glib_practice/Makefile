FLAGS := -Wall -Werror -g `pkg-config --cflags --libs glib-2.0 gtk4 gio-2.0`

hdr_dir := hdr
src_dir := src
build_dir := ../build
main_dir := ..

src_files := $(wildcard $(src_dir)/*.c)

all: clean build

build: mk_bld_dir	
	gcc -I$(hdr_dir) $(FLAGS) $(src_files) -o $(build_dir)/tuxdrop

mk_bld_dir: 
	mkdir $(build_dir)

clean:
	rm -r $(build_dir)
