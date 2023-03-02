/*
 File: ContFramePool.C
 
 Author:
 Date  : 
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

ContFramePool *ContFramePool::head = NULL;

ContFramePool::FrameState ContFramePool::get_state(unsigned long _frame_no)
{

    unsigned int bitmap_index = _frame_no / 4;
    unsigned char remainder = _frame_no % 4;
    unsigned char shiftedFrameBit = (bitmap[bitmap_index] >> 2*remainder) & 0x3;

    if (shiftedFrameBit == 0) {
        return FrameState::Free;
    } else if (shiftedFrameBit == 1) {
        return FrameState::Used;
    } else {
        return FrameState::HoS;
    }

}

void ContFramePool::set_state(unsigned long _frame_no, FrameState _state)
{

    unsigned int bitmap_index = _frame_no / 4;
    unsigned char remainder = _frame_no % 4;
    unsigned char maskBit = ~(0x3 << (2*remainder));
    bitmap[bitmap_index] &= maskBit;

    switch (_state) {
        case FrameState::Free:
            bitmap[bitmap_index] |= (0X0 << (2*remainder));
            break;
        case FrameState::Used:
            bitmap[bitmap_index] |= (0x1 << (2*remainder));
            break;
        case FrameState::HoS:
            bitmap[bitmap_index] |= (0x3 << (2*remainder));
            break;
    };

}

ContFramePool::ContFramePool(unsigned long _base_frame_no,
                        unsigned long _n_frames,
                        unsigned long _info_frame_no)
{

    base_frame_no = _base_frame_no;
    nframes = _n_frames;
    nfreeframes = _n_frames;
    info_frame_no = _info_frame_no;

    // If _info_frame_no is zero then we keep management info in the first
    // frame, else we use the provided frame to keep management info
    if (info_frame_no == 0) {
        bitmap = (unsigned char *)(base_frame_no * FRAME_SIZE);
    } else {
        bitmap = (unsigned char *)(info_frame_no * FRAME_SIZE);
    }

    // Set all frames to free
    for (int fno = base_frame_no; fno < base_frame_no + nframes; fno++) {
        set_state(fno - base_frame_no, FrameState::Free);
    }

    // Mark the frame as used when info_frame_no is 0
    if (info_frame_no == 0) {
        set_state(0, FrameState::Used);
        nfreeframes--;
    }

    // add frame pool to the list
    if (head == NULL) {
        head = this;
        head->next = NULL;
    } else {
        ContFramePool *temp = head;
        while (temp != NULL) {
            temp = temp->next;
        }
        temp = this;
        temp->next = NULL;
    }

    Console::puts("Frame pool is initialized");
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{

    assert(nfreeframes >= _n_frames)
    int start_frame;
    int count;

    for (int fno = base_frame_no; fno < base_frame_no + nframes; fno++) {
        int frame_no = fno - base_frame_no;
        if (get_state(frame_no) == FrameState::Free) {
            count = 0;
            start_frame = fno;

            // check if required frames are available
            // if available allocate used and HoS to the frames
            while (true) {
                int new_frame_no = fno - base_frame_no;
                // if frame is not free break and check again for required free frames
                if (get_state(new_frame_no) != FrameState::Free) {
                    fno = start_frame;
                    break;
                } else {
                    count++;
                    start_frame++;
                    if (count == _n_frames) {
                        // if contiguous frames found allocate used and HoS bits to the bitmap
                        unsigned int first_index = fno - base_frame_no;
                        set_state(first_index, FrameState::HoS);
                        for (int i = fno; i < fno + _n_frames; i++) {
                            unsigned int index = i - base_frame_no;
                            set_state(index, FrameState::Used);
                        }
                        nfreeframes -= _n_frames;
                        return fno;
                    }
                }
            }
        }
    }
    return 0;
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                    unsigned long _n_frames)
{
    for (int fno = _base_frame_no; fno < _base_frame_no + _n_frames; fno++) {
        assert((fno >= base_frame_no) && (fno < base_frame_no + nframes));
        unsigned int frame_no = fno - base_frame_no;
        assert(get_state(frame_no) != FrameState::Used);
        set_state(frame_no, FrameState::Used);
    }
    nfreeframes -= _n_frames;
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    ContFramePool *temp = NULL;
    for (temp = head; temp != NULL; temp = temp->next) {
    //check if the first frame number is in the pool by checking the range
        if ((_first_frame_no >= temp->base_frame_no) && (_first_frame_no <= (temp->base_frame_no + temp->nframes - 1))) {
            unsigned int frame_no = _first_frame_no - temp->base_frame_no;
            if(temp->get_state(frame_no) == FrameState::HoS) {
                for (int i = _first_frame_no; i < temp->base_frame_no + temp->nframes; i++) {
                    int frame = i - temp->base_frame_no;

                    // if head of sequence break
                    if (temp->get_state(frame) == FrameState::HoS)
                    break;

                    // if free frames break
                    if (temp->get_state(frame) == FrameState::Free)
                    break;

                    //release the frames
                    temp->set_state(frame, FrameState::Free);
                }
                break;
            }
        }
    }
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    return (_n_frames * 2) / (8 * 4 * 1024) + ((_n_frames * 2) % (8 * 4 * 1024) > 0 ? 1 : 0);
}
