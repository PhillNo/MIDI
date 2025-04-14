#include "MIDI_Decoder.h"

/* ****************************************************************************
 *  MIDI_Element
 *  ************************************************************************* */
void MIDI_Element_Decoder::clear()
{
    index = 0;
}

/* ****************************************************************************
 *  Var_Len
 *  ************************************************************************* */
MIDI_Element_Decoder::STATUS Varlen_Decoder::decode_byte(uint8_t next_byte, MIDI_Element* data)
{
    ++index;

    switch (current_state)
    {
        case STATE::READ:
        {
            varlen = (varlen << 7) | (next_byte & 0b01111111);

            if (index > 4) // MIDI variable length elements <= 32-bits deep
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }
            else if ((next_byte & 0b10000000) > 0) // bit 7 is set, expect more bytes
            {
                return STATUS::STANDBY;
            }
            else // bit 7 clear, done reading varlen
            {
                current_state = STATE::DONE;
                return STATUS::SUCCESS;
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

void Varlen_Decoder::clear()
{
    MIDI_Element_Decoder::clear();
    current_state = STATE::READ;
    varlen = 0;
}

/* ****************************************************************************
 *  Len
 *  ************************************************************************* */
MIDI_Element_Decoder::STATUS Chunk_Length_Decoder::decode_byte(uint8_t next_byte, MIDI_Element* data)
{
    ++index;

    switch (current_state)
    {
        case STATE::READ:
        {
            if (index > 4)
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }
            else
            {
                len = (uint32_t)(len << 8) | (uint32_t)next_byte;
            }

            if (index == 4)
            {
                current_state = STATE::DONE;
                return STATUS::SUCCESS;
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

void Chunk_Length_Decoder::clear()
{
    MIDI_Element_Decoder::clear();
    current_state = STATE::READ;
    len = 0;
}

/* ****************************************************************************
 *  MIDI_Event
 *  ************************************************************************* */
MIDI_Element_Decoder::STATUS MIDI_Event_Decoder::set_type(uint8_t new_running_status)
{
    // Channel Voice Messages
    switch (new_running_status & 0b11110000)
    {
        case STATUS_BYTE::NOTE_OFF:
        {
            parameter_count = 2;
            break;
        }
        case STATUS_BYTE::NOTE_ON:
        {
            parameter_count = 2;
            break;
        }
        case STATUS_BYTE::AFTERTOUCH:
        {
            parameter_count = 2;
            break;
        }
        case STATUS_BYTE::CONTROL_CHANGE:
        {
            parameter_count = 2;
            break;
        }
        case STATUS_BYTE::PATCH_CHANGE:
        {
            parameter_count = 1;
            break;
        }
        case STATUS_BYTE::CHANNEL_PRESSURE:
        {
            parameter_count = 1;
            break;
        }
        case STATUS_BYTE::PITCH_BEND:
        {
            parameter_count = 2;
            break;
        }
        default:
        {
            return STATUS::FAIL;
        }
    }

    // System Messages
    if ((new_running_status & 0xF0) == 0xF0)
    {
        switch (new_running_status)
        {
            case STATUS_BYTE::META:
            {
                return STATUS::FAIL;
            }
            case STATUS_BYTE::SYSEX_F0:
            {
                return STATUS::FAIL;
            }
            case STATUS_BYTE::SYSEX_F7:
            {
                return STATUS::FAIL;
            }
            case 0xF1: // undefined
            {
                parameter_count = 0;
                break;
            }
            case STATUS_BYTE::SONG_POSITION:
            {
                parameter_count = 2;
                break;
            }
            case STATUS_BYTE::SONG_SELECT:
            {
                parameter_count = 1;
                break;
            }
            case 0xF4: // undefined
            {
                parameter_count = 0;
                break;
            }
            case 0xF5: // undefined
            {
                parameter_count = 0;
                break;
            }
            case STATUS_BYTE::TUNE_REQUEST:
            {
                parameter_count = 0;
                break;
            }
            case STATUS_BYTE::CLOCK:
            {
                parameter_count = 0;
                break;
            }
            case 0xF9: // undefined
            {
                parameter_count = 0;
                break;
            }
            case STATUS_BYTE::START:
            {
                parameter_count = 0;
                break;
            }
            case STATUS_BYTE::CONTINUE:
            {
                parameter_count = 0;
                break;
            }
            case STATUS_BYTE::STOP:
            {
                parameter_count = 0;
                break;
            }
            case 0xFD: // undefined
            {
                parameter_count = 0;
                break;
            }
            case STATUS_BYTE::ACTIVE_SENSING:
            {
                parameter_count = 0;
                break;
            }
            default:
            {
                return STATUS::FAIL;
            }
        }
    }
    return STATUS::SUCCESS;
}

MIDI_Element_Decoder::STATUS MIDI_Event_Decoder::decode_byte(uint8_t next_byte, MIDI_Element* data)
{
    if (data == nullptr)
    {
        current_state = STATE::FAIL;
        return STATUS::FAIL;
    }

    MTrk_Event& product = static_cast<MTrk_Event&>(*data);

    ++index;

    switch (current_state)
    {
        case STATE::STATUS_BYTE:
        {
            if (index != 1)
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }

            if (set_type(next_byte) == STATUS::FAIL)
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }

            product.push_byte(next_byte);

            current_state = STATE::PAYLOAD;

            break;
        }
        case STATE::PAYLOAD:
        {
            if ((index - 1) == parameter_count)
            {
                product.push_byte(next_byte);
                current_state = STATE::DONE;
                return STATUS::SUCCESS;
            }
            else
            {
                product.push_byte(next_byte);
                return STATUS::STANDBY;
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

void MIDI_Event_Decoder::clear()
{
    MIDI_Element_Decoder::clear();
    parameter_count = 0;
    current_state = STATE::STATUS_BYTE;
    current_status = STATUS::STANDBY;
}

MIDI_Element_Decoder::STATUS Meta_Event_Decoder::decode_byte(uint8_t next_byte, MIDI_Element* data)
{
    if (data == nullptr)
    {
        current_state = STATE::FAIL;
        return STATUS::FAIL;
    }

    MTrk_Event& product = static_cast<MTrk_Event&>(*data);

    ++index;

    switch (current_state)
    {
        case STATE::FF:
        {
            if (next_byte == STATUS_BYTE::META)
            {
                product.push_byte(next_byte);
            }
            else
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }
            current_state = STATE::TYPE;
            break;
        }
        case STATE::TYPE:
        {
            product.push_byte(next_byte);
            current_state = STATE::LEN;
            break;
        }
        case STATE::LEN:
        {
            product.push_byte(next_byte);
            current_status = len_decoder.decode_byte(next_byte, nullptr);
            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    len = len_decoder.get();
                    end = (uint32_t)index + len;

                    if (len == 0)
                    {
                        return STATUS::SUCCESS;
                    }

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
            product.push_byte(next_byte);

            if (index < end)
            {
                current_status = STATUS::STANDBY;
                break;
            }
            else if (index == end)
            {
                current_state = STATE::DONE;
                current_status = STATUS::SUCCESS;
                return current_status;
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

void Meta_Event_Decoder::clear()
{
    current_state = STATE::FF;
    current_status = STATUS::STANDBY;
    len = 0;
    end = 0;
    len_decoder.clear();
}

MIDI_Element_Decoder::STATUS Sysex_Event_Decoder::decode_byte(uint8_t next_byte, MIDI_Element* data)
{
    if (data == nullptr)
    {
        current_state = STATE::FAIL;
        return STATUS::FAIL;
    }

    MTrk_Event& product = static_cast<MTrk_Event&>(*data);

    ++index;

    switch (current_state)
    {
        case STATE::TYPE:
        {
            if ((next_byte == STATUS_BYTE::SYSEX_F0) || (next_byte == STATUS_BYTE::SYSEX_F7))
            {
                product.push_byte(next_byte);
            }
            else
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }

            current_state = STATE::LEN;
            break;
        }
        case STATE::LEN:
        {

            /* SYSEX length is stored in file AND NOT sent down wire (unlike meta events)
            if a sysex ends in F7, that byte will be included in the len 
            if a status byte is encountered before F7 then listening devices will 
            assume a new event */

            current_status = varlen_decoder.decode_byte(next_byte, nullptr); // varlen not stored in payload bytes

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    len = varlen_decoder.get();
                    end = (uint32_t)index + len;

                    if (len == 0)
                    {
                        return STATUS::SUCCESS;
                    }

                    current_state = STATE::PAYLOAD;
                    break;
                }
                case STATUS::FAIL:
                {
                    return STATUS::FAIL;
                }
            }
            break;
        }
        case STATE::PAYLOAD:
        {
            if ((((next_byte & 0b10000000) && (next_byte != STATUS_BYTE::SYSEX_F7))) || (index > end)) // F7 terminates message
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }
            else
            {
                product.push_byte(next_byte);

                if (index < end)
                {
                    current_status = STATUS::STANDBY;
                    break;
                }
                else // bytes_read == end
                {
                    current_state = STATE::DONE;
                    current_status = STATUS::SUCCESS;
                    return current_status;
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

void Sysex_Event_Decoder::clear()
{
    current_state = STATE::TYPE;
    current_status = STATUS::STANDBY;
    len = 0;
    end = 0;
    varlen_decoder.clear();
}

MIDI_Element_Decoder::STATUS MTrk_Events_Decoder::decode_byte(uint8_t next_byte, MIDI_Element* data)
{
    if (data == nullptr)
    {
        current_state = STATE::FAIL;
        return STATUS::FAIL;
    }

    MTrk_Chunk& product = static_cast<MTrk_Chunk&>(*data);

    ++index;

    switch (current_state)
    {
        case STATE::DT:
        {
            current_status = varlen_decoder.decode_byte(next_byte, nullptr);

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    product.emplace_back_event();
                    product.back().set_dt(varlen_decoder.get());
                    current_state = STATE::MESSAGE_TYPE;
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
        case STATE::MESSAGE_TYPE:
        {
            if (next_byte == STATUS_BYTE::META)
            {
                current_state = STATE::META;
                meta_decoder.clear();
                return decode_byte(next_byte, &product); // RECURSION
            }
            else if ((next_byte == STATUS_BYTE::SYSEX_F0) || (next_byte == STATUS_BYTE::SYSEX_F7))
            {
                current_state = STATE::SYSEX;
                sysex_decoder.clear();
                return decode_byte(next_byte, &product); // RECURSION
            }
            else // Default to MIDI if not Meta or Sysex
            {
                current_state = STATE::MIDI;

                if (next_byte & 0b10000000) // setting new running_status
                {
                    running_status = next_byte;
                    midi_decoder.clear();
                    return decode_byte(next_byte, &product); // RECURSION
                }
                else
                {
                    // using current running status
                    midi_decoder.clear();
                    decode_byte(running_status, &product);
                    return decode_byte(next_byte, &product); // RECURSION
                }
            }
            break;
        }
        case STATE::META:
        {
            current_status = meta_decoder.decode_byte(next_byte, &(product.back()));
            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    return STATUS::STANDBY;
                }
                case STATUS::SUCCESS:
                {
                    current_state = STATE::DT;
                    varlen_decoder.clear();
                    return STATUS::SUCCESS;
                }
                case STATUS::FAIL:
                {
                    current_state = STATE::FAIL;
                    return STATUS::FAIL;
                }
            }

            break;
        }
        case STATE::SYSEX:
        {
            current_status = sysex_decoder.decode_byte(next_byte, &(product.back()));
            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    return STATUS::STANDBY;
                }
                case STATUS::SUCCESS:
                {
                    current_state = STATE::DT;
                    varlen_decoder.clear();
                    return STATUS::SUCCESS;
                }
                case STATUS::FAIL:
                {
                    current_state = STATE::FAIL;
                    return STATUS::FAIL;
                }
            }
            break;
        }
        case STATE::MIDI:
        {
            current_status = midi_decoder.decode_byte(next_byte, &(product.back()));
            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    current_state = STATE::DT;
                    varlen_decoder.clear();
                    return STATUS::SUCCESS;
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

void MTrk_Events_Decoder::clear()
{
    MIDI_Element_Decoder::clear();

    current_state = STATE::DT;
    current_status = STATUS::STANDBY;
    running_status = 0;
    payload_size = 0;
    varlen_decoder.clear();
    midi_decoder.clear();
    meta_decoder.clear();
    sysex_decoder.clear();
}

/* ****************************************************************************
 *  Chunk_Type
 *  ************************************************************************* */
MIDI_Element_Decoder::STATUS Chunk_Type_Decoder::decode_byte(uint8_t next_byte, MIDI_Element* data)
{
    ++index;

    switch (current_state)
    {
        case STATE::READ:
        {
            if (index > 4)
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }

            four_bytes = (uint32_t)(four_bytes << 8) | (uint32_t)next_byte;

            if (index == 4)
            {
                if (four_bytes == CHUNK_HEADER::MTHD)
                {
                    found = CHUNK_TYPE::MTHD;
                }
                else if (four_bytes == CHUNK_HEADER::MTRK)
                {
                    found = CHUNK_TYPE::MTRK;
                }
                else
                {
                    found = CHUNK_TYPE::UNKN;
                }

                current_state = STATE::DONE;

                return STATUS::SUCCESS;
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

void Chunk_Type_Decoder::clear()
{
    MIDI_Element_Decoder::clear();
    current_state = STATE::READ;
    four_bytes = 0;
    found = CHUNK_TYPE::UNKN;
}

CHUNK_TYPE Chunk_Type_Decoder::get_type()
{
    return found;
}

uint32_t Chunk_Type_Decoder::get_header()
{
    return four_bytes;
}

/* ****************************************************************************
 *  Chunk
 *  ************************************************************************* */
MIDI_Element_Decoder::STATUS UNkn_Chunk_Decoder::decode_byte(uint8_t next_byte, MIDI_Element* data)
{
    if (data == nullptr)
    {
        current_state = STATE::FAIL;
        return STATUS::FAIL;
    }

    UNkn_Chunk& product = static_cast<UNkn_Chunk&>(*data);

    ++index;

    switch (current_state)
    {
        case STATE::CHUNK_LEN:
        {
            current_status = chunk_len_decoder.decode_byte(next_byte, nullptr);
            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    product.set_len(chunk_len_decoder.get_len());
                    current_state = STATE::CHUNK_BODY;
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
        case STATE::CHUNK_BODY:
        {
            if ((index - 4) > product.get_len())
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }

            product.push_byte(next_byte);

            if ((index - 4) < product.get_len())
            {
                current_status = STATUS::STANDBY;
            }
            else if ((index - 4) == product.get_len())
            {
                current_state = STATE::DONE;
                current_status = STATUS::SUCCESS;
                return STATUS::SUCCESS;
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

void UNkn_Chunk_Decoder::clear()
{
    MIDI_Element_Decoder::clear();

    chunk_len_decoder.clear();

    current_state = STATE::CHUNK_LEN;
    current_status = STATUS::STANDBY;
}

MIDI_Element_Decoder::STATUS MTrk_Chunk_Decoder::decode_byte(uint8_t next_byte, MIDI_Element* data)
{
    if (data == nullptr)
    {
        current_state = STATE::FAIL;
        return STATUS::FAIL;
    }

    MTrk_Chunk& product = static_cast<MTrk_Chunk&>(*data);

    ++index;

    switch (current_state)
    {
        case STATE::CHUNK_LEN:
        {
            current_status = chunk_len_decoder.decode_byte(next_byte, nullptr);
            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    product.set_len(chunk_len_decoder.get_len());
                    current_state = STATE::EVENTS;
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
        case STATE::EVENTS:
        {
            if ((index - 4) < product.get_len())
            {
                len_status = STATUS::STANDBY;
            }
            else if ((index - 4) == product.get_len())
            {
                len_status = STATUS::SUCCESS;
            }
            else
            {
                len_status = STATUS::FAIL;
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }

            switch (len_status)
            {
                case STATUS::STANDBY:
                {
                    current_status = event_decoder.decode_byte(next_byte, &product);

                    break;
                }
                case STATUS::SUCCESS:
                {
                    current_status = event_decoder.decode_byte(next_byte, &product);

                    // last byte in chunk must coincide
                    // with last byte of last event
                    if (current_status != STATUS::SUCCESS)
                    {
                        current_state = STATE::FAIL;
                        return STATUS::FAIL;
                    }

                    current_state = STATE::DONE;

                    return STATUS::SUCCESS;

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

void MTrk_Chunk_Decoder::clear()
{
    MIDI_Element_Decoder::clear();

    chunk_len_decoder.clear();
    event_decoder.clear();

    current_state = STATE::CHUNK_LEN;
    len_status = STATUS::STANDBY;
    current_status = STATUS::STANDBY;
}

MIDI_Element_Decoder::STATUS MThd_Param_Decoder::decode_byte(uint8_t next_byte, MIDI_Element* data)
{
    return STATUS::FAIL;
}

MIDI_Element_Decoder::STATUS MThd_Param_Decoder::decode_byte(uint8_t next_byte, MIDI_Element* data, MTHD_PARAM cur_param)
{
    if (data == nullptr)
    {
        current_state = STATE::FAIL;
        return STATUS::FAIL;
    }

    MThd_Chunk& product = static_cast<MThd_Chunk&>(*data);
    
    ++index;

    switch (current_state)
    {
        case STATE::READ:
        {
            if (index > 2)
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }
            else
            {
                param = (uint16_t)(param << 8) | (uint16_t)next_byte;
            }

            if (index == 2)
            {
                current_state = STATE::DONE;

                switch (cur_param)
                {
                    case MTHD_PARAM::FMT:
                    {
                        product.set_fmt(param);
                        break;
                    }
                    case MTHD_PARAM::NTRKS:
                    {
                        product.set_ntrks(param);
                        break;
                    }
                    case MTHD_PARAM::DIV:
                    {
                        product.set_div(param);
                        break;
                    }
                }

                return STATUS::SUCCESS;
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

void MThd_Param_Decoder::clear()
{
    MIDI_Element_Decoder::clear();
    current_state = STATE::READ;
    param = 0;
}

void MThd_Chunk_Decoder::clear()
{
    MIDI_Element_Decoder::clear();

    mthd_len_decoder.clear();
    mthd_param_decoder.clear();
    current_state = STATE::LEN;
    current_status = STATUS::STANDBY;
    extended_last = 0;
}

MIDI_Element_Decoder::STATUS MThd_Chunk_Decoder::decode_byte(uint8_t next_byte, MIDI_Element* data)
{
    if (data == nullptr)
    {
        current_state = STATE::FAIL;
        return STATUS::FAIL;
    }

    MThd_Chunk& product = static_cast<MThd_Chunk&>(*data);

    ++index;

    switch (current_state)
    {
        case STATE::LEN:
        {
            if (index > 4)
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }

            current_status = mthd_len_decoder.decode_byte(next_byte, nullptr);

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    product.set_len(mthd_len_decoder.get_len());

                    if (product.get_len() > 6)
                    {
                        extended_last = 7 + product.get_len();
                    }

                    current_state = STATE::FMT;

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
        case STATE::FMT:
        {
            if (index > 6)
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }

            current_status = mthd_param_decoder.decode_byte(next_byte, &product, MThd_Param_Decoder::MTHD_PARAM::FMT);

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    current_state = STATE::NTRKS;
                    product.set_fmt(mthd_param_decoder.get());
                    mthd_param_decoder.clear();

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
        case STATE::NTRKS:
        {
            if (index > 8)
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }

            current_status = mthd_param_decoder.decode_byte(next_byte, &product, MThd_Param_Decoder::MTHD_PARAM::NTRKS);

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    current_state = STATE::DIV;
                    product.set_ntrks(mthd_param_decoder.get());
                    mthd_param_decoder.clear();

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
        case STATE::DIV:
        {
            if (index > 10)
            {
                current_state = STATE::FAIL;
                return STATUS::FAIL;
            }

            current_status = mthd_param_decoder.decode_byte(next_byte, &product, MThd_Param_Decoder::MTHD_PARAM::DIV);

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    product.set_div(mthd_param_decoder.get());
                    mthd_param_decoder.clear();
                    if (product.get_len() > 6)
                    {
                        current_state = STATE::EXTENDED;
                        extended_last = product.get_len() + 4;
                    }
                    else
                    {
                        current_state = STATE::DONE;
                        return STATUS::SUCCESS;
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
        case STATE::EXTENDED:
        {
            if (index < extended_last)
            {
                product.push_byte(next_byte);
            }
            else if (index == extended_last)
            {
                product.push_byte(next_byte);
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

/* ****************************************************************************
 *  File
 *  ************************************************************************* */
MIDI_Element_Decoder::STATUS MIDI_File_Decoder::decode_byte(uint8_t next_byte, MIDI_Element* data)
{
    if (data == nullptr)
    {
        current_state = STATE::FAIL;
        return STATUS::FAIL;
    }

    MIDI_File& product = static_cast<MIDI_File&>(*data);

    ++index;

    switch (current_state)
    {
        case STATE::CHUNK_TYPE:
        {
            current_status = chunk_type_decoder.decode_byte(next_byte, nullptr);

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    switch (chunk_type_decoder.get_type())
                    {
                        case CHUNK_TYPE::MTHD:
                        {
                            current_state = STATE::MTHD;
                            mthd_decoder.clear();
                            break;
                        }
                        case CHUNK_TYPE::MTRK:
                        {
                            product.emplace_back_mtrk();
                            current_state = STATE::MTRK;
                            mtrk_decoder.clear();
                            break;
                        }
                        case CHUNK_TYPE::UNKN:
                        {
                            product.emplace_back_unkn();
                            (static_cast<UNkn_Chunk &>(product.get_chunk(track_index))).set_header(chunk_type_decoder.get_header());
                            current_state = STATE::UNKN;
                            unkn_decoder.clear();
                            break;
                        }
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
        case STATE::MTHD:
        {
            current_status = mthd_decoder.decode_byte(next_byte, &(product.get_hdr()));

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    expected_tracks = product.get_hdr().get_ntrks();

                    if (expected_tracks > 0)
                    {
                        current_state = STATE::CHUNK_TYPE;
                        chunk_type_decoder.clear();
                        break;
                    }
                    {
                        current_state = STATE::DONE;
                        return STATUS::SUCCESS;    
                    }
                }
                case STATUS::FAIL:
                {
                    current_state = STATE::FAIL;
                    return STATUS::FAIL;
                }
            }

            break;
        }
        case STATE::MTRK:
        {
            current_status = mtrk_decoder.decode_byte(next_byte, &(static_cast<MTrk_Chunk &>(product[track_index])));

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    if ((uint16_t)track_index < (expected_tracks - 1))
                    {
                        current_state = STATE::CHUNK_TYPE;
                        chunk_type_decoder.clear();
                        ++track_index;
                    }
                    else
                    {
                        current_state = STATE::DONE;
                        return STATUS::SUCCESS;
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
        case STATE::UNKN:
        {
            current_status = unkn_decoder.decode_byte(next_byte, &(static_cast<UNkn_Chunk &>(product.get_chunk(track_index))));

            switch (current_status)
            {
                case STATUS::STANDBY:
                {
                    break;
                }
                case STATUS::SUCCESS:
                {
                    if ((uint16_t)track_index < (expected_tracks - 1))
                    {
                        current_state = STATE::CHUNK_TYPE;
                        chunk_type_decoder.clear();
                        ++track_index;
                    }
                    else
                    {
                        current_state = STATE::DONE;
                        return STATUS::SUCCESS;
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

void MIDI_File_Decoder::clear()
{
    MIDI_Element_Decoder::clear();
    chunk_type_decoder.clear();
    unkn_decoder.clear();
    mtrk_decoder.clear();
    mthd_decoder.clear();
    current_status = STATUS::STANDBY;
    current_state = STATE::CHUNK_TYPE;
    track_index = 0;
}
