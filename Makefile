all: sifToPmf.out midasToPpf.out

sifToPmf.out: SifToPmf/main.cpp
	g++ SifToPmf/main.cpp -o sifToPmf.out -std=c++11 -lboost_program_options-mt -lboost_filesystem-mt -lboost_system-mt
midasToPpf.out: MidasToPpf/main.cpp
	g++ MidasToPpf/main.cpp -o midasToPpf.out  -std=c++11  -lboost_program_options-mt -lboost_filesystem-mt -lboost_system-mt
