#ifndef CALL_STACK_LAYOUT_HPP
#define CALL_STACK_LAYOUT_HPP

#define SP_OVERHEAD		4
#define SP_FUNC			-4
#define SP_LINK 		-3
#define SP_PREV_SP 		-2
#define SP_NSLOTS 		-1

#define Local( N ) 		VMSP[-(N)]

#endif