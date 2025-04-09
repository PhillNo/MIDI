#ifndef MIDI_DATA_H
#define MIDI_DATA_H

#include <array>
#include <cstdint>
#include <list>
#include <vector>

enum        CHUNK_HEADER: uint32_t
{ 
    MTHD = 0x4D546864,
    MTRK = 0x4D54726B
};

enum class  CHUNK_TYPE
{
    MTHD,
    MTRK,
    UNKN
};

enum class  EVENT_TYPE
{
    MIDI,
    META,
    SYSEX
};

enum        STATUS_BYTE: uint8_t
{
    NOTE_ON          = 0x80,
    NOTE_OFF         = 0x90,
    AFTERTOUCH       = 0xA0,
    CONTROL_CHANGE   = 0xB0,
    PATCH_CHANGE     = 0xC0,
    CHANNEL_PRESSURE = 0xD0,
    PITCH_BEND       = 0xE0,
    SYSEX_F0         = 0xF0,
    SONG_POSITION    = 0xF2,
    SONG_SELECT      = 0xF3,
    TUNE_REQUEST     = 0xF6,
    SYSEX_F7         = 0xF7,
    CLOCK            = 0xF8,
    START            = 0xFA,
    CONTINUE         = 0xFB,
    STOP             = 0xFC,
    ACTIVE_SENSING   = 0xFE,
    META             = 0xFF //RESET when sent to devices
};

/* ****************************************************************************
*  MIDI_Data
**************************************************************************** */
class MIDI_Element
/*
 No implementation. This class provides the abaility for a pure virtual function
 to accept a MIDI_Element* parameter. This somewhat constrains what types are
 passed to the implementations of those virtual functions but static casts are
 still performed in most cases.
*/
{
protected:
    MIDI_Element(){}
};


/* ****************************************************************************
*  Variable-Length Quantity (VLQ)
*  ************************************************************************* */
class Varlen :                              public MIDI_Element
{
protected:
                    uint32_t                payload{0};
public:
        inline      void                    set_data(uint32_t new_payload){ payload = new_payload; }
        inline      uint32_t                get_data(){ return payload; };
};


/* ****************************************************************************
*  MTrk_Event
*  ************************************************************************* */
class MTrk_Event :                          public MIDI_Element
{
protected:
                    Varlen                  dt{};
                    std::vector<uint8_t>    bytes{};
public:
                    void                    set_dt(uint32_t new_dt);
    inline          uint32_t                get_dt() { return dt.get_data(); }
    inline          uint32_t                get_size() { return static_cast<uint32_t>(bytes.size()); }
                    void                    push_byte(uint8_t new_byte);
                    uint8_t                 operator[](size_t index);
};

/* ****************************************************************************
*  MIDI_Chunk
*  ************************************************************************* */
class MIDI_Chunk :                          public MIDI_Element
/*This class only to be used as a base class.*/
{
protected:
                    uint32_t                header{0};
                    uint32_t                len{0};
    
                                            MIDI_Chunk(); /* Protected constructor prohibits instantiation. */
public:
                    uint32_t                get_header();
                    void                    set_header(uint32_t new_header);
                    uint32_t                get_len();
                    void                    set_len(uint32_t new_len);
};


/* ****************************************************************************
*  MThd_Chunk
*  ************************************************************************* */
class MThd_Chunk :                          public MIDI_Chunk
{
protected:
                    uint16_t                fmt{0};
                    uint16_t                ntrks{0};
                    uint16_t                div{0};
                    std::vector<uint8_t>    extended_content{};
public:
                                            MThd_Chunk();

                    uint16_t                get_fmt();
                    uint16_t                get_ntrks();
                    uint16_t                get_div();
                    
                    void                    set_fmt(uint16_t new_fmt);
                    void                    set_ntrks(uint16_t new_ntrks);
                    void                    set_div(uint16_t new_div);
                    void                    push_byte(uint8_t);

                    uint8_t&                operator[](size_t index);
};

/* ****************************************************************************
*  MTrk_Chunk
*  ************************************************************************* */
class MTrk_Chunk :                          public MIDI_Chunk
{
protected:
                    std::list<MTrk_Event>   events{};
public:
                                            MTrk_Chunk();

                    void                    emplace_event();
                    MTrk_Event&             back();
                    MTrk_Event&             front();
            inline  size_t                  size(){ return events.size(); }
                    std::list<MTrk_Event>::iterator begin();
                    std::list<MTrk_Event>::iterator end();
                    MTrk_Event&             operator[](size_t index);
};

/* ****************************************************************************
*  UNkn_Chunk
*  ************************************************************************* */
class UNkn_Chunk :                          public MIDI_Chunk
{
protected:
                    std::vector<uint8_t>    bytes{};
public:
                                            UNkn_Chunk();

                    void                    set_len(uint32_t new_len);
                    void                    push_bytes(uint8_t next_byte);
                    uint8_t&                back();
                    uint8_t&                front();
                    std::vector<uint8_t>::iterator begin();
                    uint8_t                 operator[](size_t index);
};

/* ****************************************************************************
*  MIDI_File
*  ************************************************************************* */
class MIDI_File :                           public MIDI_Element
{
/*
A class representing a MIDI file.

Because the structure of MTrk and UNkn chunks is different, as well as the operations
that can be performed against them, they are stored in separate containers.

A master container stores all `MIDI_Chunk`s in a vector, implicitly tracking order.
Adding chunks to this container is a necessary side effect any time a new MTrk
or UNkn chunk is emplaced. The containers are therefore `protected` to ensure
the proper side effects take place, while references to elements in the containers
are still exposed through getters.
*/

protected:
                    MThd_Chunk              header{};
                    std::vector<MIDI_Chunk*> ordered_chunks{}; // pointers in vector cannot be comst because vector copies
                    std::list<MTrk_Chunk>   mtrk_chunks{};
                    std::list<UNkn_Chunk>   unkn_chunks{};
public:
                    void                    emplace_mtrk();
                    void                    emplace_unkn();
                    MThd_Chunk&             get_hdr();
                    MIDI_Chunk&             operator[](size_t index);
};

#endif
