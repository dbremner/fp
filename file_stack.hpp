#ifndef FILE_STACK_HPP
#define FILE_STACK_HPP

#include <stdio.h>

struct file_stack final
{
    file_stack()
    :cur_in{stdin}
    {
    }
    
    void pop()
    {
        fclose(cur_in);
        assert(fpos > 0);
        cur_in = fstack[--fpos];
    }
    
    void push(FILE *newf)
    {
        // Pushdown the current file, make this one it.
        fstack[fpos++] = cur_in;
        cur_in = newf;
    }
    
    int fgetc()
    {
        return ::fgetc(cur_in);
    }
    
    void ungetc(int ch)
    {
        ::ungetc(ch, cur_in);
    }
    
    bool is_stdin() const
    {
        return cur_in == stdin;
    }
    
    /// How deep can we get?
    static const size_t MAXNEST = 5;
    
    FILE *cur_in;
    /// For nested loads
    FILE *fstack[MAXNEST];
    int fpos = 0;
};

#endif
