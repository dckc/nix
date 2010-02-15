
#ifndef _LIBTERM_TERM_HH
# define _LIBTERM_TERM_HH

# include <set>

# include <boost/preprocessor/seq/seq.hpp>
# include <boost/preprocessor/control/iif.hpp>

# include "visitor_fwd.hh"

namespace term
{
  /*
  class ATermVisitor
  {
    virtual void visit(const ATerm e) = 0;
    virtual void visit(const Function& e) = 0;
  };

  class ATermConstVisitor
  {
    virtual void visit(const ATerm e) const = 0;
    virtual void visit(const Function& e) const = 0;
  };
  */


  // Performance issue: Abstract classes should be copied where fully
  // defined terms should be used with constant references.  An ATerm is
  // only composed by a pointer and should not use any virtual method.  By
  // following this rule, the compiler can optimize the size to the size of
  // its members (which is a pointer).

  class ATermImpl
  {
  public:
    ATermImplemntation()
    {
    }

    virtual ~ATermImplemntation()
    {
    }

  public:
    virtual void accept(ATermVisitor& v) const = 0;
  };


  class ATerm
  {
  private:
    inline
    ATerm(ATermImpl *ptr)
      : ptr_(ptr)
    {
      assert(ptr_);
    }

  public:
    inline
    ATerm(ATerm t)
      : ptr_(t.ptr_)
    {
    }

  public:
    inline
    void accept(ATermVisitor& v)
    {
      return ptr_->accept(v);
    }

  public:
    inline
    bool operator== (const ATerm& rhs) const
    {
      return ptr_ == rhs.ptr_;
    }

    inline
    bool operator!= (const ATerm& rhs) const
    {
      return ptr_ != rhs.ptr_;
    }

    inline
    bool operator <(const ATerm& rhs) const
    {
      return ptr_ < rhs.ptr_;
    }

  private:
    const void* ptr_;
  };


  template <typename T>
  struct Term : public ATermImpl
  {
    typedef std::set<T> term_set_type;

  public:
    inline
    bool operator <(const Function& rhs)
    {
      return false;
    }

    static inline
    bool
    matchTerm(const ATerm t)
    {
      return true;
    }

  private:
    inline
    static
    ATermImpl*
    get_identity(const T& t)
    {
      return static_cast<ATermImpl*>(&*t.set_instance().insert(t).first)
    }

    inline
    term_set_type& set_instance()
    {
      static term_set_type set();
      return set;
    };
  };


# define TRM_NIL
# define TRM_NIL_1(_)
# define TRM_NIL_2(_, _)
# define TRM_IF_EMPTY_SEQ(Seq, True, False)             \
  BOOST_PP_IF(BOOST_PP_SEQ_SIZE(Seq), False, True)

# define TRM_MAP_HELPER(R, Macro, Seq) (Macro(Elt))
# define TRM_MAP_(Macro, Seq)                           \
  BOOST_PP_SEQ_FOR_EACH(TRM_MAP_HELPER, Macro, Seq)
# define TRM_MAP(Macro, Seq)                                    \
  TRM_IF_EMPTY_SEQ(Seq, TRM_NIL_2, TRM_MAP_)(Macro, Seq)

# define TRM_APPLY_HELPER(R, Macro, Seq) Macro(Elt)
# define TRM_APPLY_(Macro, Seq)                            \
  BOOST_PP_SEQ_FOR_EACH(TRM_MAP_HELPER, Macro, Seq)
# define TRM_APPLY(Macro, Seq)                                  \
  TRM_IF_EMPTY_SEQ(Seq, TRM_NIL_2, TRM_APPLY_)(Macro, Seq)

# define TRM_HEAD(Seq) BOOST_PP_SEQ_HEAD(Seq)
# define TRM_TAIL(Seq) BOOST_PP_SEQ_TAIL(Seq)

# define TRM_SEPARATE_COMMA_HELPER(Elt) , Elt
# define TRM_SEPARATE_COMMA_(Seq)                                        \
  TRM_HEAD(Seq) TRM_APPLY(TRM_SEPARATE_COMMA_HELPER, TRM_TAIL(Seq))
# define TRM_SEPARATE_COMMA(Seq)                                \
  TRM_IF_EMPTY_SEQ(Seq, TRM_NIL_1, TRM_SEPARATE_COMMA_)(Seq)

# define TRM_TYPE_REF(Type, Name) (const Type&, Type &, Name)
# define TRM_TYPE_COPY(Type, Name) (const Type, Type &, Name)
# define TRM_TYPE_TERM(Type, Name) TRM_TYPE_COPY(A ## Type, Name)

# define TRM_CONST_ABSTRACT_COPY_DECL(Copy, Ref, Name)  Copy Name;
# define TRM_CONST_ABSTRACT_COPY_ARG(Copy, Ref, Name)  Copy Name ## _
# define TRM_ABSTRACT_REF_ARG(Copy, Ref, Name)  Ref Name ## _
# define TRM_INIT_ATTRIBUTES(Copy, Ref, Name)  Name (Name ## _)
# define TRM_ARGUMENTS(Copy, Ref, Name)  Name ## _
# define TRM_COPY_IN_ARG(Copy, Ref, Name)  Name ## _ = Name;
# define TRM_LESS_RHS_OR(Copy, Ref, Name)  Name < rhs. Name ||

// Add the implementation class as a friend class to provide access to the
// private constructor.  This reduce interaction with the implementation of
// the terms.
# define TRM_ADD_FRIEND(Name, Base, Attributes, BaseArgs)       \
  friend Name;


// Initialized all attributes of the current class and call the base class
// with the expected arguments.
# define TRM_GRAMMAR_NODE_CTR(Name, Base, Attributes, BaseArgs)         \
  private:                                                              \
    Name ( TRM_SEPARATE_COMMA(TRM_MAP(TRM_CONST_ABSTRACT_COPY_ARG,      \
                                      Attributes BaseArgs)) )           \
    : TRM_SEPARATE_COMMA(TRM_MAP(TRM_INIT_ATTRIBUTES, Attributes)) ,    \
      parent( TRM_SEPARATE_COMMA(TRM_MAP(TRM_ARGUMENTS, BaseArgs)) )    \
    {                                                                   \
    }



// This method is used to provide maximal sharing among node of the same
// types.  This create the class with the private constructor and register
// it if this is not already done.  It returns a term which has a small
// memory impact (only a pointer) which refers to the created element.
# define TRM_GRAMMAR_NODE_MAKE_METHOD(Name, Base, Attributes, BaseArgs) \
  public:                                                               \
    static inline                                                       \
    term                                                                \
    make ## Name(const APattern a1, const AExpr a2, const APos a3)      \
    {                                                                   \
      return term(get_identity(this_type(a1, a2, a3)));                 \
    }


// This method provides backward compatibility but it is inefficient.  The
// macro produce a match function which deconstruct the node by copying all
// attributes values into references given as arguments of this function.
// The first argument is the term which has to be deconstructed.
# define TRM_GRAMMAR_NODE_MATCH_METHOD(Name, Base, Attributes, BaseArgs) \
  public:                                                               \
    static inline                                                       \
    bool                                                                \
    match ## Name(                                                      \
      const ATerm t,                                                    \
      TRM_SEPARATE_COMMA(TRM_MAP(TRM_ABSTRACT_REF_ARG,                  \
                                 Attributes BaseArgs))                  \
    )                                                                   \
    {                                                                   \
      this_type* ptr;                                                   \
      if (ptr = dynamic_cast<this_type*>(t.ptr_))                       \
      {                                                                 \
        TRM_MAP(TRM_COPY_IN_ARG, Attributes);                           \
        return parent::match ## Base (                                  \
          t,                                                            \
          TRM_SEPARATE_COMMA(TRM_MAP(TRM_ARGUMENTS, BaseArgs))          \
        );                                                              \
      }                                                                 \
      return false;                                                     \
    }


// This operator is used for checking if the current ellement has not been
// create before.  It compares each attributes of the current class and
// delegate other attribute comparison to his parent class.
# define TRM_LESS_GRAMMAR_NODE_OP(Name, Base, Attributes, BaseArgs)     \
  public:                                                               \
    inline                                                              \
    bool operator <(const Name& rhs)                                    \
    {                                                                   \
      return TRM_MAP(TRM_LESS_RHS_OR, Attributes)                       \
        parent::operator < (rhs);                                       \
    }


// Declare all the attributes of the current class.
# define TRM_ATTRIBUTE_DECLS(Name, Base, Attributes, BaseArgs)  \
  public:                                                       \
    TRM_MAP(TRM_CONST_ABSTRACT_COPY_DECL, Attributes)


// A class which provide static method (no virtual) to access the
// implementation of the term.  This class is a wrapper over a pointer which
// should derivate from the ATerm class.
# define TRM_ABSTRACT_GRAMMAR_NODE(Name, Base, Attributes, BaseArgs, NoTemplate) \
  class Name;                                                           \
  class A ## Name : public A ## Base                                    \
  {                                                                     \
    typedef A ## Base parent;                                           \
                                                                        \
  public:                                                               \
    A ## Name (A ## Name t)                                             \
      : parent(t)                                                       \
    {                                                                   \
    }                                                                   \
                                                                        \
    inline                                                              \
    const Name &                                                        \
    operator() () const                                                 \
    {                                                                   \
      return *static_cast<const Name *>(ptr_);                          \
    }                                                                   \
                                                                        \
  private:                                                              \
    explicit A ## Name (ATermImpl *ptr)                                 \
      : parent(ptr)                                                     \
    {                                                                   \
    }                                                                   \
                                                                        \
    NoTemplate(Name, Base, Attributes, BaseArgs)                        \
  };


# define TRM_INTERFACE_GRAMMAR_NODE(Name, Base, Attributes, BaseArgs)   \
  template <typename T>                                                 \
  class Name : public Base<T>                                           \
  {                                                                     \
    typedef Base<T> parent;                                             \
                                                                        \
    TRM_GRAMMAR_NODE_CTR(Name, Base, Attributes, BaseArgs)              \
    TRM_GRAMMAR_NODE_MATCH_METHOD(Name, Base, Attributes, BaseArgs)     \
    TRM_LESS_GRAMMAR_NODE_OP(Name, Base, Attributes, BaseArgs)          \
    TRM_ATTRIBUTE_DECLS(Name, Base, Attributes, BaseArgs)               \
  };


# define TRM_FINAL_GRAMMAR_NODE(Name, Base, Attributes, BaseArgs)       \
  class Name : public Base<Name>                                        \
  {                                                                     \
    typedef Base<Name> parent;                                          \
    typedef A ## Name term;                                             \
    typedef Name this_type;                                             \
                                                                        \
    TRM_GRAMMAR_NODE_CTR(Name, Base, Attributes, BaseArgs)              \
    TRM_GRAMMAR_NODE_MAKE_METHOD(Name, Base, Attributes, BaseArgs)      \
    TRM_GRAMMAR_NODE_MATCH_METHOD(Name, Base, Attributes, BaseArgs)     \
                                                                        \
  public:                                                               \
    void accept(ATermVisitor& v) const                                  \
    {                                                                   \
      v.visit(*this);                                                   \
    }                                                                   \
                                                                        \
    TRM_LESS_GRAMMAR_NODE_OP(Name, Base, Attributes, BaseArgs)          \
    TRM_ATTRIBUTE_DECLS(Name, Base, Attributes, BaseArgs)               \
  };                                                                    \
                                                                        \
  using Name::make ## Name;                                             \
  using Name::match ## Name;


# define TRM_GRAMMAR_NODE_BINOP(Final, Name)                    \
  Final(Name, Expr, (TRM_TYPE_TERM(Expr, lhs)                   \
                        TRM_TYPE_TERM(Expr, rhs)), TRM_NIL)

# define TRM_GRAMMAR_NODE_SINGLETON(Final, Name)        \
  Final(Name, Expr, TRM_NIL, TRM_NIL)


// Test case.  This should be moved inside the libexpr or generated in
// another manner.
# define TRM_GAMMAR_NODES(Interface, Final)
  Interface(Loc, Term, TRM_NIL, TRM_NIL)
  Interface(Pattern, Term, TRM_NIL, TRM_NIL)
  Interface(Expr, Term, TRM_NIL, TRM_NIL)
  Final(Pos, Loc, (TRM_TYPE_REF(std::string, file)
                   TRM_TYPE_COPY(int, line)
                   TRM_TYPE_COPY(int, column)), TRM_NIL)
  Final(NoPos, Loc, TRM_NIL, TRM_NIL)
  Final(String, Expr, (TRM_TYPE_REF(std::string, value)), TRM_NIL)
  Final(Var, Expr, (TRM_TYPE_REF(std::string, name)), TRM_NIL)
  Final(Path, Expr, (TRM_TYPE_REF(std::string, filename)), TRM_NIL)
  Final(Int, Expr, (TRM_TYPE_COPY(int, value)), TRM_NIL)
  Final(Function, Expr, (TRM_TYPE_TERM(Pattern, pattern)
                         TRM_TYPE_TERM(Expr, body)
                         TRM_TYPE_TERM(Pos, position)), TRM_NIL)
  Final(Assert, Expr, (TRM_TYPE_TERM(Expr, cond)
                       TRM_TYPE_TERM(Expr, body)
                       TRM_TYPE_TERM(Pos, position)), TRM_NIL)
  Final(With, Expr, (TRM_TYPE_TERM(Expr, set)
                     TRM_TYPE_TERM(Expr, body)
                     TRM_TYPE_TERM(Pos, position)), TRM_NIL)
  Final(If, Expr, (TRM_TYPE_TERM(Expr, cond)
                   TRM_TYPE_TERM(Expr, thenPart)
                   TRM_TYPE_TERM(Expr, elsePart)), TRM_NIL)
  Final(OpNot, Expr, (TRM_TYPE_TERM(Expr, cond)), TRM_NIL)
  TRM_GRAMMAR_NODE_BINOP(Final, OpEq)
  TRM_GRAMMAR_NODE_BINOP(Final, OpNEq)
  TRM_GRAMMAR_NODE_BINOP(Final, OpAnd)
  TRM_GRAMMAR_NODE_BINOP(Final, OpOr)
  TRM_GRAMMAR_NODE_BINOP(Final, OpImpl)
  TRM_GRAMMAR_NODE_BINOP(Final, OpUpdate)
  TRM_GRAMMAR_NODE_BINOP(Final, SubPath)
  TRM_GRAMMAR_NODE_BINOP(Final, OpHasAttr)
  TRM_GRAMMAR_NODE_BINOP(Final, OpPlus)
  TRM_GRAMMAR_NODE_BINOP(Final, OpConcat)
  Final(Call, Expr, (TRM_TYPE_TERM(Expr, function)
                     TRM_TYPE_TERM(Expr, argument)), TRM_NIL)
  Final(Select, Expr, (TRM_TYPE_TERM(Expr, set)
                       TRM_TYPE_TERM(Var, var)), TRM_NIL)
  Final(BlackHole, Expr, TRM_NIL, TRM_NIL)
  Final(Undefined, Expr, TRM_NIL, TRM_NIL)
  Final(Removed, Expr, TRM_NIL, TRM_NIL)


  /*
  class AExpr : public ATerm
  {
    typedef ATerm parent;

  public:
    AExpr(AExpr t)
      : parent(t)
    {
    }

    inline
    const Expr&
    operator() () const
    {
      return *static_cast<const Expr*>(ptr_);
    }

  private:
    explicit AExpr(ATermImpl *ptr)
      : parent(ptr)
    {
    }
  };

  template <typename T>
  class Expr : public Term<T>
  {
    typedef Expr<Function> parent;

  public:
    inline
    bool operator <(const Function& rhs)
    {
      return
        parent::operator < (rhs);
    }
  };


  class Function;
  class AFunction : public AExpr
  {
    typedef AExpr parent;

  public:
    AFunction(AFunction t)
      : parent(t)
    {
    }

    inline
    const Function&
    operator() () const
    {
      return *static_cast<const Function*>(ptr_);
    }

  private:
    explicit AFunction(ATermImpl *ptr)
      : parent(ptr)
    {
    }

    friend Function;
  };

  class Function : public Expr<Function>
  {
    typedef Expr<Function> parent;
    typedef AFunction term;
    typedef Function this_type;

  private:
    Function(const APattern a1, const AExpr a2, const APos a3)
      : pattern(a1), expr(a2), pos(a3)
    {
    }

  public:
    // provide a function to create an ATerm without any reference to the
    // ATerm implementation from the user point of view.
    inline
    static
    AFunction
    makeFunction(const APattern a1, const AExpr a2, const APos a3)
    {
      return AFunction(get_identity(Function(a1, a2, a3)));
    }

    // this function provides backward compatibility but it is inefficient.
    inline
    static
    bool
    matchFunction(const ATerm t, APattern &a1, AExpr &a2, APos &a3)
    {
      Function* ptr;
      if (ptr = dynamic_cast<this_type*>(t.ptr_))
      {
        a1 = pattern;
        a2 = expr;
        a3 = pos;
        return true;
      }
      return false;
    }

  public:
    void accept(ATermVisitor& v) const
    {
      v.visit(*this);
    }

    // This operator is used for checking if the current ellement has not
    // been create before.
    bool operator <(const Function& rhs)
    {
      return
        pos < rhs.pos ||
        expr < rhs.expr ||
        pattern < rhs.pattern ||
        parent::operator < (rhs);
    }

    const APattern pattern;
    const AExpr expr;
    const APos pos;
  };

  using Function::makeFunction;
  using Function::matchFunction;
  */


  /*
  template <typename T>
  class getVisitor : ATermNoOpVisitor
  {
  public:
    getVisitor(ATerm t)
    {
      t.accept(*this);
    }

    visit(const T& t)
    {
      res = std::pair(true, &t);
    }

    std::pair<bool, T*> res;
  };

  template <typename T>
  std::pair<bool, T*>
  is_a(Aterm t)
  {
    getVisitor v(t);
    return v.res;
  }
  */
}

#endif