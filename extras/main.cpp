#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>

#include "MIDI_Data.h"
#include "MIDI_Decoder.h"
#include "MIDI_Encoder.h"

using namespace std;

int main()
{
  string file_in = "MIDI_files/MIDI_w_junk_MThd.mid";
  /*
  string path2file = "MIDI_files/extended_MThd.mid";
  string path2file = "MIDI_files/escape_sysex.mid";
  string path2file = "MIDI_files/bad_sysex.mid";
  string path2file = "MIDI_files/basic_sysex.mid";
  string path2file = "MIDI_files/sample_UNkn_chunks.mid";
  string path2file = "MIDI_files/MIDI_sample.mid";
  string path2file = "MIDI_files/no_track.mid";
  */
  char file_out[] = "MIDI_Files/encoded.mid";

  streampos size;
  unique_ptr<char[]> mid_contents;
  ifstream file_reader (file_in, ios::in|ios::binary|ios::ate);
  ofstream file_writer;

  uint8_t curr_byte{};
  MIDI_File_Decoder dec;
  MIDI_File decoded;
  MIDI_File_Encoder enc;
  vector<uint8_t> encoded{};
  
  /****************************************
  Read the input .mid file
  *****************************************/
  if (file_reader.is_open())
  {
    size = file_reader.tellg();
    mid_contents = unique_ptr<char[]>(new char[size]);
    file_reader.seekg(0, ios::beg);
    file_reader.read(mid_contents.get(), size);
    file_reader.close();
  }
  else
  {
    cout << "file failed to open." << endl;
    return 0;
  }

  /****************************************
  Deserialize the .mid data
  ****************************************/
  for (int i = 0; i < size; ++i)
  {
    curr_byte = (uint8_t)(mid_contents[i]);
    
    MIDI_Element_Decoder::STATUS parse_status = dec.decode_byte(curr_byte, &decoded);
    while ((parse_status != MIDI_Element_Decoder::STATUS::STANDBY) && (parse_status != MIDI_Element_Decoder::STATUS::FAIL))
    {
        parse_status = dec.decode_byte(curr_byte, &decoded);
    }
  }
  
  /****************************************
  Serialize the MIDI file object
  ****************************************/
  enc.set_data(&decoded);
  
  while(enc.encode_byte(curr_byte) != MIDI_Element_Encoder::STATUS::FAIL)
  {
    encoded.push_back(curr_byte);
  }
  
  for (int i = 0; i < size; ++i)
  {
    if (encoded[i] != (uint8_t)( mid_contents[i] ))
    {
      cout << "diff at: " << i << endl;
      cout << (char)encoded[i] << ", " << (char)( mid_contents[i] ) << endl;
      break;
    }
  }
  
  /****************************************
  Write serialized data to new file
  ****************************************/
  file_writer.open(file_out,ios::out | ios :: binary );
  file_writer.write((char*)&(encoded[0]), encoded.size());
  
  cout << "done" << endl;
  
  return 0;

}
