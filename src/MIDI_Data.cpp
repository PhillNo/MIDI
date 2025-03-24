#include "MIDI_Data.h"

/* ****************************************************************************
*  MTrk_Event
*  ************************************************************************* */
void MTrk_Event::set_dt(uint32_t new_dt)
{
    dt.set_data(new_dt);
}

void MTrk_Event::push_byte(uint8_t new_byte)
{
    bytes.push_back(new_byte);
}

uint8_t MTrk_Event::operator[](size_t index)
{
    return bytes[index];
}

/* ****************************************************************************
*  MIDI_Chunk
*  ************************************************************************* */
MIDI_Chunk::MIDI_Chunk(){}

uint32_t MIDI_Chunk::get_header()
{
    return header;
}

void MIDI_Chunk::set_header(uint32_t new_header)
{
    header = new_header;
}

uint32_t MIDI_Chunk::get_len()
{
    return len;
}

void MIDI_Chunk::set_len(uint32_t new_len)
{
    len = new_len;
}


/* ****************************************************************************
*  MThd_Chunk
*  ************************************************************************* */
MThd_Chunk::MThd_Chunk()
{
    header = CHUNK_HEADER::MTHD;
}

uint16_t MThd_Chunk::get_fmt()
{
    return fmt;
}

uint16_t MThd_Chunk::get_ntrks()
{
    return ntrks;
}

uint16_t MThd_Chunk::get_div()
{
    return div;
}

void MThd_Chunk::set_fmt(uint16_t new_fmt)
{
    fmt = new_fmt;
}

void MThd_Chunk::set_ntrks(uint16_t new_ntrks)
{
    ntrks = new_ntrks;
}

void MThd_Chunk::set_div(uint16_t new_div)
{
    div = new_div;
}

void MThd_Chunk::push_byte(uint8_t new_byte)
{
    extended_content.push_back(new_byte);
}

uint8_t& MThd_Chunk::operator[](size_t index)
{
    return extended_content[index];
}

/* ****************************************************************************
*  MTrk_Chunk
*  ************************************************************************* */
MTrk_Chunk::MTrk_Chunk()
{
    header = CHUNK_HEADER::MTRK;
}

void MTrk_Chunk::emplace_event()
{
    events.emplace_back();
}

MTrk_Event& MTrk_Chunk::back()
{
    return events.back();
}

MTrk_Event& MTrk_Chunk::front()
{
    return events.front();
}

std::list<MTrk_Event>::iterator MTrk_Chunk::begin()
{
    return events.begin();
}

std::list<MTrk_Event>::iterator MTrk_Chunk::end()
{
    return events.end();
}

MTrk_Event& MTrk_Chunk::operator[](size_t index)
{
    std::_List_iterator<MTrk_Event> it = events.begin();
    
    for (size_t i = 0; i < index; ++i)
    {
        ++it;
    }

    return *it;
}

/* ****************************************************************************
*  UNkn_Chunk
*  ************************************************************************* */
UNkn_Chunk::UNkn_Chunk()
{
    header = 0x554E6B63; /* magic consatnt for "UNkn" */
}

void UNkn_Chunk::set_len(uint32_t new_len)
{
    len = new_len;
    bytes = std::vector<uint8_t>();
}

void UNkn_Chunk::push_bytes(uint8_t next_byte)
{
    bytes.push_back(next_byte);
}

uint8_t& UNkn_Chunk::back()
{
    return bytes.back();
}

uint8_t& UNkn_Chunk::front()
{
    return bytes.front();
}

std::vector<uint8_t>::iterator UNkn_Chunk::begin()
{
    return bytes.begin();
}

uint8_t UNkn_Chunk::operator[](size_t index)
{
    return bytes[index];
}

/* ****************************************************************************
*  MIDI_File
*  ************************************************************************* */
uint16_t MIDI_File::get_fmt()
{
    return header.get_fmt();
}

uint16_t MIDI_File::get_ntrks()
{
    return header.get_ntrks();
}

uint16_t MIDI_File::get_div()
{
    return header.get_div();
}

void MIDI_File::set_fmt(uint16_t new_fmt)
{
    header.set_fmt(new_fmt);
}

void MIDI_File::set_ntrks(uint16_t new_ntrks)
{
    header.set_ntrks(new_ntrks);
}

void MIDI_File::set_div(uint16_t new_div)
{
    header.set_div(new_div);
}

void MIDI_File::emplace_mtrk()
{
    mtrk_chunks.emplace_back();
    ordered_chunks.push_back(&mtrk_chunks.back());
}

void MIDI_File::emplace_unkn()
{
    unkn_chunks.emplace_back();
    ordered_chunks.push_back(&unkn_chunks.back());
}

MThd_Chunk& MIDI_File::get_hdr()
{
    return header;
}

MIDI_Chunk& MIDI_File::operator[](size_t index)
{
    return *(ordered_chunks[index]);
}
