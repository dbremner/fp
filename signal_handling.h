#ifndef SIGNAL_HANDLING_H
#define SIGNAL_HANDLING_H

extern jmp_buf restart;

void set_signal_handlers();

#endif
