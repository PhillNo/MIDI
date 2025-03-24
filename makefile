default: extras/main.cpp src/MIDI_Data.cpp src/MIDI_Decoder.cpp src/MIDI_Encoder.cpp
	g++ -g -Wall -std=c++17 \
	-Iinclude/ \
	extras/main.cpp src/MIDI_Data.cpp src/MIDI_Decoder.cpp src/MIDI_Encoder.cpp \
	-o extras/main

mid:
	for file in $$(find MIDI_files -type f -name \*.hex); do xxd -p -r $$file > $$(echo $$file | sed "s:hex:mid:"); done

hex:
	for file in $$(find MIDI_files -type f -name \*.mid); do xxd    -r $$file > $$(echo $$file | sed "s:mid:hex:"); done

