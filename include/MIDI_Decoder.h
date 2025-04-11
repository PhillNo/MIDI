#ifndef MIDI_DECODER_H
#define MIDI_DECODER_H

#include "Noncopyable.h"
#include "MIDI_Data.h"

/* ****************************************************************************
*  MIDI_Element
*  ************************************************************************* */
class MIDI_Element_Decoder:                 private Noncopyable<MIDI_Element_Decoder>
{
public:
    enum class      STATUS
    {
                                            STANDBY,
                                            FAIL,
                                            SUCCESS
    };
protected:
                                            MIDI_Element_Decoder(){}; // protected constructor prevents instantiation

                    size_t                  index{0};
public:
    virtual         void                    clear() = 0; // implemented in cpp file AND descendents must still implement
    virtual         STATUS                  decode_byte(uint8_t, MIDI_Element* data) = 0;
};


/* ****************************************************************************
*  Varlen
*  ************************************************************************* */
class Varlen_Decoder:                       public MIDI_Element_Decoder
{
protected:
    enum class      STATE
    {
                                            READ, 
                                            DONE,
                                            FAIL
    };

                    STATE                   current_state{STATE::READ};
                    uint32_t                varlen{};
public:
                    void                    clear();
                    STATUS                  decode_byte(uint8_t next_byte, MIDI_Element*);
            inline  uint32_t                get(){ return varlen; }
};


/* ****************************************************************************
*  Len
*  ************************************************************************* */
class Chunk_Length_Decoder:                 public MIDI_Element_Decoder
{
protected:
    enum class      STATE
    {
                                            READ, 
                                            DONE, 
                                            FAIL
    };

                    STATE                   current_state{STATE::READ};
                    uint32_t                len{0};
public:
                    void                    clear();
                    STATUS                  decode_byte(uint8_t next_byte, MIDI_Element* data);
            inline  uint32_t                get_len(){ return len; }
};


/* ****************************************************************************
*  MIDI_Event
*  ************************************************************************* */
class MIDI_Event_Decoder:                   public MIDI_Element_Decoder
{
public:
    enum class      STATE
    {
                                            STATUS_BYTE, 
                                            PAYLOAD, 
                                            DONE,
                                            FAIL
    };
protected:
                    uint8_t                 parameter_count{0};
                    STATE                   current_state{STATE::STATUS_BYTE};
                    STATUS                  current_status{STATUS::STANDBY};
                
                    STATUS                  set_type(uint8_t new_running_status);
public:
                    void                    clear();
                    STATUS                  decode_byte(uint8_t next_byte, MIDI_Element* data);
};


/* ****************************************************************************
*  Meta_Event
*  ************************************************************************* */
class Meta_Event_Decoder:                   public MIDI_Element_Decoder
{
public: 
    enum class      STATE
    {
                                            FF, 
                                            TYPE, 
                                            LEN, 
                                            PAYLOAD, 
                                            DONE, 
                                            FAIL
    };
protected:
                    STATE                   current_state{STATE::FF};
                    STATUS                  current_status{STATUS::STANDBY};
                    uint32_t                len{0};
                    uint32_t                end{0};
                    Varlen_Decoder          len_decoder{};
public:
                    void                    clear();
                    STATUS                  decode_byte(uint8_t next_byte, MIDI_Element* data);
};


/* ****************************************************************************
*  Sysex_Event
*  ************************************************************************* */
class Sysex_Event_Decoder:                  public MIDI_Element_Decoder
{
public:
    enum class      STATE
    {
                                            TYPE, 
                                            LEN, 
                                            PAYLOAD, 
                                            DONE,
                                            FAIL
    };
protected:
                    STATE                   current_state{STATE::TYPE};
                    STATUS                  current_status{STATUS::STANDBY};
                    uint32_t                len{0};
                    uint32_t                end{0};
                    Varlen_Decoder          varlen_decoder{};
public:
                    void                    clear();
                    STATUS                  decode_byte(uint8_t next_byte, MIDI_Element* data);
};


/* ****************************************************************************
*  MTrk_Events
*  ************************************************************************* */
class MTrk_Events_Decoder:                  public MIDI_Element_Decoder
{
public:
    enum class      STATE
    {
                                            DT, 
                                            MESSAGE_TYPE, 
                                            META, 
                                            SYSEX, 
                                            MIDI, 
                                            DONE,
                                            FAIL
    };
protected:
                    STATE                   current_state{STATE::DT};
                    STATUS                  current_status{STATUS::STANDBY};
                    uint8_t                 running_status{};
                    size_t                  payload_size{};

                    Varlen_Decoder          varlen_decoder{};
                    MIDI_Event_Decoder      midi_decoder{};
                    Meta_Event_Decoder      meta_decoder{};
                    Sysex_Event_Decoder     sysex_decoder{};
public:
                    void                    clear();
                    STATUS                  decode_byte(uint8_t next_byte, MIDI_Element* data);
};


/* ****************************************************************************
*  Chunk_Type
*  ************************************************************************* */
class Chunk_Type_Decoder:                   public MIDI_Element_Decoder
{
protected:
    enum class      STATE
    {
                                            READ, 
                                            DONE, 
                                            FAIL
    };

                    STATE                   current_state{STATE::READ};
                    uint32_t                four_bytes{0};
                    CHUNK_TYPE              found{CHUNK_TYPE::UNKN};

public:
                    void                    clear();
                    STATUS                  decode_byte(uint8_t next_byte, MIDI_Element* data);
                    CHUNK_TYPE              get_type();
                    uint32_t                get_header();
};


/* ****************************************************************************
*  UNkn_Chunk
*  ************************************************************************* */
class UNkn_Chunk_Decoder:                   public MIDI_Element_Decoder
{
protected:
    enum class      STATE
    {
                                            CHUNK_LEN, 
                                            CHUNK_BODY, 
                                            DONE, 
                                            FAIL
    };

                    Chunk_Length_Decoder    chunk_len_decoder{};
                    STATE                   current_state{STATE::CHUNK_LEN};
                    STATUS                  current_status{STATUS::STANDBY};
public:
                    void                    clear();
                    STATUS                  decode_byte(uint8_t next_byte, MIDI_Element* data);
};


/* ****************************************************************************
*  MTrk_Chunk
*  ************************************************************************* */
class MTrk_Chunk_Decoder:                   public MIDI_Element_Decoder
{
protected:
    enum class      STATE
    {
                                            CHUNK_LEN, 
                                            EVENTS, 
                                            DONE,
                                            FAIL
    };

                    Chunk_Length_Decoder    chunk_len_decoder{};
                    MTrk_Events_Decoder     event_decoder{};
    
                    STATE                   current_state{STATE::CHUNK_LEN};
                    STATUS                  len_status{STATUS::STANDBY};
                    STATUS                  current_status{STATUS::STANDBY};

public:
                    void                    clear();
                    STATUS                  decode_byte(uint8_t next_byte, MIDI_Element* data);
};


/* ****************************************************************************
*  MThd_Param
*  ************************************************************************* */
class MThd_Param_Decoder:                   public MIDI_Element_Decoder
{
protected:
    enum class      STATE
    {
                                            READ, 
                                            DONE,
                                            FAIL
    };

                    STATE                   current_state{STATE::READ};
                    STATUS                  current_status{STATUS::STANDBY};
                    uint16_t                param{0};
public:
    enum class      MTHD_PARAM
    {
                                            FMT, 
                                            NTRKS, 
                                            DIV
    };
                    void                    clear();
                    STATUS                  decode_byte(uint8_t next_byte, MIDI_Element* data);
                    STATUS                  decode_byte(uint8_t next_byte, MIDI_Element* data,
                                                        MTHD_PARAM cur_param);
            inline  uint16_t                get(){ return param; }
};

/* ****************************************************************************
*  MThd_Chunk
*  ************************************************************************* */
class MThd_Chunk_Decoder:                   public MIDI_Element_Decoder
{
protected:
    enum class      STATE
    {
                                            LEN,
                                            FMT,
                                            NTRKS,
                                            DIV,
                                            EXTENDED,
                                            DONE,
                                            FAIL
    };

                    Chunk_Length_Decoder    mthd_len_decoder{};
                    MThd_Param_Decoder      mthd_param_decoder{};

                    STATE                   current_state{STATE::LEN};
                    STATUS                  current_status{STATUS::STANDBY};
                    size_t                  extended_last{};
public:
                    void                    clear();
                    STATUS                  decode_byte(uint8_t next_byte, MIDI_Element* data);
};


/* ****************************************************************************
*  MIDI_File
*  ************************************************************************* */
class MIDI_File_Decoder:                    public MIDI_Element_Decoder
{
protected:
    enum class      STATE
    {
                                            CHUNK_TYPE,
                                            MTHD,
                                            MTRK,
                                            UNKN,
                                            DONE,
                                            FAIL
    };

                    Chunk_Type_Decoder      chunk_type_decoder{};
                    UNkn_Chunk_Decoder      unkn_decoder{};
                    MTrk_Chunk_Decoder      mtrk_decoder{};
                    MThd_Chunk_Decoder      mthd_decoder{};

                    STATE                   current_state{STATE::CHUNK_TYPE};
                    STATUS                  current_status{STATUS::STANDBY};

                    uint32_t                expected_tracks{0};
                    size_t                  track_index{0};

public:
                    void                    clear();
                    STATUS                  decode_byte(uint8_t next_byte, MIDI_Element* data);
};

#endif
