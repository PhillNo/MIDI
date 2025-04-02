default: extras/main.cpp src/MIDI_Data.cpp src/MIDI_Decoder.cpp src/MIDI_Encoder.cpp
	g++ -g -Wall -std=c++17 \
	-Iinclude/ \
	extras/main.cpp src/MIDI_Data.cpp src/MIDI_Decoder.cpp src/MIDI_Encoder.cpp \
	-o extras/main

mid:
	for file in $$(find extras/MIDI_files -type f -name \*.hex); do xxd -p -r $$file > $$(echo $$file | sed "s:.hex:.mid:"); done

hex:
	for file in $$(find extras/MIDI_files -type f -name \*.mid); do xxd -p    $$file > $$(echo $$file | sed "s:.mid:.hex:"); done

tests:
	rm extras/tests.txt || true
	rm extras/tmp.txt || true
	mkdir extras/encoded_files || true
	for file in $$(find extras/MIDI_files -maxdepth 1 -type f -name \*.mid ! -name \*encoded\* | sort); do echo -n $$file >> extras/tmp.txt; echo -n " " >> extras/tmp.txt; ./extras/main $$file $$(echo $$file | sed "s:.mid:_encoded.mid:" | sed "s:MIDI_files/:encoded_files/:")>> extras/tmp.txt; done
	cat extras/tmp.txt | column -t > extras/tests.txt
	rm extras/tmp.txt
	cat extras/tests.txt