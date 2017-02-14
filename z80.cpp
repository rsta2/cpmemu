// Converted to C++ from Java at
//... https://github.com/jsanchezv/Z80Core
//... commit c4f267e3564fa89bd88fd2d1d322f4d6b0069dbd
//... GPL 3
//... v1.0.0 (13/02/2017)
//    quick & dirty conversion by dddddd (AKA deesix)

#include "z80.h"

// Constructor de la clase
Z80::Z80(Z80operations *ops) {

    bool evenBits;

    for (uint32_t idx = 0; idx < 256; idx++) {
        sz53n_addTable[idx] = 0;
        sz53pn_addTable[idx] =0;
        sz53n_subTable[idx] = 0;
        sz53pn_subTable[idx] = 0;

        if (idx > 0x7f) {
            sz53n_addTable[idx] |= SIGN_MASK;
        }

        evenBits = true;
        for (uint8_t mask = 0x01; mask != 0; mask <<= 1) {
            if ((idx & mask) != 0) {
                evenBits = !evenBits;
            }
        }

        sz53n_addTable[idx] |= (idx & FLAG_53_MASK);
        sz53n_subTable[idx] = sz53n_addTable[idx] | ADDSUB_MASK;

        if (evenBits) {
            sz53pn_addTable[idx] = sz53n_addTable[idx] | PARITY_MASK;
            sz53pn_subTable[idx] = sz53n_subTable[idx] | PARITY_MASK;
        } else {
            sz53pn_addTable[idx] = sz53n_addTable[idx];
            sz53pn_subTable[idx] = sz53n_subTable[idx];
        }
    }

    sz53n_addTable[0] |= ZERO_MASK;
    sz53pn_addTable[0] |= ZERO_MASK;
    sz53n_subTable[0] |= ZERO_MASK;
    sz53pn_subTable[0] |= ZERO_MASK;

    Z80opsImpl = ops;
    execDone = false;
    breakpointAt = new bool[65536];
    resetBreakpoints();
    reset();
}

// Acceso a registros de 8 bits
uint8_t Z80::getRegA(void) {
    return regA;
}

void Z80::setRegA(uint8_t value) {
    regA = value;
}

uint8_t Z80::getRegB(void) {
    return REG_B;
}

void Z80::setRegB(uint8_t value) {
    REG_B = value;
}

uint8_t Z80::getRegC(void) {
    return REG_C;
}

void Z80::setRegC(uint8_t value) {
    REG_C = value;
}

uint8_t Z80::getRegD(void) {
    return REG_D;
}

void Z80::setRegD(uint8_t value) {
    REG_D = value;
}

uint8_t Z80::getRegE(void) {
    return REG_E;
}

void Z80::setRegE(uint8_t value) {
    REG_E = value;
}

uint8_t Z80::getRegH(void) {
    return REG_H;
}

void Z80::setRegH(uint8_t value) {
    REG_H = value;
}

uint8_t Z80::getRegL(void) {
    return REG_L;
}

void Z80::setRegL(uint8_t value) {
    REG_L = value;
}

// Acceso a registros alternativos de 8 bits
uint8_t Z80::getRegAx(void) {
    return REG_Ax;
}

void Z80::setRegAx(uint8_t value) {
    REG_Ax = value;
}

uint8_t Z80::getRegFx(void) {
    return REG_Fx;
}

void Z80::setRegFx(uint8_t value) {
    REG_Fx = value;
}

uint8_t Z80::getRegBx(void) {
    return REG_Bx;
}

void Z80::setRegBx(uint8_t value) {
    REG_Bx = value;
}

uint8_t Z80::getRegCx(void) {
    return REG_Cx;
}

void Z80::setRegCx(uint8_t value) {
    REG_Cx = value;
}

uint8_t Z80::getRegDx(void) {
    return REG_Dx;
}

void Z80::setRegDx(uint8_t value) {
    REG_Dx = value;
}

uint8_t Z80::getRegEx(void) {
    return REG_Ex;
}

void Z80::setRegEx(uint8_t value) {
    REG_Ex = value;
}

uint8_t Z80::getRegHx(void) {
    return REG_Hx;
}

void Z80::setRegHx(uint8_t value) {
    REG_Hx = value;
}

uint8_t Z80::getRegLx(void) {
    return REG_Lx;
}

void Z80::setRegLx(uint8_t value) {
    REG_Lx = value;
}

// Acceso a registros de 16 bits
uint16_t Z80::getRegAF(void) {
    return (regA << 8) | (carryFlag ? sz5h3pnFlags | CARRY_MASK : sz5h3pnFlags);
}

void Z80::setRegAF(uint16_t word) {
    regA = word >> 8;

    sz5h3pnFlags = word & 0xfe;
    carryFlag = (word & CARRY_MASK) != 0;
}

uint16_t Z80::getRegAFx(void) {
    return REG_AFx;
}

void Z80::setRegAFx(uint16_t word) {
    REG_AFx = word;
}

uint16_t Z80::getRegBC(void) {
    return REG_BC;
}

void Z80::setRegBC(uint16_t word) {
    REG_BC = word;
}

uint16_t Z80::getRegBCx(void) {
    return REG_BCx;
}

void Z80::setRegBCx(uint16_t word) {
    REG_BCx = word;
}

uint16_t Z80::getRegDE(void) {
    return REG_DE;
}

void Z80::setRegDE(uint16_t word) {
    REG_DE = word;
}

uint16_t Z80::getRegDEx(void) {
    return REG_DEx;
}

void Z80::setRegDEx(uint16_t word) {
    REG_DEx = word;
}

uint16_t Z80::getRegHL(void) {
    return REG_HL;
}

void Z80::setRegHL(uint16_t word) {
    REG_HL = word;
}

uint16_t Z80::getRegHLx(void) {
    return REG_HLx;
}

void Z80::setRegHLx(uint16_t word) {
    REG_HLx = word;
}

// Acceso a registros de propósito específico
uint16_t Z80::getRegPC(void) {
    return regPC;
}

void Z80::setRegPC(uint16_t address) {
    regPC = address;
}

uint16_t Z80::getRegSP(void) {
    return regSP;
}

void Z80::setRegSP(uint16_t word) {
    regSP = word;
}

uint16_t Z80::getRegIX(void) {
    return REG_IX;
}

void Z80::setRegIX(uint16_t word) {
    REG_IX = word;
}

uint16_t Z80::getRegIY(void) {
    return REG_IY;
}

void Z80::setRegIY(uint16_t word) {
    REG_IY = word;
}

uint8_t Z80::getRegI(void) {
    return regI;
}

void Z80::setRegI(uint8_t value) {
    regI = value;
}

uint8_t Z80::getRegR(void) {
    return regRbit7 ? (regR & 0x7f) | SIGN_MASK : regR & 0x7f;
}

void Z80::setRegR(uint8_t value) {
    regR = value & 0x7f;
    regRbit7 = (value > 0x7f);
}

uint16_t Z80::getPairIR(void) {
    if (regRbit7) {
        return (regI << 8) | ((regR & 0x7f) | SIGN_MASK);
    }
    return (regI << 8) | (regR & 0x7f);
}

// Acceso al registro oculto MEMPTR
uint16_t Z80::getMemPtr(void) {
    return memptr;
}

void Z80::setMemPtr(uint16_t word) {
    memptr = word;
}

// Acceso a los flags uno a uno
bool Z80::isCarryFlag(void) {
    return carryFlag;
}

void Z80::setCarryFlag(bool state) {
    carryFlag = state;
}

bool Z80::isAddSubFlag(void) {
    return (sz5h3pnFlags & ADDSUB_MASK) != 0;
}

void Z80::setAddSubFlag(bool state) {
    if (state) {
        sz5h3pnFlags |= ADDSUB_MASK;
    } else {
        sz5h3pnFlags &= ~ADDSUB_MASK;
    }
}

bool Z80::isParOverFlag(void) {
    return (sz5h3pnFlags & PARITY_MASK) != 0;
}

void Z80::setParOverFlag(bool state) {
    if (state) {
        sz5h3pnFlags |= PARITY_MASK;
    } else {
        sz5h3pnFlags &= ~PARITY_MASK;
    }
}

bool Z80::isBit3Flag(void) {
    return (sz5h3pnFlags & BIT3_MASK) != 0;
}

void Z80::setBit3Fag(bool state) {
    if (state) {
        sz5h3pnFlags |= BIT3_MASK;
    } else {
        sz5h3pnFlags &= ~BIT3_MASK;
    }
}

bool Z80::isHalfCarryFlag(void) {
    return (sz5h3pnFlags & HALFCARRY_MASK) != 0;
}

void Z80::setHalfCarryFlag(bool state) {
    if (state) {
        sz5h3pnFlags |= HALFCARRY_MASK;
    } else {
        sz5h3pnFlags &= ~HALFCARRY_MASK;
    }
}

bool Z80::isBit5Flag(void) {
    return (sz5h3pnFlags & BIT5_MASK) != 0;
}

void Z80::setBit5Flag(bool state) {
    if (state) {
        sz5h3pnFlags |= BIT5_MASK;
    } else {
        sz5h3pnFlags &= ~BIT5_MASK;
    }
}

bool Z80::isZeroFlag(void) {
    return (sz5h3pnFlags & ZERO_MASK) != 0;
}

void Z80::setZeroFlag(bool state) {
    if (state) {
        sz5h3pnFlags |= ZERO_MASK;
    } else {
        sz5h3pnFlags &= ~ZERO_MASK;
    }
}

bool Z80::isSignFlag(void) {
    return (sz5h3pnFlags & SIGN_MASK) != 0;
}

void Z80::setSignFlag(bool state) {
    if (state) {
        sz5h3pnFlags |= SIGN_MASK;
    } else {
        sz5h3pnFlags &= ~SIGN_MASK;
    }
}

// Acceso a los flags F
uint8_t Z80::getFlags(void) {
    return carryFlag ? sz5h3pnFlags | CARRY_MASK : sz5h3pnFlags;
}

void Z80::setFlags(uint8_t regF) {
    sz5h3pnFlags = regF & 0xfe;

    carryFlag = (regF & CARRY_MASK) != 0;
}

// Acceso a los flip-flops de interrupción
bool Z80::isIFF1(void) {
    return ffIFF1;
}

void Z80::setIFF1(bool state) {
    ffIFF1 = state;
}

bool Z80::isIFF2(void) {
    return ffIFF2;
}

void Z80::setIFF2(bool state) {
    ffIFF2 = state;
}

bool Z80::isNMI(void) {
    return activeNMI;
}

void Z80::setNMI(bool nmi) {
    activeNMI = nmi;
}

// La línea de NMI se activa por impulso, no por nivel
void Z80::triggerNMI(void) {
    activeNMI = true;
}

// La línea INT se activa por nivel
bool Z80::isINTLine(void) {
    return activeINT;
}

void Z80::setINTLine(bool intLine) {
    activeINT = intLine;
}

//Acceso al modo de interrupción
Z80::IntMode Z80::getIM(void) {
    return modeINT;
}

void Z80::setIM(Z80::IntMode mode) {
    modeINT = mode;
}

bool Z80::isHalted(void) {
    return halted;
}

void Z80::setHalted(bool state) {
    halted = state;
}

void Z80::setPinReset(void) {
    pinReset = true;
}

bool Z80::isPendingEI(void) {
    return pendingEI;
}

void Z80::setPendingEI(bool state) {
    pendingEI = state;
}

// Reset
/* Según el documento de Sean Young, que se encuentra en
 * [http://www.myquest.com/z80undocumented], la mejor manera de emular el
 * reset es poniendo PC, IFF1, IFF2, R e IM0 a 0 y todos los demás registros
 * a 0xFFFF.
 *
 * 29/05/2011: cuando la CPU recibe alimentación por primera vez, los
 *             registros PC e IR se inicializan a cero y el resto a 0xFF.
 *             Si se produce un reset a través de la patilla correspondiente,
 *             los registros PC e IR se inicializan a 0 y el resto se preservan.
 *             En cualquier caso, todo parece depender bastante del modelo
 *             concreto de Z80, así que se escoge el comportamiento del
 *             modelo Zilog Z8400APS. Z80A CPU.
 *             http://www.worldofspectrum.org/forums/showthread.php?t=34574
 */
void Z80::reset(void) {
    if (pinReset) {
        pinReset = false;
    } else {
        regA = 0xff;
        setFlags(0xff);

        REG_AFx = 0xffff;
        REG_BC = REG_BCx = 0xffff;
        REG_DE = REG_DEx = 0xffff;
        REG_HL = REG_HLx = 0xffff;

        REG_IX = REG_IY = 0xffff;

        regSP = 0xffff;

        memptr = 0xffff;
    }

    regPC = 0;
    regI = regR = 0;
    regRbit7 = false;
    ffIFF1 = false;
    ffIFF2 = false;
    pendingEI = false;
    activeNMI = false;
    activeINT = false;
    halted = false;
    setIM(IntMode::IM0);
    lastFlagQ = false;
}

// Rota a la izquierda el valor del argumento
// El bit 0 y el flag C toman el valor del bit 7 antes de la operación
uint8_t Z80::rlc(uint8_t oper8) {
    carryFlag = (oper8 > 0x7f);
    oper8 <<= 1;
    if (carryFlag) {
        oper8 |= CARRY_MASK;
    }
    sz5h3pnFlags = sz53pn_addTable[oper8];
    flagQ = true;
    return oper8;
}
// Rota a la izquierda el valor del argumento
// El bit 7 va al carry flag
// El bit 0 toma el valor del flag C antes de la operación
uint8_t Z80::rl(uint8_t oper8) {
    bool carry = carryFlag;
    carryFlag = (oper8 > 0x7f);
    oper8 <<= 1;
    if (carry) {
        oper8 |= CARRY_MASK;
    }
    sz5h3pnFlags = sz53pn_addTable[oper8];
    flagQ = true;
    return oper8;
}

// Rota a la izquierda el valor del argumento
// El bit 7 va al carry flag
// El bit 0 toma el valor 0
uint8_t Z80::sla(uint8_t oper8) {
    carryFlag = (oper8 > 0x7f);
    oper8 <<= 1;
    sz5h3pnFlags = sz53pn_addTable[oper8];
    flagQ = true;
    return oper8;
}

// Rota a la izquierda el valor del argumento (como sla salvo por el bit 0)
// El bit 7 va al carry flag
// El bit 0 toma el valor 1
// Instrucción indocumentada
uint8_t Z80::sll(uint8_t oper8) {
    carryFlag = (oper8 > 0x7f);
    oper8 <<= 1;
    oper8 |= CARRY_MASK;
    sz5h3pnFlags = sz53pn_addTable[oper8];
    flagQ = true;
    return oper8;
}

// Rota a la derecha el valor del argumento
// El bit 7 y el flag C toman el valor del bit 0 antes de la operación
uint8_t Z80::rrc(uint8_t oper8) {
    carryFlag = (oper8 & CARRY_MASK) != 0;
    oper8 >>= 1;
    if (carryFlag) {
        oper8 |= SIGN_MASK;
    }
    sz5h3pnFlags = sz53pn_addTable[oper8];
    flagQ = true;
    return oper8;
}

// Rota a la derecha el valor del argumento
// El bit 0 va al carry flag
// El bit 7 toma el valor del flag C antes de la operación
uint8_t Z80::rr(uint8_t oper8) {
    bool carry = carryFlag;
    carryFlag = (oper8 & CARRY_MASK) != 0;
    oper8 >>= 1;
    if (carry) {
        oper8 |= SIGN_MASK;
    }
    sz5h3pnFlags = sz53pn_addTable[oper8];
    flagQ = true;
    return oper8;
}

// Rota a la derecha 1 bit el valor del argumento
// El bit 0 pasa al carry.
// El bit 7 conserva el valor que tenga
uint8_t Z80::sra(uint8_t oper8) {
    uint8_t sign = oper8 & SIGN_MASK;
    carryFlag = (oper8 & CARRY_MASK) != 0;
    oper8 = (oper8 >> 1) | sign;
    sz5h3pnFlags = sz53pn_addTable[oper8];
    flagQ = true;
    return oper8;
}

// Rota a la derecha 1 bit el valor del argumento
// El bit 0 pasa al carry.
// El bit 7 toma el valor 0
uint8_t Z80::srl(uint8_t oper8) {
    carryFlag = (oper8 & CARRY_MASK) != 0;
    oper8 >>= 1;
    sz5h3pnFlags = sz53pn_addTable[oper8];
    flagQ = true;
    return oper8;
}

/*
 * Half-carry flag:
 *
 * FLAG = (A ^ B ^ RESULT) & 0x10  for any operation
 *
 * Overflow flag:
 *
 * FLAG = ~(A ^ B) & (B ^ RESULT) & 0x80 for addition [ADD/ADC]
 * FLAG = (A ^ B) & (A ^ RESULT) &0x80 for subtraction [SUB/SBC]
 *
 * For INC/DEC, you can use following simplifications:
 *
 * INC:
 * H_FLAG = (RESULT & 0x0F) == 0x00
 * V_FLAG = RESULT == 0x80
 *
 * DEC:
 * H_FLAG = (RESULT & 0x0F) == 0x0F
 * V_FLAG = RESULT == 0x7F
 */
// Incrementa un valor de 8 bits modificando los flags oportunos
uint8_t Z80::inc8(uint8_t oper8) {
    oper8++;

    sz5h3pnFlags = sz53n_addTable[oper8];

    if ((oper8 & 0x0f) == 0) {
        sz5h3pnFlags |= HALFCARRY_MASK;
    }

    if (oper8 == 0x80) {
        sz5h3pnFlags |= OVERFLOW_MASK;
    }

    flagQ = true;
    return oper8;
}

// Decrementa un valor de 8 bits modificando los flags oportunos
uint8_t Z80::dec8(uint8_t oper8) {
    oper8--;

    sz5h3pnFlags = sz53n_subTable[oper8];

    if ((oper8 & 0x0f) == 0x0f) {
        sz5h3pnFlags |= HALFCARRY_MASK;
    }

    if (oper8 == 0x7f) {
        sz5h3pnFlags |= OVERFLOW_MASK;
    }

    flagQ = true;
    return oper8;
}

// Suma de 8 bits afectando a los flags
void Z80::add(uint8_t oper8) {
    uint16_t res = regA + oper8;

    carryFlag = res > 0xff;
    res &= 0xff;
    sz5h3pnFlags = sz53n_addTable[res];

    /* El módulo 16 del resultado será menor que el módulo 16 del registro A
     * si ha habido HalfCarry. Sucede lo mismo para todos los métodos suma
     * SIN carry */
    if ((res & 0x0f) < (regA & 0x0f)) {
        sz5h3pnFlags |= HALFCARRY_MASK;
    }

    if (((regA ^ ~oper8) & (regA ^ res)) > 0x7f) {
        sz5h3pnFlags |= OVERFLOW_MASK;
    }

    regA = res;
    flagQ = true;
}

// Suma con acarreo de 8 bits
void Z80::adc(uint8_t oper8) {
    uint16_t res = regA + oper8;

    if (carryFlag) {
        res++;
    }

    carryFlag = res > 0xff;
    res &= 0xff;
    sz5h3pnFlags = sz53n_addTable[res];

    if (((regA ^ oper8 ^ res) & 0x10) != 0) {
        sz5h3pnFlags |= HALFCARRY_MASK;
    }

    if (((regA ^ ~oper8) & (regA ^ res)) > 0x7f) {
        sz5h3pnFlags |= OVERFLOW_MASK;
    }

    regA = res;
    flagQ = true;
}

// Suma dos operandos de 16 bits sin carry afectando a los flags
uint16_t Z80::add16(uint16_t reg16, uint16_t oper16) {
    uint32_t tmp = oper16 + reg16;

    carryFlag = tmp > 0xffff;
    oper16 = tmp;
    sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZP_MASK) | ((oper16 >> 8) & FLAG_53_MASK);

    if ((oper16 & 0x0fff) < (reg16 & 0x0fff)) {
        sz5h3pnFlags |= HALFCARRY_MASK;
    }

    memptr = reg16 + 1;
    flagQ = true;
    return oper16;
}

// Suma con acarreo de 16 bits
void Z80::adc16(uint16_t reg16) {
    uint16_t tmpHL = REG_HL;
    memptr = REG_HL + 1;

    uint32_t res = REG_HL + reg16;
    if (carryFlag) {
        res++;
    }

    carryFlag = res > 0xffff;
    res &= 0xffff;
    REG_HL = (uint16_t) res;

    sz5h3pnFlags = sz53n_addTable[REG_H];
    if (res != 0) {
        sz5h3pnFlags &= ~ZERO_MASK;
    }

    if (((res ^ tmpHL ^ reg16) & 0x1000) != 0) {
        sz5h3pnFlags |= HALFCARRY_MASK;
    }

    if (((tmpHL ^ ~reg16) & (tmpHL ^ res)) > 0x7fff) {
        sz5h3pnFlags |= OVERFLOW_MASK;
    }

    flagQ = true;
}

// Resta de 8 bits
void Z80::sub(uint8_t oper8) {
    int16_t res = regA - oper8;

    carryFlag = res < 0;
    res &= 0xff;
    sz5h3pnFlags = sz53n_subTable[res];

    /* El módulo 16 del resultado será mayor que el módulo 16 del registro A
     * si ha habido HalfCarry. Sucede lo mismo para todos los métodos resta
     * SIN carry, incluido cp */
    if ((res & 0x0f) > (regA & 0x0f)) {
        sz5h3pnFlags |= HALFCARRY_MASK;
    }

    if (((regA ^ oper8) & (regA ^ res)) > 0x7f) {
        sz5h3pnFlags |= OVERFLOW_MASK;
    }

    regA = res;
    flagQ = true;
}

// Resta con acarreo de 8 bits
void Z80::sbc(uint8_t oper8) {
    int16_t res = regA - oper8;

    if (carryFlag) {
        res--;
    }

    carryFlag = res < 0;
    res &= 0xff;
    sz5h3pnFlags = sz53n_subTable[res];

    if (((regA ^ oper8 ^ res) & 0x10) != 0) {
        sz5h3pnFlags |= HALFCARRY_MASK;
    }

    if (((regA ^ oper8) & (regA ^ res)) > 0x7f) {
        sz5h3pnFlags |= OVERFLOW_MASK;
    }

    regA = res;
    flagQ = true;
}

// Resta con acarreo de 16 bits
void Z80::sbc16(uint16_t reg16) {
    uint16_t tmpHL = REG_HL;
    memptr = REG_HL + 1;

    int32_t res = REG_HL - reg16;
    if (carryFlag) {
        res--;
    }

    carryFlag = res < 0;
    res &= 0xffff;
    REG_HL = (uint16_t) res;

    sz5h3pnFlags = sz53n_subTable[REG_H];
    if (res != 0) {
        sz5h3pnFlags &= ~ZERO_MASK;
    }

    if (((res ^ tmpHL ^ reg16) & 0x1000) != 0) {
        sz5h3pnFlags |= HALFCARRY_MASK;
    }

    if (((tmpHL ^ reg16) & (tmpHL ^ res)) > 0x7fff) {
        sz5h3pnFlags |= OVERFLOW_MASK;
    }
    flagQ = true;
}

// Operación AND lógica
void Z80::and_(uint8_t oper8) {
    regA &= oper8;
    carryFlag = false;
    sz5h3pnFlags = sz53pn_addTable[regA] | HALFCARRY_MASK;
    flagQ = true;
}

// Operación XOR lógica
void Z80::xor_(uint8_t oper8) {
    regA ^= oper8;
    carryFlag = false;
    sz5h3pnFlags = sz53pn_addTable[regA];
    flagQ = true;
}

// Operación OR lógica
void Z80::or_(uint8_t oper8) {
    regA |= oper8;
    carryFlag = false;
    sz5h3pnFlags = sz53pn_addTable[regA];
    flagQ = true;
}

// Operación de comparación con el registro A
// es como SUB, pero solo afecta a los flags
// Los flags SIGN y ZERO se calculan a partir del resultado
// Los flags 3 y 5 se copian desde el operando (sigh!)
void Z80::cp(uint8_t oper8) {
    int16_t res = regA - oper8;

    carryFlag = res < 0;
    res &= 0xff;

    sz5h3pnFlags = (sz53n_addTable[oper8] & FLAG_53_MASK)
            | // No necesito preservar H, pero está a 0 en la tabla de todas formas
            (sz53n_subTable[res] & FLAG_SZHN_MASK);

    if ((res & 0x0f) > (regA & 0x0f)) {
        sz5h3pnFlags |= HALFCARRY_MASK;
    }

    if (((regA ^ oper8) & (regA ^ res)) > 0x7f) {
        sz5h3pnFlags |= OVERFLOW_MASK;
    }

    flagQ = true;
}

// DAA
void Z80::daa(void) {
    uint8_t suma = 0;
    bool carry = carryFlag;

    if ((sz5h3pnFlags & HALFCARRY_MASK) != 0 || (regA & 0x0f) > 0x09) {
        suma = 6;
    }

    if (carry || (regA > 0x99)) {
        suma |= 0x60;
    }

    if (regA > 0x99) {
        carry = true;
    }

    if ((sz5h3pnFlags & ADDSUB_MASK) != 0) {
        sub(suma);
        sz5h3pnFlags = (sz5h3pnFlags & HALFCARRY_MASK) | sz53pn_subTable[regA];
    } else {
        add(suma);
        sz5h3pnFlags = (sz5h3pnFlags & HALFCARRY_MASK) | sz53pn_addTable[regA];
    }

    carryFlag = carry;
    // Los add/sub ya ponen el resto de los flags
    flagQ = true;
}

// POP
uint16_t Z80::pop(void) {
    uint16_t word = Z80opsImpl->peek16(regSP);
    regSP = regSP + 2;
    return word;
}

// PUSH
void Z80::push(uint16_t word) {
    Z80opsImpl->poke8(--regSP, word >> 8);
    Z80opsImpl->poke8(--regSP, word);
}

// LDI
void Z80::ldi(void) {
    uint8_t work8 = Z80opsImpl->peek8(REG_HL);
    Z80opsImpl->poke8(REG_DE, work8);
    Z80opsImpl->addressOnBus(REG_DE, 2);
    REG_HL++;
    REG_DE++;
    REG_BC--;
    work8 += regA;

    sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZ_MASK) | (work8 & BIT3_MASK);

    if ((work8 & ADDSUB_MASK) != 0) {
        sz5h3pnFlags |= BIT5_MASK;
    }

    if (REG_BC != 0) {
        sz5h3pnFlags |= PARITY_MASK;
    }
    flagQ = true;
}

// LDD
void Z80::ldd(void) {
    uint8_t work8 = Z80opsImpl->peek8(REG_HL);
    Z80opsImpl->poke8(REG_DE, work8);
    Z80opsImpl->addressOnBus(REG_DE, 2);
    REG_HL--;
    REG_DE--;
    REG_BC--;
    work8 += regA;

    sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZ_MASK) | (work8 & BIT3_MASK);

    if ((work8 & ADDSUB_MASK) != 0) {
        sz5h3pnFlags |= BIT5_MASK;
    }

    if (REG_BC != 0) {
        sz5h3pnFlags |= PARITY_MASK;
    }
    flagQ = true;
}

// CPI
void Z80::cpi(void) {
    uint8_t memHL = Z80opsImpl->peek8(REG_HL);
    bool carry = carryFlag; // lo guardo porque cp lo toca
    cp(memHL);
    carryFlag = carry;
    Z80opsImpl->addressOnBus(REG_HL, 5);
    REG_HL++;
    REG_BC--;
    memHL = regA - memHL - ((sz5h3pnFlags & HALFCARRY_MASK) != 0 ? 1 : 0);
    sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHN_MASK) | (memHL & BIT3_MASK);

    if ((memHL & ADDSUB_MASK) != 0) {
        sz5h3pnFlags |= BIT5_MASK;
    }

    if (REG_BC != 0) {
        sz5h3pnFlags |= PARITY_MASK;
    }

    memptr++;
    flagQ = true;
}

// CPD
void Z80::cpd(void) {
    uint8_t memHL = Z80opsImpl->peek8(REG_HL);
    bool carry = carryFlag; // lo guardo porque cp lo toca
    cp(memHL);
    carryFlag = carry;
    Z80opsImpl->addressOnBus(REG_HL, 5);
    REG_HL--;
    REG_BC--;
    memHL = regA - memHL - ((sz5h3pnFlags & HALFCARRY_MASK) != 0 ? 1 : 0);
    sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHN_MASK) | (memHL & BIT3_MASK);

    if ((memHL & ADDSUB_MASK) != 0) {
        sz5h3pnFlags |= BIT5_MASK;
    }

    if (REG_BC != 0) {
        sz5h3pnFlags |= PARITY_MASK;
    }

    memptr--;
    flagQ = true;
}

// INI
void Z80::ini(void) {
    memptr = REG_BC;
    Z80opsImpl->addressOnBus(getPairIR(), 1);
    uint8_t work8 = Z80opsImpl->inPort(memptr++);
    Z80opsImpl->poke8(REG_HL, work8);

    REG_B--;
    REG_HL++;

    sz5h3pnFlags = sz53pn_addTable[REG_B];
    if (work8 > 0x7f) {
        sz5h3pnFlags |= ADDSUB_MASK;
    }

    carryFlag = false;
    uint16_t tmp = work8 + REG_C + 1;
    if (tmp > 0xff) {
        sz5h3pnFlags |= HALFCARRY_MASK;
        carryFlag = true;
    }

    if ((sz53pn_addTable[((tmp & 0x07) ^ REG_B)]
            & PARITY_MASK) == PARITY_MASK) {
        sz5h3pnFlags |= PARITY_MASK;
    } else {
        sz5h3pnFlags &= ~PARITY_MASK;
    }
    flagQ = true;
}

// IND
void Z80::ind(void) {
    memptr = REG_BC;
    Z80opsImpl->addressOnBus(getPairIR(), 1);
    uint8_t work8 = Z80opsImpl->inPort(memptr--);
    Z80opsImpl->poke8(REG_HL, work8);

    REG_B--;
    REG_HL--;

    sz5h3pnFlags = sz53pn_addTable[REG_B];
    if (work8 > 0x7f) {
        sz5h3pnFlags |= ADDSUB_MASK;
    }

    carryFlag = false;
    uint16_t tmp = work8 + (REG_C - 1);
    if (tmp > 0xff) {
        sz5h3pnFlags |= HALFCARRY_MASK;
        carryFlag = true;
    }

    if ((sz53pn_addTable[((tmp & 0x07) ^ REG_B)]
            & PARITY_MASK) == PARITY_MASK) {
        sz5h3pnFlags |= PARITY_MASK;
    } else {
        sz5h3pnFlags &= ~PARITY_MASK;
    }
    flagQ = true;
}

// OUTI
void Z80::outi(void) {

    Z80opsImpl->addressOnBus(getPairIR(), 1);

    REG_B--;
    memptr = REG_BC;

    uint8_t work8 = Z80opsImpl->peek8(REG_HL);
    Z80opsImpl->outPort(memptr++, work8);

    REG_HL++;

    carryFlag = false;
    if (work8 > 0x7f) {
        sz5h3pnFlags = sz53n_subTable[REG_B];
    } else {
        sz5h3pnFlags = sz53n_addTable[REG_B];
    }

    if ((REG_L + work8) > 0xff) {
        sz5h3pnFlags |= HALFCARRY_MASK;
        carryFlag = true;
    }

    if ((sz53pn_addTable[(((REG_L + work8) & 0x07) ^ REG_B)]
            & PARITY_MASK) == PARITY_MASK) {
        sz5h3pnFlags |= PARITY_MASK;
    }
    flagQ = true;
}

// OUTD
void Z80::outd(void) {

    Z80opsImpl->addressOnBus(getPairIR(), 1);

    REG_B--;
    memptr = REG_BC;

    uint8_t work8 = Z80opsImpl->peek8(REG_HL);
    Z80opsImpl->outPort(memptr--, work8);

    REG_HL--;

    carryFlag = false;
    if (work8 > 0x7f) {
        sz5h3pnFlags = sz53n_subTable[REG_B];
    } else {
        sz5h3pnFlags = sz53n_addTable[REG_B];
    }

    if ((REG_L + work8) > 0xff) {
        sz5h3pnFlags |= HALFCARRY_MASK;
        carryFlag = true;
    }

    if ((sz53pn_addTable[(((REG_L + work8) & 0x07) ^ REG_B)]
            & PARITY_MASK) == PARITY_MASK) {
        sz5h3pnFlags |= PARITY_MASK;
    }
    flagQ = true;
}

// Pone a 1 el Flag Z si el bit b del registro
// r es igual a 0
/*
 * En contra de lo que afirma el Z80-Undocumented, los bits 3 y 5 toman
 * SIEMPRE el valor de los bits correspondientes del valor a comparar para
 * las instrucciones BIT n,r. Para BIT n,(HL) toman el valor del registro
 * escondido (memptr), y para las BIT n, (IX/IY+n) toman el valor de los
 * bits superiores de la dirección indicada por IX/IY+n.
 *
 * 04/12/08 Confirmado el comentario anterior:
 *          http://scratchpad.wikia.com/wiki/Z80
 */
void Z80::bit(uint8_t mask, uint8_t reg) {
    bool zeroFlag = (mask & reg) == 0;

    sz5h3pnFlags = (sz53n_addTable[reg] & ~FLAG_SZP_MASK) | HALFCARRY_MASK;

    if (zeroFlag) {
        sz5h3pnFlags |= (PARITY_MASK | ZERO_MASK);
    }

    if (mask == SIGN_MASK && !zeroFlag) {
        sz5h3pnFlags |= SIGN_MASK;
    }
    flagQ = true;
}

//Interrupción
/* Desglose de la interrupción, según el modo:
 * IM0:
 *      M1: 7 T-Estados -> reconocer INT y decSP
 *      M2: 3 T-Estados -> escribir byte alto y decSP
 *      M3: 3 T-Estados -> escribir byte bajo y salto a N
 * IM1:
 *      M1: 7 T-Estados -> reconocer INT y decSP
 *      M2: 3 T-Estados -> escribir byte alto PC y decSP
 *      M3: 3 T-Estados -> escribir byte bajo PC y PC=0x0038
 * IM2:
 *      M1: 7 T-Estados -> reconocer INT y decSP
 *      M2: 3 T-Estados -> escribir byte alto y decSP
 *      M3: 3 T-Estados -> escribir byte bajo
 *      M4: 3 T-Estados -> leer byte bajo del vector de INT
 *      M5: 3 T-Estados -> leer byte alto y saltar a la rutina de INT
 */
void Z80::interrupt(void) {
    // Si estaba en un HALT esperando una INT, lo saca de la espera
    if (halted) {
        halted = false;
        regPC++;
    }

    Z80opsImpl->interruptHandlingTime(7);

    regR++;
    ffIFF1 = ffIFF2 = false;
    push(regPC); // el push añadirá 6 t-estados (+contended si toca)
    if (modeINT == IntMode::IM2) {
        regPC = Z80opsImpl->peek16((regI << 8) | 0xff); // +6 t-estados
    } else {
        regPC = 0x0038;
    }
    memptr = regPC;
}

//Interrupción NMI, no utilizado por ahora
/* Desglose de ciclos de máquina y T-Estados
 * M1: 5 T-Estados -> extraer opcode (pá ná, es tontería) y decSP
 * M2: 3 T-Estados -> escribe byte alto de PC y decSP
 * M3: 3 T-Estados -> escribe byte bajo de PC y PC=0x0066
 */
void Z80::nmi(void) {
    // Esta lectura consigue dos cosas:
    //      1.- La lectura del opcode del M1 que se descarta
    //      2.- Si estaba en un HALT esperando una INT, lo saca de la espera
    Z80opsImpl->fetchOpcode(regPC);
    Z80opsImpl->interruptHandlingTime(1);
    if (halted) {
        halted = false;
        regPC++;
    }
    regR++;
    ffIFF1 = false;
    push(regPC); // 3+3 t-estados + contended si procede
    regPC = memptr = 0x0066;
}

bool Z80::isBreakpoint(uint16_t address) {
    return breakpointAt[address & 0xffff];
}

void Z80::setBreakpoint(uint16_t address, bool state) {
    breakpointAt[address] = state;
}

void Z80::resetBreakpoints(void) {
    for (int i = 0; i < 0x10000; i++) {
        breakpointAt[i] = false;
    }
}

void Z80::execute(void) {
    // Primero se comprueba NMI
    if (activeNMI) {
        activeNMI = false;
        lastFlagQ = false;
        nmi();
    }

    // Ahora se comprueba si al final de la instrucción anterior se
    // encontró una interrupción enmascarable y, de ser así, se procesa.
    if (activeINT) {
        if (ffIFF1 && !pendingEI) {
            lastFlagQ = false;
            interrupt();
        }
    }

    regR++;
    opCode = Z80opsImpl->fetchOpcode(regPC);

    if (breakpointAt[regPC]) {
        opCode = Z80opsImpl->breakpoint(regPC, opCode);
    }

    regPC++;

    flagQ = false;

    decodeOpcode(opCode);

    lastFlagQ = flagQ;

    // Si está pendiente la activación de la interrupciones y el
    // código que se acaba de ejecutar no es el propio EI
    if (pendingEI && opCode != 0xFB) {
        pendingEI = false;
    }

    if (execDone) {
        Z80opsImpl->execDone();
    }
}

void Z80::decodeOpcode(uint8_t opCode) {

    switch (opCode) {
        case 0x00:
        { /* NOP */
            break;
        }
        case 0x01:
        { /* LD BC,nn */
            REG_BC = Z80opsImpl->peek16(regPC);
            regPC = regPC + 2;
            break;
        }
        case 0x02:
        { /* LD (BC),A */
            Z80opsImpl->poke8(REG_BC, regA);
            memptr = (regA << 8) | (REG_C + 1);
            break;
        }
        case 0x03:
        { /* INC BC */
            Z80opsImpl->addressOnBus(getPairIR(), 2);
            REG_BC++;
            break;
        }
        case 0x04:
        { /* INC B */
            REG_B = inc8(REG_B);
            break;
        }
        case 0x05:
        { /* DEC B */
            REG_B = dec8(REG_B);
            break;
        }
        case 0x06:
        { /* LD B,n */
            REG_B = Z80opsImpl->peek8(regPC);
            regPC++;
            break;
        }
        case 0x07:
        { /* RLCA */
            carryFlag = (regA > 0x7f);
            regA <<= 1;
            if (carryFlag) {
                regA |= CARRY_MASK;
            }
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZP_MASK) | (regA & FLAG_53_MASK);
            flagQ = true;
            break;
        }
        case 0x08:
        { /* EX AF,AF' */
            uint8_t work8 = regA;
            regA = REG_Ax;
            REG_Ax = work8;

            work8 = getFlags();
            setFlags(REG_Fx);
            REG_Fx = work8;
            break;
        }
        case 0x09:
        { /* ADD HL,BC */
            Z80opsImpl->addressOnBus(getPairIR(), 7);
            REG_HL = add16(REG_HL, REG_BC);
            break;
        }
        case 0x0A:
        { /* LD A,(BC) */
            regA = Z80opsImpl->peek8(REG_BC);
            memptr = REG_BC + 1;
            break;
        }
        case 0x0B:
        { /* DEC BC */
            Z80opsImpl->addressOnBus(getPairIR(), 2);
            REG_BC--;
            break;
        }
        case 0x0C:
        { /* INC C */
            REG_C = inc8(REG_C);
            break;
        }
        case 0x0D:
        { /* DEC C */
            REG_C = dec8(REG_C);
            break;
        }
        case 0x0E:
        { /* LD C,n */
            REG_C = Z80opsImpl->peek8(regPC);
            regPC++;
            break;
        }
        case 0x0F:
        { /* RRCA */
            carryFlag = (regA & CARRY_MASK) != 0;
            regA >>= 1;
            if (carryFlag) {
                regA |= SIGN_MASK;
            }
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZP_MASK) | (regA & FLAG_53_MASK);
            flagQ = true;
            break;
        }
        case 0x10:
        { /* DJNZ e */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            int8_t offset = Z80opsImpl->peek8(regPC);
            if (--REG_B != 0) {
                Z80opsImpl->addressOnBus(regPC, 5);
                regPC = memptr = regPC + offset + 1;
            } else {
                regPC++;
            }
            break;
        }
        case 0x11:
        { /* LD DE,nn */
            REG_DE = Z80opsImpl->peek16(regPC);
            regPC = regPC + 2;
            break;
        }
        case 0x12:
        { /* LD (DE),A */
            Z80opsImpl->poke8(REG_DE, regA);
            memptr = (regA << 8) | (REG_E + 1);
            break;
        }
        case 0x13:
        { /* INC DE */
            Z80opsImpl->addressOnBus(getPairIR(), 2);
            REG_DE++;
            break;
        }
        case 0x14:
        { /* INC D */
            REG_D = inc8(REG_D);
            break;
        }
        case 0x15:
        { /* DEC D */
            REG_D = dec8(REG_D);
            break;
        }
        case 0x16:
        { /* LD D,n */
            REG_D = Z80opsImpl->peek8(regPC);
            regPC++;
            break;
        }
        case 0x17:
        { /* RLA */
            bool oldCarry = carryFlag;
            carryFlag = regA > 0x7f;
            regA <<= 1;
            if (oldCarry) {
                regA |= CARRY_MASK;
            }
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZP_MASK) | (regA & FLAG_53_MASK);
            flagQ = true;
            break;
        }
        case 0x18:
        { /* JR e */
            int8_t offset = Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC = memptr = regPC + offset + 1;
            break;
        }
        case 0x19:
        { /* ADD HL,DE */
            Z80opsImpl->addressOnBus(getPairIR(), 7);
            REG_HL = add16(REG_HL, REG_DE);
            break;
        }
        case 0x1A:
        { /* LD A,(DE) */
            regA = Z80opsImpl->peek8(REG_DE);
            memptr = REG_DE + 1;
            break;
        }
        case 0x1B:
        { /* DEC DE */
            Z80opsImpl->addressOnBus(getPairIR(), 2);
            REG_DE--;
            break;
        }
        case 0x1C:
        { /* INC E */
            REG_E = inc8(REG_E);
            break;
        }
        case 0x1D:
        { /* DEC E */
            REG_E = dec8(REG_E);
            break;
        }
        case 0x1E:
        { /* LD E,n */
            REG_E = Z80opsImpl->peek8(regPC);
            regPC++;
            break;
        }
        case 0x1F:
        { /* RRA */
            bool oldCarry = carryFlag;
            carryFlag = (regA & CARRY_MASK) != 0;
            regA >>= 1;
            if (oldCarry) {
                regA |= SIGN_MASK;
            }
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZP_MASK) | (regA & FLAG_53_MASK);
            flagQ = true;
            break;
        }
        case 0x20:
        { /* JR NZ,e */
            int8_t offset = Z80opsImpl->peek8(regPC);
            if ((sz5h3pnFlags & ZERO_MASK) == 0) {
                Z80opsImpl->addressOnBus(regPC, 5);
                regPC += offset;
                memptr = regPC + 1;
            }
            regPC++;
            break;
        }
        case 0x21:
        { /* LD HL,nn */
            REG_HL = Z80opsImpl->peek16(regPC);
            regPC = regPC + 2;
            break;
        }
        case 0x22:
        { /* LD (nn),HL */
            memptr = Z80opsImpl->peek16(regPC);
            Z80opsImpl->poke16(memptr, REG_HL);
            memptr++;
            regPC = regPC + 2;
            break;
        }
        case 0x23:
        { /* INC HL */
            Z80opsImpl->addressOnBus(getPairIR(), 2);
            REG_HL++;
            break;
        }
        case 0x24:
        { /* INC H */
            REG_H = inc8(REG_H);
            break;
        }
        case 0x25:
        { /* DEC H */
            REG_H = dec8(REG_H);
            break;
        }
        case 0x26:
        { /* LD H,n */
            REG_H = Z80opsImpl->peek8(regPC);
            regPC++;
            break;
        }
        case 0x27:
        { /* DAA */
            daa();
            break;
        }
        case 0x28:
        { /* JR Z,e */
            int8_t offset = Z80opsImpl->peek8(regPC);
            if ((sz5h3pnFlags & ZERO_MASK) != 0) {
                Z80opsImpl->addressOnBus(regPC, 5);
                regPC += offset;
                memptr = regPC + 1;
            }
            regPC++;
            break;
        }
        case 0x29:
        { /* ADD HL,HL */
            Z80opsImpl->addressOnBus(getPairIR(), 7);
            REG_HL = add16(REG_HL, REG_HL);
            break;
        }
        case 0x2A:
        { /* LD HL,(nn) */
            memptr = Z80opsImpl->peek16(regPC);
            REG_HL = Z80opsImpl->peek16(memptr);
            memptr++;
            regPC = regPC + 2;
            break;
        }
        case 0x2B:
        { /* DEC HL */
            Z80opsImpl->addressOnBus(getPairIR(), 2);
            REG_HL--;
            break;
        }
        case 0x2C:
        { /* INC L */
            REG_L = inc8(REG_L);
            break;
        }
        case 0x2D:
        { /* DEC L */
            REG_L = dec8(REG_L);
            break;
        }
        case 0x2E:
        { /* LD L,n */
            REG_L = Z80opsImpl->peek8(regPC);
            regPC++;
            break;
        }
        case 0x2F:
        { /* CPL */
            regA ^= 0xff;
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZP_MASK) | HALFCARRY_MASK
                    | (regA & FLAG_53_MASK) | ADDSUB_MASK;
            flagQ = true;
            break;
        }
        case 0x30:
        { /* JR NC,e */
            int8_t offset = Z80opsImpl->peek8(regPC);
            if (!carryFlag) {
                Z80opsImpl->addressOnBus(regPC, 5);
                regPC += offset;
                memptr = regPC + 1;
            }
            regPC++;
            break;
        }
        case 0x31:
        { /* LD SP,nn */
            regSP = Z80opsImpl->peek16(regPC);
            regPC = regPC + 2;
            break;
        }
        case 0x32:
        { /* LD (nn),A */
            memptr = Z80opsImpl->peek16(regPC);
            Z80opsImpl->poke8(memptr, regA);
            memptr = (regA << 8) | ((memptr + 1) & 0xff);
            regPC = regPC + 2;
            break;
        }
        case 0x33:
        { /* INC SP */
            Z80opsImpl->addressOnBus(getPairIR(), 2);
            regSP++;
            break;
        }
        case 0x34:
        { /* INC (HL) */
            uint8_t work8 = inc8(Z80opsImpl->peek8(REG_HL));
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0x35:
        { /* DEC (HL) */
            uint8_t work8 = dec8(Z80opsImpl->peek8(REG_HL));
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0x36:
        { /* LD (HL),n */
            Z80opsImpl->poke8(REG_HL, Z80opsImpl->peek8(regPC));
            regPC++;
            break;
        }
        case 0x37:
        { /* SCF */
            uint8_t regQ = lastFlagQ ? sz5h3pnFlags : 0;
            carryFlag = true;
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZP_MASK) | (((regQ ^ sz5h3pnFlags) | regA) & FLAG_53_MASK);
            flagQ = true;
            break;
        }
        case 0x38:
        { /* JR C,e */
            int8_t offset = Z80opsImpl->peek8(regPC);
            if (carryFlag) {
                Z80opsImpl->addressOnBus(regPC, 5);
                regPC += offset;
                memptr = regPC + 1;
            }
            regPC++;
            break;
        }
        case 0x39:
        { /* ADD HL,SP */
            Z80opsImpl->addressOnBus(getPairIR(), 7);
            REG_HL = add16(REG_HL, regSP);
            break;
        }
        case 0x3A:
        { /* LD A,(nn) */
            memptr = Z80opsImpl->peek16(regPC);
            regA = Z80opsImpl->peek8(memptr);
            memptr++;
            regPC = regPC + 2;
            break;
        }
        case 0x3B:
        { /* DEC SP */
            Z80opsImpl->addressOnBus(getPairIR(), 2);
            regSP--;
            break;
        }
        case 0x3C:
        { /* INC A */
            regA = inc8(regA);
            break;
        }
        case 0x3D:
        { /* DEC A */
            regA = dec8(regA);
            break;
        }
        case 0x3E:
        { /* LD A,n */
            regA = Z80opsImpl->peek8(regPC);
            regPC++;
            break;
        }
        case 0x3F:
        { /* CCF */
            uint8_t regQ = lastFlagQ ? sz5h3pnFlags : 0;
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZP_MASK) | (((regQ ^ sz5h3pnFlags) | regA) & FLAG_53_MASK);
            if (carryFlag) {
                sz5h3pnFlags |= HALFCARRY_MASK;
            }
            carryFlag = !carryFlag;
            flagQ = true;
            break;
        }
//      case 0x40: {     /* LD B,B */
//           break;
//    }
        case 0x41:
        { /* LD B,C */
            REG_B = REG_C;
            break;
        }
        case 0x42:
        { /* LD B,D */
            REG_B = REG_D;
            break;
        }
        case 0x43:
        { /* LD B,E */
            REG_B = REG_E;
            break;
        }
        case 0x44:
        { /* LD B,H */
            REG_B = REG_H;
            break;
        }
        case 0x45:
        { /* LD B,L */
            REG_B = REG_L;
            break;
        }
        case 0x46:
        { /* LD B,(HL) */
            REG_B = Z80opsImpl->peek8(REG_HL);
            break;
        }
        case 0x47:
        { /* LD B,A */
            REG_B = regA;
            break;
        }
        case 0x48:
        { /* LD C,B */
            REG_C = REG_B;
            break;
        }
//        case 0x49: {     /* LD C,C */
//            break;
//        }
        case 0x4A:
        { /* LD C,D */
            REG_C = REG_D;
            break;
        }
        case 0x4B:
        { /* LD C,E */
            REG_C = REG_E;
            break;
        }
        case 0x4C:
        { /* LD C,H */
            REG_C = REG_H;
            break;
        }
        case 0x4D:
        { /* LD C,L */
            REG_C = REG_L;
            break;
        }
        case 0x4E:
        { /* LD C,(HL) */
            REG_C = Z80opsImpl->peek8(REG_HL);
            break;
        }
        case 0x4F:
        { /* LD C,A */
            REG_C = regA;
            break;
        }
        case 0x50:
        { /* LD D,B */
            REG_D = REG_B;
            break;
        }
        case 0x51:
        { /* LD D,C */
            REG_D = REG_C;
            break;
        }
//            case 0x52: {     /* LD D,D */
//                break;
//            }
        case 0x53:
        { /* LD D,E */
            REG_D = REG_E;
            break;
        }
        case 0x54:
        { /* LD D,H */
            REG_D = REG_H;
            break;
        }
        case 0x55:
        { /* LD D,L */
            REG_D = REG_L;
            break;
        }
        case 0x56:
        { /* LD D,(HL) */
            REG_D = Z80opsImpl->peek8(REG_HL);
            break;
        }
        case 0x57:
        { /* LD D,A */
            REG_D = regA;
            break;
        }
        case 0x58:
        { /* LD E,B */
            REG_E = REG_B;
            break;
        }
        case 0x59:
        { /* LD E,C */
            REG_E = REG_C;
            break;
        }
        case 0x5A:
        { /* LD E,D */
            REG_E = REG_D;
            break;
        }
//            case 0x5B: {     /* LD E,E */
//                break;
//            }
        case 0x5C:
        { /* LD E,H */
            REG_E = REG_H;
            break;
        }
        case 0x5D:
        { /* LD E,L */
            REG_E = REG_L;
            break;
        }
        case 0x5E:
        { /* LD E,(HL) */
            REG_E = Z80opsImpl->peek8(REG_HL);
            break;
        }
        case 0x5F:
        { /* LD E,A */
            REG_E = regA;
            break;
        }
        case 0x60:
        { /* LD H,B */
            REG_H = REG_B;
            break;
        }
        case 0x61:
        { /* LD H,C */
            REG_H = REG_C;
            break;
        }
        case 0x62:
        { /* LD H,D */
            REG_H = REG_D;
            break;
        }
        case 0x63:
        { /* LD H,E */
            REG_H = REG_E;
            break;
        }
//            case 0x64: {     /* LD H,H */
//                break;
//            }
        case 0x65:
        { /* LD H,L */
            REG_H = REG_L;
            break;
        }
        case 0x66:
        { /* LD H,(HL) */
            REG_H = Z80opsImpl->peek8(REG_HL);
            break;
        }
        case 0x67:
        { /* LD H,A */
            REG_H = regA;
            break;
        }
        case 0x68:
        { /* LD L,B */
            REG_L = REG_B;
            break;
        }
        case 0x69:
        { /* LD L,C */
            REG_L = REG_C;
            break;
        }
        case 0x6A:
        { /* LD L,D */
            REG_L = REG_D;
            break;
        }
        case 0x6B:
        { /* LD L,E */
            REG_L = REG_E;
            break;
        }
        case 0x6C:
        { /* LD L,H */
            REG_L = REG_H;
            break;
        }
//            case 0x6D: {     /* LD L,L */
//                break;
//            }
        case 0x6E:
        { /* LD L,(HL) */
            REG_L = Z80opsImpl->peek8(REG_HL);
            break;
        }
        case 0x6F:
        { /* LD L,A */
            REG_L = regA;
            break;
        }
        case 0x70:
        { /* LD (HL),B */
            Z80opsImpl->poke8(REG_HL, REG_B);
            break;
        }
        case 0x71:
        { /* LD (HL),C */
            Z80opsImpl->poke8(REG_HL, REG_C);
            break;
        }
        case 0x72:
        { /* LD (HL),D */
            Z80opsImpl->poke8(REG_HL, REG_D);
            break;
        }
        case 0x73:
        { /* LD (HL),E */
            Z80opsImpl->poke8(REG_HL, REG_E);
            break;
        }
        case 0x74:
        { /* LD (HL),H */
            Z80opsImpl->poke8(REG_HL, REG_H);
            break;
        }
        case 0x75:
        { /* LD (HL),L */
            Z80opsImpl->poke8(REG_HL, REG_L);
            break;
        }
        case 0x76:
        { /* HALT */
            regPC--;
            halted = true;
            break;
        }
        case 0x77:
        { /* LD (HL),A */
            Z80opsImpl->poke8(REG_HL, regA);
            break;
        }
        case 0x78:
        { /* LD A,B */
            regA = REG_B;
            break;
        }
        case 0x79:
        { /* LD A,C */
            regA = REG_C;
            break;
        }
        case 0x7A:
        { /* LD A,D */
            regA = REG_D;
            break;
        }
        case 0x7B:
        { /* LD A,E */
            regA = REG_E;
            break;
        }
        case 0x7C:
        { /* LD A,H */
            regA = REG_H;
            break;
        }
        case 0x7D:
        { /* LD A,L */
            regA = REG_L;
            break;
        }
        case 0x7E:
        { /* LD A,(HL) */
            regA = Z80opsImpl->peek8(REG_HL);
            break;
        }
//            case 0x7F: {     /* LD A,A */
//                break;
//            }
        case 0x80:
        { /* ADD A,B */
            add(REG_B);
            break;
        }
        case 0x81:
        { /* ADD A,C */
            add(REG_C);
            break;
        }
        case 0x82:
        { /* ADD A,D */
            add(REG_D);
            break;
        }
        case 0x83:
        { /* ADD A,E */
            add(REG_E);
            break;
        }
        case 0x84:
        { /* ADD A,H */
            add(REG_H);
            break;
        }
        case 0x85:
        { /* ADD A,L */
            add(REG_L);
            break;
        }
        case 0x86:
        { /* ADD A,(HL) */
            add(Z80opsImpl->peek8(REG_HL));
            break;
        }
        case 0x87:
        { /* ADD A,A */
            add(regA);
            break;
        }
        case 0x88:
        { /* ADC A,B */
            adc(REG_B);
            break;
        }
        case 0x89:
        { /* ADC A,C */
            adc(REG_C);
            break;
        }
        case 0x8A:
        { /* ADC A,D */
            adc(REG_D);
            break;
        }
        case 0x8B:
        { /* ADC A,E */
            adc(REG_E);
            break;
        }
        case 0x8C:
        { /* ADC A,H */
            adc(REG_H);
            break;
        }
        case 0x8D:
        { /* ADC A,L */
            adc(REG_L);
            break;
        }
        case 0x8E:
        { /* ADC A,(HL) */
            adc(Z80opsImpl->peek8(REG_HL));
            break;
        }
        case 0x8F:
        { /* ADC A,A */
            adc(regA);
            break;
        }
        case 0x90:
        { /* SUB B */
            sub(REG_B);
            break;
        }
        case 0x91:
        { /* SUB C */
            sub(REG_C);
            break;
        }
        case 0x92:
        { /* SUB D */
            sub(REG_D);
            break;
        }
        case 0x93:
        { /* SUB E */
            sub(REG_E);
            break;
        }
        case 0x94:
        { /* SUB H */
            sub(REG_H);
            break;
        }
        case 0x95:
        { /* SUB L */
            sub(REG_L);
            break;
        }
        case 0x96:
        { /* SUB (HL) */
            sub(Z80opsImpl->peek8(REG_HL));
            break;
        }
        case 0x97:
        { /* SUB A */
            sub(regA);
            break;
        }
        case 0x98:
        { /* SBC A,B */
            sbc(REG_B);
            break;
        }
        case 0x99:
        { /* SBC A,C */
            sbc(REG_C);
            break;
        }
        case 0x9A:
        { /* SBC A,D */
            sbc(REG_D);
            break;
        }
        case 0x9B:
        { /* SBC A,E */
            sbc(REG_E);
            break;
        }
        case 0x9C:
        { /* SBC A,H */
            sbc(REG_H);
            break;
        }
        case 0x9D:
        { /* SBC A,L */
            sbc(REG_L);
            break;
        }
        case 0x9E:
        { /* SBC A,(HL) */
            sbc(Z80opsImpl->peek8(REG_HL));
            break;
        }
        case 0x9F:
        { /* SBC A,A */
            sbc(regA);
            break;
        }
        case 0xA0:
        { /* AND B */
            and_(REG_B);
            break;
        }
        case 0xA1:
        { /* AND C */
            and_(REG_C);
            break;
        }
        case 0xA2:
        { /* AND D */
            and_(REG_D);
            break;
        }
        case 0xA3:
        { /* AND E */
            and_(REG_E);
            break;
        }
        case 0xA4:
        { /* AND H */
            and_(REG_H);
            break;
        }
        case 0xA5:
        { /* AND L */
            and_(REG_L);
            break;
        }
        case 0xA6:
        { /* AND (HL) */
            and_(Z80opsImpl->peek8(REG_HL));
            break;
        }
        case 0xA7:
        { /* AND A */
            and_(regA);
            break;
        }
        case 0xA8:
        { /* XOR B */
            xor_(REG_B);
            break;
        }
        case 0xA9:
        { /* XOR C */
            xor_(REG_C);
            break;
        }
        case 0xAA:
        { /* XOR D */
            xor_(REG_D);
            break;
        }
        case 0xAB:
        { /* XOR E */
            xor_(REG_E);
            break;
        }
        case 0xAC:
        { /* XOR H */
            xor_(REG_H);
            break;
        }
        case 0xAD:
        { /* XOR L */
            xor_(REG_L);
            break;
        }
        case 0xAE:
        { /* XOR (HL) */
            xor_(Z80opsImpl->peek8(REG_HL));
            break;
        }
        case 0xAF:
        { /* XOR A */
            xor_(regA);
            break;
        }
        case 0xB0:
        { /* OR B */
            or_(REG_B);
            break;
        }
        case 0xB1:
        { /* OR C */
            or_(REG_C);
            break;
        }
        case 0xB2:
        { /* OR D */
            or_(REG_D);
            break;
        }
        case 0xB3:
        { /* OR E */
            or_(REG_E);
            break;
        }
        case 0xB4:
        { /* OR H */
            or_(REG_H);
            break;
        }
        case 0xB5:
        { /* OR L */
            or_(REG_L);
            break;
        }
        case 0xB6:
        { /* OR (HL) */
            or_(Z80opsImpl->peek8(REG_HL));
            break;
        }
        case 0xB7:
        { /* OR A */
            or_(regA);
            break;
        }
        case 0xB8:
        { /* CP B */
            cp(REG_B);
            break;
        }
        case 0xB9:
        { /* CP C */
            cp(REG_C);
            break;
        }
        case 0xBA:
        { /* CP D */
            cp(REG_D);
            break;
        }
        case 0xBB:
        { /* CP E */
            cp(REG_E);
            break;
        }
        case 0xBC:
        { /* CP H */
            cp(REG_H);
            break;
        }
        case 0xBD:
        { /* CP L */
            cp(REG_L);
            break;
        }
        case 0xBE:
        { /* CP (HL) */
            cp(Z80opsImpl->peek8(REG_HL));
            break;
        }
        case 0xBF:
        { /* CP A */
            cp(regA);
            break;
        }
        case 0xC0:
        { /* RET NZ */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            if ((sz5h3pnFlags & ZERO_MASK) == 0) {
                regPC = memptr = pop();
            }
            break;
        }
        case 0xC1:
        { /* POP BC */
            REG_BC = pop();
            break;
        }
        case 0xC2:
        { /* JP NZ,nn */
            memptr = Z80opsImpl->peek16(regPC);
            if ((sz5h3pnFlags & ZERO_MASK) == 0) {
                regPC = memptr;
                break;
            }
            regPC = regPC + 2;
            break;
        }
        case 0xC3:
        { /* JP nn */
            memptr = regPC = Z80opsImpl->peek16(regPC);
            break;
        }
        case 0xC4:
        { /* CALL NZ,nn */
            memptr = Z80opsImpl->peek16(regPC);
            if ((sz5h3pnFlags & ZERO_MASK) == 0) {
                Z80opsImpl->addressOnBus(regPC + 1, 1);
                push(regPC + 2);
                regPC = memptr;
                break;
            }
            regPC = regPC + 2;
            break;
        }
        case 0xC5:
        { /* PUSH BC */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            push(REG_BC);
            break;
        }
        case 0xC6:
        { /* ADD A,n */
            add(Z80opsImpl->peek8(regPC));
            regPC++;
            break;
        }
        case 0xC7:
        { /* RST 00H */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            push(regPC);
            regPC = memptr = 0x00;
            break;
        }
        case 0xC8:
        { /* RET Z */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            if ((sz5h3pnFlags & ZERO_MASK) != 0) {
                regPC = memptr = pop();
            }
            break;
        }
        case 0xC9:
        { /* RET */
            regPC = memptr = pop();
            break;
        }
        case 0xCA:
        { /* JP Z,nn */
            memptr = Z80opsImpl->peek16(regPC);
            if ((sz5h3pnFlags & ZERO_MASK) != 0) {
                regPC = memptr;
                break;
            }
            regPC = regPC + 2;
            break;
        }
        case 0xCB:
        { /* Subconjunto de instrucciones */
            decodeCB();
            break;
        }
        case 0xCC:
        { /* CALL Z,nn */
            memptr = Z80opsImpl->peek16(regPC);
            if ((sz5h3pnFlags & ZERO_MASK) != 0) {
                Z80opsImpl->addressOnBus(regPC + 1, 1);
                push(regPC + 2);
                regPC = memptr;
                break;
            }
            regPC = regPC + 2;
            break;
        }
        case 0xCD:
        { /* CALL nn */
            memptr = Z80opsImpl->peek16(regPC);
            Z80opsImpl->addressOnBus(regPC + 1, 1);
            push(regPC + 2);
            regPC = memptr;
            break;
        }
        case 0xCE:
        { /* ADC A,n */
            adc(Z80opsImpl->peek8(regPC));
            regPC++;
            break;
        }
        case 0xCF:
        { /* RST 08H */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            push(regPC);
            regPC = memptr = 0x08;
            break;
        }
        case 0xD0:
        { /* RET NC */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            if (!carryFlag) {
                regPC = memptr = pop();
            }
            break;
        }
        case 0xD1:
        { /* POP DE */
            REG_DE = pop();
            break;
        }
        case 0xD2:
        { /* JP NC,nn */
            memptr = Z80opsImpl->peek16(regPC);
            if (!carryFlag) {
                regPC = memptr;
                break;
            }
            regPC = regPC + 2;
            break;
        }
        case 0xD3:
        { /* OUT (n),A */
            uint8_t work8 = Z80opsImpl->peek8(regPC);
            regPC++;
            memptr = regA << 8;
            Z80opsImpl->outPort(memptr | work8, regA);
            memptr |= (work8 + 1);
            break;
        }
        case 0xD4:
        { /* CALL NC,nn */
            memptr = Z80opsImpl->peek16(regPC);
            if (!carryFlag) {
                Z80opsImpl->addressOnBus(regPC + 1, 1);
                push(regPC + 2);
                regPC = memptr;
                break;
            }
            regPC = regPC + 2;
            break;
        }
        case 0xD5:
        { /* PUSH DE */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            push(REG_DE);
            break;
        }
        case 0xD6:
        { /* SUB n */
            sub(Z80opsImpl->peek8(regPC));
            regPC++;
            break;
        }
        case 0xD7:
        { /* RST 10H */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            push(regPC);
            regPC = memptr = 0x10;
            break;
        }
        case 0xD8:
        { /* RET C */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            if (carryFlag) {
                regPC = memptr = pop();
            }
            break;
        }
        case 0xD9:
        { /* EXX */
            uint16_t tmp;
            tmp = REG_BC;
            REG_BC = REG_BCx;
            REG_BCx = tmp;

            tmp = REG_DE;
            REG_DE = REG_DEx;
            REG_DEx = tmp;

            tmp = REG_HL;
            REG_HL = REG_HLx;
            REG_HLx = tmp;
            break;
        }
        case 0xDA:
        { /* JP C,nn */
            memptr = Z80opsImpl->peek16(regPC);
            if (carryFlag) {
                regPC = memptr;
                break;
            }
            regPC = regPC + 2;
            break;
        }
        case 0xDB:
        { /* IN A,(n) */
            memptr = (regA << 8) | Z80opsImpl->peek8(regPC);
            regPC++;
            regA = Z80opsImpl->inPort(memptr);
            memptr++;
            break;
        }
        case 0xDC:
        { /* CALL C,nn */
            memptr = Z80opsImpl->peek16(regPC);
            if (carryFlag) {
                Z80opsImpl->addressOnBus(regPC + 1, 1);
                push(regPC + 2);
                regPC = memptr;
                break;
            }
            regPC = regPC + 2;
            break;
        }
        case 0xDD:
        { /* Subconjunto de instrucciones */
            regIX = decodeDDFD(regIX);
            break;
        }
        case 0xDE:
        { /* SBC A,n */
            sbc(Z80opsImpl->peek8(regPC));
            regPC++;
            break;
        }
        case 0xDF:
        { /* RST 18H */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            push(regPC);
            regPC = memptr = 0x18;
            break;
        }
        case 0xE0: /* RET PO */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            if ((sz5h3pnFlags & PARITY_MASK) == 0) {
                regPC = memptr = pop();
            }
            break;
        case 0xE1: /* POP HL */
            REG_HL = pop();
            break;
        case 0xE2: /* JP PO,nn */
            memptr = Z80opsImpl->peek16(regPC);
            if ((sz5h3pnFlags & PARITY_MASK) == 0) {
                regPC = memptr;
                break;
            }
            regPC = regPC + 2;
            break;
        case 0xE3:
        { /* EX (SP),HL */
            // Instrucción de ejecución sutil.
            RegisterPair work = regHL;
            REG_HL = Z80opsImpl->peek16(regSP);
            Z80opsImpl->addressOnBus(regSP + 1, 1);
            // No se usa poke16 porque el Z80 escribe los bytes AL REVES
            Z80opsImpl->poke8(regSP + 1, work.byte8.hi);
            Z80opsImpl->poke8(regSP, work.byte8.lo);
            Z80opsImpl->addressOnBus(regSP, 2);
            memptr = REG_HL;
            break;
        }
        case 0xE4: /* CALL PO,nn */
            memptr = Z80opsImpl->peek16(regPC);
            if ((sz5h3pnFlags & PARITY_MASK) == 0) {
                Z80opsImpl->addressOnBus(regPC + 1, 1);
                push(regPC + 2);
                regPC = memptr;
                break;
            }
            regPC = regPC + 2;
            break;
        case 0xE5: /* PUSH HL */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            push(REG_HL);
            break;
        case 0xE6: /* AND n */
            and_(Z80opsImpl->peek8(regPC));
            regPC++;
            break;
        case 0xE7: /* RST 20H */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            push(regPC);
            regPC = memptr = 0x20;
            break;
        case 0xE8: /* RET PE */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            if ((sz5h3pnFlags & PARITY_MASK) != 0) {
                regPC = memptr = pop();
            }
            break;
        case 0xE9: /* JP (HL) */
            regPC = REG_HL;
            break;
        case 0xEA: /* JP PE,nn */
            memptr = Z80opsImpl->peek16(regPC);
            if ((sz5h3pnFlags & PARITY_MASK) != 0) {
                regPC = memptr;
                break;
            }
            regPC = regPC + 2;
            break;
        case 0xEB:
        { /* EX DE,HL */
            uint16_t tmp = REG_HL;
            REG_HL = REG_DE;
            REG_DE = tmp;
            break;
        }
        case 0xEC: /* CALL PE,nn */
            memptr = Z80opsImpl->peek16(regPC);
            if ((sz5h3pnFlags & PARITY_MASK) != 0) {
                Z80opsImpl->addressOnBus(regPC + 1, 1);
                push(regPC + 2);
                regPC = memptr;
                break;
            }
            regPC = regPC + 2;
            break;
        case 0xED: /*Subconjunto de instrucciones*/
            decodeED();
            break;
        case 0xEE: /* XOR n */
            xor_(Z80opsImpl->peek8(regPC));
            regPC++;
            break;
        case 0xEF: /* RST 28H */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            push(regPC);
            regPC = memptr = 0x28;
            break;
        case 0xF0: /* RET P */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            if (sz5h3pnFlags < SIGN_MASK) {
                regPC = memptr = pop();
            }
            break;
        case 0xF1: /* POP AF */
            setRegAF(pop());
            break;
        case 0xF2: /* JP P,nn */
            memptr = Z80opsImpl->peek16(regPC);
            if (sz5h3pnFlags < SIGN_MASK) {
                regPC = memptr;
                break;
            }
            regPC = regPC + 2;
            break;
        case 0xF3: /* DI */
            ffIFF1 = ffIFF2 = false;
            break;
        case 0xF4: /* CALL P,nn */
            memptr = Z80opsImpl->peek16(regPC);
            if (sz5h3pnFlags < SIGN_MASK) {
                Z80opsImpl->addressOnBus(regPC + 1, 1);
                push(regPC + 2);
                regPC = memptr;
                break;
            }
            regPC = regPC + 2;
            break;
        case 0xF5: /* PUSH AF */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            push(getRegAF());
            break;
        case 0xF6: /* OR n */
            or_(Z80opsImpl->peek8(regPC));
            regPC++;
            break;
        case 0xF7: /* RST 30H */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            push(regPC);
            regPC = memptr = 0x30;
            break;
        case 0xF8: /* RET M */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            if (sz5h3pnFlags > 0x7f) {
                regPC = memptr = pop();
            }
            break;
        case 0xF9: /* LD SP,HL */
            Z80opsImpl->addressOnBus(getPairIR(), 2);
            regSP = REG_HL;
            break;
        case 0xFA: /* JP M,nn */
            memptr = Z80opsImpl->peek16(regPC);
            if (sz5h3pnFlags > 0x7f) {
                regPC = memptr;
                break;
            }
            regPC = regPC + 2;
            break;
        case 0xFB: /* EI */
            ffIFF1 = ffIFF2 = true;
            pendingEI = true;
            break;
        case 0xFC: /* CALL M,nn */
            memptr = Z80opsImpl->peek16(regPC);
            if (sz5h3pnFlags > 0x7f) {
                Z80opsImpl->addressOnBus(regPC + 1, 1);
                push(regPC + 2);
                regPC = memptr;
                break;
            }
            regPC = regPC + 2;
            break;
        case 0xFD: /* Subconjunto de instrucciones */
            regIY = decodeDDFD(regIY);
            break;
        case 0xFE: /* CP n */
            cp(Z80opsImpl->peek8(regPC));
            regPC++;
            break;
        case 0xFF: /* RST 38H */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            push(regPC);
            regPC = memptr = 0x38;
    } /* del switch( codigo ) */
}

//Subconjunto de instrucciones 0xCB

void Z80::decodeCB(void) {
    regR++;
    opCode = Z80opsImpl->fetchOpcode(regPC);
    regPC++;

    switch (opCode) {
        case 0x00:
        { /* RLC B */
            REG_B = rlc(REG_B);
            break;
        }
        case 0x01:
        { /* RLC C */
            REG_C = rlc(REG_C);
            break;
        }
        case 0x02:
        { /* RLC D */
            REG_D = rlc(REG_D);
            break;
        }
        case 0x03:
        { /* RLC E */
            REG_E = rlc(REG_E);
            break;
        }
        case 0x04:
        { /* RLC H */
            REG_H = rlc(REG_H);
            break;
        }
        case 0x05:
        { /* RLC L */
            REG_L = rlc(REG_L);
            break;
        }
        case 0x06:
        { /* RLC (HL) */
            uint8_t work8 = rlc(Z80opsImpl->peek8(REG_HL));
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0x07:
        { /* RLC A */
            regA = rlc(regA);
            break;
        }
        case 0x08:
        { /* RRC B */
            REG_B = rrc(REG_B);
            break;
        }
        case 0x09:
        { /* RRC C */
            REG_C = rrc(REG_C);
            break;
        }
        case 0x0A:
        { /* RRC D */
            REG_D = rrc(REG_D);
            break;
        }
        case 0x0B:
        { /* RRC E */
            REG_E = rrc(REG_E);
            break;
        }
        case 0x0C:
        { /* RRC H */
            REG_H = rrc(REG_H);
            break;
        }
        case 0x0D:
        { /* RRC L */
            REG_L = rrc(REG_L);
            break;
        }
        case 0x0E:
        { /* RRC (HL) */
            uint8_t work8 = rrc(Z80opsImpl->peek8(REG_HL));
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0x0F:
        { /* RRC A */
            regA = rrc(regA);
            break;
        }
        case 0x10:
        { /* RL B */
            REG_B = rl(REG_B);
            break;
        }
        case 0x11:
        { /* RL C */
            REG_C = rl(REG_C);
            break;
        }
        case 0x12:
        { /* RL D */
            REG_D = rl(REG_D);
            break;
        }
        case 0x13:
        { /* RL E */
            REG_E = rl(REG_E);
            break;
        }
        case 0x14:
        { /* RL H */
            REG_H = rl(REG_H);
            break;
        }
        case 0x15:
        { /* RL L */
            REG_L = rl(REG_L);
            break;
        }
        case 0x16:
        { /* RL (HL) */
            uint8_t work8 = rl(Z80opsImpl->peek8(REG_HL));
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0x17:
        { /* RL A */
            regA = rl(regA);
            break;
        }
        case 0x18:
        { /* RR B */
            REG_B = rr(REG_B);
            break;
        }
        case 0x19:
        { /* RR C */
            REG_C = rr(REG_C);
            break;
        }
        case 0x1A:
        { /* RR D */
            REG_D = rr(REG_D);
            break;
        }
        case 0x1B:
        { /* RR E */
            REG_E = rr(REG_E);
            break;
        }
        case 0x1C:
        { /*RR H*/
            REG_H = rr(REG_H);
            break;
        }
        case 0x1D:
        { /* RR L */
            REG_L = rr(REG_L);
            break;
        }
        case 0x1E:
        { /* RR (HL) */
            uint8_t work8 = rr(Z80opsImpl->peek8(REG_HL));
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0x1F:
        { /* RR A */
            regA = rr(regA);
            break;
        }
        case 0x20:
        { /* SLA B */
            REG_B = sla(REG_B);
            break;
        }
        case 0x21:
        { /* SLA C */
            REG_C = sla(REG_C);
            break;
        }
        case 0x22:
        { /* SLA D */
            REG_D = sla(REG_D);
            break;
        }
        case 0x23:
        { /* SLA E */
            REG_E = sla(REG_E);
            break;
        }
        case 0x24:
        { /* SLA H */
            REG_H = sla(REG_H);
            break;
        }
        case 0x25:
        { /* SLA L */
            REG_L = sla(REG_L);
            break;
        }
        case 0x26:
        { /* SLA (HL) */
            uint8_t work8 = sla(Z80opsImpl->peek8(REG_HL));
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0x27:
        { /* SLA A */
            regA = sla(regA);
            break;
        }
        case 0x28:
        { /* SRA B */
            REG_B = sra(REG_B);
            break;
        }
        case 0x29:
        { /* SRA C */
            REG_C = sra(REG_C);
            break;
        }
        case 0x2A:
        { /* SRA D */
            REG_D = sra(REG_D);
            break;
        }
        case 0x2B:
        { /* SRA E */
            REG_E = sra(REG_E);
            break;
        }
        case 0x2C:
        { /* SRA H */
            REG_H = sra(REG_H);
            break;
        }
        case 0x2D:
        { /* SRA L */
            REG_L = sra(REG_L);
            break;
        }
        case 0x2E:
        { /* SRA (HL) */
            uint8_t work8 = sra(Z80opsImpl->peek8(REG_HL));
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0x2F:
        { /* SRA A */
            regA = sra(regA);
            break;
        }
        case 0x30:
        { /* SLL B */
            REG_B = sll(REG_B);
            break;
        }
        case 0x31:
        { /* SLL C */
            REG_C = sll(REG_C);
            break;
        }
        case 0x32:
        { /* SLL D */
            REG_D = sll(REG_D);
            break;
        }
        case 0x33:
        { /* SLL E */
            REG_E = sll(REG_E);
            break;
        }
        case 0x34:
        { /* SLL H */
            REG_H = sll(REG_H);
            break;
        }
        case 0x35:
        { /* SLL L */
            REG_L = sll(REG_L);
            break;
        }
        case 0x36:
        { /* SLL (HL) */
            uint8_t work8 = sll(Z80opsImpl->peek8(REG_HL));
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0x37:
        { /* SLL A */
            regA = sll(regA);
            break;
        }
        case 0x38:
        { /* SRL B */
            REG_B = srl(REG_B);
            break;
        }
        case 0x39:
        { /* SRL C */
            REG_C = srl(REG_C);
            break;
        }
        case 0x3A:
        { /* SRL D */
            REG_D = srl(REG_D);
            break;
        }
        case 0x3B:
        { /* SRL E */
            REG_E = srl(REG_E);
            break;
        }
        case 0x3C:
        { /* SRL H */
            REG_H = srl(REG_H);
            break;
        }
        case 0x3D:
        { /* SRL L */
            REG_L = srl(REG_L);
            break;
        }
        case 0x3E:
        { /* SRL (HL) */
            uint8_t work8 = srl(Z80opsImpl->peek8(REG_HL));
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0x3F:
        { /* SRL A */
            regA = srl(regA);
            break;
        }
        case 0x40:
        { /* BIT 0,B */
            bit(0x01, REG_B);
            break;
        }
        case 0x41:
        { /* BIT 0,C */
            bit(0x01, REG_C);
            break;
        }
        case 0x42:
        { /* BIT 0,D */
            bit(0x01, REG_D);
            break;
        }
        case 0x43:
        { /* BIT 0,E */
            bit(0x01, REG_E);
            break;
        }
        case 0x44:
        { /* BIT 0,H */
            bit(0x01, REG_H);
            break;
        }
        case 0x45:
        { /* BIT 0,L */
            bit(0x01, REG_L);
            break;
        }
        case 0x46:
        { /* BIT 0,(HL) */
            bit(0x01, Z80opsImpl->peek8(REG_HL));
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHP_MASK)
                    | ((memptr >> 8) & FLAG_53_MASK);
            Z80opsImpl->addressOnBus(REG_HL, 1);
            break;
        }
        case 0x47:
        { /* BIT 0,A */
            bit(0x01, regA);
            break;
        }
        case 0x48:
        { /* BIT 1,B */
            bit(0x02, REG_B);
            break;
        }
        case 0x49:
        { /* BIT 1,C */
            bit(0x02, REG_C);
            break;
        }
        case 0x4A:
        { /* BIT 1,D */
            bit(0x02, REG_D);
            break;
        }
        case 0x4B:
        { /* BIT 1,E */
            bit(0x02, REG_E);
            break;
        }
        case 0x4C:
        { /* BIT 1,H */
            bit(0x02, REG_H);
            break;
        }
        case 0x4D:
        { /* BIT 1,L */
            bit(0x02, REG_L);
            break;
        }
        case 0x4E:
        { /* BIT 1,(HL) */
            bit(0x02, Z80opsImpl->peek8(REG_HL));
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHP_MASK)
                    | ((memptr >> 8) & FLAG_53_MASK);
            Z80opsImpl->addressOnBus(REG_HL, 1);
            break;
        }
        case 0x4F:
        { /* BIT 1,A */
            bit(0x02, regA);
            break;
        }
        case 0x50:
        { /* BIT 2,B */
            bit(0x04, REG_B);
            break;
        }
        case 0x51:
        { /* BIT 2,C */
            bit(0x04, REG_C);
            break;
        }
        case 0x52:
        { /* BIT 2,D */
            bit(0x04, REG_D);
            break;
        }
        case 0x53:
        { /* BIT 2,E */
            bit(0x04, REG_E);
            break;
        }
        case 0x54:
        { /* BIT 2,H */
            bit(0x04, REG_H);
            break;
        }
        case 0x55:
        { /* BIT 2,L */
            bit(0x04, REG_L);
            break;
        }
        case 0x56:
        { /* BIT 2,(HL) */
            bit(0x04, Z80opsImpl->peek8(REG_HL));
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHP_MASK)
                    | ((memptr >> 8) & FLAG_53_MASK);
            Z80opsImpl->addressOnBus(REG_HL, 1);
            break;
        }
        case 0x57:
        { /* BIT 2,A */
            bit(0x04, regA);
            break;
        }
        case 0x58:
        { /* BIT 3,B */
            bit(0x08, REG_B);
            break;
        }
        case 0x59:
        { /* BIT 3,C */
            bit(0x08, REG_C);
            break;
        }
        case 0x5A:
        { /* BIT 3,D */
            bit(0x08, REG_D);
            break;
        }
        case 0x5B:
        { /* BIT 3,E */
            bit(0x08, REG_E);
            break;
        }
        case 0x5C:
        { /* BIT 3,H */
            bit(0x08, REG_H);
            break;
        }
        case 0x5D:
        { /* BIT 3,L */
            bit(0x08, REG_L);
            break;
        }
        case 0x5E:
        { /* BIT 3,(HL) */
            bit(0x08, Z80opsImpl->peek8(REG_HL));
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHP_MASK)
                    | ((memptr >> 8) & FLAG_53_MASK);
            Z80opsImpl->addressOnBus(REG_HL, 1);
            break;
        }
        case 0x5F:
        { /* BIT 3,A */
            bit(0x08, regA);
            break;
        }
        case 0x60:
        { /* BIT 4,B */
            bit(0x10, REG_B);
            break;
        }
        case 0x61:
        { /* BIT 4,C */
            bit(0x10, REG_C);
            break;
        }
        case 0x62:
        { /* BIT 4,D */
            bit(0x10, REG_D);
            break;
        }
        case 0x63:
        { /* BIT 4,E */
            bit(0x10, REG_E);
            break;
        }
        case 0x64:
        { /* BIT 4,H */
            bit(0x10, REG_H);
            break;
        }
        case 0x65:
        { /* BIT 4,L */
            bit(0x10, REG_L);
            break;
        }
        case 0x66:
        { /* BIT 4,(HL) */
            bit(0x10, Z80opsImpl->peek8(REG_HL));
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHP_MASK)
                    | ((memptr >> 8) & FLAG_53_MASK);
            Z80opsImpl->addressOnBus(REG_HL, 1);
            break;
        }
        case 0x67:
        { /* BIT 4,A */
            bit(0x10, regA);
            break;
        }
        case 0x68:
        { /* BIT 5,B */
            bit(0x20, REG_B);
            break;
        }
        case 0x69:
        { /* BIT 5,C */
            bit(0x20, REG_C);
            break;
        }
        case 0x6A:
        { /* BIT 5,D */
            bit(0x20, REG_D);
            break;
        }
        case 0x6B:
        { /* BIT 5,E */
            bit(0x20, REG_E);
            break;
        }
        case 0x6C:
        { /* BIT 5,H */
            bit(0x20, REG_H);
            break;
        }
        case 0x6D:
        { /* BIT 5,L */
            bit(0x20, REG_L);
            break;
        }
        case 0x6E:
        { /* BIT 5,(HL) */
            bit(0x20, Z80opsImpl->peek8(REG_HL));
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHP_MASK)
                    | ((memptr >> 8) & FLAG_53_MASK);
            Z80opsImpl->addressOnBus(REG_HL, 1);
            break;
        }
        case 0x6F:
        { /* BIT 5,A */
            bit(0x20, regA);
            break;
        }
        case 0x70:
        { /* BIT 6,B */
            bit(0x40, REG_B);
            break;
        }
        case 0x71:
        { /* BIT 6,C */
            bit(0x40, REG_C);
            break;
        }
        case 0x72:
        { /* BIT 6,D */
            bit(0x40, REG_D);
            break;
        }
        case 0x73:
        { /* BIT 6,E */
            bit(0x40, REG_E);
            break;
        }
        case 0x74:
        { /* BIT 6,H */
            bit(0x40, REG_H);
            break;
        }
        case 0x75:
        { /* BIT 6,L */
            bit(0x40, REG_L);
            break;
        }
        case 0x76:
        { /* BIT 6,(HL) */
            bit(0x40, Z80opsImpl->peek8(REG_HL));
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHP_MASK)
                    | ((memptr >> 8) & FLAG_53_MASK);
            Z80opsImpl->addressOnBus(REG_HL, 1);
            break;
        }
        case 0x77:
        { /* BIT 6,A */
            bit(0x40, regA);
            break;
        }
        case 0x78:
        { /* BIT 7,B */
            bit(0x80, REG_B);
            break;
        }
        case 0x79:
        { /* BIT 7,C */
            bit(0x80, REG_C);
            break;
        }
        case 0x7A:
        { /* BIT 7,D */
            bit(0x80, REG_D);
            break;
        }
        case 0x7B:
        { /* BIT 7,E */
            bit(0x80, REG_E);
            break;
        }
        case 0x7C:
        { /* BIT 7,H */
            bit(0x80, REG_H);
            break;
        }
        case 0x7D:
        { /* BIT 7,L */
            bit(0x80, REG_L);
            break;
        }
        case 0x7E:
        { /* BIT 7,(HL) */
            bit(0x80, Z80opsImpl->peek8(REG_HL));
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHP_MASK)
                    | ((memptr >> 8) & FLAG_53_MASK);
            Z80opsImpl->addressOnBus(REG_HL, 1);
            break;
        }
        case 0x7F:
        { /* BIT 7,A */
            bit(0x80, regA);
            break;
        }
        case 0x80:
        { /* RES 0,B */
            REG_B &= 0xFE;
            break;
        }
        case 0x81:
        { /* RES 0,C */
            REG_C &= 0xFE;
            break;
        }
        case 0x82:
        { /* RES 0,D */
            REG_D &= 0xFE;
            break;
        }
        case 0x83:
        { /* RES 0,E */
            REG_E &= 0xFE;
            break;
        }
        case 0x84:
        { /* RES 0,H */
            REG_H &= 0xFE;
            break;
        }
        case 0x85:
        { /* RES 0,L */
            REG_L &= 0xFE;
            break;
        }
        case 0x86:
        { /* RES 0,(HL) */
            uint8_t work8 = Z80opsImpl->peek8(REG_HL) & 0xFE;
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0x87:
        { /* RES 0,A */
            regA &= 0xFE;
            break;
        }
        case 0x88:
        { /* RES 1,B */
            REG_B &= 0xFD;
            break;
        }
        case 0x89:
        { /* RES 1,C */
            REG_C &= 0xFD;
            break;
        }
        case 0x8A:
        { /* RES 1,D */
            REG_D &= 0xFD;
            break;
        }
        case 0x8B:
        { /* RES 1,E */
            REG_E &= 0xFD;
            break;
        }
        case 0x8C:
        { /* RES 1,H */
            REG_H &= 0xFD;
            break;
        }
        case 0x8D:
        { /* RES 1,L */
            REG_L &= 0xFD;
            break;
        }
        case 0x8E:
        { /* RES 1,(HL) */
            uint8_t work8 = Z80opsImpl->peek8(REG_HL) & 0xFD;
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0x8F:
        { /* RES 1,A */
            regA &= 0xFD;
            break;
        }
        case 0x90:
        { /* RES 2,B */
            REG_B &= 0xFB;
            break;
        }
        case 0x91:
        { /* RES 2,C */
            REG_C &= 0xFB;
            break;
        }
        case 0x92:
        { /* RES 2,D */
            REG_D &= 0xFB;
            break;
        }
        case 0x93:
        { /* RES 2,E */
            REG_E &= 0xFB;
            break;
        }
        case 0x94:
        { /* RES 2,H */
            REG_H &= 0xFB;
            break;
        }
        case 0x95:
        { /* RES 2,L */
            REG_L &= 0xFB;
            break;
        }
        case 0x96:
        { /* RES 2,(HL) */
            uint8_t work8 = Z80opsImpl->peek8(REG_HL) & 0xFB;
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0x97:
        { /* RES 2,A */
            regA &= 0xFB;
            break;
        }
        case 0x98:
        { /* RES 3,B */
            REG_B &= 0xF7;
            break;
        }
        case 0x99:
        { /* RES 3,C */
            REG_C &= 0xF7;
            break;
        }
        case 0x9A:
        { /* RES 3,D */
            REG_D &= 0xF7;
            break;
        }
        case 0x9B:
        { /* RES 3,E */
            REG_E &= 0xF7;
            break;
        }
        case 0x9C:
        { /* RES 3,H */
            REG_H &= 0xF7;
            break;
        }
        case 0x9D:
        { /* RES 3,L */
            REG_L &= 0xF7;
            break;
        }
        case 0x9E:
        { /* RES 3,(HL) */
            uint8_t work8 = Z80opsImpl->peek8(REG_HL) & 0xF7;
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0x9F:
        { /* RES 3,A */
            regA &= 0xF7;
            break;
        }
        case 0xA0:
        { /* RES 4,B */
            REG_B &= 0xEF;
            break;
        }
        case 0xA1:
        { /* RES 4,C */
            REG_C &= 0xEF;
            break;
        }
        case 0xA2:
        { /* RES 4,D */
            REG_D &= 0xEF;
            break;
        }
        case 0xA3:
        { /* RES 4,E */
            REG_E &= 0xEF;
            break;
        }
        case 0xA4:
        { /* RES 4,H */
            REG_H &= 0xEF;
            break;
        }
        case 0xA5:
        { /* RES 4,L */
            REG_L &= 0xEF;
            break;
        }
        case 0xA6:
        { /* RES 4,(HL) */
            uint8_t work8 = Z80opsImpl->peek8(REG_HL) & 0xEF;
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0xA7:
        { /* RES 4,A */
            regA &= 0xEF;
            break;
        }
        case 0xA8:
        { /* RES 5,B */
            REG_B &= 0xDF;
            break;
        }
        case 0xA9:
        { /* RES 5,C */
            REG_C &= 0xDF;
            break;
        }
        case 0xAA:
        { /* RES 5,D */
            REG_D &= 0xDF;
            break;
        }
        case 0xAB:
        { /* RES 5,E */
            REG_E &= 0xDF;
            break;
        }
        case 0xAC:
        { /* RES 5,H */
            REG_H &= 0xDF;
            break;
        }
        case 0xAD:
        { /* RES 5,L */
            REG_L &= 0xDF;
            break;
        }
        case 0xAE:
        { /* RES 5,(HL) */
            uint8_t work8 = Z80opsImpl->peek8(REG_HL) & 0xDF;
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0xAF:
        { /* RES 5,A */
            regA &= 0xDF;
            break;
        }
        case 0xB0:
        { /* RES 6,B */
            REG_B &= 0xBF;
            break;
        }
        case 0xB1:
        { /* RES 6,C */
            REG_C &= 0xBF;
            break;
        }
        case 0xB2:
        { /* RES 6,D */
            REG_D &= 0xBF;
            break;
        }
        case 0xB3:
        { /* RES 6,E */
            REG_E &= 0xBF;
            break;
        }
        case 0xB4:
        { /* RES 6,H */
            REG_H &= 0xBF;
            break;
        }
        case 0xB5:
        { /* RES 6,L */
            REG_L &= 0xBF;
            break;
        }
        case 0xB6:
        { /* RES 6,(HL) */
            uint8_t work8 = Z80opsImpl->peek8(REG_HL) & 0xBF;
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0xB7:
        { /* RES 6,A */
            regA &= 0xBF;
            break;
        }
        case 0xB8:
        { /* RES 7,B */
            REG_B &= 0x7F;
            break;
        }
        case 0xB9:
        { /* RES 7,C */
            REG_C &= 0x7F;
            break;
        }
        case 0xBA:
        { /* RES 7,D */
            REG_D &= 0x7F;
            break;
        }
        case 0xBB:
        { /* RES 7,E */
            REG_E &= 0x7F;
            break;
        }
        case 0xBC:
        { /* RES 7,H */
            REG_H &= 0x7F;
            break;
        }
        case 0xBD:
        { /* RES 7,L */
            REG_L &= 0x7F;
            break;
        }
        case 0xBE:
        { /* RES 7,(HL) */
            uint8_t work8 = Z80opsImpl->peek8(REG_HL) & 0x7F;
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0xBF:
        { /* RES 7,A */
            regA &= 0x7F;
            break;
        }
        case 0xC0:
        { /* SET 0,B */
            REG_B |= 0x01;
            break;
        }
        case 0xC1:
        { /* SET 0,C */
            REG_C |= 0x01;
            break;
        }
        case 0xC2:
        { /* SET 0,D */
            REG_D |= 0x01;
            break;
        }
        case 0xC3:
        { /* SET 0,E */
            REG_E |= 0x01;
            break;
        }
        case 0xC4:
        { /* SET 0,H */
            REG_H |= 0x01;
            break;
        }
        case 0xC5:
        { /* SET 0,L */
            REG_L |= 0x01;
            break;
        }
        case 0xC6:
        { /* SET 0,(HL) */
            uint8_t work8 = Z80opsImpl->peek8(REG_HL) | 0x01;
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0xC7:
        { /* SET 0,A */
            regA |= 0x01;
            break;
        }
        case 0xC8:
        { /* SET 1,B */
            REG_B |= 0x02;
            break;
        }
        case 0xC9:
        { /* SET 1,C */
            REG_C |= 0x02;
            break;
        }
        case 0xCA:
        { /* SET 1,D */
            REG_D |= 0x02;
            break;
        }
        case 0xCB:
        { /* SET 1,E */
            REG_E |= 0x02;
            break;
        }
        case 0xCC:
        { /* SET 1,H */
            REG_H |= 0x02;
            break;
        }
        case 0xCD:
        { /* SET 1,L */
            REG_L |= 0x02;
            break;
        }
        case 0xCE:
        { /* SET 1,(HL) */
            uint8_t work8 = Z80opsImpl->peek8(REG_HL) | 0x02;
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0xCF:
        { /* SET 1,A */
            regA |= 0x02;
            break;
        }
        case 0xD0:
        { /* SET 2,B */
            REG_B |= 0x04;
            break;
        }
        case 0xD1:
        { /* SET 2,C */
            REG_C |= 0x04;
            break;
        }
        case 0xD2:
        { /* SET 2,D */
            REG_D |= 0x04;
            break;
        }
        case 0xD3:
        { /* SET 2,E */
            REG_E |= 0x04;
            break;
        }
        case 0xD4:
        { /* SET 2,H */
            REG_H |= 0x04;
            break;
        }
        case 0xD5:
        { /* SET 2,L */
            REG_L |= 0x04;
            break;
        }
        case 0xD6:
        { /* SET 2,(HL) */
            uint8_t work8 = Z80opsImpl->peek8(REG_HL) | 0x04;
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0xD7:
        { /* SET 2,A */
            regA |= 0x04;
            break;
        }
        case 0xD8:
        { /* SET 3,B */
            REG_B |= 0x08;
            break;
        }
        case 0xD9:
        { /* SET 3,C */
            REG_C |= 0x08;
            break;
        }
        case 0xDA:
        { /* SET 3,D */
            REG_D |= 0x08;
            break;
        }
        case 0xDB:
        { /* SET 3,E */
            REG_E |= 0x08;
            break;
        }
        case 0xDC:
        { /* SET 3,H */
            REG_H |= 0x08;
            break;
        }
        case 0xDD:
        { /* SET 3,L */
            REG_L |= 0x08;
            break;
        }
        case 0xDE:
        { /* SET 3,(HL) */
            uint8_t work8 = Z80opsImpl->peek8(REG_HL) | 0x08;
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0xDF:
        { /* SET 3,A */
            regA |= 0x08;
            break;
        }
        case 0xE0:
        { /* SET 4,B */
            REG_B |= 0x10;
            break;
        }
        case 0xE1:
        { /* SET 4,C */
            REG_C |= 0x10;
            break;
        }
        case 0xE2:
        { /* SET 4,D */
            REG_D |= 0x10;
            break;
        }
        case 0xE3:
        { /* SET 4,E */
            REG_E |= 0x10;
            break;
        }
        case 0xE4:
        { /* SET 4,H */
            REG_H |= 0x10;
            break;
        }
        case 0xE5:
        { /* SET 4,L */
            REG_L |= 0x10;
            break;
        }
        case 0xE6:
        { /* SET 4,(HL) */
            uint8_t work8 = Z80opsImpl->peek8(REG_HL) | 0x10;
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0xE7:
        { /* SET 4,A */
            regA |= 0x10;
            break;
        }
        case 0xE8:
        { /* SET 5,B */
            REG_B |= 0x20;
            break;
        }
        case 0xE9:
        { /* SET 5,C */
            REG_C |= 0x20;
            break;
        }
        case 0xEA:
        { /* SET 5,D */
            REG_D |= 0x20;
            break;
        }
        case 0xEB:
        { /* SET 5,E */
            REG_E |= 0x20;
            break;
        }
        case 0xEC:
        { /* SET 5,H */
            REG_H |= 0x20;
            break;
        }
        case 0xED:
        { /* SET 5,L */
            REG_L |= 0x20;
            break;
        }
        case 0xEE:
        { /* SET 5,(HL) */
            uint8_t work8 = Z80opsImpl->peek8(REG_HL) | 0x20;
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0xEF:
        { /* SET 5,A */
            regA |= 0x20;
            break;
        }
        case 0xF0:
        { /* SET 6,B */
            REG_B |= 0x40;
            break;
        }
        case 0xF1:
        { /* SET 6,C */
            REG_C |= 0x40;
            break;
        }
        case 0xF2:
        { /* SET 6,D */
            REG_D |= 0x40;
            break;
        }
        case 0xF3:
        { /* SET 6,E */
            REG_E |= 0x40;
            break;
        }
        case 0xF4:
        { /* SET 6,H */
            REG_H |= 0x40;
            break;
        }
        case 0xF5:
        { /* SET 6,L */
            REG_L |= 0x40;
            break;
        }
        case 0xF6:
        { /* SET 6,(HL) */
            uint8_t work8 = Z80opsImpl->peek8(REG_HL) | 0x40;
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0xF7:
        { /* SET 6,A */
            regA |= 0x40;
            break;
        }
        case 0xF8:
        { /* SET 7,B */
            REG_B |= 0x80;
            break;
        }
        case 0xF9:
        { /* SET 7,C */
            REG_C |= 0x80;
            break;
        }
        case 0xFA:
        { /* SET 7,D */
            REG_D |= 0x80;
            break;
        }
        case 0xFB:
        { /* SET 7,E */
            REG_E |= 0x80;
            break;
        }
        case 0xFC:
        { /* SET 7,H */
            REG_H |= 0x80;
            break;
        }
        case 0xFD:
        { /* SET 7,L */
            REG_L |= 0x80;
            break;
        }
        case 0xFE:
        { /* SET 7,(HL) */
            uint8_t work8 = Z80opsImpl->peek8(REG_HL) | 0x80;
            Z80opsImpl->addressOnBus(REG_HL, 1);
            Z80opsImpl->poke8(REG_HL, work8);
            break;
        }
        case 0xFF:
        { /* SET 7,A */
            regA |= 0x80;
            break;
        }
        default:
        {
            break;
        }
    }
}

//Subconjunto de instrucciones 0xDD / 0xFD
/*
 * Hay que tener en cuenta el manejo de secuencias códigos DD/FD que no
 * hacen nada. Según el apartado 3.7 del documento
 * [http://www.myquest.nl/z80undocumented/z80-documented-v0.91.pdf]
 * secuencias de códigos como FD DD 00 21 00 10 NOP NOP NOP LD HL,1000h
 * activan IY con el primer FD, IX con el segundo DD y vuelven al
 * registro HL con el código NOP. Es decir, si detrás del código DD/FD no
 * viene una instrucción que maneje el registro HL, el código DD/FD
 * "se olvida" y hay que procesar la instrucción como si nunca se
 * hubiera visto el prefijo (salvo por los 4 t-estados que ha costado).
 * Naturalmente, en una serie repetida de DDFD no hay que comprobar las
 * interrupciones entre cada prefijo.
 */
RegisterPair Z80::decodeDDFD(RegisterPair tmpIXY) {
    regR++;
    opCode = Z80opsImpl->fetchOpcode(regPC);
    regPC++;

    switch (opCode) {
        case 0x09:
        { /* ADD IX,BC */
            Z80opsImpl->addressOnBus(getPairIR(), 7);
            tmpIXY.word = add16(tmpIXY.word, REG_BC);
            break;
        }
        case 0x19:
        { /* ADD IX,DE */
            Z80opsImpl->addressOnBus(getPairIR(), 7);
            tmpIXY.word = add16(tmpIXY.word, REG_DE);
            break;
        }
        case 0x21:
        { /* LD IX,nn */
            tmpIXY.word = Z80opsImpl->peek16(regPC);
            regPC = regPC + 2;
            break;
        }
        case 0x22:
        { /* LD (nn),IX */
            memptr = Z80opsImpl->peek16(regPC);
            Z80opsImpl->poke16(memptr++, tmpIXY.word);
            regPC = regPC + 2;
            break;
        }
        case 0x23:
        { /* INC IX */
            Z80opsImpl->addressOnBus(getPairIR(), 2);
            tmpIXY.word++;
            break;
        }
        case 0x24:
        { /* INC IXh */
            tmpIXY.byte8.hi = inc8(tmpIXY.byte8.hi);
            break;
        }
        case 0x25:
        { /* DEC IXh */
            tmpIXY.byte8.hi = dec8(tmpIXY.byte8.hi);
            break;
        }
        case 0x26:
        { /* LD IXh,n */
            tmpIXY.byte8.hi = Z80opsImpl->peek8(regPC);
            regPC++;
            break;
        }
        case 0x29:
        { /* ADD IX,IX */
            Z80opsImpl->addressOnBus(getPairIR(), 7);
            tmpIXY.word = add16(tmpIXY.word, tmpIXY.word);
            break;
        }
        case 0x2A:
        { /* LD IX,(nn) */
            memptr = Z80opsImpl->peek16(regPC);
            tmpIXY.word = Z80opsImpl->peek16(memptr++);
            regPC = regPC + 2;
            break;
        }
        case 0x2B:
        { /* DEC IX */
            Z80opsImpl->addressOnBus(getPairIR(), 2);
            tmpIXY.word--;
            break;
        }
        case 0x2C:
        { /* INC IXl */
            tmpIXY.byte8.lo = inc8(tmpIXY.byte8.lo);
            break;
        }
        case 0x2D:
        { /* DEC IXl */
            tmpIXY.byte8.lo = dec8(tmpIXY.byte8.lo);
            break;
        }
        case 0x2E:
        { /* LD IXl,n */
            tmpIXY.byte8.lo = Z80opsImpl->peek8(regPC);
            regPC++;
            break;
        }
        case 0x34:
        { /* INC (IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            uint8_t work8 = Z80opsImpl->peek8(memptr);
            Z80opsImpl->addressOnBus(memptr, 1);
            Z80opsImpl->poke8(memptr, inc8(work8));
            break;
        }
        case 0x35:
        { /* DEC (IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            uint8_t work8 = Z80opsImpl->peek8(memptr);
            Z80opsImpl->addressOnBus(memptr, 1);
            Z80opsImpl->poke8(memptr, dec8(work8));
            break;
        }
        case 0x36:
        { /* LD (IX+d),n */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            regPC++;
            uint8_t work8 = Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 2);
            regPC++;
            Z80opsImpl->poke8(memptr, work8);
            break;
        }
        case 0x39:
        { /* ADD IX,SP */
            Z80opsImpl->addressOnBus(getPairIR(), 7);
            tmpIXY.word = add16(tmpIXY.word, regSP);
            break;
        }
        case 0x44:
        { /* LD B,IXh */
            REG_B = tmpIXY.byte8.hi;
            break;
        }
        case 0x45:
        { /* LD B,IXl */
            REG_B = tmpIXY.byte8.lo;
            break;
        }
        case 0x46:
        { /* LD B,(IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            REG_B = Z80opsImpl->peek8(memptr);
            break;
        }
        case 0x4C:
        { /* LD C,IXh */
            REG_C = tmpIXY.byte8.hi;
            break;
        }
        case 0x4D:
        { /* LD C,IXl */
            REG_C = tmpIXY.byte8.lo;
            break;
        }
        case 0x4E:
        { /* LD C,(IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            REG_C = Z80opsImpl->peek8(memptr);
            break;
        }
        case 0x54:
        { /* LD D,IXh */
            REG_D = tmpIXY.byte8.hi;
            break;
        }
        case 0x55:
        { /* LD D,IXl */
            REG_D = tmpIXY.byte8.lo;
            break;
        }
        case 0x56:
        { /* LD D,(IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            REG_D = Z80opsImpl->peek8(memptr);
            break;
        }
        case 0x5C:
        { /* LD E,IXh */
            REG_E = tmpIXY.byte8.hi;
            break;
        }
        case 0x5D:
        { /* LD E,IXl */
            REG_E = tmpIXY.byte8.lo;
            break;
        }
        case 0x5E:
        { /* LD E,(IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            REG_E = Z80opsImpl->peek8(memptr);
            break;
        }
        case 0x60:
        { /* LD IXh,B */
            tmpIXY.byte8.hi = REG_B;
            break;
        }
        case 0x61:
        { /* LD IXh,C */
            tmpIXY.byte8.hi = REG_C;
            break;
        }
        case 0x62:
        { /* LD IXh,D */
            tmpIXY.byte8.hi = REG_D;
            break;
        }
        case 0x63:
        { /* LD IXh,E */
            tmpIXY.byte8.hi = REG_E;
            break;
        }
        case 0x64:
        { /* LD IXh,IXh */
            break;
        }
        case 0x65:
        { /* LD IXh,IXl */
            tmpIXY.byte8.hi = tmpIXY.byte8.lo;
            break;
        }
        case 0x66:
        { /* LD H,(IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            REG_H = Z80opsImpl->peek8(memptr);
            break;
        }
        case 0x67:
        { /* LD IXh,A */
            tmpIXY.byte8.hi = regA;
            break;
        }
        case 0x68:
        { /* LD IXl,B */
            tmpIXY.byte8.lo = REG_B;
            break;
        }
        case 0x69:
        { /* LD IXl,C */
            tmpIXY.byte8.lo = REG_C;
            break;
        }
        case 0x6A:
        { /* LD IXl,D */
            tmpIXY.byte8.lo = REG_D;
            break;
        }
        case 0x6B:
        { /* LD IXl,E */
            tmpIXY.byte8.lo = REG_E;
            break;
        }
        case 0x6C:
        { /* LD IXl,IXh */
            tmpIXY.byte8.lo = tmpIXY.byte8.hi;
            break;
        }
        case 0x6D:
        { /* LD IXl,IXl */
            break;
        }
        case 0x6E:
        { /* LD L,(IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            REG_L = Z80opsImpl->peek8(memptr);
            break;
        }
        case 0x6F:
        { /* LD IXl,A */
            tmpIXY.byte8.lo = regA;
            break;
        }
        case 0x70:
        { /* LD (IX+d),B */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            Z80opsImpl->poke8(memptr, REG_B);
            break;
        }
        case 0x71:
        { /* LD (IX+d),C */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            Z80opsImpl->poke8(memptr, REG_C);
            break;
        }
        case 0x72:
        { /* LD (IX+d),D */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            Z80opsImpl->poke8(memptr, REG_D);
            break;
        }
        case 0x73:
        { /* LD (IX+d),E */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            Z80opsImpl->poke8(memptr, REG_E);
            break;
        }
        case 0x74:
        { /* LD (IX+d),H */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            Z80opsImpl->poke8(memptr, REG_H);
            break;
        }
        case 0x75:
        { /* LD (IX+d),L */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            Z80opsImpl->poke8(memptr, REG_L);
            break;
        }
        case 0x77:
        { /* LD (IX+d),A */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            Z80opsImpl->poke8(memptr, regA);
            break;
        }
        case 0x7C:
        { /* LD A,IXh */
            regA = tmpIXY.byte8.hi;
            break;
        }
        case 0x7D:
        { /* LD A,IXl */
            regA = tmpIXY.byte8.lo;
            break;
        }
        case 0x7E:
        { /* LD A,(IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            regA = Z80opsImpl->peek8(memptr);
            break;
        }
        case 0x84:
        { /* ADD A,IXh */
            add(tmpIXY.byte8.hi);
            break;
        }
        case 0x85:
        { /* ADD A,IXl */
            add(tmpIXY.byte8.lo);
            break;
        }
        case 0x86:
        { /* ADD A,(IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            add(Z80opsImpl->peek8(memptr));
            break;
        }
        case 0x8C:
        { /* ADC A,IXh */
            adc(tmpIXY.byte8.hi);
            break;
        }
        case 0x8D:
        { /* ADC A,IXl */
            adc(tmpIXY.byte8.lo);
            break;
        }
        case 0x8E:
        { /* ADC A,(IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            adc(Z80opsImpl->peek8(memptr));
            break;
        }
        case 0x94:
        { /* SUB IXh */
            sub(tmpIXY.byte8.hi);
            break;
        }
        case 0x95:
        { /* SUB IXl */
            sub(tmpIXY.byte8.lo);
            break;
        }
        case 0x96:
        { /* SUB (IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            sub(Z80opsImpl->peek8(memptr));
            break;
        }
        case 0x9C:
        { /* SBC A,IXh */
            sbc(tmpIXY.byte8.hi);
            break;
        }
        case 0x9D:
        { /* SBC A,IXl */
            sbc(tmpIXY.byte8.lo);
            break;
        }
        case 0x9E:
        { /* SBC A,(IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            sbc(Z80opsImpl->peek8(memptr));
            break;
        }
        case 0xA4:
        { /* AND IXh */
            and_(tmpIXY.byte8.hi);
            break;
        }
        case 0xA5:
        { /* AND IXl */
            and_(tmpIXY.byte8.lo);
            break;
        }
        case 0xA6:
        { /* AND (IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            and_(Z80opsImpl->peek8(memptr));
            break;
        }
        case 0xAC:
        { /* XOR IXh */
            xor_(tmpIXY.byte8.hi);
            break;
        }
        case 0xAD:
        { /* XOR IXl */
            xor_(tmpIXY.byte8.lo);
            break;
        }
        case 0xAE:
        { /* XOR (IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            xor_(Z80opsImpl->peek8(memptr));
            break;
        }
        case 0xB4:
        { /* OR IXh */
            or_(tmpIXY.byte8.hi);
            break;
        }
        case 0xB5:
        { /* OR IXl */
            or_(tmpIXY.byte8.lo);
            break;
        }
        case 0xB6:
        { /* OR (IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            or_(Z80opsImpl->peek8(memptr));
            break;
        }
        case 0xBC:
        { /* CP IXh */
            cp(tmpIXY.byte8.hi);
            break;
        }
        case 0xBD:
        { /* CP IXl */
            cp(tmpIXY.byte8.lo);
            break;
        }
        case 0xBE:
        { /* CP (IX+d) */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 5);
            regPC++;
            cp(Z80opsImpl->peek8(memptr));
            break;
        }
        case 0xCB:
        { /* Subconjunto de instrucciones */
            memptr = tmpIXY.word + (int8_t) Z80opsImpl->peek8(regPC);
            regPC++;
            opCode = Z80opsImpl->peek8(regPC);
            Z80opsImpl->addressOnBus(regPC, 2);
            regPC++;
            decodeDDFDCB(opCode, memptr);
            break;
        }
        case 0xE1:
        { /* POP IX */
            tmpIXY.word = pop();
            break;
        }
        case 0xE3:
        { /* EX (SP),IX */
            // Instrucción de ejecución sutil como pocas... atento al dato.
            RegisterPair work16 = tmpIXY;
            tmpIXY.word = Z80opsImpl->peek16(regSP);
            Z80opsImpl->addressOnBus(regSP + 1, 1);
            // I can't call to poke16 from here because the Z80 do the writes in inverted order
            // Same for EX (SP), HL
            Z80opsImpl->poke8(regSP + 1, work16.byte8.hi);
            Z80opsImpl->poke8(regSP, work16.byte8.lo);
            Z80opsImpl->addressOnBus(regSP, 2);
            memptr = tmpIXY.word;
            break;
        }
        case 0xE5:
        { /* PUSH IX */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            push(tmpIXY.word);
            break;
        }
        case 0xE9:
        { /* JP (IX) */
            regPC = tmpIXY.word;
            break;
        }
        case 0xF9:
        { /* LD SP,IX */
            Z80opsImpl->addressOnBus(getPairIR(), 2);
            regSP = tmpIXY.word;
            break;
        }
        default:
        {
            // Detrás de un DD/FD o varios en secuencia venía un código
            // que no correspondía con una instrucción que involucra a
            // IX o IY. Se trata como si fuera un código normal.
            // Sin esto, además de emular mal, falla el test
            // ld <bcdexya>,<bcdexya> de ZEXALL.

            if (breakpointAt[regPC]) {
                opCode = Z80opsImpl->breakpoint(regPC, opCode);
            }

            decodeOpcode(opCode);
            break;
        }
    }
    return tmpIXY;
}

// Subconjunto de instrucciones 0xDDCB
void Z80::decodeDDFDCB(uint8_t opCode, uint16_t address) {

    switch (opCode) {
        case 0x00:
        { /* RLC (IX+d),B */
            REG_B = rlc(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0x01:
        { /* RLC (IX+d),C */
            REG_C = rlc(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0x02:
        { /* RLC (IX+d),D */
            REG_D = rlc(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0x03:
        { /* RLC (IX+d),E */
            REG_E = rlc(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0x04:
        { /* RLC (IX+d),H */
            REG_H = rlc(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0x05:
        { /* RLC (IX+d),L */
            REG_L = rlc(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0x06:
        { /* RLC (IX+d) */
            uint8_t work8 = rlc(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0x07:
        { /* RLC (IX+d),A */
            regA = rlc(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0x08:
        { /* RRC (IX+d),B */
            REG_B = rrc(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0x09:
        { /* RRC (IX+d),C */
            REG_C = rrc(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0x0A:
        { /* RRC (IX+d),D */
            REG_D = rrc(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0x0B:
        { /* RRC (IX+d),E */
            REG_E = rrc(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0x0C:
        { /* RRC (IX+d),H */
            REG_H = rrc(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0x0D:
        { /* RRC (IX+d),L */
            REG_L = rrc(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0x0E:
        { /* RRC (IX+d) */
            uint8_t work8 = rrc(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0x0F:
        { /* RRC (IX+d),A */
            regA = rrc(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0x10:
        { /* RL (IX+d),B */
            REG_B = rl(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0x11:
        { /* RL (IX+d),C */
            REG_C = rl(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0x12:
        { /* RL (IX+d),D */
            REG_D = rl(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0x13:
        { /* RL (IX+d),E */
            REG_E = rl(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0x14:
        { /* RL (IX+d),H */
            REG_H = rl(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0x15:
        { /* RL (IX+d),L */
            REG_L = rl(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0x16:
        { /* RL (IX+d) */
            uint8_t work8 = rl(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0x17:
        { /* RL (IX+d),A */
            regA = rl(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0x18:
        { /* RR (IX+d),B */
            REG_B = rr(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0x19:
        { /* RR (IX+d),C */
            REG_C = rr(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0x1A:
        { /* RR (IX+d),D */
            REG_D = rr(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0x1B:
        { /* RR (IX+d),E */
            REG_E = rr(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0x1C:
        { /* RR (IX+d),H */
            REG_H = rr(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0x1D:
        { /* RR (IX+d),L */
            REG_L = rr(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0x1E:
        { /* RR (IX+d) */
            uint8_t work8 = rr(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0x1F:
        { /* RR (IX+d),A */
            regA = rr(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0x20:
        { /* SLA (IX+d),B */
            REG_B = sla(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0x21:
        { /* SLA (IX+d),C */
            REG_C = sla(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0x22:
        { /* SLA (IX+d),D */
            REG_D = sla(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0x23:
        { /* SLA (IX+d),E */
            REG_E = sla(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0x24:
        { /* SLA (IX+d),H */
            REG_H = sla(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0x25:
        { /* SLA (IX+d),L */
            REG_L = sla(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0x26:
        { /* SLA (IX+d) */
            uint8_t work8 = sla(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0x27:
        { /* SLA (IX+d),A */
            regA = sla(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0x28:
        { /* SRA (IX+d),B */
            REG_B = sra(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0x29:
        { /* SRA (IX+d),C */
            REG_C = sra(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0x2A:
        { /* SRA (IX+d),D */
            REG_D = sra(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0x2B:
        { /* SRA (IX+d),E */
            REG_E = sra(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0x2C:
        { /* SRA (IX+d),H */
            REG_H = sra(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0x2D:
        { /* SRA (IX+d),L */
            REG_L = sra(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0x2E:
        { /* SRA (IX+d) */
            uint8_t work8 = sra(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0x2F:
        { /* SRA (IX+d),A */
            regA = sra(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0x30:
        { /* SLL (IX+d),B */
            REG_B = sll(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0x31:
        { /* SLL (IX+d),C */
            REG_C = sll(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0x32:
        { /* SLL (IX+d),D */
            REG_D = sll(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0x33:
        { /* SLL (IX+d),E */
            REG_E = sll(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0x34:
        { /* SLL (IX+d),H */
            REG_H = sll(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0x35:
        { /* SLL (IX+d),L */
            REG_L = sll(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0x36:
        { /* SLL (IX+d) */
            uint8_t work8 = sll(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0x37:
        { /* SLL (IX+d),A */
            regA = sll(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0x38:
        { /* SRL (IX+d),B */
            REG_B = srl(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0x39:
        { /* SRL (IX+d),C */
            REG_C = srl(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0x3A:
        { /* SRL (IX+d),D */
            REG_D = srl(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0x3B:
        { /* SRL (IX+d),E */
            REG_E = srl(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0x3C:
        { /* SRL (IX+d),H */
            REG_H = srl(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0x3D:
        { /* SRL (IX+d),L */
            REG_L = srl(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0x3E:
        { /* SRL (IX+d) */
            uint8_t work8 = srl(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0x3F:
        { /* SRL (IX+d),A */
            regA = srl(Z80opsImpl->peek8(address));
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        { /* BIT 0,(IX+d) */
            bit(0x01, Z80opsImpl->peek8(address));
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHP_MASK)
                    | ((address >> 8) & FLAG_53_MASK);
            Z80opsImpl->addressOnBus(address, 1);
            break;
        }
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:
        { /* BIT 1,(IX+d) */
            bit(0x02, Z80opsImpl->peek8(address));
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHP_MASK)
                    | ((address >> 8) & FLAG_53_MASK);
            Z80opsImpl->addressOnBus(address, 1);
            break;
        }
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:
        { /* BIT 2,(IX+d) */
            bit(0x04, Z80opsImpl->peek8(address));
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHP_MASK)
                    | ((address >> 8) & FLAG_53_MASK);
            Z80opsImpl->addressOnBus(address, 1);
            break;
        }
        case 0x58:
        case 0x59:
        case 0x5A:
        case 0x5B:
        case 0x5C:
        case 0x5D:
        case 0x5E:
        case 0x5F:
        { /* BIT 3,(IX+d) */
            bit(0x08, Z80opsImpl->peek8(address));
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHP_MASK)
                    | ((address >> 8) & FLAG_53_MASK);
            Z80opsImpl->addressOnBus(address, 1);
            break;
        }
        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x66:
        case 0x67:
        { /* BIT 4,(IX+d) */
            bit(0x10, Z80opsImpl->peek8(address));
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHP_MASK)
                    | ((address >> 8) & FLAG_53_MASK);
            Z80opsImpl->addressOnBus(address, 1);
            break;
        }
        case 0x68:
        case 0x69:
        case 0x6A:
        case 0x6B:
        case 0x6C:
        case 0x6D:
        case 0x6E:
        case 0x6F:
        { /* BIT 5,(IX+d) */
            bit(0x20, Z80opsImpl->peek8(address));
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHP_MASK)
                    | ((address >> 8) & FLAG_53_MASK);
            Z80opsImpl->addressOnBus(address, 1);
            break;
        }
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
        { /* BIT 6,(IX+d) */
            bit(0x40, Z80opsImpl->peek8(address));
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHP_MASK)
                    | ((address >> 8) & FLAG_53_MASK);
            Z80opsImpl->addressOnBus(address, 1);
            break;
        }
        case 0x78:
        case 0x79:
        case 0x7A:
        case 0x7B:
        case 0x7C:
        case 0x7D:
        case 0x7E:
        case 0x7F:
        { /* BIT 7,(IX+d) */
            bit(0x80, Z80opsImpl->peek8(address));
            sz5h3pnFlags = (sz5h3pnFlags & FLAG_SZHP_MASK)
                    | ((address >> 8) & FLAG_53_MASK);
            Z80opsImpl->addressOnBus(address, 1);
            break;
        }
        case 0x80:
        { /* RES 0,(IX+d),B */
            REG_B = Z80opsImpl->peek8(address) & 0xFE;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0x81:
        { /* RES 0,(IX+d),C */
            REG_C = Z80opsImpl->peek8(address) & 0xFE;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0x82:
        { /* RES 0,(IX+d),D */
            REG_D = Z80opsImpl->peek8(address) & 0xFE;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0x83:
        { /* RES 0,(IX+d),E */
            REG_E = Z80opsImpl->peek8(address) & 0xFE;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0x84:
        { /* RES 0,(IX+d),H */
            REG_H = Z80opsImpl->peek8(address) & 0xFE;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0x85:
        { /* RES 0,(IX+d),L */
            REG_L = Z80opsImpl->peek8(address) & 0xFE;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0x86:
        { /* RES 0,(IX+d) */
            uint8_t work8 = Z80opsImpl->peek8(address) & 0xFE;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0x87:
        { /* RES 0,(IX+d),A */
            regA = Z80opsImpl->peek8(address) & 0xFE;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0x88:
        { /* RES 1,(IX+d),B */
            REG_B = Z80opsImpl->peek8(address) & 0xFD;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0x89:
        { /* RES 1,(IX+d),C */
            REG_C = Z80opsImpl->peek8(address) & 0xFD;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0x8A:
        { /* RES 1,(IX+d),D */
            REG_D = Z80opsImpl->peek8(address) & 0xFD;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0x8B:
        { /* RES 1,(IX+d),E */
            REG_E = Z80opsImpl->peek8(address) & 0xFD;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0x8C:
        { /* RES 1,(IX+d),H */
            REG_H = Z80opsImpl->peek8(address) & 0xFD;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0x8D:
        { /* RES 1,(IX+d),L */
            REG_L = Z80opsImpl->peek8(address) & 0xFD;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0x8E:
        { /* RES 1,(IX+d) */
            uint8_t work8 = Z80opsImpl->peek8(address) & 0xFD;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0x8F:
        { /* RES 1,(IX+d),A */
            regA = Z80opsImpl->peek8(address) & 0xFD;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0x90:
        { /* RES 2,(IX+d),B */
            REG_B = Z80opsImpl->peek8(address) & 0xFB;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0x91:
        { /* RES 2,(IX+d),C */
            REG_C = Z80opsImpl->peek8(address) & 0xFB;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0x92:
        { /* RES 2,(IX+d),D */
            REG_D = Z80opsImpl->peek8(address) & 0xFB;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0x93:
        { /* RES 2,(IX+d),E */
            REG_E = Z80opsImpl->peek8(address) & 0xFB;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0x94:
        { /* RES 2,(IX+d),H */
            REG_H = Z80opsImpl->peek8(address) & 0xFB;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0x95:
        { /* RES 2,(IX+d),L */
            REG_L = Z80opsImpl->peek8(address) & 0xFB;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0x96:
        { /* RES 2,(IX+d) */
            uint8_t work8 = Z80opsImpl->peek8(address) & 0xFB;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0x97:
        { /* RES 2,(IX+d),A */
            regA = Z80opsImpl->peek8(address) & 0xFB;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0x98:
        { /* RES 3,(IX+d),B */
            REG_B = Z80opsImpl->peek8(address) & 0xF7;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0x99:
        { /* RES 3,(IX+d),C */
            REG_C = Z80opsImpl->peek8(address) & 0xF7;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0x9A:
        { /* RES 3,(IX+d),D */
            REG_D = Z80opsImpl->peek8(address) & 0xF7;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0x9B:
        { /* RES 3,(IX+d),E */
            REG_E = Z80opsImpl->peek8(address) & 0xF7;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0x9C:
        { /* RES 3,(IX+d),H */
            REG_H = Z80opsImpl->peek8(address) & 0xF7;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0x9D:
        { /* RES 3,(IX+d),L */
            REG_L = Z80opsImpl->peek8(address) & 0xF7;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0x9E:
        { /* RES 3,(IX+d) */
            uint8_t work8 = Z80opsImpl->peek8(address) & 0xF7;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0x9F:
        { /* RES 3,(IX+d),A */
            regA = Z80opsImpl->peek8(address) & 0xF7;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0xA0:
        { /* RES 4,(IX+d),B */
            REG_B = Z80opsImpl->peek8(address) & 0xEF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0xA1:
        { /* RES 4,(IX+d),C */
            REG_C = Z80opsImpl->peek8(address) & 0xEF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0xA2:
        { /* RES 4,(IX+d),D */
            REG_D = Z80opsImpl->peek8(address) & 0xEF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0xA3:
        { /* RES 4,(IX+d),E */
            REG_E = Z80opsImpl->peek8(address) & 0xEF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0xA4:
        { /* RES 4,(IX+d),H */
            REG_H = Z80opsImpl->peek8(address) & 0xEF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0xA5:
        { /* RES 4,(IX+d),L */
            REG_L = Z80opsImpl->peek8(address) & 0xEF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0xA6:
        { /* RES 4,(IX+d) */
            uint8_t work8 = Z80opsImpl->peek8(address) & 0xEF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0xA7:
        { /* RES 4,(IX+d),A */
            regA = Z80opsImpl->peek8(address) & 0xEF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0xA8:
        { /* RES 5,(IX+d),B */
            REG_B = Z80opsImpl->peek8(address) & 0xDF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0xA9:
        { /* RES 5,(IX+d),C */
            REG_C = Z80opsImpl->peek8(address) & 0xDF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0xAA:
        { /* RES 5,(IX+d),D */
            REG_D = Z80opsImpl->peek8(address) & 0xDF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0xAB:
        { /* RES 5,(IX+d),E */
            REG_E = Z80opsImpl->peek8(address) & 0xDF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0xAC:
        { /* RES 5,(IX+d),H */
            REG_H = Z80opsImpl->peek8(address) & 0xDF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0xAD:
        { /* RES 5,(IX+d),L */
            REG_L = Z80opsImpl->peek8(address) & 0xDF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0xAE:
        { /* RES 5,(IX+d) */
            uint8_t work8 = Z80opsImpl->peek8(address) & 0xDF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0xAF:
        { /* RES 5,(IX+d),A */
            regA = Z80opsImpl->peek8(address) & 0xDF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0xB0:
        { /* RES 6,(IX+d),B */
            REG_B = Z80opsImpl->peek8(address) & 0xBF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0xB1:
        { /* RES 6,(IX+d),C */
            REG_C = Z80opsImpl->peek8(address) & 0xBF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0xB2:
        { /* RES 6,(IX+d),D */
            REG_D = Z80opsImpl->peek8(address) & 0xBF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0xB3:
        { /* RES 6,(IX+d),E */
            REG_E = Z80opsImpl->peek8(address) & 0xBF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0xB4:
        { /* RES 6,(IX+d),H */
            REG_H = Z80opsImpl->peek8(address) & 0xBF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0xB5:
        { /* RES 6,(IX+d),L */
            REG_L = Z80opsImpl->peek8(address) & 0xBF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0xB6:
        { /* RES 6,(IX+d) */
            uint8_t work8 = Z80opsImpl->peek8(address) & 0xBF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0xB7:
        { /* RES 6,(IX+d),A */
            regA = Z80opsImpl->peek8(address) & 0xBF;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0xB8:
        { /* RES 7,(IX+d),B */
            REG_B = Z80opsImpl->peek8(address) & 0x7F;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0xB9:
        { /* RES 7,(IX+d),C */
            REG_C = Z80opsImpl->peek8(address) & 0x7F;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0xBA:
        { /* RES 7,(IX+d),D */
            REG_D = Z80opsImpl->peek8(address) & 0x7F;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0xBB:
        { /* RES 7,(IX+d),E */
            REG_E = Z80opsImpl->peek8(address) & 0x7F;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0xBC:
        { /* RES 7,(IX+d),H */
            REG_H = Z80opsImpl->peek8(address) & 0x7F;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0xBD:
        { /* RES 7,(IX+d),L */
            REG_L = Z80opsImpl->peek8(address) & 0x7F;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0xBE:
        { /* RES 7,(IX+d) */
            uint8_t work8 = Z80opsImpl->peek8(address) & 0x7F;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0xBF:
        { /* RES 7,(IX+d),A */
            regA = Z80opsImpl->peek8(address) & 0x7F;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0xC0:
        { /* SET 0,(IX+d),B */
            REG_B = Z80opsImpl->peek8(address) | 0x01;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0xC1:
        { /* SET 0,(IX+d),C */
            REG_C = Z80opsImpl->peek8(address) | 0x01;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0xC2:
        { /* SET 0,(IX+d),D */
            REG_D = Z80opsImpl->peek8(address) | 0x01;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0xC3:
        { /* SET 0,(IX+d),E */
            REG_E = Z80opsImpl->peek8(address) | 0x01;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0xC4:
        { /* SET 0,(IX+d),H */
            REG_H = Z80opsImpl->peek8(address) | 0x01;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0xC5:
        { /* SET 0,(IX+d),L */
            REG_L = Z80opsImpl->peek8(address) | 0x01;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0xC6:
        { /* SET 0,(IX+d) */
            uint8_t work8 = Z80opsImpl->peek8(address) | 0x01;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0xC7:
        { /* SET 0,(IX+d),A */
            regA = Z80opsImpl->peek8(address) | 0x01;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0xC8:
        { /* SET 1,(IX+d),B */
            REG_B = Z80opsImpl->peek8(address) | 0x02;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0xC9:
        { /* SET 1,(IX+d),C */
            REG_C = Z80opsImpl->peek8(address) | 0x02;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0xCA:
        { /* SET 1,(IX+d),D */
            REG_D = Z80opsImpl->peek8(address) | 0x02;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0xCB:
        { /* SET 1,(IX+d),E */
            REG_E = Z80opsImpl->peek8(address) | 0x02;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0xCC:
        { /* SET 1,(IX+d),H */
            REG_H = Z80opsImpl->peek8(address) | 0x02;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0xCD:
        { /* SET 1,(IX+d),L */
            REG_L = Z80opsImpl->peek8(address) | 0x02;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0xCE:
        { /* SET 1,(IX+d) */
            uint8_t work8 = Z80opsImpl->peek8(address) | 0x02;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0xCF:
        { /* SET 1,(IX+d),A */
            regA = Z80opsImpl->peek8(address) | 0x02;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0xD0:
        { /* SET 2,(IX+d),B */
            REG_B = Z80opsImpl->peek8(address) | 0x04;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0xD1:
        { /* SET 2,(IX+d),C */
            REG_C = Z80opsImpl->peek8(address) | 0x04;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0xD2:
        { /* SET 2,(IX+d),D */
            REG_D = Z80opsImpl->peek8(address) | 0x04;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0xD3:
        { /* SET 2,(IX+d),E */
            REG_E = Z80opsImpl->peek8(address) | 0x04;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0xD4:
        { /* SET 2,(IX+d),H */
            REG_H = Z80opsImpl->peek8(address) | 0x04;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0xD5:
        { /* SET 2,(IX+d),L */
            REG_L = Z80opsImpl->peek8(address) | 0x04;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0xD6:
        { /* SET 2,(IX+d) */
            uint8_t work8 = Z80opsImpl->peek8(address) | 0x04;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0xD7:
        { /* SET 2,(IX+d),A */
            regA = Z80opsImpl->peek8(address) | 0x04;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0xD8:
        { /* SET 3,(IX+d),B */
            REG_B = Z80opsImpl->peek8(address) | 0x08;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0xD9:
        { /* SET 3,(IX+d),C */
            REG_C = Z80opsImpl->peek8(address) | 0x08;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0xDA:
        { /* SET 3,(IX+d),D */
            REG_D = Z80opsImpl->peek8(address) | 0x08;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0xDB:
        { /* SET 3,(IX+d),E */
            REG_E = Z80opsImpl->peek8(address) | 0x08;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0xDC:
        { /* SET 3,(IX+d),H */
            REG_H = Z80opsImpl->peek8(address) | 0x08;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0xDD:
        { /* SET 3,(IX+d),L */
            REG_L = Z80opsImpl->peek8(address) | 0x08;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0xDE:
        { /* SET 3,(IX+d) */
            uint8_t work8 = Z80opsImpl->peek8(address) | 0x08;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0xDF:
        { /* SET 3,(IX+d),A */
            regA = Z80opsImpl->peek8(address) | 0x08;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0xE0:
        { /* SET 4,(IX+d),B */
            REG_B = Z80opsImpl->peek8(address) | 0x10;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0xE1:
        { /* SET 4,(IX+d),C */
            REG_C = Z80opsImpl->peek8(address) | 0x10;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0xE2:
        { /* SET 4,(IX+d),D */
            REG_D = Z80opsImpl->peek8(address) | 0x10;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0xE3:
        { /* SET 4,(IX+d),E */
            REG_E = Z80opsImpl->peek8(address) | 0x10;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0xE4:
        { /* SET 4,(IX+d),H */
            REG_H = Z80opsImpl->peek8(address) | 0x10;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0xE5:
        { /* SET 4,(IX+d),L */
            REG_L = Z80opsImpl->peek8(address) | 0x10;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0xE6:
        { /* SET 4,(IX+d) */
            uint8_t work8 = Z80opsImpl->peek8(address) | 0x10;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0xE7:
        { /* SET 4,(IX+d),A */
            regA = Z80opsImpl->peek8(address) | 0x10;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0xE8:
        { /* SET 5,(IX+d),B */
            REG_B = Z80opsImpl->peek8(address) | 0x20;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0xE9:
        { /* SET 5,(IX+d),C */
            REG_C = Z80opsImpl->peek8(address) | 0x20;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0xEA:
        { /* SET 5,(IX+d),D */
            REG_D = Z80opsImpl->peek8(address) | 0x20;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0xEB:
        { /* SET 5,(IX+d),E */
            REG_E = Z80opsImpl->peek8(address) | 0x20;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0xEC:
        { /* SET 5,(IX+d),H */
            REG_H = Z80opsImpl->peek8(address) | 0x20;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0xED:
        { /* SET 5,(IX+d),L */
            REG_L = Z80opsImpl->peek8(address) | 0x20;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0xEE:
        { /* SET 5,(IX+d) */
            uint8_t work8 = Z80opsImpl->peek8(address) | 0x20;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0xEF:
        { /* SET 5,(IX+d),A */
            regA = Z80opsImpl->peek8(address) | 0x20;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0xF0:
        { /* SET 6,(IX+d),B */
            REG_B = Z80opsImpl->peek8(address) | 0x40;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0xF1:
        { /* SET 6,(IX+d),C */
            REG_C = Z80opsImpl->peek8(address) | 0x40;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0xF2:
        { /* SET 6,(IX+d),D */
            REG_D = Z80opsImpl->peek8(address) | 0x40;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0xF3:
        { /* SET 6,(IX+d),E */
            REG_E = Z80opsImpl->peek8(address) | 0x40;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0xF4:
        { /* SET 6,(IX+d),H */
            REG_H = Z80opsImpl->peek8(address) | 0x40;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0xF5:
        { /* SET 6,(IX+d),L */
            REG_L = Z80opsImpl->peek8(address) | 0x40;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0xF6:
        { /* SET 6,(IX+d) */
            uint8_t work8 = Z80opsImpl->peek8(address) | 0x40;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0xF7:
        { /* SET 6,(IX+d),A */
            regA = Z80opsImpl->peek8(address) | 0x40;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
        case 0xF8:
        { /* SET 7,(IX+d),B */
            REG_B = Z80opsImpl->peek8(address) | 0x80;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_B);
            break;
        }
        case 0xF9:
        { /* SET 7,(IX+d),C */
            REG_C = Z80opsImpl->peek8(address) | 0x80;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_C);
            break;
        }
        case 0xFA:
        { /* SET 7,(IX+d),D */
            REG_D = Z80opsImpl->peek8(address) | 0x80;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_D);
            break;
        }
        case 0xFB:
        { /* SET 7,(IX+d),E */
            REG_E = Z80opsImpl->peek8(address) | 0x80;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_E);
            break;
        }
        case 0xFC:
        { /* SET 7,(IX+d),H */
            REG_H = Z80opsImpl->peek8(address) | 0x80;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_H);
            break;
        }
        case 0xFD:
        { /* SET 7,(IX+d),L */
            REG_L = Z80opsImpl->peek8(address) | 0x80;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, REG_L);
            break;
        }
        case 0xFE:
        { /* SET 7,(IX+d) */
            uint8_t work8 = Z80opsImpl->peek8(address) | 0x80;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, work8);
            break;
        }
        case 0xFF:
        { /* SET 7,(IX+d),A */
            regA = Z80opsImpl->peek8(address) | 0x80;
            Z80opsImpl->addressOnBus(address, 1);
            Z80opsImpl->poke8(address, regA);
            break;
        }
    }
}

//Subconjunto de instrucciones 0xED

void Z80::decodeED(void) {
    regR++;
    opCode = Z80opsImpl->fetchOpcode(regPC);
    regPC++;

    switch (opCode) {
        case 0x40:
        { /* IN B,(C) */
            memptr = REG_BC;
            REG_B = Z80opsImpl->inPort(memptr);
            memptr++;
            sz5h3pnFlags = sz53pn_addTable[REG_B];
            flagQ = true;
            break;
        }
        case 0x41:
        { /* OUT (C),B */
            memptr = REG_BC;
            Z80opsImpl->outPort(memptr, REG_B);
            memptr++;
            break;
        }
        case 0x42:
        { /* SBC HL,BC */
            Z80opsImpl->addressOnBus(getPairIR(), 7);
            sbc16(REG_BC);
            break;
        }
        case 0x43:
        { /* LD (nn),BC */
            memptr = Z80opsImpl->peek16(regPC);
            Z80opsImpl->poke16(memptr, REG_BC);
            memptr++;
            regPC = regPC + 2;
            break;
        }
        case 0x44:
        case 0x4C:
        case 0x54:
        case 0x5C:
        case 0x64:
        case 0x6C:
        case 0x74:
        case 0x7C:
        { /* NEG */
            uint8_t aux = regA;
            regA = 0;
            carryFlag = false;
            sbc(aux);
            break;
        }
        case 0x45:
        case 0x4D: /* RETI */
        case 0x55:
        case 0x5D:
        case 0x65:
        case 0x6D:
        case 0x75:
        case 0x7D:
        { /* RETN */
            ffIFF1 = ffIFF2;
            regPC = memptr = pop();
            break;
        }
        case 0x46:
        case 0x4E:
        case 0x66:
        case 0x6E:
        { /* IM 0 */
            modeINT = IntMode::IM0;
            break;
        }
        case 0x47:
        { /* LD I,A */
            /*
             * El par IR se pone en el bus de direcciones *antes*
             * de poner A en el registro I. Detalle importante.
             */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            regI = regA;
            break;
        }
        case 0x48:
        { /* IN C,(C) */
            memptr = REG_BC;
            REG_C = Z80opsImpl->inPort(memptr);
            memptr++;
            sz5h3pnFlags = sz53pn_addTable[REG_C];
            flagQ = true;
            break;
        }
        case 0x49:
        { /* OUT (C),C */
            memptr = REG_BC;
            Z80opsImpl->outPort(memptr, REG_C);
            memptr++;
            break;
        }
        case 0x4A:
        { /* ADC HL,BC */
            Z80opsImpl->addressOnBus(getPairIR(), 7);
            adc16(REG_BC);
            break;
        }
        case 0x4B:
        { /* LD BC,(nn) */
            memptr = Z80opsImpl->peek16(regPC);
            REG_BC = Z80opsImpl->peek16(memptr);
            memptr++;
            regPC = regPC + 2;
            break;
        }
        case 0x4F:
        { /* LD R,A */
            /*
             * El par IR se pone en el bus de direcciones *antes*
             * de poner A en el registro R. Detalle importante.
             */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            setRegR(regA);
            break;
        }
        case 0x50:
        { /* IN D,(C) */
            memptr = REG_BC;
            REG_D = Z80opsImpl->inPort(memptr);
            memptr++;
            sz5h3pnFlags = sz53pn_addTable[REG_D];
            flagQ = true;
            break;
        }
        case 0x51:
        { /* OUT (C),D */
            memptr = REG_BC;
            Z80opsImpl->outPort(memptr++, REG_D);
            break;
        }
        case 0x52:
        { /* SBC HL,DE */
            Z80opsImpl->addressOnBus(getPairIR(), 7);
            sbc16(REG_DE);
            break;
        }
        case 0x53:
        { /* LD (nn),DE */
            memptr = Z80opsImpl->peek16(regPC);
            Z80opsImpl->poke16(memptr++, REG_DE);
            regPC = regPC + 2;
            break;
        }
        case 0x56:
        case 0x76:
        { /* IM 1 */
            modeINT = IntMode::IM1;
            break;
        }
        case 0x57:
        { /* LD A,I */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            regA = regI;
            sz5h3pnFlags = sz53n_addTable[regA];
            if (ffIFF2) {
                sz5h3pnFlags |= PARITY_MASK;
            }
            flagQ = true;
            break;
        }
        case 0x58:
        { /* IN E,(C) */
            memptr = REG_BC;
            REG_E = Z80opsImpl->inPort(memptr++);
            sz5h3pnFlags = sz53pn_addTable[REG_E];
            flagQ = true;
            break;
        }
        case 0x59:
        { /* OUT (C),E */
            memptr = REG_BC;
            Z80opsImpl->outPort(memptr++, REG_E);
            break;
        }
        case 0x5A:
        { /* ADC HL,DE */
            Z80opsImpl->addressOnBus(getPairIR(), 7);
            adc16(REG_DE);
            break;
        }
        case 0x5B:
        { /* LD DE,(nn) */
            memptr = Z80opsImpl->peek16(regPC);
            REG_DE = Z80opsImpl->peek16(memptr++);
            regPC = regPC + 2;
            break;
        }
        case 0x5E:
        case 0x7E:
        { /* IM 2 */
            modeINT = IntMode::IM2;
            break;
        }
        case 0x5F:
        { /* LD A,R */
            Z80opsImpl->addressOnBus(getPairIR(), 1);
            regA = getRegR();
            sz5h3pnFlags = sz53n_addTable[regA];
            if (ffIFF2) {
                sz5h3pnFlags |= PARITY_MASK;
            }
            flagQ = true;
            break;
        }
        case 0x60:
        { /* IN H,(C) */
            memptr = REG_BC;
            REG_H = Z80opsImpl->inPort(memptr++);
            sz5h3pnFlags = sz53pn_addTable[REG_H];
            flagQ = true;
            break;
        }
        case 0x61:
        { /* OUT (C),H */
            memptr = REG_BC;
            Z80opsImpl->outPort(memptr++, REG_H);
            break;
        }
        case 0x62:
        { /* SBC HL,HL */
            Z80opsImpl->addressOnBus(getPairIR(), 7);
            sbc16(REG_HL);
            break;
        }
        case 0x63:
        { /* LD (nn),HL */
            memptr = Z80opsImpl->peek16(regPC);
            Z80opsImpl->poke16(memptr++, REG_HL);
            regPC = regPC + 2;
            break;
        }
        case 0x67:
        { /* RRD */
            // A = A7 A6 A5 A4 (HL)3 (HL)2 (HL)1 (HL)0
            // (HL) = A3 A2 A1 A0 (HL)7 (HL)6 (HL)5 (HL)4
            // Los bits 3,2,1 y 0 de (HL) se copian a los bits 3,2,1 y 0 de A.
            // Los 4 bits bajos que había en A se copian a los bits 7,6,5 y 4 de (HL).
            // Los 4 bits altos que había en (HL) se copian a los 4 bits bajos de (HL)
            // Los 4 bits superiores de A no se tocan. ¡p'habernos matao!
            uint8_t aux = regA << 4;
            memptr = REG_HL;
            uint16_t memHL = Z80opsImpl->peek8(memptr);
            regA = (regA & 0xf0) | (memHL & 0x0f);
            Z80opsImpl->addressOnBus(memptr, 4);
            Z80opsImpl->poke8(memptr++, (memHL >> 4) | aux);
            sz5h3pnFlags = sz53pn_addTable[regA];
            flagQ = true;
            break;
        }
        case 0x68:
        { /* IN L,(C) */
            memptr = REG_BC;
            REG_L = Z80opsImpl->inPort(memptr++);
            sz5h3pnFlags = sz53pn_addTable[REG_L];
            flagQ = true;
            break;
        }
        case 0x69:
        { /* OUT (C),L */
            memptr = REG_BC;
            Z80opsImpl->outPort(memptr++, REG_L);
            break;
        }
        case 0x6A:
        { /* ADC HL,HL */
            Z80opsImpl->addressOnBus(getPairIR(), 7);
            adc16(REG_HL);
            break;
        }
        case 0x6B:
        { /* LD HL,(nn) */
            memptr = Z80opsImpl->peek16(regPC);
            REG_HL = Z80opsImpl->peek16(memptr++);
            regPC = regPC + 2;
            break;
        }
        case 0x6F:
        { /* RLD */
            // A = A7 A6 A5 A4 (HL)7 (HL)6 (HL)5 (HL)4
            // (HL) = (HL)3 (HL)2 (HL)1 (HL)0 A3 A2 A1 A0
            // Los 4 bits bajos que había en (HL) se copian a los bits altos de (HL).
            // Los 4 bits altos que había en (HL) se copian a los 4 bits bajos de A
            // Los bits 3,2,1 y 0 de A se copian a los bits 3,2,1 y 0 de (HL).
            // Los 4 bits superiores de A no se tocan. ¡p'habernos matao!
            uint8_t aux = regA & 0x0f;
            memptr = REG_HL;
            uint16_t memHL = Z80opsImpl->peek8(memptr);
            regA = (regA & 0xf0) | (memHL >> 4);
            Z80opsImpl->addressOnBus(memptr, 4);
            Z80opsImpl->poke8(memptr++, (memHL << 4) | aux);
            sz5h3pnFlags = sz53pn_addTable[regA];
            flagQ = true;
            break;
        }
        case 0x70:
        { /* IN (C) */
            memptr = REG_BC;
            uint8_t inPort = Z80opsImpl->inPort(memptr++);
            sz5h3pnFlags = sz53pn_addTable[inPort];
            flagQ = true;
            break;
        }
        case 0x71:
        { /* OUT (C),0 */
            memptr = REG_BC;
            Z80opsImpl->outPort(memptr++, 0x00);
            break;
        }
        case 0x72:
        { /* SBC HL,SP */
            Z80opsImpl->addressOnBus(getPairIR(), 7);
            sbc16(regSP);
            break;
        }
        case 0x73:
        { /* LD (nn),SP */
            memptr = Z80opsImpl->peek16(regPC);
            Z80opsImpl->poke16(memptr++, regSP);
            regPC = regPC + 2;
            break;
        }
        case 0x78:
        { /* IN A,(C) */
            memptr = REG_BC;
            regA = Z80opsImpl->inPort(memptr++);
            sz5h3pnFlags = sz53pn_addTable[regA];
            flagQ = true;
            break;
        }
        case 0x79:
        { /* OUT (C),A */
            memptr = REG_BC;
            Z80opsImpl->outPort(memptr++, regA);
            break;
        }
        case 0x7A:
        { /* ADC HL,SP */
            Z80opsImpl->addressOnBus(getPairIR(), 7);
            adc16(regSP);
            break;
        }
        case 0x7B:
        { /* LD SP,(nn) */
            memptr = Z80opsImpl->peek16(regPC);
            regSP = Z80opsImpl->peek16(memptr++);
            regPC = regPC + 2;
            break;
        }
        case 0xA0:
        { /* LDI */
            ldi();
            break;
        }
        case 0xA1:
        { /* CPI */
            cpi();
            break;
        }
        case 0xA2:
        { /* INI */
            ini();
            break;
        }
        case 0xA3:
        { /* OUTI */
            outi();
            break;
        }
        case 0xA8:
        { /* LDD */
            ldd();
            break;
        }
        case 0xA9:
        { /* CPD */
            cpd();
            break;
        }
        case 0xAA:
        { /* IND */
            ind();
            break;
        }
        case 0xAB:
        { /* OUTD */
            outd();
            break;
        }
        case 0xB0:
        { /* LDIR */
            ldi();
            if (REG_BC != 0) {
                regPC = regPC - 2;
                memptr = regPC + 1;
                Z80opsImpl->addressOnBus(REG_DE - 1, 5);
            }
            break;
        }
        case 0xB1:
        { /* CPIR */
            cpi();
            if ((sz5h3pnFlags & PARITY_MASK) == PARITY_MASK
                    && (sz5h3pnFlags & ZERO_MASK) == 0) {
                regPC = regPC - 2;
                memptr = regPC + 1;
                Z80opsImpl->addressOnBus(REG_HL - 1, 5);
            }
            break;
        }
        case 0xB2:
        { /* INIR */
            ini();
            if (REG_B != 0) {
                regPC = regPC - 2;
                Z80opsImpl->addressOnBus(REG_HL - 1, 5);
            }
            break;
        }
        case 0xB3:
        { /* OTIR */
            outi();
            if (REG_B != 0) {
                regPC = regPC - 2;
                Z80opsImpl->addressOnBus(REG_BC, 5);
            }
            break;
        }
        case 0xB8:
        { /* LDDR */
            ldd();
            if (REG_BC != 0) {
                regPC = regPC - 2;
                memptr = regPC + 1;
                Z80opsImpl->addressOnBus(REG_DE + 1, 5);
            }
            break;
        }
        case 0xB9:
        { /* CPDR */
            cpd();
            if ((sz5h3pnFlags & PARITY_MASK) == PARITY_MASK
                    && (sz5h3pnFlags & ZERO_MASK) == 0) {
                regPC = regPC - 2;
                memptr = regPC + 1;
                Z80opsImpl->addressOnBus(REG_HL + 1, 5);
            }
            break;
        }
        case 0xBA:
        { /* INDR */
            ind();
            if (REG_B != 0) {
                regPC = regPC - 2;
                Z80opsImpl->addressOnBus(REG_HL + 1, 5);
            }
            break;
        }
        case 0xBB:
        { /* OTDR */
            outd();
            if (REG_B != 0) {
                regPC = regPC - 2;
                Z80opsImpl->addressOnBus(REG_BC, 5);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}
