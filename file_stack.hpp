#ifndef FILE_STACK_HPP
#define FILE_STACK_HPP

#include <stdio.h>

struct file_stack final
{
    file_stack()
    :cur_in{stdin}
    {
    }
    
    file_stack(file_stack &&other) = default;
    
    file_stack& operator=(file_stack &&other) = default;
    
    void pop()
    {
        fclose(cur_in);
        assert(fpos > 0);
        cur_in = fstack[--fpos];
        assert(cur_in);
    }
    
    void push(FILE * _Nonnull newf)
    {
        assert(newf);
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
        assert(ch != EOF);
        ::ungetc(ch, cur_in);
    }
    
    bool is_stdin() const
    {
        assert(cur_in);
        return cur_in == stdin;
    }
    
    bool is_full() const
    {
        return fpos == file_stack::MAXNEST-1;
    }
    
    /// How deep can we get?
    static const size_t MAXNEST = 5;

private:
    FILE * _Nonnull cur_in;
    /// For nested loads
    FILE * _Nonnull fstack[MAXNEST];
    int fpos = 0;
};

#endif
