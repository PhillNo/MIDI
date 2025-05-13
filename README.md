# MIDI

## Project Structure
```
.
|-- extras
|   |-- decode_reencode.cpp
|   |-- jobs
|   `-- MIDI_files
|-- include
|   |-- MIDI_Data.h
|   |-- MIDI_Decoder.h
|   |-- MIDI_Encoder.h
|   `-- Noncopyable.h
|-- makefile
|-- README.md
`-- src
    |-- MIDI_Data.cpp
    |-- MIDI_Decoder.cpp
    `-- MIDI_Encoder.cpp

```

## MIDI File Structure
```
 _____________________________________________________________
| MIDI File                                                   |
|    ______________    ____________________________________   |
|   | MThd chunk   |  | MTrk chunk                         |  |
|   |   length     |  |   length                           |  |
|   |   format     |  |    ______________________________  |  |
|   |   ntrks      |  |   | event                        | |  |
|   |   division   |  |   |   delta time                 | |  |
|   |   undefined  |  |   |   event payload              | |  |
|   |______________|  |   |______________________________| |  |
|                     |                ...                 |  |
|                     |    ______________________________  |  |
|                     |   | nth event                    | |  |
|                     |   |______________________________| |  |
|                     |____________________________________|  |
|                                      ...                    |
|                      ____________________________________   |
|                     | nth MTrk chunk                     |  |
|                     |____________________________________|  |
|                                                             |
|_____________________________________________________________|
```

## Header Files

### MIDI_Data.h
This file contains classes representing the data encoded in the different parts of a MIDI file as well as the `MIDI_File` class.

There are currently three classes to represent three different chunk types: MThd, MTrk, and \<Unknown\> in the cases where a MIDI file contains an unrecognized chunk header. The UNkn chunk type will store all of the bytes contained in that chunk in case a user does not want to use them but wishes to reencode them in a new MIDI file. For MThd chunks, the MIDI standard does not currently specify any data beyond length, format, number of tracks and division. The length attribute should not be hardcoded and ignored when decoding a file, however; MThd chunks must be open to additional data (which will currenty be stored as raw bytes without any interpretation).

The MIDI standard defines different MTrk event types, but in code they are all represented as a `MTrk_Event` object. The type of the event is implicit in the first byte of the event payload `bytes`. 

### MIDI_Decoder.h
A `MIDI_File_Decoder` object hydrates a `MIDI_File`. It reads bytes one-at-a-time with expectation that they follow the standard MIDI file specification. Because bytes are interpreted one-at-a-time by the decoder, they do not all need to be loaded into memory at once, and the decoding process is minimally-blocking since it can be done increments.

The `MIDI_File_Decoder` returns a `Status` to indicate if it expects to continue reading byes, if it has completely read a MIDI file, or if it has encountered an error in the input data. It does not rely on exceptions and it receives a pointer to the `MIDI_Element` it will hydrate.

The `MIDI_File_Decoder` is implemented by a finite state machine that contains decoder objects to determine chunk types as it encounters them, and objects to decode the different chunk types. This is a recursive-like way of decoding the different MIDI chunks that together compose a MIDI file, the events that compose the chunks, the time and payload information that composes the events, and so on.

A common interface is defined for the decoder objects with pure virtual functions that must be implemented. While the programmer of the decoders is forced to implement functions like `STATUS decode_byte(uint8_t next_byte, MIDI_Element* data) = 0;`, it is still up to the discipline of the programmer to static cast the `MIDI_Element` pointer to the correct MIDI object type to be hydrated by a particular decoder type.

Also to note is that, unlike a generic MTrk event type to representing MIDI data, there are separate decoders for the three event types: MIDI, Meta, and Sysex. This is necessary because the structure of how the events are stored in a file is different (i.e. length and status bytes) and not all of the information stored/omitted by the file is ultimately sent to MIDI devices during a performance (i.e. sysex *length* is stored in MIDI files but not sent to devices during performances).

### MIDI_Encoder.h
The `MIDI_File_Encoder` object is the inverse of the decoder and has an analogous interface: bytes are encoded one-at-a-time, a `STATUS` is returned, and a pointer for the data to by hydrated is an expected parameter. This again does not force the need for all of the MIDI file data to exist in memory and is minimally-blocking. The `MIDI_File_Encoder` is also implemented as a finite state machine making recursive-like calls to the FSMs that compose it.

Again, the encoder objects do not correspond to each MIDI data type (i.e. Sysex length is implicit in the length of `bytes` but the length must be explicitly encoded in a file, though not sent to devices during a performance).

## Tests
Tests are handled by a bash script for each test case. `make tests` will run each script immediately under `extras/jobs`. Scripts will return the string `pass` or `fail`, with more detailed test results stored in `extras/jobs/results/` as well as encoded MIDI files, if any, in `extras/jobs/encoded_files`.

Most tests work by decoding a file, encoding it again, and comparing byte-for-byte the input and output. In cases where MIDI file contents are edited, an input and separate expected output file are necessary.

.hex files in `extras/MIDI_files` are modified by hand and converted to the .mid files representing each test case.
