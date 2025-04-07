#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>

#include "MIDI_Data.h"
#include "MIDI_Decoder.h"
#include "MIDI_Encoder.h"

using namespace std;

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    return 1;
  }

  char const* file_in  = argv[1];
  char const* file_out = argv[2];

  uint8_t curr_byte{};
  MIDI_File_Decoder dec{};
  MIDI_File decoded{};
  MIDI_File_Encoder enc{};
  vector<uint8_t> encoded{};

  streampos size{};
  unique_ptr<char[]> midi_contents;
  ofstream file_writer{};
  ifstream file_reader (file_in, ios::in|ios::binary|ios::ate);
  
  /****************************************
  Read the input .mid file
  *****************************************/
  if (file_reader.is_open())
  {
    size = file_reader.tellg();
    midi_contents = unique_ptr<char[]>(new char[size]);
    file_reader.seekg(0, ios::beg);
    file_reader.read(midi_contents.get(), size);
    file_reader.close();
  }
  else
  {
    cout << "file failed to open .mid file ";
    return 1;
  }

  /****************************************
  Deserialize the .mid data
  ****************************************/
  for (int i = 0; i < size; ++i)
  {
    curr_byte = (uint8_t)(midi_contents[i]);
    
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
    if (encoded[i] != (uint8_t)( midi_contents[i] ))
    {
      cout << "diff at: " << i << " ";
      //cout << (char)encoded[i] << ", " << (char)( midi_contents[i] ) << endl;
      return 1;
    }
  }
  
  /****************************************
  Write serialized data to new file
  ****************************************/
  file_writer.open(file_out,ios::out | ios :: binary );
  file_writer.write((char*)&(encoded[0]), encoded.size());
  
  cout << "pass" << endl;
  
  return 0;

}
