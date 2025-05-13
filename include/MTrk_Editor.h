#ifndef MTRK_EDITOR_H
#define MTRK_EDITOR_H

#include <list>
#include <utility>
#include <vector>

#include "Noncopyable.h"
#include "MIDI_Data.h"

/* ****************************************************************************
*   MTrk_Editor
*
*   A class for editing the event list in an MTrk chunk.
*
*   Because MTrk events store time differentials instead of the absolute time
*   an event occurs, and because the events are stored in a list, it makes
*   more sense to have a specialized editor class.
*
*   The MTrk editor will store events in a Vector, which is expected to be
*   faster at sorting. Sorting will be used when reseting a target event
*   list. Events will be placed at the back of the vector, the vector will 
*   be sorted by absolute time of events, then the time differentials will be
*   resolved when hydrating the target event list.
*
*   Because vectors use more memory when copying and potential embedded 
*   applications might not need editing features, the `MIDI_Editor`'s 
*   features are not built into any `MIDI_Data` structures.
* ************************************************************************** */
class MTrk_Editor:                          private Noncopyable<MTrk_Editor>
{
protected:
                    MTrk_Chunk*             src_chunk;
                    std::list<MTrk_Event>*  src;
                    std::list<MTrk_Event>*  dst;

                    std::vector<std::pair<uint32_t, MTrk_Event>> 
                                            data{};

                    bool                    cmp(std::vector<std::pair<uint32_t, MTrk_Event>> a,
                                                std::vector<std::pair<uint32_t, MTrk_Event>> b);

public:
                                            MTrk_Editor(MTrk_Chunk);
                                            MTrk_Editor(std::list<MTrk_Event> new_src);
                                            MTrk_Editor(std::list<MTrk_Event> new_src, std::list<MTrk_Event> new_dst);

                    void                    insert(uint32_t time, MTrk_Event& event);
                    //void                    remove_time(uint32_t t); multiple events can occur at the same time
                    void                    remove(size_t index);

                    void                    write_dst();
                    void                    write_dst(std::list<MTrk_Event>* dst);
};

#endif