/*
 Z80 emulator code derived from Lin Ke-Fong source. Copyright says:

 Copyright (c) 2016, 2017 Lin Ke-Fong

 This code is free, do whatever you want with it.

 2020 adapted by Fabrizio Di Vittorio for fabgl ESP32 library
 */


#pragma once


/**
 * @file
 *
 * @brief This file contains fabgl::Z80 definition.
 */



#include <stdint.h>



namespace fabgl {


/* Define this macro if the host processor is big endian. */

/* #define Z80_BIG_ENDIAN */

/* Emulation can be speed up a little bit by emulating only the documented
 * flags.
 */

/* #define Z80_DOCUMENTED_FLAGS_ONLY */

/* HALT, DI, EI, RETI, and RETN instructions can be catched. When such an
 * instruction is catched, the emulator is stopped and the PC register points
 * at the opcode to be executed next. The catched instruction can be determined
 * from the Z80_STATE's status value. Keep in mind that no interrupt can be
 * accepted at the instruction right after a DI or EI on an actual processor.
 */

/*
 #define Z80_CATCH_HALT
 #define Z80_CATCH_DI
 #define Z80_CATCH_EI
 #define Z80_CATCH_RETI
 #define Z80_CATCH_RETN
 */

/* Undefined 0xed prefixed opcodes may be catched, otherwise they are treated
 * like NOP instructions. When one is catched, Z80_STATUS_ED_UNDEFINED is set
 * in Z80_STATE's status member and the PC register points at the 0xed prefix
 * before the undefined opcode.
 */

/* #define Z80_CATCH_ED_UNDEFINED */

/* The emulator cannot be stopped between prefixed opcodes. This can be a
 * problem if there is a long sequence of 0xdd and/or 0xfd prefixes. But if
 * Z80_PREFIX_FAILSAFE is defined, it will always be able to stop after at
 * least numbers_cycles are executed, in which case Z80_STATE's status is set
 * to Z80_STATUS_PREFIX. Note that if the memory where the opcodes are read,
 * has wait states (slow memory), then the additional cycles for a one byte
 * fetch (the non executed prefix) must be substracted. Even if it is safer,
 * most program won't need this feature.
 */

/* #define Z80_PREFIX_FAILSAFE */

/* By defining this macro, the emulator will always fetch the displacement or
 * address of a conditionnal jump or call instruction, even if the condition
 * is false and the fetch can be avoided. Define this macro if you need to
 * account for memory wait states on code read.
 */

/* #define Z80_FALSE_CONDITION_FETCH */

/* It may be possible to overwrite the opcode of the currently executing LDIR,
 * LDDR, INIR, or OTDR instruction. Define this macro if you need to handle
 * these pathological cases.
 */

/* #define Z80_HANDLE_SELF_MODIFYING_CODE */

/* For interrupt mode 2, bit 0 of the 16-bit address to the interrupt vector
 * can be masked to zero. Some documentation states that this bit is forced to
 * zero. For instance, Zilog's application note about interrupts, states that
 * "only 7 bits are required" and "the least significant bit is zero". Yet,
 * this is quite unclear, even from Zilog's manuals. So this is left as an
 * option.
 */

/* #define Z80_MASK_IM2_VECTOR_ADDRESS */






/* If Z80_STATE's status is non-zero, the emulation has been stopped for some
 * reason other than emulating the requested number of cycles.
 */

enum {

  Z80_STATUS_HALT = 1,
  Z80_STATUS_DI,
  Z80_STATUS_EI,
  Z80_STATUS_RETI,
  Z80_STATUS_RETN,
  Z80_STATUS_ED_UNDEFINED,
  Z80_STATUS_PREFIX

};



/* The main registers are stored inside Z80_STATE as an union of arrays named
 * registers. They are referenced using indexes. Words are stored in the
 * endianness of the host processor. The alternate set of word registers AF',
 * BC', DE', and HL' is stored in the alternates member of Z80_STATE, as an
 * array using the same ordering.
 */

#ifdef Z80_BIG_ENDIAN

#       define Z80_B            0
#       define Z80_C            1
#       define Z80_D            2
#       define Z80_E            3
#       define Z80_H            4
#       define Z80_L            5
#       define Z80_A            6
#       define Z80_F            7

#       define Z80_IXH          8
#       define Z80_IXL          9
#       define Z80_IYH          10
#       define Z80_IYL          11

#else

#       define Z80_B            1
#       define Z80_C            0
#       define Z80_D            3
#       define Z80_E            2
#       define Z80_H            5
#       define Z80_L            4
#       define Z80_A            7
#       define Z80_F            6

#       define Z80_IXH          9
#       define Z80_IXL          8
#       define Z80_IYH          11
#       define Z80_IYL          10

#endif

#define Z80_BC                  0
#define Z80_DE                  1
#define Z80_HL                  2
#define Z80_AF                  3

#define Z80_IX                  4
#define Z80_IY                  5
#define Z80_SP                  6



/* Z80's flags. */

#define Z80_S_FLAG_SHIFT        7
#define Z80_Z_FLAG_SHIFT        6
#define Z80_Y_FLAG_SHIFT        5
#define Z80_H_FLAG_SHIFT        4
#define Z80_X_FLAG_SHIFT        3
#define Z80_PV_FLAG_SHIFT       2
#define Z80_N_FLAG_SHIFT        1
#define Z80_C_FLAG_SHIFT        0

#define Z80_S_FLAG              (1 << Z80_S_FLAG_SHIFT)
#define Z80_Z_FLAG              (1 << Z80_Z_FLAG_SHIFT)
#define Z80_Y_FLAG              (1 << Z80_Y_FLAG_SHIFT)
#define Z80_H_FLAG              (1 << Z80_H_FLAG_SHIFT)
#define Z80_X_FLAG              (1 << Z80_X_FLAG_SHIFT)
#define Z80_PV_FLAG             (1 << Z80_PV_FLAG_SHIFT)
#define Z80_N_FLAG              (1 << Z80_N_FLAG_SHIFT)
#define Z80_C_FLAG              (1 << Z80_C_FLAG_SHIFT)

#define Z80_P_FLAG_SHIFT        Z80_PV_FLAG_SHIFT
#define Z80_V_FLAG_SHIFT        Z80_PV_FLAG_SHIFT
#define Z80_P_FLAG              Z80_PV_FLAG
#define Z80_V_FLAG              Z80_PV_FLAG



/* Z80's three interrupt modes. */

enum {
  Z80_INTERRUPT_MODE_0,
  Z80_INTERRUPT_MODE_1,
  Z80_INTERRUPT_MODE_2
};



struct Z80_STATE {
  int               status;

  union {
    unsigned char   byte[14];
    unsigned short  word[7];
  } registers;

  unsigned short    alternates[4];

  int               i, r, pc, iff1, iff2, im;

  /* Register decoding tables. */

  void              * register_table[16];
  void              * dd_register_table[16];
  void              * fd_register_table[16];
};



// callbacks

typedef int  (*Z80ReadByte_t)(void * context, int addr);
typedef void (*Z80WriteByte_t)(void * context, int addr, int value);
typedef int  (*Z80ReadWord_t)(void * context, int addr);
typedef void (*Z80WriteWord_t)(void * context, int addr, int value);
typedef int  (*Z80ReadIO_t)(void * context, int addr);
typedef void (*Z80WriteIO_t)(void * context, int addr, int value);




/**
 * @brief Zilog Z80 CPU emulator
 */
class Z80 {

public:

  void setCallbacks(void * context, Z80ReadByte_t readByte, Z80WriteByte_t writeByte, Z80ReadWord_t readWord, Z80WriteWord_t writeWord, Z80ReadIO_t readIO, Z80WriteIO_t writeIO) {
    m_context   = context;
    m_readByte  = readByte;
    m_writeByte = writeByte;
    m_readWord  = readWord;
    m_writeWord = writeWord;
    m_readIO    = readIO;
    m_writeIO   = writeIO;
  }

  /* Initialize processor's state to power-on default. */
  void reset();

  /* Trigger an interrupt according to the current interrupt mode and return the
  * number of cycles elapsed to accept it. If maskable interrupts are disabled,
  * this will return zero. In interrupt mode 0, data_on_bus must be a single
  * byte opcode.
  */
  int interrupt(int data_on_bus);

  /* Trigger a non maskable interrupt, then return the number of cycles elapsed
   * to accept it.
   */
  int nonMaskableInterrupt();

  /* Execute instructions as long as the number of elapsed cycles is smaller than
   * number_cycles, and return the number of cycles emulated. The emulator can be
   * set to stop early on some conditions. The user macros also control the emulation.
   */
  int emulate(int number_cycles);


  // CPU registers access

  uint8_t readRegByte(int reg)                { return state.registers.byte[reg]; }
  void writeRegByte(int reg, uint8_t value)   { state.registers.byte[reg] = value; }

  uint16_t readRegWord(int reg)               { return state.registers.word[reg]; }
  void writeRegWord(int reg, uint16_t value)  { state.registers.word[reg] = value; }

  uint16_t getPC()                            { return state.pc; }
  void setPC(uint16_t value)                  { state.pc = value; }


private:

  int intemulate(int opcode, int elapsed_cycles, int number_cycles);


  Z80_STATE        state;

  // callbacks

  void *           m_context;

  Z80ReadByte_t    m_readByte;
  Z80WriteByte_t   m_writeByte;
  Z80ReadWord_t    m_readWord;
  Z80WriteWord_t   m_writeWord;
  Z80ReadIO_t      m_readIO;
  Z80WriteIO_t     m_writeIO;

};


};   // fabgl namespace







