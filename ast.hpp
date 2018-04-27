#ifndef AST_HPP
#define AST_HPP

/// An AST
struct ast {
    int tag = 0;
    YYSTYPE val{};
    ast_ptr left = nullptr;
    ast_ptr middle = nullptr;
    ast_ptr right = nullptr;
    
    ast(int tag_, ast_ptr left_, ast_ptr middle_, ast_ptr right_)
    {
        tag = tag_;
        left = left_;
        middle = middle_;
        right = right_;
    }
};

#endif
