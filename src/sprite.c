#include "sprite.h"


Sprite create_sprite(i16 w, i16 h) {

    Sprite s;

    s.width = w;
    s.height = h;
    s.frame = 0;
    s.count = 0;
    s.row = 0;

    return s;
}


void spr_animate(Sprite* s, 
    i16 row, i16 start, i16 end, 
    i16 speed, i16 steps) {

    if(start == end) {
    
		s->count = 0;
		s->frame = start;
		s->row = row;
		return;
	}

	if(s->row != row) {
	
		s->count = 0;
		s->frame = end > start ? start : end;
		s->row = row;
	}

	if((start < end && s->frame < start) ||
        (start > end && s->frame > start)) {
	
		s->frame = start;
    }

	s->count += steps;
	if(s->count > speed) {
	
        if(start < end) {
        
            if(++ s->frame > end) {
                
                s->frame = start;
            }
        }
        else {
        
            if(-- s->frame < end) {
            
                s->frame = start;
            }
        }

		s->count -= speed;
	}
}


void spr_set_frame(Sprite* s, i16 frame, i16 row) {

    s->frame = frame;
    s->row = row;
    s->count = 0;
}
