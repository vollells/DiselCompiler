#ifndef __CODEGEN_HH__
#define __CODEGEN_HH__

#include <fstream>

#include "quads.hh"
#include "symtab.hh"

using namespace std;


/* These are the registers we will be using. */
enum register_type { RAX, RCX, RDX };


// Maximum number of formal parameters allowed.
const int MAX_PARAMETERS = 127;

// This is the width/size of a single address on the stack (in bytes).
const int STACK_WIDTH = 8;

/* This class generates assembler code for the Intel architecture. */
class code_generator
{
private:
    // Register array.
    string reg[3];

    // Output file stream.
    ofstream out;

    //! Aligns a stack frame on an 8-byte boundary.
    int  align(int);

    /*! \brief Generates code to create the activation record

      This includes the display area and allocating space for local
      variables and temporaries.
     */
    void prologue(symbol *);

    //! Generates code to release the activation record.
    void epilogue(symbol *);

    /*!
      Translates a quad list to assembler code using the methods above.
      You will need to write code for expanding #q_param and #q_call quads.
     */
    void expand(quad_list *q);

    /*!
      Returns the display register level and offset for a variable,
      array or parameter to the symbol table.

      Note that parameters are stored in the caller’s activation record
      and therefore have positive offset while local and temporary
      variables have negative offset.

      Also note that the method modifies its arguments, which is why
      they are passed as pointers.
     */
    void find(sym_index, int *, int *);

    //! Retrieves the value of a variable, parameter or constant to a given register.
    void fetch(sym_index, const register_type);

    /*! \brief Pushes the value of a variable, paramters or constant to the FPU.

      Note that this method will never generate code for constant integers
      but will for constant reals.
     */
    void fetch_float(sym_index);

    //! Stores the value of a register in a variable or parameter.
    void store(const register_type, sym_index);

    //! Pops the FPU stack and stores the value in a variable or parameter.
    void store_float(sym_index);

    /*! \brief Retrieves the base address of an array to a register.

      The method is called when expanding the quadruples
      #q_lindex, #q_irindex, and #q_rrindex.
     */
    void array_address(sym_index, const register_type);

    /*! Given a lexical level and a register, stores the base address
        of the corresponding frame from the display area.
     */
    void frame_address(int level, const register_type);
public:
    // Constructor. Arg = filename of assembler outfile.
    code_generator(const string);

    // Destructor.
    ~code_generator();

    /*!
      This interface method is called from parser.y to start assembler
      expansion of a code block represented as a quad list.
     */
    void generate_assembler(quad_list *, symbol *env);
};

#endif
