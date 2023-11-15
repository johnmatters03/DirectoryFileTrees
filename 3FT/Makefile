GCC=gcc217

all: ft

clean:
	rm -f ft

clobber: clean
	rm -f ft_client.o *~

ft: dynarray.o path.o checkerFT.o nodeFT.o ft.o ft_client.o
	$(GCC) -g $^ -o $@

dynarray.o: dynarray.c dynarray.h
	$(GCC) -g -c $<

path.o: path.c dynarray.h path.h a4def.h
	$(GCC) -g -c $<

ft_client.o: ft_client.c ft.h a4def.h
	$(GCC) -g -c $<

checkerFT.o: checkerFT.c dynarray.h checkerFT.h nodeFT.h path.h a4def.h
	$(GCC) -g -c $<

nodeFT.o: nodeFT.c dynarray.h checkerFT.h nodeFT.h path.h a4def.h
	$(GCC) -g -c $<

ft.o: ft.c dynarray.h checkerFT.h nodeFT.h ft.h path.h a4def.h
	$(GCC) -g -c $<

