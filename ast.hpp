#ifndef AST_HPP
#define AST_HPP

/// An AST
struct ast {
    int tag = 0;
    YYSTYPE val{};
    ast_ptr left = nullptr;
    ast_ptr middle = nullptr;
    ast_ptr right = nullptr;
    
    live_ast_ptr live_left()
    {
        assert(left);
        auto result = static_cast<live_ast_ptr>(left);
        return result;
    }
    
    live_ast_ptr live_middle()
    {
        assert(middle);
        auto result = static_cast<live_ast_ptr>(middle);
        return result;
    }
    
    live_ast_ptr live_right()
    {
        assert(right);
        auto result = static_cast<live_ast_ptr>(right);
        return result;
    }
    
    ast(int tag_, ast_ptr left_, ast_ptr middle_, ast_ptr right_)
    {
        tag = tag_;
        left = left_;
        middle = middle_;
        right = right_;
    }
};

#endif
