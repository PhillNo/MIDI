#include "MIDI_Encoder.h"

/* ****************************************************************************
 *  MIDI_Element_Encoder
 *  ************************************************************************ */
void MIDI_Element_Encoder::clear()
{
    src = nullptr;
    specific_index = 0;
    tmp.clear();
}


/* ****************************************************************************
 *  Varlen_Encoder
 *  ************************************************************************ */
void Varlen_Encoder::clear()
{
    MIDI_Element_Encoder::clear();
    current_state = STATE::READ;
}

MIDI_Element_Encoder::STATUS Varlen_Encoder::encode_byte(uint8_t& product)
{
    switch (current_state)
    {
        case STATE::READ:
        {
            if (specific_index < 3)
            {
                if (tmp[specific_index] == 128) // if leading byte stored in tmp is zero then skip
                {
                    ++specific_index;
                    return Varlen_Encoder::encode_byte(product); // RECURSION
                }

                if (!(tmp[specific_index] & 0b10000000))
                {
                    // test that last byte has bit 7 clear
                    return STATUS::FAIL;
                }
                else
                {
                    product = tmp[specific_index]; // last has msb clear
                    ++specific_index;
                }
            }
            else if (specific_index == 3)
            {
                if (tmp[specific_index] & 0b10000000)
                {
                    // test that last byte has bit 7 clear
                    current_state = STATE::FAIL;
                    return STATUS::FAIL;
                }
                else
                {
                    product = tmp[specific_index];
                    current_state = STATE::DONE;
                    return STATUS::SUCCESS;
                }
            }
            else
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }

            break;
        }
        case STATE::DONE:
        {
            return STATUS::FAIL;
        }
        case STATE::FAIL:
        {
            return STATUS::FAIL;
        }
    }
    

    return STATUS::STANDBY;
}

MIDI_Element_Encoder::STATUS Varlen_Encoder::set_data(uint32_t data)
{
    Varlen_Encoder::clear();

    uint32_t accumulator = data;

    // separate data into 4 8-bit bytes
    tmp.insert(tmp.end(), {0, 0, 0, 0});

    tmp[3] = (uint8_t)(accumulator & 0b01111111);
    accumulator >>= 7;

    tmp[2] = (uint8_t)(accumulator & 0b01111111);
    tmp[2] |= 0b10000000; // all bytes have bit 7 set except last/least sig.
    accumulator >>= 7;

    tmp[1] = (uint8_t)(accumulator & 0b01111111);
    tmp[1] |= 0b10000000;
    accumulator >>= 7;

    tmp[0] = (uint8_t)(accumulator & 0b01111111);
    tmp[0] |= 0b10000000;
    accumulator >>= 7;

    return MIDI_Element_Encoder::STATUS::SUCCESS;

}

MIDI_Element_Encoder::STATUS Varlen_Encoder::set_data(MIDI_Element* data)
{
    if (data == nullptr)
    {
        return STATUS::FAIL;
    }

    src = data;

    return Varlen_Encoder::set_data((static_cast<Varlen*>(src))->get_data());
}


/* ****************************************************************************
 * MIDI_Message_Encoder
 *  ************************************************************************ */
void MIDI_Message_Encoder::clear()
{
    MIDI_Element_Encoder::clear();
    src_event = nullptr;
    varlen_encoder.clear();
    current_state = STATE::DT;
    current_status = STATUS::STANDBY;   
}

MIDI_Element_Encoder::STATUS MIDI_Message_Encoder::encode_byte(uint8_t& product)
{
    switch (current_state)
    {
        case STATE::DT:
        {
            current_status = varlen_encoder.encode_byte(product);

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    current_state = STATE::PAYLOAD;
                    break;
                }
                case STATUS::FAIL:
                {
                    current_state = STATE::FAIL;
                    return STATUS::FAIL;
                }
            }
            
            break;
        }
        case STATE::PAYLOAD:
        {
            if (specific_index < src_event->get_size())
            {
                product = (*src_event)[specific_index];
                ++specific_index;
            }

            if (specific_index == src_event->get_size())
            {
                current_state = STATE::DONE;
                return STATUS::SUCCESS;
            }

            if (specific_index > src_event->get_size())
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }

            break;
        }
        case STATE::DONE:
        {
            return STATUS::FAIL;
        }
        case STATE::FAIL:
        {
            return STATUS::FAIL;
        }

    }

    return STATUS::STANDBY;
}

MIDI_Element_Encoder::STATUS MIDI_Message_Encoder::set_data(MIDI_Element* data)
{
    if (data == nullptr)
    {
        return STATUS::FAIL;
    }

    MIDI_Message_Encoder::clear();

    src = data;
    src_event = static_cast<MTrk_Event*>(src);
    varlen_encoder.set_data(src_event->get_dt());

    return STATUS::SUCCESS;
}

void MIDI_Message_Encoder::skip_status()
{
    if ((specific_index == 0) && (src_event->get_size() > 1))
    {
        specific_index = 1;
    }
}


/* ****************************************************************************
 * Meta_Message_Encoder
 *  ************************************************************************ */
void Meta_Message_Encoder::clear()
{
    MIDI_Element_Encoder::clear();
    src_event = nullptr;
    varlen_encoder.clear();
    current_state = STATE::DT;
    current_status = STATUS::STANDBY;   
}

MIDI_Element_Encoder::STATUS Meta_Message_Encoder::encode_byte(uint8_t& product)
{
    switch (current_state)
    {
        case STATE::DT:
        {
            current_status = varlen_encoder.encode_byte(product);

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    current_state = STATE::PAYLOAD;
                    break;
                }
                case STATUS::FAIL:
                {
                    current_state = STATE::FAIL;
                    return STATUS::FAIL;
                }
            }
            
            break;
        }
        case STATE::PAYLOAD:
        {
            if (specific_index < src_event->get_size())
            {
                product = (*src_event)[specific_index];
                ++specific_index;
            }

            if (specific_index == src_event->get_size())
            {
                current_state = STATE::DONE;
                return STATUS::SUCCESS;
            }

            if (specific_index > src_event->get_size())
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }

            break;
        }
        case STATE::DONE:
        {
            return STATUS::FAIL;
        }
        case STATE::FAIL:
        {
            return STATUS::FAIL;
        }
    }

    return STATUS::STANDBY;
}

MIDI_Element_Encoder::STATUS Meta_Message_Encoder::set_data(MIDI_Element* data)
{
    if (data == nullptr)
    {
        return STATUS::FAIL;
    }

    Meta_Message_Encoder::clear();

    src = data;
    src_event = static_cast<MTrk_Event*>(src);
    varlen_encoder.set_data(src_event->get_dt());

    return STATUS::SUCCESS;
}


/* ****************************************************************************
 * Sysex_Encoder
 * ************************************************************************* */
void Sysex_Message_Encoder::clear()
{
    MIDI_Element_Encoder::clear();

    src_event = nullptr;
    dt_encoder.clear();
    len_encoder.clear();
    current_state = STATE::DT;
    current_status = STATUS::STANDBY;
}

MIDI_Element_Encoder::STATUS Sysex_Message_Encoder::encode_byte(uint8_t& product)
{
    switch (current_state)
    {
        case STATE::DT:
        {
            current_status = dt_encoder.encode_byte(product);
            
            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    current_state = STATE::TYPE;
                    break;
                }
                case STATUS::FAIL:
                {
                    current_state = STATE::FAIL;
                    return STATUS::FAIL;
                }
            }

            break;
        }
        case STATE::TYPE:
        {
            if (specific_index != 0)
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }

            if (((*src_event)[0] == STATUS_BYTE::SYSEX_F0) || ((*src_event)[0] == STATUS_BYTE::SYSEX_F7))
            {
                product = (*src_event)[0];
                ++specific_index;
                current_state = STATE::LEN;
            }
            else
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }

            break;
        }
        case STATE::LEN:
        {
            current_status = len_encoder.encode_byte(product);

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    current_state = STATE::PAYLOAD;
                    break;
                }
                case STATUS::FAIL:
                {
                    current_state = STATE::FAIL;
                    return STATUS::FAIL;
                }
            }

            break;
        }
        case STATE::PAYLOAD:
        {
            if (specific_index < src_event->get_size())
            {
                product = (*src_event)[specific_index];
                ++specific_index;

                if (specific_index == src_event->get_size())
                {
                    current_state = STATE::DONE;
                    return STATUS::SUCCESS;
                }

            }
            else
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }
            
            break;
        }
        case STATE::DONE:
        {
            return STATUS::FAIL;
        }
        case STATE::FAIL:
        {
            return STATUS::FAIL;
        }
    }

    return STATUS::STANDBY;
}

MIDI_Element_Encoder::STATUS Sysex_Message_Encoder::set_data(MIDI_Element* data)
{
    if (data == nullptr)
    {
        return STATUS::FAIL;
    }

    Sysex_Message_Encoder::clear();

    src = data;
    src_event = static_cast<MTrk_Event*>(src);
    dt_encoder.set_data(src_event->get_dt());
    len_encoder.set_data(src_event->get_size() - 1); // len is byte count AFTER F0/F7

    return STATUS::SUCCESS;
}


/* ****************************************************************************
 * MTrk_Encoder
 *  ************************************************************************ */
void MTrk_Encoder::clear()
{
    MIDI_Element_Encoder::clear();
    
    src_chunk = nullptr;
    current_state = STATE::HEADER;
    current_status = STATUS::STANDBY;
    event_index = 0;
    running_status = 0;
    meta_encoder.clear();
    midi_encoder.clear();
    sysex_encoder.clear();
}

MIDI_Element_Encoder::STATUS MTrk_Encoder::encode_byte(uint8_t& product)
{
    switch (current_state)
    {
        case STATE::HEADER:
        {
            if (specific_index <= tmp.size())
            {
                product = tmp[specific_index];
                
                ++specific_index;

                if (specific_index == tmp.size())
                {
                    current_state = STATE::EVENT_TYPE;
                }
            }

            break;
        }
        case STATE::EVENT_TYPE:
        {
            // event index/bounds
            if (event_index < src_chunk->size())
            {
                // first byte of event PAYLOAD
                if ((*src_chunk)[event_index][0] == STATUS_BYTE::META)
                {
                    current_state = STATE::META_EVENT;
                    meta_encoder.set_data(&((*src_chunk)[event_index]));
                    ++event_index;
                    running_status = 0;
                    return encode_byte(product); // RECURSION
                }
                else if ((*src_chunk)[event_index][0] == STATUS_BYTE::SYSEX_F0 )
                {
                    current_state = STATE::SYSEX_EVENT;
                    sysex_encoder.set_data(&((*src_chunk)[event_index]));
                    ++event_index;
                    running_status = 0;
                    return encode_byte(product); // RECURSION
                }
                else if ((*src_chunk)[event_index][0] == STATUS_BYTE::SYSEX_F7 )
                {
                    current_state = STATE::SYSEX_EVENT;
                    sysex_encoder.set_data(&((*src_chunk)[event_index]));
                    ++event_index;
                    running_status = 0;
                    return encode_byte(product); // RECURSION
                }
                else if ((*src_chunk)[event_index][0] & 0b10000000 ) // First bit set AND not SYSEX_F0
                {

                    current_state = STATE::MIDI_EVENT;
                    midi_encoder.set_data(&((*src_chunk)[event_index]));
                    
                    if (running_status == ((*src_chunk)[event_index][0]) && ENCODE_RUNNING_STATUS)
                    {
                        midi_encoder.skip_status();
                    }

                    running_status = (*src_chunk)[event_index][0];
                    
                    ++event_index;

                    return encode_byte(product); // RECURSION
                }
            }
            else if (event_index == src_chunk->size())
            {
                current_state = STATE::DONE;
                return STATUS::SUCCESS;
            }
            else
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }

            break;
        }
        case STATE::MIDI_EVENT:
        {
            current_status = midi_encoder.encode_byte(product);
            
            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    if (event_index == src_chunk->size())
                    {
                        current_state = STATE::DONE;
                        return STATUS::SUCCESS;
                    }
                    else
                    {
                        current_state = STATE::EVENT_TYPE;
                    }
                    break;
                }
                case STATUS::FAIL:
                {
                    return STATUS::FAIL;
                }
            }
            break;
        }
        case STATE::META_EVENT:
        {
            current_status = meta_encoder.encode_byte(product);
            
            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    if (event_index == src_chunk->size())
                    {
                        current_state = STATE::DONE;
                        return STATUS::SUCCESS;
                    }
                    else
                    {
                        current_state = STATE::EVENT_TYPE;
                    }
                    break;
                }
                case STATUS::FAIL:
                {
                    return STATUS::FAIL;
                }
            }
            break;
        }
        case STATE::SYSEX_EVENT:
        {
            current_status = sysex_encoder.encode_byte(product);

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    if (event_index == src_chunk->size())
                    {
                        current_state = STATE::DONE;
                        return STATUS::SUCCESS;
                    }
                    else
                    {
                        current_state = STATE::EVENT_TYPE;
                    }
                    
                    break;
                }
                case STATUS::FAIL:
                {
                    current_state = STATE::FAIL;
                    return STATUS::FAIL;
                }
            }

            break;
        }
        case STATE::DONE:
        {
            return STATUS::FAIL;
        }
        case STATE::FAIL:
        {
            return STATUS::FAIL;
        }
    }

    return STATUS::STANDBY;
}

MIDI_Element_Encoder::STATUS MTrk_Encoder::set_data(MIDI_Element* data)
{
    if (data == nullptr)
    {
        return STATUS::FAIL;
    }

    MTrk_Encoder::clear();

    src = data;
    src_chunk = static_cast<MTrk_Chunk*>(src);

    event_index = 0;

    tmp.insert(tmp.end(),
    {
        (uint8_t)(((*src_chunk).get_header() >> 24) & 0xFF),
        (uint8_t)(((*src_chunk).get_header() >> 16) & 0xFF),
        (uint8_t)(((*src_chunk).get_header() >>  8) & 0xFF),
        (uint8_t)(((*src_chunk).get_header()      ) & 0xFF),

        (uint8_t)(((*src_chunk).get_len()    >> 24) & 0xFF),
        (uint8_t)(((*src_chunk).get_len()    >> 16) & 0xFF),
        (uint8_t)(((*src_chunk).get_len()    >>  8) & 0xFF),
        (uint8_t)(((*src_chunk).get_len()         ) & 0xFF)
    });

    return STATUS::SUCCESS;
}

/* ****************************************************************************
 * UNkn_Encoder
 *  ************************************************************************ */
void UNkn_Encoder::clear()
{
    MIDI_Element_Encoder::clear();
    src_chunk = nullptr;
    current_state = STATE::HEAD;
    payload_index = 0;
}

MIDI_Element_Encoder::STATUS UNkn_Encoder::encode_byte(uint8_t& product)
{
    switch (current_state)
    {
        case STATE::HEAD:
        {
            // chunk type encoded in `tmp` by MIDI_File_Encoder
            if (specific_index < tmp.size()) // `tmp` stores header bytes
            {
                product = tmp[specific_index];
                ++specific_index;

                if (specific_index == tmp.size())
                {
                    current_state = STATE::BODY;
                    break;
                }
            }

            break;
        }
        case STATE::BODY:
        {
            if (payload_index >= src_chunk->get_len())
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }
            else
            {
                product = (*src_chunk)[payload_index];
                ++specific_index;
                ++payload_index;
        
                if (payload_index == src_chunk->get_len())
                {
                    current_state = STATE::DONE;
                    return STATUS::SUCCESS;
                }
            }

            break;
        }
        case STATE::DONE:
        {
            return STATUS::FAIL;
        }
        case STATE::FAIL:
        {
            return STATUS::FAIL;
        }
    }
    
    return STATUS::STANDBY;
}

MIDI_Element_Encoder::STATUS UNkn_Encoder::set_data(MIDI_Element* data)
{
    if (data == nullptr)
    {
        return STATUS::FAIL;
    }

    UNkn_Encoder::clear();

    src = data;
    src_chunk = static_cast<UNkn_Chunk*>(src);

    tmp.insert(tmp.end(),
    {
        (uint8_t)(((*src_chunk).get_header() >> 24) & 0xFF),
        (uint8_t)(((*src_chunk).get_header() >> 16) & 0xFF),
        (uint8_t)(((*src_chunk).get_header() >>  8) & 0xFF),
        (uint8_t)(((*src_chunk).get_header()      ) & 0xFF),

        (uint8_t)(((*src_chunk).get_len()    >> 24) & 0xFF),
        (uint8_t)(((*src_chunk).get_len()    >> 16) & 0xFF),
        (uint8_t)(((*src_chunk).get_len()    >>  8) & 0xFF),
        (uint8_t)(((*src_chunk).get_len()         ) & 0xFF)
    });

    return STATUS::SUCCESS;
}


/* ****************************************************************************
 * MThd__Encoder
 *  ************************************************************************ */
void MThd_Encoder::clear()
{
    MIDI_Element_Encoder::clear();
    src_chunk = nullptr;
    current_state = STATE::MAIN;
    payload_index = 0;
}

MIDI_Element_Encoder::STATUS MThd_Encoder::encode_byte(uint8_t& product)
{
    switch (current_state)
    {
        case STATE::MAIN:
        {
            // chunk type encoded in `tmp` by MIDI_File_Encoder
            if (specific_index < tmp.size()) // `tmp` stores header bytes
            {
                product = tmp[specific_index];
                ++specific_index;

                if (specific_index == tmp.size())
                {
                    if (src_chunk->get_len() > 6)
                    {
                        current_state = STATE::EXTRA;
                        break;
                    }
                    else
                    {
                        current_state = STATE::DONE;
                        return STATUS::SUCCESS;
                    }
                }
            }

            break;
        }
        case STATE::EXTRA:
        {
            if (payload_index > src_chunk->get_len() - 6) // 6 for fmt, ntrks, div
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }
            else
            {
                product = (*src_chunk)[payload_index];
                ++specific_index;
                ++payload_index;
        
                if (payload_index == (src_chunk->get_len() - 6))
                {
                    current_state = STATE::DONE;
                    return STATUS::SUCCESS;
                }
            }

            break;
        }
        case STATE::DONE:
        {
            return STATUS::FAIL;
        }
        case STATE::FAIL:
        {
            return STATUS::FAIL;
        }
    }
    
    return STATUS::STANDBY;
}

MIDI_Element_Encoder::STATUS MThd_Encoder::set_data(MIDI_Element* data)
{
    if (data == nullptr)
    {
        return STATUS::FAIL;
    }

    MThd_Encoder::clear();

    src = data;
    src_chunk = static_cast<MThd_Chunk*>(src);

    tmp.insert(tmp.end(),
    {
        (uint8_t)(((*src_chunk).get_header() >> 24) & 0xFF),
        (uint8_t)(((*src_chunk).get_header() >> 16) & 0xFF),
        (uint8_t)(((*src_chunk).get_header() >>  8) & 0xFF),
        (uint8_t)(((*src_chunk).get_header()      ) & 0xFF),

        (uint8_t)(((*src_chunk).get_len()    >> 24) & 0xFF),
        (uint8_t)(((*src_chunk).get_len()    >> 16) & 0xFF),
        (uint8_t)(((*src_chunk).get_len()    >>  8) & 0xFF),
        (uint8_t)(((*src_chunk).get_len()         ) & 0xFF),

        (uint8_t)(((*src_chunk).get_fmt()    >>  8) & 0xFF),
        (uint8_t)(((*src_chunk).get_fmt()         ) & 0xFF),

        (uint8_t)(((*src_chunk).get_ntrks()  >>  8) & 0xFF),
        (uint8_t)(((*src_chunk).get_ntrks()       ) & 0xFF),

        (uint8_t)(((*src_chunk).get_div()    >>  8) & 0xFF),
        (uint8_t)(((*src_chunk).get_div()         ) & 0xFF),
    });

    return STATUS::SUCCESS;
}


/* ****************************************************************************
 *  MIDI_File_Encoder
 *  ************************************************************************ */
void MIDI_File_Encoder::clear()
{
    MIDI_Element_Encoder::clear();
    
    src_file = nullptr;
    current_state = STATE::MTHD_HEADER; 
    current_status = STATUS::STANDBY; 
    chunk_index = 0;
    mthd_encoder.clear();
    mtrk_encoder.clear();
    unkn_encoder.clear();
}

MIDI_Element_Encoder::STATUS MIDI_File_Encoder::encode_byte(uint8_t& product)
{
    switch (current_state)
    {
        case STATE::MTHD_HEADER:
        {
            current_status = mthd_encoder.encode_byte(product);

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    if (src_file->get_hdr().get_ntrks() > 0)
                    {
                        current_state = STATE::CHUNK_TYPE;
                        break;
                    }
                    else
                    {
                        current_state = STATE::DONE;
                        return STATUS::SUCCESS;
                    }
                }
                case MIDI_Element_Encoder::STATUS::FAIL:
                {
                    current_state = STATE::FAIL;
                    return STATUS::FAIL;
                }
            }

            break;
        }
        case STATE::CHUNK_TYPE:
        {
            if (chunk_index >= src_file->get_hdr().get_ntrks())
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }
            
            if ( (*src_file).get_chunk(chunk_index).get_header() == CHUNK_HEADER::MTRK)
            {
                current_state = STATE::MTRK;
                mtrk_encoder.clear();
                mtrk_encoder.set_data(&((*src_file).get_chunk(chunk_index)));
            }
            else
            {
                current_state = STATE::UNKN;
                unkn_encoder.clear();
                unkn_encoder.set_data(&((*src_file).get_chunk(chunk_index)));
            }

            ++chunk_index;

            return encode_byte(product); // RECURSION
        }
        case STATE::MTRK:
        {
            current_status = mtrk_encoder.encode_byte(product);

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    if (chunk_index == (src_file->get_hdr().get_ntrks()))
                    {
                        current_state = STATE::DONE;
                        return STATUS::SUCCESS;
                    }
                    else if (chunk_index > (src_file->get_hdr().get_ntrks()))
                    {
                        current_state = STATE::FAIL;
                        return STATUS::FAIL;
                    }

                    current_state = STATE::CHUNK_TYPE;
                    break;
                }
                case STATUS::FAIL:
                {
                    current_state = STATE::FAIL;
                    return STATUS::FAIL;
                }
            }
            break;
        }
        case STATE::UNKN:
        {
            current_status = unkn_encoder.encode_byte(product);

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    if (chunk_index == (src_file->get_hdr().get_ntrks()))
                    {
                        current_state = STATE::DONE;
                        return STATUS::SUCCESS;
                    }
                    else if (chunk_index > (src_file->get_hdr().get_ntrks()))
                    {
                        current_state = STATE::FAIL;
                        return STATUS::FAIL;
                    }

                    current_state = STATE::CHUNK_TYPE;
                    break;
                }
                case STATUS::FAIL:
                {
                    current_state = STATE::FAIL;
                    return STATUS::FAIL;
                }
            }
            break;
        }
        case STATE::DONE:
        {
            return STATUS::FAIL;
        }
        case STATE::FAIL:
        {
            return STATUS::FAIL;
        }
    }

    return STATUS::STANDBY;
}

MIDI_Element_Encoder::STATUS MIDI_File_Encoder::set_data(MIDI_Element* data)
{
    if (data == nullptr)
    {
        return STATUS::FAIL;
    }

    clear();

    src = data;
    src_file = static_cast<MIDI_File*>(src);

    mthd_encoder.set_data(&(src_file->get_hdr()));

    return STATUS::SUCCESS;

}
