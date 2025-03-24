#ifndef MIDI_ENCODER_H
#define MIDI_ENCODER_H

#include <cstdint>
#include <list>
#include <vector>

#include "Noncopyable.h"
#include "MIDI_Data.h"

#define ENCODE_RUNNING_STATUS true

/* ****************************************************************************
*  MIDI_Element_Encoder
*  ************************************************************************* */
class MIDI_Element_Encoder :                private Noncopyable<MIDI_Element_Encoder>
{
public:
    enum class      STATUS
    {
                                            STANDBY,
                                            FAIL,
                                            SUCCESS
    };
protected:
                    MIDI_Element*           src{nullptr};
                    size_t                  specific_index{};
                    std::vector<uint8_t>    tmp{};
public: 
    virtual         void                    clear() = 0;
    virtual         STATUS                  encode_byte(uint8_t& product) = 0;
    virtual         STATUS                  set_data(MIDI_Element* data) = 0;
};


/* ****************************************************************************
*  Varlen_Encoder
*  ************************************************************************* */
class Varlen_Encoder :                      public MIDI_Element_Encoder
{
protected:
    enum class      STATE
    {
                                            READ,
                                            DONE,
                                            FAIL
    };

                    STATE                   current_state{STATE::READ};
public:
                    void                    clear();
                    STATUS                  encode_byte(uint8_t& product);
                    STATUS                  set_data(uint32_t data);
                    STATUS                  set_data(MIDI_Element* data);
};


/* ****************************************************************************
*  MIDI_Message_Encoder
*  ************************************************************************* */
class MIDI_Message_Encoder :                public MIDI_Element_Encoder
{
protected:
    enum class      STATE
    {
                                            DT,
                                            PAYLOAD,
                                            DONE,
                                            FAIL
    };

                    MTrk_Event*             src_event{};
                    Varlen_Encoder          varlen_encoder{};
                    STATE                   current_state{STATE::DT};
                    STATUS                  current_status{STATUS::STANDBY};
public:
                    void                    clear();
                    STATUS                  encode_byte(uint8_t& product);
                    STATUS                  set_data(MIDI_Element* data);
                    void                    skip_status();
};


/* ****************************************************************************
*  Meta_Message_Encoder
*  ************************************************************************* */
class Meta_Message_Encoder :                public MIDI_Element_Encoder
{
protected:
    enum class      STATE
    {
                                            DT,
                                            PAYLOAD,
                                            DONE,
                                            FAIL
    };

                    MTrk_Event*             src_event{};
                    Varlen_Encoder          varlen_encoder{};
                    STATE                   current_state{STATE::DT};
                    STATUS                  current_status{STATUS::STANDBY};
public:
                    void                    clear();
                    STATUS                  encode_byte(uint8_t& product);
                    STATUS                  set_data(MIDI_Element* data);
};


/* ****************************************************************************
*  Sysex_Message_Encoder
*  ************************************************************************* */
class Sysex_Message_Encoder :               public MIDI_Element_Encoder
{
    enum class      STATE
    {
                                            DT,
                                            TYPE,
                                            LEN,
                                            PAYLOAD,
                                            DONE,
                                            FAIL
    };

                    MTrk_Event*             src_event{};
                    Varlen_Encoder          dt_encoder{};
                    Varlen_Encoder          len_encoder{};
                    STATE                   current_state{STATE::DT};
                    STATUS                  current_status{STATUS::STANDBY};
public:
                    void                    clear();
                    STATUS                  encode_byte(uint8_t& product);
                    STATUS                  set_data(MIDI_Element* data);
};


/* ****************************************************************************
*  MTrk_Encoder
*  ************************************************************************* */
class MTrk_Encoder :                        public MIDI_Element_Encoder
{
protected:
    enum class      STATE
    {
                                            HEADER,
                                            EVENT_TYPE,
                                            MIDI_EVENT,
                                            META_EVENT,
                                            SYSEX_EVENT,
                                            DONE,
                                            FAIL
    };

                    MTrk_Chunk*             src_chunk{};
                    STATE                   current_state{STATE::HEADER};
                    STATUS                  current_status{STATUS::STANDBY};
                    size_t                  event_index{};
                    uint8_t                 running_status{0};

                    Meta_Message_Encoder    meta_encoder{};
                    MIDI_Message_Encoder    midi_encoder{};
                    Sysex_Message_Encoder   sysex_encoder{};

public:
                    void                    clear();
                    STATUS                  encode_byte(uint8_t& product);
                    STATUS                  set_data(MIDI_Element* data);
};


/* ****************************************************************************
*  UNkn_Encoder
*  ************************************************************************* */
class UNkn_Encoder :                        public MIDI_Element_Encoder
{
protected:
    enum class      STATE
    {
                                            HEAD,
                                            BODY,
                                            DONE,
                                            FAIL
    };

                    UNkn_Chunk*             src_chunk{nullptr};
                    STATE                   current_state{STATE::HEAD};
                    size_t                  payload_index{0};
public:
                    void                    clear();
                    STATUS                  encode_byte(uint8_t& product);
                    STATUS                  set_data(MIDI_Element* data);
};


/* ****************************************************************************
*  MThd_Encoder
*  ************************************************************************* */
class MThd_Encoder :                        public MIDI_Element_Encoder
{
protected:
    enum class      STATE
    {
                                            MAIN,
                                            EXTRA,
                                            DONE,
                                            FAIL
    };

                    MThd_Chunk*             src_chunk{nullptr};
                    STATE                   current_state{STATE::MAIN};
                    size_t                  payload_index{0};
public:
                    void                    clear();
                    STATUS                  encode_byte(uint8_t& product);
                    STATUS                  set_data(MIDI_Element* data);
};


/* ****************************************************************************
*  MIDI_File_Encoder
*  ************************************************************************* */
class MIDI_File_Encoder :                   public MIDI_Element_Encoder
{
protected:
    enum class      STATE
    {
                                            MTHD_HEADER,
                                            CHUNK_TYPE,
                                            MTRK,
                                            UNKN,
                                            DONE,
                                            FAIL
    };

                    MIDI_File*              src_file{nullptr};
                    STATE                   current_state{STATE::MTHD_HEADER};
                    STATUS                  current_status{STATUS::STANDBY};
                    size_t                  chunk_index{0};

                    MThd_Encoder            mthd_encoder{};
                    MTrk_Encoder            mtrk_encoder{};
                    UNkn_Encoder            unkn_encoder{};
public:
                    void                    clear();
                    STATUS                  encode_byte(uint8_t& product);
                    STATUS                  set_data(MIDI_Element* data);
};

#endif
