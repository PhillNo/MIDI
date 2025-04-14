#include "MIDI_Data.h"

/* ****************************************************************************
*  Varlen
*  ************************************************************************* */
int Varlen::byte_count(uint32_t vlq)
{
    int count = 1;

    if (vlq < 0x8000)
    {
        count = 1;
    }
    else if (vlq <= 0x800000)
    {
        count = 2;
    }
    else if (vlq < 0x80000000)
    {
        count = 3;
    }
    else
    {
        count = 4;
    }

    return count;
}

int Varlen::byte_count()
{
    return byte_count(payload);
}

/* ****************************************************************************
*  MTrk_Event
*  ************************************************************************* */
void MTrk_Event::set_dt(uint32_t new_dt)
{
    dt.set_data(new_dt);
}

uint32_t MTrk_Event::get_size()
{
    uint32_t size{0};

    size += dt.byte_count();
    size += get_payload_size();

    if ((bytes[0] == STATUS_BYTE::SYSEX_F0) || (bytes[0] == STATUS_BYTE::SYSEX_F7))
    {
        size += Varlen::byte_count(get_payload_size());
    }

    return size;

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

uint32_t MTrk_Chunk::update_chunk_size()
{
    uint32_t size = 0;
    auto it = events.begin();

    while(it != events.end())
    {
        size += (*it).get_size();

        ++it;
    }

    return size;

}

MTrk_Event& MTrk_Chunk::emplace_back_event()
{
    auto tmp = events.emplace_back();
    update_chunk_size();
    return tmp;
}

MTrk_Event& MTrk_Chunk::emplace_event(size_t index)
{
    auto it = events.begin();

    for (size_t i = 0; i < index; ++i)
    {
        if (it == events.end())
        {
            return *events.end();
        }
        else
        {
            ++it;
        }
    }

    auto tmp = *events.emplace(it);
    update_chunk_size();
    return tmp;
}

MTrk_Event& MTrk_Chunk::insert_event(size_t index, MTrk_Event& event)
{
    auto it = events.begin();

    for (size_t i = 0; i < index; ++i)
    {
        if (it == events.end())
        {
            return *events.end();
        }
        else
        {
            ++it;
        }
    }

    auto tmp = *events.insert(it, event);
    update_chunk_size();
    return tmp;
}

void MTrk_Chunk::erase(size_t index)
{
    auto it = events.begin();

    for (size_t i = 0; i < index; ++i)
    {
        if (it == events.end())
        {
            break;
        }
        else
        {
            ++it;
        }
    }

    events.erase(it);
    update_chunk_size();
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
    header = 0x554E6B6E; /* magic consatnt for "UNkn" */
}

void UNkn_Chunk::set_len(uint32_t new_len)
{
    len = new_len;
    bytes = std::vector<uint8_t>();
}

void UNkn_Chunk::push_byte(uint8_t next_byte)
{
    bytes.push_back(next_byte);
}

uint8_t UNkn_Chunk::operator[](size_t index)
{
    return bytes[index];
}

/* ****************************************************************************
*  MIDI_File
*  ************************************************************************* */
MTrk_Chunk& MIDI_File::emplace_back_mtrk()
{
    mtrk_chunks.emplace_back();
    ordered_chunks.push_back(&mtrk_chunks.back());

    hdr.set_ntrks((uint16_t)ordered_chunks.size());

    return mtrk_chunks.back();
}

MTrk_Chunk& MIDI_File::emplace_mtrk(size_t absolute_index)
{
    auto combined_index = ordered_chunks.begin();
    auto mtrk_index = mtrk_chunks.begin();
    auto unkn_index = unkn_chunks.begin();

    for (size_t i = 0; i < absolute_index; ++i)
    {
        switch ((*combined_index)->get_header())
        {
            case CHUNK_HEADER::MTRK:
            {
                ++mtrk_index;
                break;
            }
            default:
            {
                ++unkn_index;
                break;
            }
        }

        ++combined_index;
    }

    mtrk_chunks.emplace(mtrk_index);
    ordered_chunks.emplace(combined_index, &(*mtrk_index));

    hdr.set_ntrks((uint16_t)ordered_chunks.size());
    
    return *mtrk_index;
}

MTrk_Chunk& MIDI_File::insert_mtrk(size_t absolute_index, const MTrk_Chunk& new_chunk)
{
    auto combined_index = ordered_chunks.begin();
    auto mtrk_index = mtrk_chunks.begin();
    auto unkn_index = unkn_chunks.begin();

    for (size_t i = 0; i < absolute_index; ++i)
    {
        switch ((*combined_index)->get_header())
        {
            case CHUNK_HEADER::MTRK:
            {
                ++mtrk_index;
                break;
            }
            default:
            {
                ++unkn_index;
                break;
            }
        }

        ++combined_index;
    }

    mtrk_chunks.emplace(mtrk_index, new_chunk);
    ordered_chunks.emplace(combined_index, &(*mtrk_index));

    hdr.set_ntrks((uint16_t)ordered_chunks.size());

    return *mtrk_index;
}


UNkn_Chunk& MIDI_File::emplace_back_unkn()
{
    unkn_chunks.emplace_back();
    ordered_chunks.push_back(&unkn_chunks.back());

    hdr.set_ntrks((uint16_t)ordered_chunks.size());

    return unkn_chunks.back();
}

UNkn_Chunk& MIDI_File::emplace_unkn(size_t absolute_index)
{
    auto combined_index = ordered_chunks.begin();
    auto mtrk_index = mtrk_chunks.begin();
    auto unkn_index = unkn_chunks.begin();

    for (size_t i = 0; i < absolute_index; ++i)
    {
        switch ((*combined_index)->get_header())
        {
            case CHUNK_HEADER::MTRK:
            {
                ++mtrk_index;
                break;
            }
            default:
            {
                ++unkn_index;
                break;
            }
        }

        ++combined_index;
    }

    unkn_chunks.emplace(unkn_index);
    ordered_chunks.emplace(combined_index, &(*unkn_index));

    hdr.set_ntrks((uint16_t)ordered_chunks.size());
    
    return *unkn_index;
}

UNkn_Chunk& MIDI_File::insert_unkn(size_t absolute_index, const UNkn_Chunk& new_chunk)
{
    auto combined_index = ordered_chunks.begin();
    auto mtrk_index = mtrk_chunks.begin();
    auto unkn_index = unkn_chunks.begin();

    for (size_t i = 0; i < absolute_index; ++i)
    {
        switch ((*combined_index)->get_header())
        {
            case CHUNK_HEADER::MTRK:
            {
                ++mtrk_index;
                break;
            }
            default:
            {
                ++unkn_index;
                break;
            }
        }

        ++combined_index;
    }

    unkn_chunks.emplace(unkn_index, new_chunk);
    ordered_chunks.emplace(combined_index, &(*unkn_index));

    hdr.set_ntrks((uint16_t)ordered_chunks.size());
    
    return *unkn_index;
}

void MIDI_File::erase(size_t absolute_index)
{
    auto combined_index = ordered_chunks.begin();
    auto mtrk_index = mtrk_chunks.begin();
    auto unkn_index = unkn_chunks.begin();

    for (size_t i = 0; i < absolute_index; ++i)
    {
        switch ((*combined_index)->get_header())
        {
            case CHUNK_HEADER::MTRK:
            {
                ++mtrk_index;
                break;
            }
            default:
            {
                ++unkn_index;
                break;
            }
        }

        ++combined_index;
    }

    unkn_chunks.erase(unkn_index);
    ordered_chunks.erase(combined_index);

    hdr.set_ntrks((uint16_t)ordered_chunks.size());
    
}

MThd_Chunk& MIDI_File::get_hdr()
{
    return hdr;
}

MIDI_Chunk& MIDI_File::get_chunk(size_t index)
{
    return *(ordered_chunks[index]);
}

MTrk_Chunk& MIDI_File::get_MTrk(size_t index)
{
    auto it = mtrk_chunks.begin();

    for (size_t i = 0; i < index; ++i)
    {
        if (it == mtrk_chunks.end())
        {
            return *it;
        }
        else
        {
            ++it;
        }
    }

    return *it;
}

MTrk_Chunk& MIDI_File::operator[](size_t index)
{
    return get_MTrk(index);
}

